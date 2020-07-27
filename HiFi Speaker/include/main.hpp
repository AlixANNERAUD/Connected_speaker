#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <Arduino.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>

#include <SPIFFS.h>
#include "configuration.hpp"

#define POWER_OFF_STATE 0 // black
#define POWER_ON_WIFI_STATION 1 //blue
#define POWER_ON_WIFI_ACCESS_POINT 2 //green
#define POWER_ON_WIFI_DISABLED 3 //yellow
#define POWER_OFF_ERROR 4 //red

uint8_t State = 0;

uint8_t Defined_Volume = 0;

xTaskHandle Check_Infrared_Receiver_Handle;

IRrecv Infrared_Receiver(INFRARED_RECEIVER_PIN);
decode_results Received_Data;

AsyncWebServer Web_Server(80);

File Temporary_File;

// Configuration variable
String Password = "admin1234";

String Device_Name = "esp32";

uint16_t LED_Frequency = 60; //Low as possible in order to do not disturb IR receiver

//Color code

uint32_t Power_Off_Color = POWER_OFF_COLOR; // black
uint32_t Power_On_WiFi_Station_Color = POWER_ON_WIFI_STATION_COLOR; // blue
uint32_t Power_On_WiFi_Access_Point_Color = POWER_ON_WIFI_ACCESS_POINT_COLOR; //yellow

uint32_t Power_On_Disabled_Color = POWER_ON_WIFI_DISABLED_COLOR; //yellow
uint32_t Power_Off_Error_Color = POWER_OFF_ERROR_COLOR;

uint32_t Power_Code = 0xE0E0F00F;
uint32_t Volume_Down_Code = 0xE0E0D02F;
uint32_t Volume_Up_Code = 0xE0E0E01F;
uint32_t State_Code = 0xE0E0E01F;


bool Logged;

void Start();
void Shutdown();
void Set_LED_Color(uint8_t const&, uint8_t const&, uint8_t const&);
void Set_Volume();
// Task
void Check_Infrared_Receiver(void*);
uint8_t Load_Configuration();
uint8_t Save_Configuration();
#endif