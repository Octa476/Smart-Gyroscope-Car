#include "remote.h"
#include <Arduino.h>

void setup() {
  // Here in the remote we use more Arduino.h as this board is a cheap chinese copy of the Arduino Nano board.
  Serial.begin(9600);
  sei();
}

// Great systicks, I believe this is the best thing you can implement with a timer!.
extern volatile long unsigned int remote_systicks;

void loop() {
  Serial.println("Hello");
  Remote remote = Remote();
  
  while (true) {
    delay(50); // The same delay we use for the car.
    remote.update_remote();
    Serial.println(remote_systicks);
  }
}