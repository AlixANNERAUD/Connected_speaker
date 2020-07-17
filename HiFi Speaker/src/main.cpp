#include <Arduino.h>
#include <IRremote.h>

#define IR_RECEIVER_THRESOLD 40

#define MUTE_CODE 0xE0E0F00F
#define VOLUME_DOWN_CODE 0xE0E0D02F
#define VOLUME_UP_CODE 0xE0E0E01F
#define A_CODE 0xE0E036C9

#define RECEIVER_PIN 15

IRrecv IR_Receiver(RECEIVER_PIN);

decode_results Received_Data;

void Shutdown()
{
}

void setup()
{

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, HIGH);
  esp_sleep_enable_touchpad_wakeup();
  esp_sleep_wakeup_cause_t Wakeup_Reason = esp_sleep_get_wakeup_cause();
  if (Wakeup_Reason != ESP_SLEEP_WAKEUP_EXT0)
  {
    esp_deep_sleep_start();
  }
  Serial.begin(9600);
  IR_Receiver.enableIRIn(); // Start the receiver
  if (!IR_Receiver.decode(&Received_Data) || Received_Data.value != MUTE_CODE)
  {
    esp_deep_sleep_start();
  }
}

void loop()
{
  if (IR_Receiver.decode(&Received_Data))
  {
    if (Received_Data.value == MUTE_CODE)
    {
      Serial.println(F("Shutdown"));
      esp_deep_sleep_start();
    }
    else if (Received_Data.value == VOLUME_DOWN_CODE)
    {
      Serial.println(F("Volume Down"));
    }
    else if (Received_Data.value == VOLUME_UP_CODE)
    {
      Serial.println(F("Volume Up"));
    }
    else if (Received_Data.value == A_CODE)
    {
      Serial.println(F("Disable wifi"));
    }
    else
    {
      Serial.println(F("Unknow code"));
    }
    IR_Receiver.resume();
  }
}