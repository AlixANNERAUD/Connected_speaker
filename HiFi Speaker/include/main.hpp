#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <Arduino.h>
#include <IRremote.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>

#define POWER_OFF_STATE 0 // black
#define POWER_ON_WIFI_STATION 1 //blue
#define POWER_ON_WIFI_ACCESS_POINT 2 //green
#define POWER_ON_WIFI_DISABLED 3 //yellow
#define POWER_OFF_ERROR 4 //red



uint8_t State = 0;

uint8_t Defined_Volume = 0;
uint8_t Current_Volume = 0;

xTaskHandle Check_Infrared_Receiver_Handle;

IRrecv Infrared_Receiver(INFRARED_RECEIVER_PIN);
decode_results Received_Data;

AsyncWebServer Web_Server(80);

// Configuration variable
String User, Password;

uint16_t LED_Frequency = 60; //Low as possible in order to do not disturb IR receiver
uint32_t Power_Off_Color = 0x000000; // black
uint32_t Power_On_WiFi_Station_Color = 0x0000FF; // blue
uint32_t Power_On_WiFi_Access_Point_Color = 0x00FF00; //yellow
uint32_t Power_On_Disabled = 0xFF00FF; //yellow
uint32_t Power_Off_Error = 0x000000;


bool Logged;

void Start();
void Shutdown();
void Set_LED_Color(uint8_t const&, uint8_t const&, uint8_t const&);
void Set_Volume(int16_t const&);
// Task
void Check_Infrared_Receiver(void*);
uint8_t Get_Configuration();

#endif