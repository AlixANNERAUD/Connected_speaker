#ifndef SMART_HI_FI_HPP_INCLUDED
#define SMART_HI_FI_HPP_INCLUDED

#include "Xila.hpp"
#include "Arduino.h"
#include "HTTPClient.h"

#define Smart_Hi_Fi_Directory_Path Software_Directory_Path "/Hi_Fi"
#define Smart_Hi_Fi_Registry_Path Software_Directory_Path "/Hi_Fi/Registry.xrf"

class Smart_HiFi_Class : public Software_Class
{
private:
    
    static Smart_HiFi_Class* Instance_Pointer;
    
    char Device_Name[64];
    char Device_Password[64];

    char Temporary_String[256];

    uint8_t Volume = 0;
    uint8_t Mute = false;

    uint32_t Next_Refresh_Query = 0;
    uint32_t Next_Login_Query = 0;
    bool Logged = false;

    void Set_Mute();
    void Get_Mute();

    bool Login();

    void Get_Data();

    void Scan();

    void Power_Off();

    void Set_Variable(const void *Variable, uint8_t Type, uint8_t Adress, uint8_t Size);

    void Set_Volume();
    void Get_Volume();

    void Refresh_Interface();

    bool Load_Registry();
    bool Save_Registry();


public:
    Smart_HiFi_Class();
    ~Smart_HiFi_Class();

    static void Main_Task(void*);

    enum Image
    {
        Icon_32 = 44
    };

    static Software_Class* Load();

};

extern Software_Handle_Class Smart_Hi_Fi_Handle;



#endif