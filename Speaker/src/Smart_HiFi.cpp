#include "Smart_HiFi.hpp"

Smart_HiFi_Class *Smart_HiFi_Class::Instance_Pointer = NULL;

Smart_HiFi_Class::Smart_HiFi_Class()
    : Infrared_Receiver(15),
      Web_Server(Web_Server_Port),
      Null_IP_Address(0, 0, 0, 0)
{
    if (Instance_Pointer != NULL)
    {
        delete Instance_Pointer;
    }
    Instance_Pointer = this;
}

Smart_HiFi_Class::~Smart_HiFi_Class()
{
    if (Instance_Pointer != this)
    {
        delete Instance_Pointer;
    }

    Instance_Pointer = NULL;
}

void Smart_HiFi_Class::Start()
{
    //Serial.begin(115200);

    if (!SPIFFS.begin())
    {
        TRACE();
    }

    if (!Load_Remote_Registry())
    {
        Save_Remote_Registry();
    }

    esp_sleep_wakeup_cause_t Wakeup_Reason;
    Wakeup_Reason = esp_sleep_get_wakeup_cause();

    if (Wakeup_Reason == ESP_SLEEP_WAKEUP_EXT0)
    {
        Infrared_Receiver.enableIRIn();

        uint32_t Timeout = millis() + Wakeup_Timeout;
        while (millis() < Timeout)
        {
            if (Infrared_Receiver.decode(&Received_Data))
            {
                for (uint8_t i = 0; i < Maximum_Remotes; i++)
                {
                    if (Received_Data.value == Power_Code[i])
                    {
                        Timeout = 0;
                        break;
                    }
                }
                Received_Data.value = 0;
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        if (Timeout != 0)
        {
            Deep_Sleep();
        }
    }
    else if (Wakeup_Reason == ESP_SLEEP_WAKEUP_UNDEFINED)
    {
    }
    else
    {
        Deep_Sleep();
    }

    Setup_LED();

    uint32_t Timeout = millis() + 4000;

    Set_Mode(Fading);

    // Setup

    if (!Load_Device_Registry())
    {
        TRACE();
        Save_Device_Registry();
    }
    Setup_Sound();
    Setup_Infrared_Receiver();
    Setup_WiFi();
    Setup_Web_Server();

    while (millis() < Timeout)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    Set_Mode(None);
}

void Smart_HiFi_Class::Set_Volume_Offset(int16_t Volume_Offset)
{
    // Positive volume offset
    if (Volume_Offset > 0 && (Defined_Volume + Volume_Offset) <= 255)
    {
        Defined_Volume += Volume_Offset;
        return;
    }
    else if (Volume_Offset > 0)
    {
        Defined_Volume = 255;
        return;
    }
    // Negative volume offset
    if (Volume_Offset < 0 && (-Volume_Offset) <= Defined_Volume)
    {
        Defined_Volume += Volume_Offset;
        return;
    }
    else if (Volume_Offset < 0)
    {
        Defined_Volume = 0;
        return;
    }
}

void Smart_HiFi_Class::Deep_Sleep()
{
    esp_sleep_enable_ext0_wakeup(Infrared_Receiver_Pin, LOW);
    esp_deep_sleep_start();
}

bool Smart_HiFi_Class::Check_Authentification(AsyncWebServerRequest *Request, bool Refresh_Timer)
{
    for (uint8_t i = 0; i < Maximum_Logged_Clients; i++)
    {
        // Timeout check for all clients
        if (Logged_Clients_Timer[i] < millis())
        {
            Logged_Clients_IP_Adress[i] = Null_IP_Address;
            Logged_Clients_Timer[i] = 0;
        }
        // Check if requested client is logged
        if (Logged_Clients_IP_Adress[i] == Request->client()->remoteIP())
        {
            if (Refresh_Timer == true)
            {
                Logged_Clients_Timer[i] = millis() + Loggin_Timeout;
            }
            return true;
        }
    }
    return false;
}

void Smart_HiFi_Class::Setup_Web_Server()
{

    Web_Server.end();
    Web_Server.on("/set", HTTP_POST, [](AsyncWebServerRequest *Request) {
        if (!Instance_Pointer->Check_Authentification(Request))
        {
            Request->redirect("/login");
            return;
        }
        else
        {
            Request->send(204);
        }
        if (Request->hasParam("volume", true))
        {
            String Volume_To_Set = Request->getParam("volume", true)->value();
            Instance_Pointer->Set_Volume((uint8_t)Volume_To_Set.toInt());
        }
        if (Request->hasParam("device_name", true))
        {
            Instance_Pointer->Device_Name = Request->getParam("device_name", true)->value();
            Instance_Pointer->Save_Device_Registry();
        }
        if (Request->hasParam("device_password", true))
        {
            Instance_Pointer->Device_Password = Request->getParam("device_password", true)->value();
            Instance_Pointer->Save_Device_Registry();
        }
        if (Request->hasParam("wifi_ssid", true))
        {
            Instance_Pointer->SSID = Request->getParam("wifi_ssid", true)->value();
            Instance_Pointer->Password = Request->getParam("wifi_password", true)->value();
            Instance_Pointer->Save_WiFi_Registry();
            Instance_Pointer->Setup_WiFi();
        }
        if (Request->hasParam("mute", true))
        {
            if (Request->getParam("mute", true)->value() == "true")
            {
                Instance_Pointer->Mute = true;
                Request->send(200);
            }
            else
            {
                Instance_Pointer->Mute = false;
                Request->send(200);
            }
        }
        if (Request->hasParam("state", true))
        {
            if (Request->getParam("state", true)->value() == "off")
            {
                Request->send(200);
                Instance_Pointer->Shutdown();
            }
            else if (Request->getParam("state", true)->value() == "disconnect")
            {
                for (uint8_t i = 0; i < Maximum_Logged_Clients; i++)
                {
                    if (Instance_Pointer->Logged_Clients_IP_Adress[i] == Request->client()->remoteIP())
                    {
                        Instance_Pointer->Logged_Clients_IP_Adress[i] = Instance_Pointer->Null_IP_Address;
                        Instance_Pointer->Logged_Clients_Timer[i] = 0;
                    }
                }
                Request->send(200);
            }
        }
        if (Request->hasParam("remote", true))
        {
            Instance_Pointer->Set_Mode(Fading);
            Instance_Pointer->Selected_Remote = 255;
            Instance_Pointer->Selected_Button = 255;
            if (Request->getParam("remote", true)->value() == "0")
            {
                Instance_Pointer->Selected_Remote = 0;
            }
            else if (Request->getParam("remote", true)->value() == "1")
            {
                Instance_Pointer->Selected_Remote = 1;
            }
            else if (Request->getParam("remote", true)->value() == "2")
            {
                Instance_Pointer->Selected_Remote = 2;
            }

            else if (Request->getParam("remote", true)->value() == "3")
            {
                Instance_Pointer->Selected_Remote = 3;
            }

            else if (Request->getParam("remote", true)->value() == "1")
            {
                Instance_Pointer->Selected_Remote = 4;
            }

            if (Request->getParam("button", true)->value() == "power")
            {
                Instance_Pointer->Selected_Button = Power_Button;
            }
            else if (Request->getParam("button", true)->value() == "volume-up")
            {
                Instance_Pointer->Selected_Button = Volume_Up_Button;
            }
            else if (Request->getParam("button", true)->value() == "volume-down")
            {
                Instance_Pointer->Selected_Button = Volume_Down_Button;
            }
            else if (Request->getParam("button", true)->value() == "mute")
            {
                Instance_Pointer->Selected_Button = Mute_Button;
            }

            Instance_Pointer->Set_Mode(Fading);
            Instance_Pointer->Infrared_Receiver_Mode = 1;
        }
    });

    Web_Server.on("/get", HTTP_POST, [](AsyncWebServerRequest *Request) {
        if (!Instance_Pointer->Check_Authentification(Request))
        {
            if (Request->hasParam(F("device_name"), true))
            {
                Request->send(200, "text/plain", Instance_Pointer->Device_Name);
            }
            if (Request->hasParam(F("password"), true))
            {
                String Password_To_Check = Request->getParam("password", true)->value();
                if (Password_To_Check == Instance_Pointer->Device_Password)
                {
                    for (uint8_t i = 0; i < Maximum_Logged_Clients; i++)
                    {
                        if (Instance_Pointer->Logged_Clients_IP_Adress[i] == Instance_Pointer->Null_IP_Address)
                        {
                            Instance_Pointer->Logged_Clients_IP_Adress[i] = Request->client()->remoteIP();
                            Instance_Pointer->Logged_Clients_Timer[i] = millis() + Loggin_Timeout;
                            Request->send(200, "text/plain", "true");
                            return;
                        }
                    }
                    Request->send(204, "text/plain", "failed");
                }
                else
                {
                    Request->send(204, "text/plain", "false");
                }
            }
        }
        else
        {
            if (Request->hasParam(F("password"), true))
            {
                String Password_To_Check = Request->getParam("password", true)->value();
                if (Password_To_Check == Instance_Pointer->Device_Password)
                {
                    Request->send(200, "text/plain", "true");
                }
                else
                {
                    Request->send(204, "text/plain", "false");
                }
            }
            if (Request->hasParam(F("connection-query"), true))
            {
                Request->send(200, "text/plain", "true");
            }
            if (Request->hasParam(F("device_name"), true))
            {
                Request->send(200, "text/plain", Instance_Pointer->Device_Name);
            }
            if (Request->hasParam(F("volume"), true))
            {
                Request->send(200, "text/plain", String(Instance_Pointer->Get_Current_Volume()));
            }
        }
    });

    Web_Server.on("/", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Instance_Pointer->Check_Authentification(Request))
        {
            Request->redirect("/sound");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/login", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (!Instance_Pointer->Check_Authentification(Request))
        {
            Request->send(SPIFFS, "/login.html", "text/html");
        }
        else
        {
            Request->redirect("/sound");
        }
    });

    Web_Server.on("/sound", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Instance_Pointer->Check_Authentification(Request))
        {

            Request->send(SPIFFS, "/sound.html", "text/html");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/device", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Instance_Pointer->Check_Authentification(Request))
        {
            Request->send(SPIFFS, "/device.html", "text/html");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Instance_Pointer->Check_Authentification(Request))
        {
            Request->send(SPIFFS, "/wifi.html", "text/html");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/remote", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Instance_Pointer->Check_Authentification(Request))
        {
            Request->send(SPIFFS, "/remote.html", "text/html");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    // Images

    Web_Server.on("/lock.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/lock.svg");
    });

    Web_Server.on("/bars.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/bars.svg");
    });

    Web_Server.on("/fast-backward.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/fast_backward.svg");
    });

    Web_Server.on("/link.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/link.svg");
    });
    Web_Server.on("/microchip.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/microchip.svg");
    });
    Web_Server.on("/music.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/music.svg");
    });
    Web_Server.on("/plus.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/plus.svg");
    });
    Web_Server.on("/power-off.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/power-off.svg");
    });

    Web_Server.on("/wifi.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/wifi.svg");
    });

    //Script

    Web_Server.on("/common.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/common.js", "text/javascript");
    });
    Web_Server.on("/sound.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/sound.js", "text/javascript");
    });
    Web_Server.on("/login.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/login.js", "text/javascript");
    });
    Web_Server.on("/remote.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/remote.js", "text/javascript");
    });
    Web_Server.on("/wifi.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/wifi.js", "text/javascript");
    });
    Web_Server.on("/device.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/device.js", "text/javascript");
    });
    Web_Server.on("/jquery-3.5.1.min.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/jquery-3.5.1.min.js", "text/javascript");
    });

    // Style sheet

    Web_Server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/style.css", "text/css");
    });

    Web_Server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/w3.css", "text/css");
    });

    Web_Server.begin();

    MDNS.addService("http", "tcp", 80);
}

void Smart_HiFi_Class::Setup_WiFi()
{
    MDNS.end();

    if (!Load_WiFi_Registry())
    {
        Save_WiFi_Registry();
    }

    for (uint8_t i = 0; i < Maximum_Logged_Clients; i++)
    {
        Logged_Clients_IP_Adress[i] = Null_IP_Address;
        Logged_Clients_Timer[i] = 0;
    }

    // WiFi initialize
    char Temporary_SSID[sizeof(SSID)], Temporary_Password[sizeof(Password)];
    SSID.toCharArray(Temporary_SSID, sizeof(SSID));
    Password.toCharArray(Temporary_Password, sizeof(Password));

    char Temporary_Device_Name[sizeof(Device_Name)], Temporary_Device_Password[sizeof(Device_Password)];
    Device_Name.toCharArray(Temporary_Device_Name, sizeof(Device_Name));
    Device_Password.toCharArray(Temporary_Device_Password, sizeof(Device_Password));

    WiFi.begin(Temporary_SSID, Temporary_Password);

    uint32_t Timeout = millis() + WiFi_Timeout;
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (Timeout < millis())
        {
            break;
        }
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        if (Device_Password.length() < 8)
        {
            WiFi.softAP(Temporary_Device_Name);
        }
        else
        {
            WiFi.softAP(Temporary_Device_Name);
        }
    }

    WiFi.setHostname(Temporary_Device_Name);

    if (!MDNS.begin(Temporary_Device_Name))
    {

        DUMP("Error setting up MDNS responder!");
    }

    ArduinoOTA.onStart([]() {
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
        }
        else
        {
            Instance_Pointer->Web_Server.end();
            SPIFFS.end();
        }
    });

    ArduinoOTA.begin();
}

void Smart_HiFi_Class::LED_Task(void *) //fade LED color
{
    while (1)
    {
        if (Instance_Pointer->Mode == None)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                Instance_Pointer->Set_Color(WiFi_Station_Color);
            }
            else
            {
                Instance_Pointer->Set_Color(WiFi_Access_Point_Color);
            }

            while (Instance_Pointer->Current_Color[0] != Instance_Pointer->Defined_Color[0] || Instance_Pointer->Current_Color[1] != Instance_Pointer->Defined_Color[1] || Instance_Pointer->Current_Color[2] != Instance_Pointer->Defined_Color[2])
            {
                DUMP("");
                // -- Hue
                if (Instance_Pointer->Current_Color[0] < Instance_Pointer->Defined_Color[0])
                {
                    Instance_Pointer->Current_Color[0]++;
                }
                else if (Instance_Pointer->Current_Color[0] > Instance_Pointer->Defined_Color[0])
                {
                    Instance_Pointer->Current_Color[0]--;
                }
                // -- Saturation
                if (Instance_Pointer->Current_Color[1] < Instance_Pointer->Defined_Color[1])
                {
                    Instance_Pointer->Current_Color[1]++;
                }
                else if (Instance_Pointer->Current_Color[1] > Instance_Pointer->Defined_Color[1])
                {
                    Instance_Pointer->Current_Color[1]--;
                }
                // -- Value
                if (Instance_Pointer->Current_Color[2] < Instance_Pointer->Defined_Color[2])
                {
                    Instance_Pointer->Current_Color[2]++;
                }
                else if (Instance_Pointer->Current_Color[2] > Instance_Pointer->Defined_Color[2])
                {
                    Instance_Pointer->Current_Color[2]--;
                }

                Instance_Pointer->Refresh_LED();

                vTaskDelay(pdMS_TO_TICKS(LED_Fade_Delay));
            }
        }
        else if (Instance_Pointer->Mode == Two_Blink)
        {
            ledcWrite(0, 255);
            ledcWrite(1, 255);
            ledcWrite(2, 255);
            vTaskDelay(pdMS_TO_TICKS(50));
            Instance_Pointer->Refresh_LED();
            vTaskDelay(pdMS_TO_TICKS(50));
            ledcWrite(0, 255);
            ledcWrite(1, 255);
            ledcWrite(2, 255);
            vTaskDelay(pdMS_TO_TICKS(50));
            Instance_Pointer->Refresh_LED();
            Instance_Pointer->Set_Mode(None);
        }

        else if (Instance_Pointer->Mode == Fading)
        {
            uint16_t i = 0;
            while (Instance_Pointer->Mode == Fading)
            {
                Instance_Pointer->Current_Color[0] = i;
                Instance_Pointer->Refresh_LED();
                vTaskDelay(pdMS_TO_TICKS(LED_Fade_Delay));
                i++;
                if (i >= 360)
                {
                    i = 0;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

void Smart_HiFi_Class::Refresh_LED()
{

    double s = Current_Color[1] / 100;
    double v = Current_Color[2] / 100;

    int i = (int)floor(Current_Color[0] / 60.0);

    double f = (Current_Color[0] / 60.0) - i;

    double p = v * (1 - s);
    double q = v * (1 - s * f);
    double t = v * (1 - s * (1 - f));

    switch (i)
    {
    case 0:
        Temporary_RGB[0] = v * 255;
        Temporary_RGB[1] = t * 255;
        Temporary_RGB[2] = p * 255;
        break;
    case 1:
        Temporary_RGB[0] = q * 255;
        Temporary_RGB[1] = v * 255;
        Temporary_RGB[2] = p * 255;
        break;
    case 2:
        Temporary_RGB[0] = p * 255;
        Temporary_RGB[1] = v * 255;
        Temporary_RGB[2] = t * 255;
        break;
    case 3:
        Temporary_RGB[0] = p * 255;
        Temporary_RGB[1] = q * 255;
        Temporary_RGB[2] = v * 255;
        break;
    case 4:
        Temporary_RGB[0] = t * 255;
        Temporary_RGB[1] = p * 255;
        Temporary_RGB[2] = v * 255;
        break;
    case 5:
        Temporary_RGB[0] = v * 255;
        Temporary_RGB[1] = p * 255;
        Temporary_RGB[2] = q * 255;
        break;
    }

    Temporary_RGB[0] = constrain((int)Temporary_RGB[0], 0, 255);
    Temporary_RGB[1] = constrain((int)Temporary_RGB[1], 0, 255);
    Temporary_RGB[2] = constrain((int)Temporary_RGB[2], 0, 255);

    ledcWrite(0, 255 - Temporary_RGB[0]);
    ledcWrite(1, 255 - Temporary_RGB[1]);
    ledcWrite(2, 255 - Temporary_RGB[2]);
}

void Smart_HiFi_Class::Shutdown()
{
    TRACE();

    Set_Mode(Fading);

    vTaskDelay(2500);

    digitalWrite(Power_Pin, LOW);

    Deep_Sleep();
}

void Smart_HiFi_Class::Set_Mode(LED_Mode Mode)
{
    this->Mode = Mode;
}

void Smart_HiFi_Class::Set_Color(double Hue, double Saturation, double Value)
{

    Defined_Color[0] = Hue;
    Defined_Color[1] = Saturation;
    Defined_Color[2] = Value;
}

void Smart_HiFi_Class::Infrared_Receiver_Task(void *)
{
    Instance_Pointer->Infrared_Receiver.enableIRIn();
    Instance_Pointer->Infrared_Receiver.resume();
    uint32_t Timeout = 0;
    uint32_t Temporary_Code;
    while (1)
    {
        if (Instance_Pointer->Infrared_Receiver_Mode == 1)
        {

            if (Timeout == 0)
            {
                Timeout = millis() + 10000;
                Instance_Pointer->Set_Mode(Fading);
            }
            else if (Timeout < millis())
            {
                Instance_Pointer->Infrared_Receiver_Mode = 0;
                Timeout = 0;
                Instance_Pointer->Set_Mode(None);
                Instance_Pointer->Selected_Remote = 255;
                Instance_Pointer->Selected_Button = 255;
            }
        }
        if (Instance_Pointer->Infrared_Receiver.decode(&Instance_Pointer->Received_Data))
        {
            // -- Normal mode
            if (Instance_Pointer->Infrared_Receiver_Mode == 0) //
            {
                for (uint8_t i = 0; i < Maximum_Remotes; i++)
                {
                    if (Instance_Pointer->Received_Data.value == Instance_Pointer->Power_Code[i])
                    {
                        Instance_Pointer->Set_Mode(Two_Blink);
                        vTaskDelay(pdMS_TO_TICKS(200));
                        Instance_Pointer->Shutdown();
                        break;
                    }
                    else if (Instance_Pointer->Received_Data.value == Instance_Pointer->Volume_Up_Code[i])
                    {
                        Instance_Pointer->Set_Mode(Two_Blink);
                        Instance_Pointer->Set_Volume_Offset(256 / Volume_Step);
                        break;
                    }
                    else if (Instance_Pointer->Received_Data.value == Instance_Pointer->Volume_Down_Code[i])
                    {
                        Instance_Pointer->Set_Mode(Two_Blink);
                        Instance_Pointer->Set_Volume_Offset(-256 / Volume_Step);
                        break;
                    }
                    else if (Instance_Pointer->Received_Data.value == Instance_Pointer->Mute_Code[i])
                    {
                        Instance_Pointer->Set_Mode(Two_Blink);
                        Instance_Pointer->Mute = !Instance_Pointer->Mute;
                        break;
                    }
                    else
                    {
                        Instance_Pointer->Set_Mode(None);
                    }
                }
                Timeout = 0;
                vTaskDelay(pdMS_TO_TICKS(160));
            }
            // -- Remote setting mode
            else if (Instance_Pointer->Infrared_Receiver_Mode == 1 && Instance_Pointer->Selected_Remote < Maximum_Remotes && Instance_Pointer->Selected_Button != 255)
            {
                Temporary_Code = Instance_Pointer->Received_Data.value;

                do
                {

                    Instance_Pointer->Infrared_Receiver.resume();
                    while (!Instance_Pointer->Infrared_Receiver.decode(&Instance_Pointer->Received_Data) && Timeout >= millis())
                    {
                        vTaskDelay(pdMS_TO_TICKS(40));
                    }

                    if (Temporary_Code == Instance_Pointer->Received_Data.value)
                    {
                        switch (Instance_Pointer->Selected_Button)
                        {
                        case Power_Button:
                            Instance_Pointer->Power_Code[Instance_Pointer->Selected_Remote] = Instance_Pointer->Received_Data.value;
                            break;
                        case Volume_Up_Button:
                            Instance_Pointer->Volume_Up_Code[Instance_Pointer->Selected_Remote] = Instance_Pointer->Received_Data.value;
                            break;
                        case Volume_Down_Button:
                            Instance_Pointer->Volume_Down_Code[Instance_Pointer->Selected_Remote] = Instance_Pointer->Received_Data.value;
                            break;
                        case Mute_Button:
                            Instance_Pointer->Mute_Code[Instance_Pointer->Selected_Remote] = Instance_Pointer->Received_Data.value;
                            break;
                        default:
                            break;
                        }
                        Instance_Pointer->Save_Remote_Registry();
                        break;
                    }
                    else
                    {
                        Temporary_Code = Instance_Pointer->Received_Data.value;
                        Timeout = millis() + 10000;
                    }

                } while (Timeout >= millis());

                Instance_Pointer->Selected_Remote = 255;
                Instance_Pointer->Selected_Button = 255;
                Instance_Pointer->Infrared_Receiver_Mode = 0;
                Timeout = 0;
                Instance_Pointer->Set_Mode(None);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            Instance_Pointer->Infrared_Receiver.resume();
        }
        // Handle OTA
        ArduinoOTA.handle();

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

void Smart_HiFi_Class::Sound_Task(void *)
{
    uint8_t Current_Volume;
    uint8_t Delta;
    while (1)
    {

        // Set volume
        Current_Volume = Instance_Pointer->Get_Current_Volume();

        if (Current_Volume > Instance_Pointer->Defined_Volume)
        {
            Delta = Current_Volume - Instance_Pointer->Defined_Volume;
        }
        else
        {
            Delta = Instance_Pointer->Defined_Volume - Current_Volume;
        }

        if (Instance_Pointer->Defined_Volume == 0 || Instance_Pointer->Mute == true)
        {
            digitalWrite(Power_Pin, LOW);
        }
        else
        {
            digitalWrite(Power_Pin, HIGH);
        }

        if (Delta > (Volume_Step / 2))
        {

            while (Current_Volume > Instance_Pointer->Defined_Volume)
            {
                digitalWrite(Down_Pin, HIGH);
                digitalWrite(Up_Pin, LOW);
                vTaskDelay(pdMS_TO_TICKS(2));
                Current_Volume = Instance_Pointer->Get_Current_Volume();
            }

            while (Current_Volume < Instance_Pointer->Defined_Volume)
            {
                digitalWrite(Down_Pin, LOW);
                digitalWrite(Up_Pin, HIGH);
                vTaskDelay(pdMS_TO_TICKS(2));
                Current_Volume = Instance_Pointer->Get_Current_Volume();
            }
        }

        digitalWrite(Down_Pin, LOW);
        digitalWrite(Up_Pin, LOW);

        // Handle OTA
        ArduinoOTA.handle();

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

bool Smart_HiFi_Class::Save_Device_Registry()
{
    File Temporary_File = SPIFFS.open(Device_Registry_Path, FILE_WRITE);
    DynamicJsonDocument Device_Registry(512);
    Device_Registry["Registry"] = "Device";
    Device_Registry["Name"] = Device_Name;
    Device_Registry["Password"] = Device_Password;
    if (serializeJson(Device_Registry, Temporary_File) == 0)
    {
        Temporary_File.close();
        return false;
    }
    Temporary_File.close();
    return true;
}

bool Smart_HiFi_Class::Load_Device_Registry()
{
    File Temporary_File = SPIFFS.open(Device_Registry_Path);
    DynamicJsonDocument Device_Registry(512);
    Device_Name = Default_Device_Name;
    Device_Password = Default_Device_Password;
    if (deserializeJson(Device_Registry, Temporary_File) != DeserializationError::Ok)
    {
        Temporary_File.close();
        return false;
    }
    Temporary_File.close();
    if (strcmp("Device", Device_Registry["Registry"] | "") != 0)
    {
        return false;
    }
    Device_Name = Device_Registry["Name"] | Default_Device_Name;
    Device_Password = Device_Registry["Password"] | Default_Device_Password;
    return true;
}

bool Smart_HiFi_Class::Save_WiFi_Registry()
{
    File Temporary_File = SPIFFS.open(WiFi_Registry_Path, FILE_WRITE);
    DynamicJsonDocument WiFi_Registry(512);
    WiFi_Registry["Registry"] = "WiFi";
    WiFi_Registry["Name"] = SSID;
    WiFi_Registry["Password"] = Password;
    if (serializeJson(WiFi_Registry, Temporary_File) == 0)
    {
        Temporary_File.close();
        return false;
    }
    Temporary_File.close();
    return true;
}

bool Smart_HiFi_Class::Load_WiFi_Registry()
{
    File Temporary_File = SPIFFS.open(WiFi_Registry_Path);
    DynamicJsonDocument WiFi_Registry(512);
    if (deserializeJson(WiFi_Registry, Temporary_File) != DeserializationError::Ok)
    {
        Temporary_File.close();
        return false;
    }
    Temporary_File.close();
    if (strcmp("WiFi", WiFi_Registry["Registry"] | "") != 0)
    {
        return false;
    }
    SSID = WiFi_Registry["Name"] | "";
    Password = WiFi_Registry["Password"] | "";
    return true;
}

bool Smart_HiFi_Class::Save_Remote_Registry()
{
    File Temporary_File = SPIFFS.open(Remote_Registry_Path, FILE_WRITE);
    DynamicJsonDocument Remote_Registry(512);
    Remote_Registry["Registry"] = "Remote";
    JsonArray Codes_Arrays = Remote_Registry.createNestedArray("Codes");

    for (uint8_t i = 0; i < Maximum_Remotes; i++)
    {
        JsonArray Codes_Array = Codes_Arrays.createNestedArray();

        Codes_Array.add(Power_Code[i]);
        Codes_Array.add(Volume_Up_Code[i]);
        Codes_Array.add(Volume_Down_Code[i]);
        Codes_Array.add(Mute_Code[i]);
    }

    if (serializeJson(Remote_Registry, Temporary_File) == 0)
    {
        Temporary_File.close();
        return false;
    }
    Temporary_File.close();
    return true;
}

bool Smart_HiFi_Class::Load_Remote_Registry()
{
    File Temporary_File = SPIFFS.open(Remote_Registry_Path);
    DynamicJsonDocument Remote_Registry(512);
    if (deserializeJson(Remote_Registry, Temporary_File) != DeserializationError::Ok)
    {
        Temporary_File.close();
        return false;
    }
    Temporary_File.close();
    if (strcmp("Remote", Remote_Registry["Registry"] | "") != 0)
    {
        return false;
    }
    for (uint8_t i = 0; i < Maximum_Remotes; i++)
    {
        JsonArray Codes_Array = Remote_Registry["Codes"][i];

        Power_Code[i] = Codes_Array[0];
        Volume_Up_Code[i] = Codes_Array[1];
        Volume_Down_Code[i] = Codes_Array[2];
        Mute_Code[i] = Codes_Array[3];
    }

    return true;
}

void Smart_HiFi_Class::Set_Volume(uint8_t Volume)
{
    Defined_Volume = Volume;
}

uint8_t Smart_HiFi_Class::Get_Defined_Volume()
{
    return Defined_Volume;
}

uint8_t Smart_HiFi_Class::Get_Current_Volume()
{
    return (255 - ((analogRead(Potentiometer_Pin) * 256) / 4096));
}

void Smart_HiFi_Class::Setup_LED()
{
    ledcAttachPin(Red_LED_Pin, 0);
    ledcAttachPin(Green_LED_Pin, 1);
    ledcAttachPin(Blue_LED_Pin, 2);

    ledcSetup(0, LED_Frequency, 8);
    ledcSetup(1, LED_Frequency, 8);
    ledcSetup(2, LED_Frequency, 8);

    ledcWrite(0, 255);
    ledcWrite(1, 255);
    ledcWrite(2, 255);

    xTaskCreatePinnedToCore(LED_Task, "LED Task", 2 * 1024, NULL, 2, &LED_Task_Handle, 0);
}

void Smart_HiFi_Class::Setup_Sound()
{
    pinMode(Potentiometer_Pin, INPUT);

    pinMode(Down_Pin, OUTPUT);
    pinMode(Up_Pin, OUTPUT);

    pinMode(Power_Pin, OUTPUT);

    Defined_Volume = Get_Current_Volume();

    xTaskCreatePinnedToCore(Sound_Task, "Sound Task", 3 * 1024, NULL, 2, &Sound_Task_Handle, 0);
}

void Smart_HiFi_Class::Setup_Infrared_Receiver()
{
    xTaskCreatePinnedToCore(Infrared_Receiver_Task, "CIR", 6 * 1024, NULL, 2, &Infrared_Receiver_Task_Handle, 0);
}