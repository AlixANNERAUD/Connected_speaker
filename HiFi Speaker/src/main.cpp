#include <main.hpp>

void setup()
{
    Serial.begin(9600);

    // Setup Potentiometer
    pinMode(POTENTIOMETER_PIN, INPUT);

    // Setup LED

    ledcAttachPin(RED_LED_PIN, 0);
    ledcAttachPin(GREEN_LED_PIN, 1);
    ledcAttachPin(BLUE_LED_PIN, 2);
    ledcSetup(0, 4000, 8);
    ledcSetup(1, 4000, 8);
    ledcSetup(2, 4000, 8);
    Set_LED_Color(0, 0, 0);

    // Setup Power
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, LOW);

    // Task
    xTaskCreatePinnedToCore(Check_Infrared_Receiver, "IR Receiver", 2 * 1024, NULL, 2, &Check_Infrared_Receiver_Handle, 1);
}

void loop()
{
    vTaskDelete(NULL);
}

void Start()
{
    State = 1;
    digitalWrite(POWER_PIN, HIGH); // turn the amplifier power supply on
    uint8_t i;
    for (i = 0; i <= 255; i++)
    {
        Set_LED_Color(i, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 0; i <= 255; i++)
    {
        Set_LED_Color(255 - i, i, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 0; i <= 255; i++)
    {
        Set_LED_Color(0, 255 - i, i);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
 void Shutdown()
{
    State = 0;
    // animation
    uint8_t i;
    for (i = 0; i <= 255; i++)
    {
        Set_LED_Color(0, 0, i);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 0; i <= 255; i++)
    {
        Set_LED_Color(0, i, 255 - i);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 0; i <= 255; i++)
    {
        Set_LED_Color(i, 255 - i, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    for (i = 255; i >= 0; i--)
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
                    Set_Volume(8);
                    break;
                case VOLUME_DOWN_CODE:
                    Set_LED_Color(255 - Defined_Volume, Defined_Volume, 0);
                    Set_Volume(-8);
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
        vTaskDelay(pdMS_TO_TICKS(20));
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

    Current_Volume = map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 255);
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
        vTaskDelay(pdMS_TO_TICKS(10));
        Current_Volume = map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 255);
    }
}