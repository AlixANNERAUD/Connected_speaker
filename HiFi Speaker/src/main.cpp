#include "main.hpp"

void setup()
{
    Serial.begin(115200);
    Infrared_Receiver.enableIRIn();

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

    WiFi.begin();

    uint32_t i = millis();
    while (WiFi.status() != WL_CONNECTE)
    {
        Serial.print(".");
        delay(100);
        if (WIFI_TIMEOUT < millis() - i)
        {
            State = POWER_ON_WIFI_ACCESS_POINT;
            WiFi.softAP("ESP32", "1234");
        }
    

    Web_Server.on("/", HTTP_GET, [](AsyncWebServerRequest *Request) {
        if (Logged)
        {
            Request->redirect("/volume");
        }
        else
        {
            Request->redirect("/loggin");
        }
    });

    Web_Server.on("/login", HTTP_GET, [](AsyncWebServerRequest * Request))
    {
        if (Logged)
        {
            Request->redirect("/volume");
        }
        else
        {
            if(Request->hasParam("user", true) && Request->hasParam("password", true))
            {
                String User = Request->getParam("user");
                String Password = Request->getParam("password");
                
            }
            Request->send(SPIFFS, "/login.html", "text/html");
        }
        Request->send(204);
    });

    Web_Server.on("/refresh-volume", HTTP_GET, [](AsyncWebServerRequest) *Request)
    {
        if (Logged)
        {
            Request->send(200, "text/plain", Current_Volume);
        }
    }

    Web_Server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/w3.css", "text/css");
    });

    Web_Server.begin();


}

void Start()
{
    Serial.println(F("Start"));
    State = 1;

    Defined_Volume = 15 - map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 15);

    digitalWrite(POWER_PIN, HIGH); // turn the amplifier power supply on
    uint8_t i;
    for (i = 0; i < 255; i++)
    {
        Set_LED_Color(i, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 0; i < 255; i++)
    {
        Set_LED_Color(255 - i, i, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 0; i < 255; i++)
    {
        Set_LED_Color(0, 255 - i, i);
        vTaskDelay(pdMS_TO_TICKS(5));
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
            Serial.println(Received_Data.value, HEX);
            Infrared_Receiver.resume();
            if (State == POWER_OFF_STATE)
            {
                if (Received_Data.value == MUTE_CODE)
                {
                    Start();
                }
            }
            else
            {
                switch (Received_Data.value)
                {
                case MUTE_CODE:
                    Shutdown();
                    break;
                case VOLUME_UP_CODE:
                    Set_LED_Color(255 - Defined_Volume, Defined_Volume, 0);
                    Set_Volume(1);
                    break;
                case VOLUME_DOWN_CODE:
                    Set_LED_Color(255 - Defined_Volume, Defined_Volume, 0);
                    Set_Volume(-1);
                    break;
                case A_CODE:
                    if (State != 0 && State < 3)
                    {
                        State++;
                    }
                    else if (State >= 3)
                    {
                        State = 1;
                    }
                    break;
                default:
                    break;
                }
            }

            Infrared_Receiver.resume();
        }
        switch (State)
        {
        case POWER_ON_WIFI_STATION_STATE:
            Set_LED_Color(0, 0, 255);
            break;
        case POWER_ON_WIFI_ACCESS_POINT_STATE:
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