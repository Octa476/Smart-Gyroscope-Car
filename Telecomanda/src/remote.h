#pragma once

#include <Wire.h>
#include <BMI160Gen.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

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

class Remote {
    private:
        // Antenna information.
        struct Message recv_message;
        struct Message send_message;
        RF24 radio {CE, CSN};
        uint32_t reply = 0;
        uint32_t missed_receives = 0;
        static constexpr byte address[6] = "00001";

        // Led information.
        bool danger_white = false;
        // 0 for not calibrated, 1 for calibrating, 2 for calibrated.
        uint8_t calibration_blue = 0;
        const uint16_t led_interval = 1000;


        // Gyroscope information.
        const uint16_t samples = 1000;
        // These two values are used for dynamic calibration.
        float pitch0 = 0;
        float roll0 = 0;

        // Button information.
        uint8_t num_speed_level = 3;

        // Display information.
        Adafruit_ST7789 display = Adafruit_ST7789(CS_DISPLAY, DC_DISPLAY, RST_DISPLAY);

        void buttons_init();
        void leds_init();
        void antenna_init();
        void display_init();
        void gyro_init();

        void autoCalibrateAccelerometer();
        
        void update_buttons();
        void update_leds();
        void update_display();
        void update_antenna();
        void update_gyro();

    public:
        Remote();
        void update_remote();
};