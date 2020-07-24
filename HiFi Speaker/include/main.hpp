#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <Arduino.h>
#include <IRremote.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define POWER_OFF_STATE 0 // black
#define POWER_ON_WIFI_STATION 1 //blue
#define POWER_ON_WIFI_ACCESS_POINT 2 //green
#define POWER_ON_WIFI_DISABLED 3 //yellow

uint8_t State = 0;

uint8_t Defined_Volume = 0;
uint8_t Current_Volume = 0;

xTaskHandle Check_Infrared_Receiver_Handle;

IRrecv Infrared_Receiver(INFRARED_RECEIVER_PIN);
decode_results Received_Data;

AsyncWebServer Web_Server(80);

char Current_User[32];
char Current_Password[32];

bool Logged;

void Start();
void Shutdown();
void Set_LED_Color(uint8_t const&, uint8_t const&, uint8_t const&);
void Set_Volume(int16_t const&);
// Task
void Check_Infrared_Receiver(void*);

#endif