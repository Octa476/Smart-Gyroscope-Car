#include "remote.h"
#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  sei();
}

extern volatile long unsigned int remote_systicks;

void loop() {
  Serial.println("bunaa");
  Remote remote = Remote();
  while (true) {
    delay(50);
    remote.update_remote();
    Serial.println(remote_systicks);
  }
}