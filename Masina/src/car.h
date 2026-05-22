#pragma once

#include <stdint.h>
#include <Wire.h>
#include <VL53L0X.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Arduino.h>

#include "definitions.h"

// This struct will be used to comunicate with the remote.
// It contains the angles of the remote and the information if there is an incoming obstacle or not.
struct Message {
    float x_angle; // Angle between [-90, 90]
    float y_angle; // Angle between [-90, 90]
    uint16_t speed_level; // 0, 1, 2
    uint8_t danger_front; // 1 for danger, 0 otherwise
    uint8_t danger_back; // 1 for danger, 0 otherwise
};

class Car {
    private:
        // Motor information.
        int16_t right_motor_speed {0};
        int16_t left_motor_speed {0};
        // For safety meaurements.
        int16_t speed_limit = 180;
        uint8_t num_speed_level = 3;

        // ToF sensors information.
        uint16_t distance_to_obstacle1 {250};
        uint16_t distance_to_obstacle2 {250};
        VL53L0X tof_sensor1;
        VL53L0X tof_sensor2;
        const uint16_t danger_buffer {240};
        const uint16_t sensor_distance_limit {40};
        
        // Antenna information.
        struct Message recv_message;
        struct Message send_message;
        bool remote_connected {false};
        RF24 radio {CE_ARD, CSN_ARD};
        uint32_t reply = 0;
        uint32_t missed_receives = 0;
        static constexpr byte address[6] = "00001";

        // Buzzer information.
        // The frequency of 1KHz will be divided by this number to signal an obstacle.
        // If the value is 0, then no obstacle is close.
        uint8_t frequency_divider = 0;
        // The frequency is 1KHz / range_of_dividers, as this variable gets smaller,
        // the sound becomes more annoying.
        const uint8_t range_of_dividers = 5;
        const uint16_t interval_of_buzzing = 1200;

        void tof_init();
        void buzzer_init();
        void antenna_init();
        void motors_init();
        
        void update_buzzer();
        void update_tof();
        void update_motors();
        void update_antenna();

    public:
        Car();
        void update_car();
};