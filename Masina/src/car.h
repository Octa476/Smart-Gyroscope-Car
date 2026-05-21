#pragma once

#include <stdint.h>
#include <Wire.h>
#include <VL53L0X.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Arduino.h>

#include "definitions.h"


class Car {
    private:
        // Motor information.
        uint8_t right_motor_speed {0};
        uint8_t left_motor_speed {0};

        // ToF sensors information.
        uint16_t distance_to_obstacle1 {250};
        uint16_t distance_to_obstacle2 {250};
        VL53L0X tof_sensor1;
        VL53L0X tof_sensor2;
        const uint8_t danger_zone {50};
        
        // Antenna information.
        bool remote_connected {false};
        RF24 radio;
        static constexpr byte address[6] = "00001";

        void tof_init();
        void buzzer_init();
        
        void update_buzzer();
        void calculate_distance();
        void update_motors();
        void read_antenna();
        void write_antenna();
    public:
        Car();

        void update();
};