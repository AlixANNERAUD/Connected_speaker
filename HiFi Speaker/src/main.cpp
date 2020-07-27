#include "main.hpp"

void setup()
{
    Serial.begin(115200);
    Infrared_Receiver.enableIRIn();

    // Setup SPIFFS

    if (!SPIFFS.begin())
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        State = POWER_OFF_ERROR;
        return;
    }

    // Load configuration

    if (!Load_Configuration())
    {
    }

    Start();

    // Setup Web Server

    Web_Server.on("/get", HTTP_POST, [](AsyncWebServerRequest *Request) {
        if (!Logged)
        {
            
            if (Request->hasParam("password", true))
            {
                Request->send(204);
                Serial.print("Received passord :");
                String Password_To_Check = Request->getParam("password", true)->value();
                Serial.println(Password_To_Check);
                if (Password_To_Check == Password)
                {
                    Serial.println("Good password !");
                    Logged = true;
                    Request->redirect("/volume");
                }
                else
                {
                    Serial.println("Wrong password !");
                    Logged = false;
                }
            }
            else
            {
                Request->redirect("/login");
            }
            
        }
        else
        {
            if (Request->hasParam("set-code", true))
            {
                vTaskSuspend(Check_Infrared_Receiver_Handle);
                while (!Infrared_Receiver.decode(&Received_Data))
                {
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                if (Request->getParam("set-code", true)->value() == "mute")
                {

                    Power_Code = Received_Data.value;
                }
                else if (Request->getParam("set-code", true)->value() == "volume-up")
                {
                    
                    Volume_Up_Code = Received_Data.value;
                }
                else if (Request->getParam("set-code", true)->value() == "volume-down")
                {
                    Volume_Down_Code = Received_Data.value;
                }
                else if (Request->getParam("set-code", true)->value() == "wifi-switch")
                {
                    Volume_Up_Code = Received_Data.value;
                }
                Infrared_Receiver.resume();
                vTaskResume(Check_Infrared_Receiver_Handle);
            }
            else if (Request->hasParam("get_volume", true))
            {
                Request->send(200, "text/plain", String(map(Defined_Volume, 0, 255, 0, VOLUME_STEP)));
            }
            else if (Request->hasParam("set_volume", true))
            {
                Request->send(204);
                String Volume_To_Set = Request->getParam("set_volume", true)->value();
                Defined_Volume = (uint8_t)map(Volume_To_Set.toInt(), 0, VOLUME_STEP, 0, 255);
                Set_Volume();
            }
        }
    });

    Web_Server.on("/", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {
            Request->redirect("/sound");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/login", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {
            Request->redirect("/sound");
        }
        else
        {     
            Request->send(SPIFFS, "/login.html", "text/html");
            
        }
    });

    Web_Server.on("/sound", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {

            Request->send(SPIFFS, "/sound.html", "text/html");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/update", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {
            Request->send(SPIFFS, "/update.html", "text/html");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {
            Request->send(SPIFFS, "/wifi.html", "text/html");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/remote", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {
            Request->send(SPIFFS, "/remote.html");
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
        Request->send(SPIFFS, "/bars.svg", "");
    });
    Web_Server.on("/bluetooth-b.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/bluetooth-b.svg", "");
    });
    Web_Server.on("/fast-backward.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/fast_backward.svg", "");
    });
    Web_Server.on("/lightbulb.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/lightbulb.svg", "");
    });
    Web_Server.on("/link.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/link.svg", "");
    });
    Web_Server.on("/microchip.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/microchip.svg", "");
    });
    Web_Server.on("/music.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/music.svg", "");
    });
    Web_Server.on("/plus.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/plus.svg", "");
    });
    Web_Server.on("/power-off.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/power-off.svg", "");
    });
    Web_Server.on("/sync-alt.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/sync-alt.js", "text/javascript");
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

    Web_Server.on("/jquery-3.5.1.min.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/jquery-3.5.1.min.js", "text/javascript");
    });

    // Style sheet

    Web_Server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *Request)
    {
        Request->send(SPIFFS, "/style.css", "text/css");
    });

    Web_Server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/w3.css", "text/css");
    });

    Web_Server.begin();

    xTaskCreatePinnedToCore(Check_Infrared_Receiver, "CIR", 6 * 2014, NULL, 2, NULL, 1);
}

void loop()
{
    vTaskDelete(NULL);
}

void Start()
{
    Serial.println(F("Start"));
    State = 1;

    // Setup LED

    ledcAttachPin(RED_LED_PIN, 0);
    ledcAttachPin(GREEN_LED_PIN, 1);
    ledcAttachPin(BLUE_LED_PIN, 2);
    ledcSetup(0, LED_FREQUENCY, 8);
    ledcSetup(1, LED_FREQUENCY, 8);
    ledcSetup(2, LED_FREQUENCY, 8);
    ledcWrite(0, 255);
    ledcWrite(1, 255);
    ledcWrite(2, 255);
    Set_LED_Color(0, 0, 0);

    // Setup Power
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, LOW);

    // Setup Potentiometer

    pinMode(POTENTIOMETER_PIN, INPUT);
    pinMode(DOWN_PIN, OUTPUT);
    pinMode(UP_PIN, OUTPUT);

    Defined_Volume = VOLUME_STEP - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, VOLUME_STEP);

    // WiFi initialize
    WiFi.begin("Avrupa", "0749230994");
    uint32_t Timeout = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
        if (WIFI_TIMEOUT < millis() - Timeout)
        {
            Serial.println(F("Create AP"));
            State = POWER_ON_WIFI_ACCESS_POINT;
            //WiFi.softAP(Device_Name, Password);
        }
    }
    Serial.println(F("Connected"));

    if (!MDNS.begin("esp32")) {
        Serial.println("Error setting up MDNS responder!");

    }

    digitalWrite(POWER_PIN, HIGH); // turn the amplifier power supply on
    uint8_t i;
    for (i = 0; i < 255; i++)
    {
        Set_LED_Color(i, 0, 0);
        delay(1);
    }
    for (i = 0; i < 255; i++)
    {
        Set_LED_Color(255 - i, i, 0);
        delay(1);
    }
    for (i = 0; i < 255; i++)
    {
        Set_LED_Color(0, 255 - i, i);
        delay(1);
    }
}

void Shutdown()
{
    Serial.println(F("Shutdown"));
    State = 0;

    // animation
    uint8_t i;
    for (i = 0; i < 255; i++)
    {
        Set_LED_Color(0, 0, i);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 0; i < 255; i++)
    {
        Set_LED_Color(0, i, 255 - i);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 0; i < 255; i++)
    {
        Set_LED_Color(i, 255 - i, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 255; i > 0; i--)
    {
        Set_LED_Color(i, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    digitalWrite(POWER_PIN, LOW);
    Set_LED_Color(0, 0, 0);
}

void Set_LED_Color(uint8_t const &Red, uint8_t const &Green, uint8_t const &Blue)
{
    ledcWrite(0, 255 - Red);
    ledcWrite(1, 255 - Green);
    ledcWrite(2, 255 - Blue);
}

void Check_Infrared_Receiver(void *pvParameters)
{
    (void)pvParameters;
    while (1)
    {

        if (Infrared_Receiver.decode(&Received_Data))
        {
            Serial.print(F("IR :"));
            serialPrintUint64(Received_Data.value, HEX);
            Infrared_Receiver.resume();
            if (State == POWER_OFF_STATE)
            {
                if (Received_Data.value == Power_Code)
                {
                    Start();
                }
            }
            else
            {
                if (Received_Data.value == Power_Code)
                {
                    Shutdown();
                    break;
                }
                else if (Received_Data.value == Volume_Down_Code)
                {
                    Set_LED_Color(255 - Defined_Volume, Defined_Volume, 0);
                    Defined_Volume += 255 / VOLUME_STEP;
                    Set_Volume();
                    break;
                }
                else if (Received_Data.value == Volume_Up_Code)
                {
                    Set_LED_Color(255 - Defined_Volume, Defined_Volume, 0);
                    Defined_Volume -= 255 / VOLUME_STEP;
                    Set_Volume();
                    break;
                }
                else if (Received_Data.value == State_Code)
                {
                    if (State != 0 && State < 3)
                    {
                        State++;
                    }
                    else if (State >= 3)
                    {
                        State = 1;
                    }
                    break;
                }
            }
            Infrared_Receiver.resume();
        }
        switch (State)
        {
        case POWER_ON_WIFI_STATION:
            Set_LED_Color(0, 0, 255);
            break;
        case POWER_ON_WIFI_ACCESS_POINT:
            Set_LED_Color(0, 255, 0);
            break;
        case POWER_ON_WIFI_DISABLED:
            Set_LED_Color(255, 255, 0);
        default:
            break;
        }
    }
}

void Set_Volume() 
{
    static uint8_t Current_Volume;

    Serial.print(F("Set volume to :"));
    Serial.println(Defined_Volume);

    Current_Volume = 255 - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 255);
    
    while (Current_Volume != Defined_Volume)
    {
        if (Current_Volume > Defined_Volume)
        {
            digitalWrite(DOWN_PIN, HIGH);
            digitalWrite(UP_PIN, LOW);
        }
        else if (Current_Volume < Defined_Volume)
        {
            digitalWrite(DOWN_PIN, LOW);
            digitalWrite(UP_PIN, HIGH);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        Current_Volume = 255 - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 255);
    }
    digitalWrite(DOWN_PIN, LOW);
    digitalWrite(UP_PIN, LOW);
}

uint8_t Load_Configuration()
{
    Serial.println(F("Load configuration"));
    Temporary_File = SPIFFS.open("/password", FILE_READ);
    if (Temporary_File)
    {
        Password = Temporary_File.readString();
    }

    Temporary_File = SPIFFS.open("/code", FILE_READ);
    if (Temporary_File)
    {
        //Power_Code = (uint32) Temporary_File.read();
    }

    Temporary_File = SPIFFS.open("/color", FILE_READ);
    {
        //
    }

    return true;
}

uint8_t Save_Configuration()
{
    return true;
}