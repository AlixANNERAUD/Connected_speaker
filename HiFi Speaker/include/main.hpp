#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <Arduino.h>
#include <IRremote.h>

// Pinout
#define POTENTIOMETER_PIN 33

#define GREEN_LED_PIN 32
#define RED_LED_PIN 27
#define BLUE_LED_PIN 21

#define UP_PIN 16
#define DOWN_PIN 17

#define RECEIVER_PIN 15

#define POWER_PIN 13

// IR Code
#define MUTE_CODE 0xE0E0F00F
#define VOLUME_DOWN_CODE 0xE0E0D02F
#define VOLUME_UP_CODE 0xE0E0E01F
#define A_CODE 0xE0E036C9

//

#define POWER_OFF_STATE 0 // black
#define POWER_ON_WIFI_STATION_STATE 1 //blue
#define POWER_ON_WIFI_ACCESS_POINT_STATE 2 //green
#define POWER_ON_WIFI_DISABLED 3 //yellow


uint8_t State = 0;

uint8_t Defined_Volume = 0;
uint8_t Current_Volume = 0;

xTaskHandle Check_Infrared_Receiver_Handle;

IRrecv Infrared_Receiver(RECEIVER_PIN);
decode_results Received_Data;


void Start();
void Shutdown();
void Set_LED_Color(uint8_t const&, uint8_t const&, uint8_t const&);
void Set_Volume(int16_t const&);
// Task
void Check_Infrared_Receiver(void*);

#endif