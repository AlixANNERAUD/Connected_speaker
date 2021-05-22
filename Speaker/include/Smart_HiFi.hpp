#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <Arduino.h>

#include <ArduinoJson.h>

#include <ArduinoTrace.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#include "ArduinoOTA.h"

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>

#include <SPIFFS.h>
#include "configuration.hpp"


class Smart_HiFi_Class
{
public:
    enum LED_Mode : uint8_t
    {
        None,
        One_Blink,
        Two_Blink,
        Three_Blink,
        Fading
    };

    enum Color
    {
        WiFi_Station_Color = 240,      // Blue
        WiFi_Access_Point_Color = 120 // Green
    };

    void Start();

    Smart_HiFi_Class();
    ~Smart_HiFi_Class();

private:


    static Smart_HiFi_Class *Instance_Pointer;

    enum Power_States
    {
        Off,
        On,
        WiFi_Station,
        Access_Point,
        WiFi_Disabled,
        Error
    };

    // Task handle

    xTaskHandle Infrared_Receiver_Task_Handle;
    xTaskHandle Sound_Task_Handle;
    xTaskHandle LED_Task_Handle;


    uint8_t Defined_Volume = 0;

    // Infrared receiver
    IRrecv Infrared_Receiver;
    decode_results Received_Data;

    // Webserver
    AsyncWebServer Web_Server;

    // Configuration variable
    String Device_Password = "";
    String Device_Name = "";

    String SSID = "";
    String Password = "";

    // -- LED
    LED_Mode Mode;
    uint32_t Duration;
    double Temporary_RGB[3] = {0};
    uint16_t Defined_Color[3] = {0, 100, 100}; // HSV
    uint16_t Current_Color[3] = {0, 100, 100}; 
    void HSV_To_RGB(double* HSV, uint8_t* RGB);

    // -- Remote

    enum Button {
        Power_Button,
        Volume_Up_Button,
        Volume_Down_Button,
        Mute_Button
    };

    uint8_t Infrared_Receiver_Mode = 0;
    uint8_t Selected_Remote = 255;
    uint8_t Selected_Button = 255;

    uint32_t Power_Code[Maximum_Remotes] = {0};
    uint32_t Volume_Up_Code[Maximum_Remotes] = {0};
    uint32_t Volume_Down_Code[Maximum_Remotes] = {0};
    uint32_t Mute_Code[Maximum_Remotes] = {0};

    // -- Authentification

    IPAddress Null_IP_Address;
    IPAddress Logged_Clients_IP_Adress[Maximum_Logged_Clients];
    uint32_t Logged_Clients_Timer[Maximum_Logged_Clients];

    bool Check_Authentification(AsyncWebServerRequest *Request, bool Refresh_Timer = true);

    // -- Autn

    void Deep_Sleep();
    void Shutdown();

    // -- Led color
    void Refresh_LED();

    void Set_Color(double Hue, double Saturation = 100, double Value = 100);
    void Set_Mode(LED_Mode Mode);

    // -- Volume

    bool Mute;
    void Set_Volume(uint8_t Volume);
    void Set_Volume_Offset(int16_t Volume);
    uint8_t Get_Defined_Volume();
    uint8_t Get_Current_Volume();

    void Apply_Volume();

    // Task
    static void LED_Task(void *);
    static void Sound_Task(void *);
    static void Infrared_Receiver_Task(void *);

    // -- Registry management
    bool Load_Device_Registry();
    bool Load_WiFi_Registry();
    bool Load_Remote_Registry();

    bool Save_Device_Registry();
    bool Save_WiFi_Registry();
    bool Save_Remote_Registry();

    // -- Setup

    void Setup_LED();
    void Setup_Sound();
    void Setup_Infrared_Receiver();
    void Setup_WiFi();
    void Setup_Web_Server();

};

#endif