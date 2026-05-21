#include "car.h"
#include "usart.h"
#include <util/delay.h>

volatile unsigned long long car_systicks = 0;

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
    PORTB ^= (1 << BUZ);
}

void Car::tof_init() {
    Wire.begin();

    pinMode(XSHUT1_ARD, OUTPUT);
    pinMode(XSHUT2_ARD, OUTPUT);

    // Shut the two sensors.
    digitalWrite(XSHUT1_ARD, LOW);
    digitalWrite(XSHUT2_ARD, LOW);

    _delay_ms(50);

    digitalWrite(XSHUT1_ARD, HIGH);
    _delay_ms(50);

    if (!tof_sensor1.init()) {
        USART0_print("Sensor1 FAIL");
        while (1);
    }

    tof_sensor1.setAddress(0x30);

    digitalWrite(XSHUT2_ARD, HIGH);
    _delay_ms(50);

    if (!tof_sensor2.init()) {
        USART0_print("Sensor2 FAIL");
        while (1);
    }

    tof_sensor2.setAddress(0x31);

    // Make the sensors take measurements every 10 miliseconds.
    tof_sensor1.startContinuous(10);
    tof_sensor2.startContinuous(10);

    USART0_print("VL53L0X READY\n");
}

Car::Car() {
    // Initialise all the devices the car is using.
    USART0_print("bunasiua\n");
    Timer2_init_systicks(); // Global systicks.
    USART0_print("bunasiua\n");
    USART0_print("VL53L0X READYtttt\n");
    tof_init();
    // buzzer_init();
}

void Car::calculate_distance() {
    distance_to_obstacle1 = tof_sensor1.readRangeContinuousMillimeters();
    distance_to_obstacle2 = tof_sensor2.readRangeContinuousMillimeters();
}

void Car::buzzer_init() {
    DDRB |= (1 << BUZ);
}

void Car::update_buzzer() {

}

void Car::update() {

}
