/*
* Arduino Wireless Communication Tutorial
*     Example 1 - Transmitter Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "car.h"
#include "usart.h"
#include <string.h>

#define PM_BAUD 9600

void setup_init() {
  init(); // We need this initialisation just for the antenna, as the library uses
  // the delay() function from Arduino.h, and as I use the AVR int main(), instead of Arduino's setup()
  // and loop() function, I need this initialisation just for a quick use of delay(); after the initialisation
  // of the motors, the Timer0 configuration will be overwritten, so delay() won't work anymore.
  // This is the reason we use systicks and _delay_ms() as this function is blocking and is not using timers.
  USART0_init(CALC_USART_UBRR(PM_BAUD));
  sei();
  _delay_ms(5000);
  USART0_print("Masina se pregateste de lupta!\n");
}

extern volatile long unsigned int car_systicks;

int main() {
  setup_init();
  Car car = Car();

  while (true) {
    _delay_ms(50); // Update the state of the car every 50ms, I believe this is a fair interval!

    // char text[100];
    // sprintf(text, "%lu\n", car_systicks);
    // USART0_print(text);

    car.update_car();
  }
}