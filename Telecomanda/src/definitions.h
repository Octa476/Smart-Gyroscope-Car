#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

// Macros for all the pins used.

// The ARDUINO definitions are used as this microcontroller is 
// CHEAP CHINESE ARDUINO NANO CLONE.

// Buttons.
#define BUT1 A0
#define BUT2 A1
// Leds.
#define LED1 A2
#define LED2 A3

// Gyroscope.
#define SDA A4
#define SCL A5

// GPS module.
#define RX_GPS 3
#define TX_GPS 4

// NRF24L01
#define CE 7
#define CSN 8

// Display.
#define DC 5
#define RST 6
#define BL 9
#define CS 10

// General SPI.
#define MOSI 11
#define MISO 12
#define SCK 13