#include "main.hpp"

void setup()
{
    Serial.begin(115200);

    esp_sleep_wakeup_cause_t Wakeup_Reason;
    Wakeup_Reason = esp_sleep_get_wakeup_cause();

    if (Wakeup_Reason == ESP_SLEEP_WAKEUP_EXT0)
    {
        Serial.println("IR sensor");
        Infrared_Receiver.enableIRIn();
        uint32_t Timeout = millis();
        while (millis() - Timeout < WAKEUP_TIMEOUT)
        {
            if (Infrared_Receiver.decode(&Received_Data))
            {
                if (Received_Data.value == Power_Code[0] || Received_Data.value == Power_Code[1] || Received_Data.value == Volume_Up_Code[0] || Received_Data.value == Volume_Up_Code[1] || Received_Data.value == Volume_Down_Code[0] || Received_Data.value == Volume_Down_Code[1] || Received_Data.value == State_Code[0] || Received_Data.value == State_Code[1])
                {
                    Infrared_Receiver.disableIRIn();
                    Start();
                    Timeout = 0;
                }
                Received_Data.value = 0;
                Infrared_Receiver.resume();
            }
        }
        if (Timeout != 0)
        {
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW);
            esp_deep_sleep_start();
        }
    }
    else if (Wakeup_Reason == ESP_SLEEP_WAKEUP_UNDEFINED)
    {
        Serial.println("Plugged into power wall");
        Infrared_Receiver.enableIRIn();
        Start();
    }
    else
    {
        Serial.println("Other wakeup reason");
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW);
        esp_deep_sleep_start();
    }
}

void loop()
{
    vTaskDelete(NULL);
}

void Setup_Web_Server()
{

    Serial.println(F("Setup Web Server"));

    Web_Server.on("/set", HTTP_POST, [](AsyncWebServerRequest *Request) {
        Serial.print(F("/set :"));
        if (Logged)
        {
            if (Request->hasParam("volume", true))
            {
                Serial.println(F("volume"));
                Request->send(204);
                String Volume_To_Set = Request->getParam("volume", true)->value();
                Defined_Volume = (uint8_t)map(Volume_To_Set.toInt(), 0, VOLUME_STEP, 0, 255);
            }
            else if (Request->hasParam("device_name"))
            {
                Serial.println(F("device name"));
                Device_Name = Request->getParam("device_name", true)->value();
                Save_Configuration(1);
            }
            else if (Request->hasParam("device_password"))
            {
                Serial.println(F("device passwd"));
                Device_Password = Request->getParam("device_password", true)->value();
                Save_Configuration(1);
            }
            else if (Request->hasParam("wifi_ssid", true))
            {
                Serial.println(F("wifi ssid"));
                SSID = Request->getParam("wifi_ssid", true)->value();
                Password = Request->getParam("wifi_password", true)->value();
                Save_Configuration(2);
                ESP.restart();
            }
            else if (Request->hasParam("code", true))
            {
                Serial.println(F("code"));
                vTaskSuspend(Infrared_Receiver_Handle);
                while (!Infrared_Receiver.decode(&Received_Data))
                {
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
                if (Request->getParam("code", true)->value() == "power-0")
                {
                    Power_Code[0] = Received_Data.value;
                }
                else if (Request->getParam("code", true)->value() == "power-1")
                {
                    Power_Code[1] = Received_Data.value;
                }
                else if (Request->getParam("code", true)->value() == "volume-up-0")
                {
                    Volume_Up_Code[0] = Received_Data.value;
                }
                else if (Request->getParam("code", true)->value() == "volume-up-1")
                {
                    Volume_Up_Code[1] = Received_Data.value;
                }
                else if (Request->getParam("code", true)->value() == "volume-down-0")
                {
                    Volume_Down_Code[0] = Received_Data.value;
                }
                else if (Request->getParam("code", true)->value() == "volume-down-1")
                {
                    Volume_Down_Code[1] = Received_Data.value;
                }
                else if (Request->getParam("code", true)->value() == "state-0")
                {
                    State_Code[0] = Received_Data.value;
                }
                else if (Request->getParam("code", true)->value() == "state-1")
                {
                    State_Code[1] = Received_Data.value;
                }
                Infrared_Receiver.resume();
                vTaskResume(Infrared_Receiver_Handle);
            }
        }
        else
        {
            Request->redirect("/login");
        }
    });

    Web_Server.on("/get", HTTP_POST, [](AsyncWebServerRequest *Request) {
        Serial.print(F("/get :"));
 
        if (Request->hasParam("device_name", true))
        {
            Serial.println(F("device name"));
            Request->send(200, "text/plain", Device_Name); 
        }
        else
        {
            if (!Logged)
            {
                if (Request->hasParam("password", true))
                {
                    Serial.println(F("password"));
                    Serial.print("Received passord :");
                    String Password_To_Check = Request->getParam("password", true)->value();
                    Serial.println(Password_To_Check);
                    if (Password_To_Check == Device_Password)
                    {
                        Serial.println("Good password !");
                        Logged_Client = Request->client()->remoteIP();
                        Request->send(200, "text/plain", "true");
                    }
                    else
                    {
                        Request->send(204);
                    }
                }
                else
                {
                    Request->redirect("/login");
                }
            }
            else
            {

                if (Request->hasParam("volume", true))
                {
                    Serial.println(F("volume"));
                    Request->send(200, "text/plain", String(map(Defined_Volume, 0, 255, 0, VOLUME_STEP)));
                }


            }
        }
    });

    Web_Server.on("/", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("/"));
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
        Serial.println(F("/login"));
        /*if (Logged)
        {
            Request->redirect("/login");
        }
        else
        {*/
            Request->send(SPIFFS, "/login.html", "text/html");
        //}
    });

    Web_Server.on("/sound", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("/sound"));
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
        Serial.println(F("/update"));
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
        Serial.println(F("/wifi"));
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
        Serial.println(F("/remote"));
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
        
        Serial.println(F("lock.svg"));
        Request->send(SPIFFS, "/lock.svg");
        
    });

    Web_Server.on("/bars.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("bars.svg"));
        Request->send(SPIFFS, "/bars.svg");
        
    });
    Web_Server.on("/bluetooth-b.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("bt.svg"));
        Request->send(SPIFFS, "/bluetooth-b.svg");
        
    });
    Web_Server.on("/fast-backward.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("fast back.svg"));
        Request->send(SPIFFS, "/fast_backward.svg");
        
    });
    Web_Server.on("/lightbulb.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Request->send(SPIFFS, "/lightbulb.svg");
        Serial.println(F("lightbulb.svg"));
    });
    Web_Server.on("/link.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("link.svg"));
        Request->send(SPIFFS, "/link.svg");
        
    });
    Web_Server.on("/microchip.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("microchip.svg"));
        Request->send(SPIFFS, "/microchip.svg");
        
    });
    Web_Server.on("/music.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("music.svg"));
        Request->send(SPIFFS, "/music.svg");
        
    });
    Web_Server.on("/plus.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("plus.svg"));
        Request->send(SPIFFS, "/plus.svg");
        
    });
    Web_Server.on("/power-off.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("power.svg"));
        Request->send(SPIFFS, "/power-off.svg");
        
    });
    Web_Server.on("/sync-alt.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("sync.svg"));
        Request->send(SPIFFS, "/sync-alt.svg");
        
    });

    Web_Server.on("/wifi.svg", HTTP_GET, [](AsyncWebServerRequest *Request) {
        
        Serial.println(F("wifi.svg"));
        Request->send(SPIFFS, "/wifi.svg");
        
    });

    //Script

    Web_Server.on("/common.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("com.js"));
        Request->send(SPIFFS, "/common.js", "text/javascript");
        
    });
    Web_Server.on("/sound.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("sound.js"));
        Request->send(SPIFFS, "/sound.js", "text/javascript");
        
    });
    Web_Server.on("/login.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("login.js"));
        Request->send(SPIFFS, "/login.js", "text/javascript");
        
    });
    Web_Server.on("/remote.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("remote.js"));
        Request->send(SPIFFS, "/remote.js", "text/javascript");
        
    });
    Web_Server.on("/led.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("led.js"));
        Request->send(SPIFFS, "/led.js", "text/javascript");
        
       
    });

    Web_Server.on("/jquery-3.5.1.min.js", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("jq.js"));
        Request->send(SPIFFS, "/jquery-3.5.1.min.js", "text/javascript");
    });

    // Style sheet

    /*Web_Server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("style.css"));
        Request->send(SPIFFS, "/style.css", "text/css");
    });*/

    Web_Server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *Request) {
        Serial.println(F("W3.css"));
        Request->send(SPIFFS, "/w3.css", "text/css");
    });

    Web_Server.begin();
}

void WiFi_Initialize()
{
    // WiFi initialize
    Web_Server.end();

    char Temporary_SSID[sizeof(SSID)], Temporary_Password[sizeof(Password)];
    SSID.toCharArray(Temporary_SSID, sizeof(SSID));
    Password.toCharArray(Temporary_Password, sizeof(Password));
    char Temporary_Device_Name[sizeof(Device_Name)], Temporary_Device_Password[sizeof(Device_Password)];
    Device_Name.toCharArray(Temporary_Device_Name, sizeof(Device_Name));
    Device_Password.toCharArray(Temporary_Device_Password, sizeof(Device_Password));
    WiFi.begin(Temporary_SSID, Temporary_Password);
    WiFi.setHostname(Temporary_Device_Name);
    uint32_t Timeout = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        vTaskDelay(pdMS_TO_TICKS(100));
        if (WIFI_TIMEOUT < millis() - Timeout)
        {
            break;
        }
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(F("Create AP"));
        Set_State(POWER_ON_WIFI_ACCESS_POINT_STATE);
        WiFi.softAP(Temporary_Device_Name, Temporary_Device_Password);
    }
    else
    {
        Serial.println(F("Connected"));
        Set_State(POWER_ON_WIFI_STATION_STATE);
    }

    if (!MDNS.begin(Temporary_Device_Name))
    {
        Serial.println("Error setting up MDNS responder!");
    }
}

void Start()
{
    Serial.println(F("Start"));
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

    xTaskCreatePinnedToCore(LED_Task, "LED", 2 * 1024, NULL, 2, &LED_Handle, 1);

    Set_State(POWER_ON_START);

    // Setup Power

    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, LOW);

    // Setup Potentiometer

    pinMode(POTENTIOMETER_PIN, INPUT);
    pinMode(DOWN_PIN, OUTPUT);
    pinMode(UP_PIN, OUTPUT);

    Defined_Volume = 255 - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 255);

    // Setup SPIFFS
    if (!SPIFFS.begin())
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        Set_State(POWER_ON_ERROR_STATE);
        return;
    }

    // Load configuration

    if (!Load_Configuration())
    {
        Set_State(POWER_ON_ERROR_STATE);
    }

    WiFi_Initialize();

    // Initialize OTA

    ArduinoOTA.onStart([]() {
        uint8_t Update_Type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
            Update_Type = 'C'; //code
        }
        else
        {
            Update_Type = 'F'; //filesystem
            Web_Server.end();
            SPIFFS.end();
        }
    });

    ArduinoOTA.begin();

    //

    Setup_Web_Server();

    // Finaly : turn the amplifier on
    digitalWrite(POWER_PIN, HIGH); // turn the amplifier power supply on

    /*uint8_t i;
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
    }*/

    // Setup Web Server

    xTaskCreatePinnedToCore(Infrared_Receiver_Task, "CIR", 6 * 1024, NULL, 2, &Infrared_Receiver_Handle, 1);
}

void Set_State(uint8_t const &State_To_Set)
{
    State = State_To_Set;
    switch (State)
    {
    case POWER_OFF_STATE:
        Set_LED_Color(0x000000);
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW);
        esp_deep_sleep_start();
    case POWER_ON_START:
        Set_LED_Color(POWER_ON_START_COLOR);
        break;
    case POWER_ON_WIFI_STATION_STATE:
        Set_LED_Color(Power_On_WiFi_Station_Color);
        break;
    case POWER_ON_WIFI_ACCESS_POINT_STATE:
        Set_LED_Color(Power_On_WiFi_Access_Point_Color);
        break;
    case POWER_ON_WIFI_DISABLED_STATE:
        WiFi.mode(WIFI_MODE_NULL);
        Set_LED_Color(Power_On_WiFi_Disabled_Color);
        break;
    case POWER_ON_ERROR_STATE:
        Set_LED_Color(POWER_ON_ERROR_COLOR);
        break;
    default:
        break;
    }
}

void LED_Task(void *pvParameters) //fade LED color
{
    (void)pvParameters;
    uint8_t Current_Red = 0, Current_Green = 0, Current_Blue = 0;

    while (1)
    {

        if (Current_Red == Defined_Red && Current_Green == Defined_Green && Current_Blue == Defined_Blue)
        {
            LED_Task_Running = false;
            vTaskSuspend(LED_Handle);
        }

        if (Current_Red < Defined_Red)
        {
            Current_Red++;
            ledcWrite(0, 255 - Current_Red);
        }
        else if (Current_Red > Defined_Red)
        {
            Current_Red--;
            ledcWrite(0, 255 - Current_Red);
        }

        if (Current_Green < Defined_Green)
        {
            Current_Green++;
            ledcWrite(0, 255 - Current_Green);
        }
        else if (Current_Green > Defined_Green)
        {
            Current_Green--;
            ledcWrite(0, 255 - Current_Green);
        }

        if (Current_Blue < Defined_Blue)
        {
            Current_Blue++;
            ledcWrite(0, 255 - Current_Blue);
        }
        else if (Current_Blue > Defined_Blue)
        {
            Current_Blue--;
            ledcWrite(0, 255 - Current_Blue);
        }

        vTaskDelay(pdMS_TO_TICKS(LED_FADE_DELAY));
    }
}

void Shutdown()
{
    Serial.println(F("Shutdown"));

    // animation
    /*uint8_t i;
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
    }*/

    digitalWrite(POWER_PIN, LOW);

    //Set_LED_Color(0x000000);

    Set_State(POWER_OFF_STATE);
}

void Callback()
{
}

void Set_LED_Color(uint32_t Color_To_Set)
{
    while (LED_Task_Running == true)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    Serial.println(Color_To_Set, HEX);
    Defined_Red = (uint8_t)(Color_To_Set >> 16);
    Defined_Green = (uint8_t)(Color_To_Set >> 8);
    Defined_Blue = (uint8_t)(Color_To_Set);
    Serial.println("Decoded color :");
    Serial.println(Defined_Red, HEX);
    Serial.println(Defined_Green, HEX);
    Serial.println(Defined_Blue, HEX);
    LED_Task_Running = true;
    vTaskResume(LED_Handle);
}

void Infrared_Receiver_Task(void *pvParameters)
{
    (void)pvParameters;
    uint8_t Current_Volume;
    uint8_t Delta;
    Infrared_Receiver.enableIRIn();
    while (1)
    {
        if (Infrared_Receiver.decode(&Received_Data))
        {
            Serial.print(F("IR :"));
            serialPrintUint64(Received_Data.value, HEX);
            if (Received_Data.value == Power_Code[0] || Received_Data.value == Power_Code[1])
            {
                Shutdown();
            }
            else if (Received_Data.value == Volume_Down_Code[0] || Received_Data.value == Volume_Down_Code[1])
            {
                //Set_LED_Color(255 - Defined_Volume, Defined_Volume, 0);
                if (Defined_Volume >= (0 + 255 / VOLUME_STEP))
                {
                    Defined_Volume -= 255 / VOLUME_STEP;
                }
            }
            else if (Received_Data.value == Volume_Up_Code[0] || Received_Data.value == Volume_Up_Code[1])
            {
                //Set_LED_Color(255 - Defined_Volume, Defined_Volume, 0);
                if (Defined_Volume <= (255 - 255 / VOLUME_STEP))
                {
                    Defined_Volume += 255 / VOLUME_STEP;
                }
            }
            else if (Received_Data.value == State_Code[0] || Received_Data.value == Volume_Up_Code[1])
            {
                if (State != 0 && State < 3)
                {
                    State++;
                    Set_State(State);
                }
                else if (State >= 3)
                {
                    State = 1;
                    Set_State(State);
                }
            }

            Infrared_Receiver.resume();
        }

        // Set volume
        Current_Volume = 255 - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 255);
        Delta = sqrt(sq(Current_Volume - Defined_Volume));

        while (Delta > 255 / VOLUME_STEP)
        {
            Serial.println(F("Delta:"));
            Serial.println(Delta);

            if (Defined_Volume < VOLUME_STEP)
            {
                digitalWrite(POWER_PIN, LOW);
            }
            else
            {
                digitalWrite(POWER_PIN, HIGH);
            }

            while (Current_Volume >= Defined_Volume)
            {
                digitalWrite(DOWN_PIN, HIGH);
                digitalWrite(UP_PIN, LOW);
                vTaskDelay(pdMS_TO_TICKS(10));
                Current_Volume = 255 - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 255);
            }

            while (Current_Volume <= Defined_Volume)
            {
                digitalWrite(DOWN_PIN, LOW);
                digitalWrite(UP_PIN, HIGH);
                vTaskDelay(pdMS_TO_TICKS(10));
                Current_Volume = 255 - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 255);
            }

            vTaskDelay(pdMS_TO_TICKS(50));
            Delta = sqrt(sq(Current_Volume - Defined_Volume));
        }
        digitalWrite(DOWN_PIN, LOW);
        digitalWrite(UP_PIN, LOW);

        // Handle OTA
        ArduinoOTA.handle();

    }
}

uint8_t Load_Configuration()
{
    Serial.println(F("Load configuration"));
    char Temporary_Char;
    // Device

    Temporary_File = SPIFFS.open("/device", FILE_READ);
    if (Temporary_File)
    {
        Device_Password = "";
        Device_Name = "";
        Temporary_File.seek(0);
        while (Temporary_File.available())
        {
            Temporary_Char = Temporary_File.read();
            if (Temporary_Char == 0x0D)
            {
                Temporary_File.read();
                break;
            }
            else if (Temporary_Char == 0x0A)
            {
                break;
            }
            else
            {
                Device_Name += Temporary_Char;
            }
        }
        while (Temporary_File.available())
        {
            Temporary_Char = Temporary_File.read();
            if (Temporary_Char == 0x0D)
            {
                Temporary_File.read();
                break;
            }
            else if (Temporary_Char == 0x0A)
            {
                break;
            }
            else
            {
                Device_Password += Temporary_Char;
            }
        }
        Serial.println(Device_Name);
        Serial.println(Device_Password);
        Temporary_File.close();
    }

    Temporary_File = SPIFFS.open("/wifi");
    if (Temporary_File)
    {
        SSID = "";
        Password = "";
        Temporary_File.seek(0);
        while (Temporary_File.available())
        {
            Temporary_Char = Temporary_File.read();
            if (Temporary_Char == 0x0D)
            {
                Temporary_File.read();
                break;
            }
            else if (Temporary_Char == 0x0A)
            {
                break;
            }
            else
            {
                SSID += Temporary_Char;
            }
        }
        while (Temporary_File.available())
        {
            Temporary_Char = Temporary_File.read();
            if (Temporary_Char == 0x0D)
            {
                Temporary_File.read();
                break;
            }
            else if (Temporary_Char == 0x0A)
            {
                break;
            }
            else
            {
                Password += Temporary_Char;
            }
        }
        Temporary_File.close();
        Serial.println(SSID);
        Serial.println(Password);
    }

    // Remote
    Temporary_File = SPIFFS.open(REMOTE_FILE, FILE_READ);
    if (Temporary_File)
    {
        Temporary_File.seek(0);
        Power_Code[0] = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(4);
        Power_Code[1] = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(8);
        Volume_Up_Code[0] = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(12);
        Volume_Up_Code[1] = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(16);
        Volume_Down_Code[0] = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(20);
        Volume_Down_Code[1] = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(24);
        State_Code[0] = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(28);
        State_Code[1] = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Serial.println(Power_Code[0], HEX);
        Serial.println(Volume_Up_Code[0], HEX);
        Serial.println(Volume_Down_Code[0], HEX);
        Serial.println(State_Code[0], HEX);
        Serial.println(Power_Code[1], HEX);
        Serial.println(Volume_Up_Code[1], HEX);
        Serial.println(Volume_Down_Code[1], HEX);
        Serial.println(State_Code[1], HEX);
    }
    Temporary_File.close();

    Temporary_File = SPIFFS.open(LED_FILE, FILE_READ);
    if (Temporary_File)
    {
        /*Temporary_File.seek(0);
        Power_Off_Color = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(4);
        Power_On_WiFi_Station_Color = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(8);
        Power_On_WiFi_Access_Point_Color = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Temporary_File.seek(12);
        Power_On_WiFi_Disabled_Color = ((uint32_t)Temporary_File.read() << 24) | ((uint32_t)Temporary_File.read() << 16) | ((uint32_t)Temporary_File.read() << 8) | (uint32_t)Temporary_File.read();
        Serial.println(Power_Off_Color, HEX);*/
        Serial.println(Power_On_WiFi_Station_Color, HEX);
        Serial.println(Power_On_WiFi_Station_Color, HEX);
        Serial.println(Power_On_WiFi_Disabled_Color, HEX);
    }

    return true;
}

uint8_t Save_Configuration(uint8_t const& Parameters_To_Save)
{
    switch (Parameters_To_Save)
    {
    case 0: //all
        break;
    case 1: //device name + password
        Temporary_File = SPIFFS.open(DEVICE_FILE, FILE_WRITE);
        Temporary_File.print(Device_Name);
        Temporary_File.write(0x0D);
        Temporary_File.write(0x0A);
        Temporary_File.print(Device_Password);
        Temporary_File.close();
    case 2: //wifi ssid + password
        Temporary_File = SPIFFS.open(WIFI_FILE, FILE_WRITE);
        Temporary_File.print(SSID);
        Temporary_File.write(0x0D);
        Temporary_File.write(0x0A);
        Temporary_File.print(Password);
        Temporary_File.write(0x0D);
        Temporary_File.write(0x0A);
        Temporary_File.close();
    case 3: // IR code
        Temporary_File = SPIFFS.open(REMOTE_FILE, FILE_WRITE);
        Temporary_File.print(Power_Code[0]);
        Temporary_File.print(Power_Code[1]);
        Temporary_File.print(Volume_Up_Code[0]);
        Temporary_File.print(Volume_Up_Code[1]);
        Temporary_File.print(Volume_Down_Code[0]);
        Temporary_File.print(Volume_Down_Code[1]);
        Temporary_File.print(State_Code[0]);
        Temporary_File.print(State_Code[1]);
        Temporary_File.close();
    case 4: // LED color
        Temporary_File = SPIFFS.open(LED_FILE, FILE_WRITE);
        Temporary_File.print(Power_Off_Color);
        Temporary_File.print(Power_On_WiFi_Station_Color);
        Temporary_File.print(Power_On_WiFi_Access_Point_Color);
        Temporary_File.print(Power_On_WiFi_Disabled_Color);
        Temporary_File.close();
    default:
        break;
    }
    return true;
}