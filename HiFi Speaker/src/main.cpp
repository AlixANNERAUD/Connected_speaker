#include <Arduino.h>

void setup()
{
    pinMode(27, OUTPUT);
}

void loop()
{
    digitalWrite(27, HIGH);
    delay(5000);
    digitalWrite(27, LOW);
    delay(5000);
}