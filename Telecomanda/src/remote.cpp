#include "definitions.h"
#include "remote.h"

#define BMI160_I2C_ADDRESS 0x68
#define ACCEL_SENSITIVITY 16384.0 // Sensitivity for ±2g in LSB/g (adjust based on your configuration)

volatile long unsigned int remote_systicks = 0;
volatile uint8_t blue = 0;
volatile uint16_t blue_interval = 0;

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

    // INPUT_PULLUP
    pinMode(BUT1, INPUT_PULLUP);
    pinMode(BUT2, INPUT_PULLUP);

    // Enable PCINT for PORTC
    PCICR |= (1 << PCIE1);

    // Enable interrupts for PC0 and PC1
    PCMSK1 |= (1 << PCINT8) | (1 << PCINT9);
}

// ISR for PORTC pin change interrupts
unsigned long int last_tick = 0;
ISR(PCINT1_vect) {
    if ((remote_systicks - last_tick >= 5) && (remote_systicks - last_tick <= 500)) {
        // BUT1 (A0 / PC0)
        if (!(PINC & (1 << PC0))) {
            but1_pressed = true;
            // Serial.println("Buton1");
        }

        // BUT2 (A1 / PC1)
        if (!(PINC & (1 << PC1))) {
            but2_pressed = true;
            // Serial.println("Buton2");
        }
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

void Remote::antenna_init() {
    radio.begin();

    radio.openWritingPipe(address);
    radio.openReadingPipe(0, address);

    radio.setPALevel(RF24_PA_LOW);
}

void Remote::display_init() {

}

void autoCalibrateAccelerometer() {
  // Configure accelerometer for auto-calibration
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x7E); // Command register
  Wire.write(0x37); // Start accelerometer offset calibration
  Wire.endTransmission();
  delay(100);
 
  // Wait for calibration to complete
  delay(2000);
  Serial.println("Accelerometer Auto-Calibration Complete");
}

void Remote::gyro_init() {
    // Initialize I2C with custom SDA and SCL pins
    Wire.begin();         // Initialize I2C communication
    
    // Initialize BMI160 accelerometer
    Wire.beginTransmission(BMI160_I2C_ADDRESS);
    Wire.write(0x7E); // Command register
    Wire.write(0x11); // Set accelerometer to normal mode
    Wire.endTransmission();
    delay(100);
    
    // Perform accelerometer auto-calibration
    autoCalibrateAccelerometer();
    
    Serial.println("BMI160 Initialized and Calibrated");
    calibration_blue = 2;
}

void Remote::calibrate() {

}

Remote::Remote() {
    Timer2_init_systicks();
    gyro_init();
    buttons_init();
    leds_init();
    antenna_init();
    display_init();
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
        calibration_blue = 1;
        but1_pressed = false;
    }

    if (but2_pressed) {
        send_message.speed_level = (send_message.speed_level + 1) % num_speed_level;
        Serial.println("speed: ");
        Serial.println(send_message.speed_level);
        but2_pressed = false;
    }

    blue = calibration_blue;
}

void Remote::update_leds() {
    if (calibration_blue == 2)
        digitalWrite(LED1, HIGH);
    if (danger_red)
        digitalWrite(LED2, HIGH);
}

void Remote::update_display() {

}

void Remote::update_antenna() {

}

void Remote::update_gyro() {
    if (calibration_blue == 1) {
        autoCalibrateAccelerometer();
    } else {
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
        
        // Print tilt angles
        Serial.print("Pitch: ");
        Serial.print(pitch, 2);
        Serial.print("°, Roll: ");
        Serial.print(roll, 2);
        Serial.println("°");
    }
}