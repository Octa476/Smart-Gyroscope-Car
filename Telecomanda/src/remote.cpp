#include "definitions.h"
#include "remote.h"

#define BMI160_I2C_ADDRESS 0x68
#define ACCEL_SENSITIVITY 16384.0 // Sensitivity for ±2g in LSB/g.

volatile long unsigned int remote_systicks = 0;
volatile uint8_t blue = 0; // This variable controls how the blue led behaves.
volatile uint16_t blue_interval = 0; // This variable is trivial.

// Systicks and buzzer control.
void Timer2_init_systicks(void)
{
    /* Clear previous settings */
    TCCR2A = 0;
    TCCR2B = 0;

    /* Set CTC mode */
    TCCR2A |= (1 << WGM21);

    /* Set prescaler to 128 */
    TCCR2B |= (1 << CS22) | (1 << CS20);

    /* Activate Compare A interrupt */
    TIMSK2 |= (1 << OCIE2A);

    /* 16MHz / 128 => 125 kHz */
    OCR2A = 125;
}

ISR(TIMER2_COMPA_vect)
{
    /* Will get called [almost] once every 1ms! */
    remote_systicks++;

    // If the remote is calibrating, then toggle the blue led.
    if (blue == 1) {
        if (remote_systicks % blue_interval < blue_interval / 2) {
            digitalWrite(LED1, HIGH);
        } else {
            digitalWrite(LED1, LOW);
        }
    }
}

volatile bool but1_pressed = false;
volatile bool but2_pressed = false;

void buttons_interrupt_init() {

    // INPUT_PULLUP, I don't believe this is necessary, but why modifying something that works!
    pinMode(BUT1, INPUT_PULLUP);
    pinMode(BUT2, INPUT_PULLUP);

    // Enable PCINT for PORTC.
    PCICR |= (1 << PCIE1);

    // Enable interrupts for PC0 and PC1.
    PCMSK1 |= (1 << PCINT8) | (1 << PCINT9);
}

// ISR for PORTC pin change interrupts.
volatile unsigned long int last_tick = 0;
ISR(PCINT1_vect) {
    // No debounce here, as the remote is too slow fro this matter to be a problem.
    if (!(PINC & (1 << PC0))) {
        but1_pressed = true;
    }

    if (!(PINC & (1 << PC1))) {
        but2_pressed = true;
    }

    last_tick = remote_systicks;
}

void Remote::buttons_init() {
    pinMode(BUT1, INPUT);
    pinMode(BUT2, INPUT);
    buttons_interrupt_init();
}

void Remote::leds_init() {
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    blue_interval = led_interval;
    blue = calibration_blue;
}

// The remote is a sender by default.
void Remote::antenna_init() {
    radio.begin();

    radio.openWritingPipe(address);
    radio.openReadingPipe(0, address);

    radio.setPALevel(RF24_PA_LOW);
}

void Remote::display_init() {
    // Display resolution.
    display.init(170, 320);

    // To be as it should be.
    display.setRotation(2);

    // Full black baby.
    display.fillScreen(ST77XX_BLACK);

    // Put some message for the user.
    display.setTextSize(4);
    display.setTextColor(ST77XX_WHITE);

    display.setCursor(20, 40);
    display.println("Hello!");
}

void Remote::autoCalibrateAccelerometer() {
    // Configure accelerometer for auto-calibration.
    Wire.beginTransmission(BMI160_I2C_ADDRESS);
    Wire.write(0x7E); // Command register
    Wire.write(0x37); // Start accelerometer offset calibration
    Wire.endTransmission();
    delay(100);
    
    // Wait for calibration to complete.
    delay(2000);
    Serial.println("Accelerometer Auto-Calibration Complete");

    // Compute the pitch and roll of the current position as it will be used as an offset
    // for the future measurements.
    int16_t ax, ay, az;
    // Read accelerometer data.
    Wire.beginTransmission(BMI160_I2C_ADDRESS);
    Wire.write(0x12); // Start register for accelerometer data.
    Wire.endTransmission(false);
    Wire.requestFrom(BMI160_I2C_ADDRESS, 6);

    if (Wire.available() == 6) {
        ax = (Wire.read() | (Wire.read() << 8));
        ay = (Wire.read() | (Wire.read() << 8));
        az = (Wire.read() | (Wire.read() << 8));
    }

    // Convert raw accelerometer values to g.
    float ax_g = ax / ACCEL_SENSITIVITY;
    float ay_g = ay / ACCEL_SENSITIVITY;
    float az_g = az / ACCEL_SENSITIVITY;

    // Calculate tilt angles (pitch0 and roll0) in degrees.
    pitch0 = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI;
    roll0 = atan2(-ax_g, az_g) * 180.0 / PI;
    but1_pressed = false;
}

void Remote::gyro_init() {
    // Initialize I2C with custom SDA and SCL pins
    Wire.begin(); // Initialize I2C communication
    
    // Initialize BMI160 accelerometer
    Wire.beginTransmission(BMI160_I2C_ADDRESS);
    Wire.write(0x7E); // Command register
    Wire.write(0x11); // Set accelerometer to normal mode
    Wire.endTransmission();
    delay(100);
    
    // Perform accelerometer auto-calibration.
    autoCalibrateAccelerometer();
    
    Serial.println("BMI160 Initialized and Calibrated");
    calibration_blue = 2; // The calibration ended.
}

Remote::Remote() {
    Timer2_init_systicks();
    gyro_init();
    buttons_init();
    leds_init();
    antenna_init();
    display_init();

    // The remote default information.
    send_message.speed_level = 0;
    recv_message.danger_back = false;
    recv_message.danger_front = false;
}

void Remote::update_remote() {
    update_antenna();
    update_buttons();
    update_display();
    update_leds();
    update_gyro();
}

void Remote::update_buttons() {
    // Start calibration.
    if (but1_pressed) {
        Serial.println("Buton1");
        calibration_blue = 1;
        but1_pressed = false;
    }

    // Change the speed level of the car.
    if (but2_pressed) {
        Serial.println("Buton2");
        send_message.speed_level = (send_message.speed_level + 1) % num_speed_level;
        Serial.println("speed: ");
        Serial.println(send_message.speed_level);
        but2_pressed = false;
    }

    blue = calibration_blue;
}

// Signal a danger or the calibration action.
void Remote::update_leds() {
    danger_white = recv_message.danger_back || recv_message.danger_front;
    if (calibration_blue == 2)
        digitalWrite(LED1, HIGH);
    if (danger_white)
        digitalWrite(LED2, HIGH);
    else
        digitalWrite(LED2, LOW);
}

// A horrific function, but this is it, it's too late, and I have so little time left!
void Remote::update_display() {
    display.fillScreen(ST77XX_BLACK);
    if (calibration_blue == 1) {
        display.setTextSize(3);
        display.setCursor(10, 40);
        display.println("Gyroscope calibration...");
    } else {
        // Raw dogging this messages. 
        display.setTextSize(3);
        display.setCursor(10, 40);
        display.println("Angles:");
        display.setCursor(10, 80);
        display.print("X:");
        display.println(send_message.x_angle);
        display.setCursor(10, 120);
        display.print("Y:");
        display.println(send_message.y_angle);
        display.setCursor(10, 160);
        display.print("Speed: ");
        display.println(send_message.speed_level + 1);

        // Danger messages, we have, back, front, and all; if all is set, the the car is simply frozen.
        if (recv_message.danger_back && recv_message.danger_front) {
            display.setCursor(10, 200);
            display.print("Danger A");
        } else if (recv_message.danger_back) {
            display.setCursor(10, 200);
            display.print("Danger B");
        } else if (recv_message.danger_front) {
            display.setCursor(10, 200);
            display.print("Danger F");
        }
    }
}

void Remote::update_antenna() {
    // TX mode as default.
    radio.stopListening();

    radio.write(&send_message, sizeof(send_message));

    Serial.print("TX: ");
    Serial.println(reply);

    // RX mode but just for some time.
    radio.startListening();

    unsigned long start = remote_systicks;

    while (!radio.available()) {

        if (remote_systicks - start > 200) {

            Serial.println("timeout");
            return;
        }
    }
    reply++;

    radio.read(&recv_message, sizeof(recv_message));

    Serial.print("RX: ");
    Serial.println(reply);
}

void Remote::update_gyro() {
    if (calibration_blue == 1) {
        autoCalibrateAccelerometer();
        calibration_blue = 2;
    } else {
        // Make a reading, and calculate the pitch and roll.
        int16_t ax, ay, az;
 
        // Read accelerometer data
        Wire.beginTransmission(BMI160_I2C_ADDRESS);
        Wire.write(0x12); // Start register for accelerometer data
        Wire.endTransmission(false);
        Wire.requestFrom(BMI160_I2C_ADDRESS, 6);
        
        if (Wire.available() == 6) {
            ax = (Wire.read() | (Wire.read() << 8));
            ay = (Wire.read() | (Wire.read() << 8));
            az = (Wire.read() | (Wire.read() << 8));
        }
        
        // Convert raw accelerometer values to g
        float ax_g = ax / ACCEL_SENSITIVITY;
        float ay_g = ay / ACCEL_SENSITIVITY;
        float az_g = az / ACCEL_SENSITIVITY;
        
        // Calculate tilt angles (pitch and roll) in degrees
        float pitch = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI;
        float roll = atan2(-ax_g, az_g) * 180.0 / PI;

        pitch -= pitch0;
        roll -= roll0;
        
        // Set the data computed to the message struct.
        send_message.x_angle = -pitch;
        send_message.y_angle = roll;

        // Print tilt angles.
        Serial.print("Pitch: ");
        Serial.print(pitch, 2);
        Serial.print("°, Roll: ");
        Serial.print(roll, 2);
        Serial.println("°");
    }
}