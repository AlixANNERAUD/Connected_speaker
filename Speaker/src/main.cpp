#include "Smart_HiFi.hpp"

Smart_HiFi_Class Smart_HiFi;

void setup()
{
    Smart_HiFi.Start();
}

void loop()
{
    vTaskDelete(NULL);
}

