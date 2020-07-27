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

    //Start();

    // Load configuration

    if (!Load_Configuration())
    {
    }

    // Setup Web Server

    Web_Server.on("/get", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (!Logged)
        {
            if (Request->hasParam("password"))
            {
                String Password_To_Check = Request->getParam("password")->value();
                if (Password_To_Check == Password)
                {
                    Logged = true;
                    Request->redirect("/volume");
                }
                else
                {
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
            if (Request->hasParam("set-code"))
            {
                vTaskSuspend(Check_Infrared_Receiver_Handle);
                while (!Infrared_Receiver.decode(&Received_Data))
                {
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                if (Request->getParam("set-code")->value() == "mute")
                {

                    Power_Code = Received_Data.value;
                }
                else if (Request->getParam("set-code")->value() == "volume-up")
                {
                    Volume_Up_Code = Received_Data.value;
                }
                else if (Request->getParam("set-code")->value() == "volume-down")
                {
                    Volume_Down_Code = Received_Data.value;
                }
                else if (Request->getParam("set-code")->value() == "wifi-switch")
                {
                    Volume_Up_Code = Received_Data.value;
                }
                Infrared_Receiver.resume();
                vTaskResume(Check_Infrared_Receiver_Handle);
            }
        }
    });

    Web_Server.on("/", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {
            Request->redirect("/volume");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/login", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {
            Request->redirect("/volume");
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
        if (Logged)
        {
            Request->send(SPIFFS, "/lock.svg");
        }
        else
        {
            Request->redirect("/login");
        }
    });


    Web_Server.on("/wifi.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {
            Request->send(SPIFFS, "/wifi.svg");
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/w3.css", "text/css");
    });

    Web_Server.begin();

    // Setup Potentiometer
    pinMode(POTENTIOMETER_PIN, INPUT);
    pinMode(DOWN_PIN, OUTPUT);
    pinMode(UP_PIN, OUTPUT);
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

    Defined_Volume = VOLUME_STEP - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, VOLUME_STEP);

    // WiFi initialize
    WiFi.begin();
    uint32_t Timeout = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
        if (WIFI_TIMEOUT < millis() - Timeout)
        {
            State = POWER_ON_WIFI_ACCESS_POINT;
            //WiFi.softAP(Device_Name, Password);
        }
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
                    Set_Volume(-1);
                    break;
                }
                else if (Received_Data.value == Volume_Up_Code)
                {
                    Set_LED_Color(255 - Defined_Volume, Defined_Volume, 0);
                    Set_Volume(1);
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

void Set_Volume(int16_t const &Volume_To_Set)
{

    if (Volume_To_Set + Defined_Volume > 255)
    {
        Defined_Volume = 255;
    }
    else if (Volume_To_Set + Defined_Volume < 0)
    {
        Defined_Volume = 0;
    }
    else
    {
        Defined_Volume += Volume_To_Set;
    }

    Serial.print(F("Volume defined :"));
    Serial.println(Defined_Volume);

    Current_Volume = VOLUME_STEP - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, VOLUME_STEP);
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
        delay(50);
        Current_Volume = VOLUME_STEP - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, VOLUME_STEP);
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