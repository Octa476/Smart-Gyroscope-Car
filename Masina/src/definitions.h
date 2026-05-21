#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>

// Macros for all the pins used.

// The hardware mapping on the ATmega328P using the port names.
// Buzzer.
#define BUZ PB0

// Motor inputs.
#define PWM4 PB1
#define PWM3 PB2
#define PWM2 PD5
#define PWM1 PD6

#define LEFT_CONTROL_FRONT OCR1A
#define LEFT_CONTROL_BACK OCR1B
#define RIGHT_CONTROL_FRONT OCR0A
#define RIGHT_CONTROL_BACK OCR0B

// NRF24L01.
#define MOSI PB3
#define MISO PB4
#define SCK PB5
#define CE PC1
#define CSN PC2

// ToF sensors.
#define SDA PC4
#define SCL PC5
#define XSHUT1 PC0
#define XSHUT2 PC3

// GPS module.
#define RX_GPS PD2
#define TX_GPS PD3


// The ARDUINO definitions used by some libraries.

// Buzzer.
#define BUZ_ARD 8

// Motor inputs.
#define PWM4_ARD 9
#define PWM3_ARD 10
#define PWM2_ARD 5
#define PWM1_ARD 6

// NRF24L01.
#define MOSI_ARD 11
#define MISO_ARD 12
#define SCK_ARD 13
#define CE_ARD 15
#define CSN_ARD 16

// ToF sensors.
#define SDA_ARD 18
#define SCL_ARD 19
#define XSHUT1_ARD 14
#define XSHUT2_ARD 17

// GPS module.
#define RX_GPS_ARD 2
#define TX_GPS_ARD 3