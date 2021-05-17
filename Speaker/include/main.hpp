#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <Arduino.h>

#include <ArduinoJson.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#include "ArduinoOTA.h"

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>

#include <SPIFFS.h>
#include "configuration.hpp"

#define LOGGIN_TIMEOUT 300000
#define MAXIMUM_REMOTES 5
#define MAXIMUM_LOGGED_CLIENTS 5

enum Power_States
{
Off,
On,
WiFi_Station,
Access_Point,
WiFi_Disabled,
Error
};

uint8_t State = 0;

uint8_t Defined_Volume = 0;

xTaskHandle Infrared_Receiver_Handle;
xTaskHandle LED_Handle;

IRrecv Infrared_Receiver(INFRARED_RECEIVER_PIN);
decode_results Received_Data;

AsyncWebServer Web_Server(80);

bool Logged = 1;

File Temporary_File;

// Configuration variable
String Device_Password = "";
String Device_Name = "";

String SSID = "";
String Password = "";

uint8_t Defined_Red = 0, Defined_Green = 0, Defined_Blue = 0;
bool LED_Task_Running = false;

uint16_t LED_Frequency = 60; //Low as possible in order to do not disturb IR receiver

//Color code

uint32_t Power_Off_Color = POWER_OFF_COLOR; // black

uint32_t Power_On_WiFi_Station_Color = POWER_ON_WIFI_STATION_COLOR; // blue
uint32_t Power_On_WiFi_Access_Point_Color = POWER_ON_WIFI_ACCESS_POINT_COLOR; //yellow

uint32_t Power_On_WiFi_Disabled_Color = POWER_ON_WIFI_DISABLED_COLOR; //yellow
//uint32_t Power_Off_Error_Color = POWER_ON_ERROR_COLOR;

uint32_t Power_Code[2] = {0xE0E0F00F, 0xCF000EF1};
uint32_t Volume_Down_Code[2] = {0xE0E0D02F, 0xCF001EE1};
uint32_t Volume_Up_Code[2] = {0xE0E0E01F, 0xCF00EE11};
uint32_t Mute_Code[2] = {0xCF008E71, 0x000000000};
uint32_t State_Code[2] = {0xE0E0E01F, 0xCF00B946};

IPAddress Logged_Clients_IP_Adress[MAXIMUM_LOGGED_CLIENTS];
uint32_t Logged_Clients_Timer[MAXIMUM_LOGGED_CLIENTS];


bool Mute;

bool Check_Authentification(AsyncWebServerRequest* Request, bool Refresh_Timer = true);

void Start();
void Shutdown();
void Set_LED_Color(uint32_t);
void LED_Task(void*);
void Set_Volume();
void Set_State(uint8_t const&);

// Task
void Infrared_Receiver_Task(void*);
uint8_t Load_Configuration();
uint8_t Reset_Configuration();
uint8_t Save_Configuration(uint8_t const&);

//bool Logged();

void WiFi_Initialize();
void Setup_Web_Server();

void Callback();

#endif