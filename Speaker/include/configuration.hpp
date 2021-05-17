#ifndef CONFIGURATION_HPP_INCLUDED
#define CONFIGURATION_HPP_INCLUDED

// Pinout
#define POTENTIOMETER_PIN 33

#define GREEN_LED_PIN 32
#define RED_LED_PIN 21
#define BLUE_LED_PIN 14

#define UP_PIN 16
#define DOWN_PIN 17

#define INFRARED_RECEIVER_PIN 15

#define POWER_PIN 13

// Volume

#define VOLUME_STEP 16
//

#define IR_THRESHOLD 255

//Color State

#define POWER_OFF_COLOR 0x000000 // Black
#define POWER_ON_START_COLOR 0xFF8000 // Orange
#define POWER_ON_WIFI_STATION_COLOR 0x0000FF // Blue
#define POWER_ON_WIFI_ACCESS_POINT_COLOR 0x00FF00 // Green
#define POWER_ON_WIFI_DISABLED_COLOR 0xFF00FF //Yellow
#define POWER_ON_ERROR_COLOR 0xFF0000 //red

#define LED_FREQUENCY 60

#define WIFI_TIMEOUT 10000
#define WAKEUP_TIMEOUT 2000

#define LED_FADE_DELAY 1

#define LED_FILE "/led"
#define REMOTE_FILE "/remote"
#define WIFI_FILE "/wifi"
#define DEVICE_FILE "/device"



#endif