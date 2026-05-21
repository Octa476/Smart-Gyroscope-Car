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
  USART0_init(CALC_USART_UBRR(PM_BAUD));
  sei();
  _delay_ms(5000);
  USART0_print("Masina se pregateste de lupta!\n");
}

// int counter = 0;
extern volatile long unsigned int car_systicks;

int main() {
  setup_init();
  Car car = Car();
  while (true) {
    _delay_ms(100);
    char text[100];
    sprintf(text, "%lu\n", car_systicks);

    // USART0_print(text);
    car.update_car();
  }
}