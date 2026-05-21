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
// const byte address[6] = "00001";

void setup_init() {
  USART0_init(CALC_USART_UBRR(PM_BAUD));
  sei();
  // radio.begin();
  // radio.openWritingPipe(address);
  // radio.setPALevel(RF24_PA_MIN);
  // radio.stopListening();
  _delay_ms(5000);
  USART0_print("bunasiua\n");
}

// int counter = 0;
extern volatile unsigned long long car_systicks;

int main() {
  setup_init();
  // RF24 radio(A1, A2); // CE, CSN
  USART0_print("Bunaaa");
  Car car = Car();
  USART0_print("Bunaaa");
  while (true) {
    // // const char text[] = "Hello World";
    // counter++;
    // radio.write(&counter, sizeof(counter));
    // // delay(100);
    _delay_ms(1000);
    char text[100];
    sprintf(text, "%lu\n", car_systicks);

    USART0_print(text);
  }
}

// #include <Wire.h>
// #include <VL53L0X.h>

// VL53L0X sensor1;
// VL53L0X sensor2;

// #define XSHUT1 A0
// #define XSHUT2 A3

// void setup() {

//   Serial.begin(9600);
//   Wire.begin();

//   pinMode(XSHUT1, OUTPUT);
//   pinMode(XSHUT2, OUTPUT);

//   // Oprim ambii senzori
//   digitalWrite(XSHUT1, LOW);
//   digitalWrite(XSHUT2, LOW);

//   delay(50);

//   // Pornim senzorul 1
//   digitalWrite(XSHUT1, HIGH);
//   delay(50);

//   if (!sensor1.init()) {
//     Serial.println("Sensor1 FAIL");
//     while (1);
//   }

//   sensor1.setAddress(0x30);

//   // Pornim senzorul 2
//   digitalWrite(XSHUT2, HIGH);
//   delay(50);

//   if (!sensor2.init()) {
//     Serial.println("Sensor2 FAIL");
//     while (1);
//   }

//   sensor2.setAddress(0x31);

//   sensor1.startContinuous();
//   sensor2.startContinuous();

//   Serial.println("VL53L0X READY");
// }

// void loop() {

//   Serial.print("S1: ");
//   Serial.print(sensor1.readRangeContinuousMillimeters());
//   Serial.print(" mm");

//   Serial.print(" | S2: ");
//   Serial.print(sensor2.readRangeContinuousMillimeters());
//   Serial.println(" mm");

//   delay(200);
// }