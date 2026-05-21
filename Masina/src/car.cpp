#include "car.h"
#include "usart.h"
#include <util/delay.h>

volatile long unsigned int car_systicks = 0;
volatile uint8_t buzzer_divider = 0;
volatile uint8_t buzzer_max_devide = 0;
volatile uint16_t base_interval_of_buzzing = 0;

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
    car_systicks++;

    if (buzzer_divider) {
        uint16_t interval = base_interval_of_buzzing / (buzzer_max_devide - buzzer_divider + 1);
        if (car_systicks % interval < interval / 2) {
            if (car_systicks % buzzer_divider == 0)
                PORTB ^= (1 << BUZ);
        } else {
            PORTB &= ~(1 << BUZ);
        } 
    } else {
        PORTB &= ~(1 << BUZ);
    }
}

// Motors control via manual PWM initialization.

/* Initialize Timer0 for Fast PWM mode (8 bits). */
void Timer0_init_pwm(void)
{
    /* Clear previous settings */
    TCCR0A = 0;
    TCCR0B = 0;

    /* TODO Task 1: set Fast PWM (8 bits) */
    TCCR0A |= (1 << WGM00) | (1 << WGM01);

    /* TODO Task 1: set inverting output for OC0A and OC0B */
    TCCR0A |= (1 << COM0A1) | (1 << COM0A0);
    TCCR0A |= (1 << COM0B1) | (1 << COM0B0);

    /* TODO Task 1: set prescaler to 64 */
    TCCR0B |= (1 << CS01) | (1 << CS00);

    /* Set duty cycle to 0%; TOP = 255 */
    OCR0A = 0;
    OCR0B = 0;
}

/* Initialize Timer1 for Fast PWM mode (8 bits). */
void Timer1_init_pwm(void)
{
    /* Clear previous settings */
    TCCR1A = 0;
    TCCR1B = 0;

    /* TODO Task 1: set Fast PWM (8 bits) */
    TCCR1A |= (1 << WGM10);
    TCCR1B |= (1 << WGM12);

    /* TODO Task 1: set inverting output for OC1A and OC1B */
    TCCR1A |= (1 << COM1A1) | (1 << COM1A0);
    TCCR1A |= (1 << COM1B1) | (1 << COM1B0);

    /* TODO Task 1: set prescaler to 64 */
    TCCR1B |= (1 << CS11) | (1 << CS10);

    /* Set duty cycle to 0%; TOP = 255 */
    OCR1A = 0;
    OCR1B = 0;
}


void Car::tof_init() {
    Wire.begin();

    DDRC |= (1 << XSHUT1);
    DDRC |= (1 << XSHUT2);
    // pinMode(XSHUT1_ARD, OUTPUT);
    // pinMode(XSHUT2_ARD, OUTPUT);

    // Shut the two sensors.
    PORTC &= ~(1 << XSHUT1);
    PORTC &= ~(1 << XSHUT2);
    // digitalWrite(XSHUT1_ARD, LOW);
    // digitalWrite(XSHUT2_ARD, LOW);

    _delay_ms(50);

    PORTC |= (1 << XSHUT1);
    // digitalWrite(XSHUT1_ARD, HIGH);
    _delay_ms(50);

    if (!tof_sensor1.init()) {
        USART0_print("Sensor1 FAIL");
        while (1);
    }

    tof_sensor1.setAddress(0x30);

    PORTC |= (1 << XSHUT2);
    // digitalWrite(XSHUT2_ARD, HIGH);
    _delay_ms(50);

    if (!tof_sensor2.init()) {
        USART0_print("Sensor2 FAIL");
        while (1);
    }

    tof_sensor2.setAddress(0x31);

    // Make the sensors take measurements as frequently as possible.
    tof_sensor1.startContinuous();
    tof_sensor2.startContinuous();

    USART0_print("VL53L0X READY\n");
}

void Car::buzzer_init() {
    DDRB |= (1 << BUZ);
    buzzer_max_devide = range_of_dividers;
    base_interval_of_buzzing = interval_of_buzzing;
}

void Car::motors_init() {
    Timer0_init_pwm();
    Timer1_init_pwm();
    DDRB |= (1 << PWM3);
    DDRB |= (1 << PWM4);
    DDRD |= (1 << PWM1);
    DDRD |= (1 << PWM2);
}

void Car::antenna_init() {
    radio.begin();

    radio.openReadingPipe(0, address);
    radio.openWritingPipe(address);

    radio.setPALevel(RF24_PA_LOW);

    radio.startListening();
}

Car::Car() {
    // Initialise all the devices the car is using.
    Timer2_init_systicks(); // Global systicks.

    tof_init();
    buzzer_init();
    antenna_init();
    motors_init();
}

void Car::update_tof() {
    distance_to_obstacle1 = tof_sensor1.readRangeContinuousMillimeters();
    distance_to_obstacle2 = tof_sensor2.readRangeContinuousMillimeters();
}

void Car::update_buzzer() {
    int16_t distance_to_danger1 = distance_to_obstacle1;
    int16_t distance_to_danger2 = distance_to_obstacle2;

    distance_to_danger1 = (distance_to_obstacle1 < sensor_distance_limit) ? 500 : distance_to_danger1;
    distance_to_danger2 = (distance_to_obstacle2 < sensor_distance_limit) ? 500 : distance_to_danger2;

    int16_t min_distance = fmin(distance_to_danger1, distance_to_danger2);
    // USART0_print("MIN: ");
    // USART0_print_num(min_distance);

    // Determine how dangerous the distance is, and set the buzzer acordingly.
    frequency_divider = (min_distance - sensor_distance_limit) * range_of_dividers / danger_buffer + 1;

    if (frequency_divider > range_of_dividers)
        frequency_divider = 0;


    buzzer_divider = frequency_divider;
    // USART0_print_num(buzzer_divider);
}

void Car::update_motors() {
    // Testing with the angles.
    recv_message.x_angle = -25;
    recv_message.y_angle = -10;
    recv_message.speed_level = 2;
    remote_connected = true;

    // Calculate the speed of the motors based on the input received from the remote.
    int16_t x_ang = recv_message.x_angle;
    int16_t y_ang = recv_message.y_angle;
    uint16_t speed_lvl = recv_message.speed_level;
    USART0_print("Angle: ");
    USART0_print_num(recv_message.x_angle);

    int16_t speed = speed_limit * (1 + speed_lvl) / 3;
    left_motor_speed = x_ang * speed / 45;
    right_motor_speed = x_ang * speed / 45;

    USART0_print("Speed: ");
    USART0_print_num(speed);

    if (y_ang > 5) {
        y_ang = (y_ang > 45) ? 45 : y_ang;
        if (right_motor_speed > 0)
            right_motor_speed -= right_motor_speed * y_ang / 90;
        if (right_motor_speed < 0)
            right_motor_speed -= right_motor_speed * y_ang / 90;
    } else if (y_ang < -5) {
        y_ang = (y_ang < -45) ? -45 : y_ang;
        if (left_motor_speed > 0)
            left_motor_speed -= left_motor_speed * (-y_ang) / 90;
        if (left_motor_speed < 0)
            left_motor_speed -= left_motor_speed * (-y_ang) / 90;
    }
    

    // Safety measures.
    left_motor_speed = (left_motor_speed > speed_limit) ? speed_limit : left_motor_speed;
    right_motor_speed = (right_motor_speed > speed_limit) ? speed_limit : right_motor_speed;

    left_motor_speed = (left_motor_speed < -speed_limit) ? -speed_limit : left_motor_speed;
    right_motor_speed = (right_motor_speed < -speed_limit) ? -speed_limit : right_motor_speed;


    // Calculate the distance to an obstacle.
    int16_t distance_to_danger_back = distance_to_obstacle1;
    int16_t distance_to_danger_front = distance_to_obstacle2;

    distance_to_danger_back = (distance_to_obstacle1 < sensor_distance_limit) ? 500 : distance_to_danger_back;
    distance_to_danger_front = (distance_to_obstacle2 < sensor_distance_limit) ? 500 : distance_to_danger_front;

    // Stop the motors if the given command would result in a collision.
    if (distance_to_danger_front < danger_buffer * 2 / 3) {
        send_message.danger_front = 1;
        left_motor_speed = (left_motor_speed > 0) ? 0 : left_motor_speed;
        right_motor_speed = (right_motor_speed > 0) ? 0 : right_motor_speed;
    } else {
        send_message.danger_front = 0;
    }

    if (distance_to_danger_back < danger_buffer) {
        send_message.danger_back = 1;
        left_motor_speed = (left_motor_speed < 0) ? 0 : left_motor_speed;
        right_motor_speed = (right_motor_speed < 0) ? 0 : right_motor_speed;
    } else {
        send_message.danger_back = 0;
    }

    // If the remote is not connected then, the car, just stops.
    if (!remote_connected) {
        left_motor_speed = 0;
        right_motor_speed = 0;
    }

    USART0_print("Right_speed: ");
    USART0_print_num(right_motor_speed);
    USART0_print("Left_speed: ");
    USART0_print_num(left_motor_speed);

    // Set the speed of the motors.
    if (left_motor_speed < 0) {
        LEFT_CONTROL_FRONT = 0;
        LEFT_CONTROL_BACK = -left_motor_speed;
    } else {
        LEFT_CONTROL_FRONT = left_motor_speed;
        LEFT_CONTROL_BACK = 0;
    }

    if (right_motor_speed < 0) {
        RIGHT_CONTROL_FRONT = 0;
        RIGHT_CONTROL_BACK = -right_motor_speed;
    } else {
        RIGHT_CONTROL_FRONT = right_motor_speed;
        RIGHT_CONTROL_BACK = 0;
    }
}

void Car::update_antenna() {
    // Rx mode.
    if (radio.available()) {
        radio.read(&recv_message, sizeof(recv_message));

        USART0_print("RX: ");
        USART0_print_num(recv_message.y_angle);

        // switch to TX
        radio.stopListening();

        reply++;

        radio.write(&send_message, sizeof(send_message));

        Serial.print("TX: ");
        Serial.println(reply);

        // back to RX
        radio.startListening();
        missed_receives = 0;
        remote_connected = true;
    } else {
        missed_receives++;
    }

    if (missed_receives > 10) {
        remote_connected = false;
    }
}

void Car::update_car() {
    update_antenna();
    update_tof();
    update_buzzer();
    update_motors();
}
