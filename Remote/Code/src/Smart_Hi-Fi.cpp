#include "Software/Smart_HiFi.hpp"

Software_Handle_Class Smart_Hi_Fi_Handle("Smart Hi-Fi", Smart_HiFi_Class::Icon_32, Smart_HiFi_Class::Load);

Smart_HiFi_Class *Smart_HiFi_Class::Instance_Pointer = NULL;

Smart_HiFi_Class::Smart_HiFi_Class() : Software_Class(Smart_Hi_Fi_Handle)
{
    Xila.Task.Create(Main_Task, "Smart Hi-Fi", Memory_Chunk(4), NULL, &Task_Handle);
}

Smart_HiFi_Class::~Smart_HiFi_Class()
{
    if (Instance_Pointer != this)
    {
        delete Instance_Pointer;
    }
    Instance_Pointer = NULL;
}

Software_Class *Smart_HiFi_Class::Load()
{
    if (Instance_Pointer != NULL)
    {
        delete Instance_Pointer;
    }
    Instance_Pointer = new Smart_HiFi_Class;
    return Instance_Pointer;
}

void Smart_HiFi_Class::Main_Task(void *)
{
    while (1)
    {
        switch (Instance_Pointer->Get_Instruction())
        {
        case Idle:
            if (Xila.Software.Get_State(Smart_Hi_Fi_Handle) == Minimized)
            {
                Xila.Task.Delay(50);
            }

            Xila.Task.Delay(60);
            break;
        case Open:
            Xila.Display.Set_Current_Page(F("Smart_Hi_Fi"));
            Instance_Pointer->Load_Registry();
            Instance_Pointer->Send_Instruction('R', 'e');
            break;
        case Minimize:
            Instance_Pointer->Save_Registry();
            break;
        case Maximize:
            Xila.Display.Set_Current_Page(F("Smart_Hi_Fi"));
            Instance_Pointer->Send_Instruction('R', 'e');
            break;
        case Close:
            Instance_Pointer->Save_Registry();
            delete Instance_Pointer;
            Xila.Task.Delete();
            break;

        case Shutdown:
        case Restart:
        case Hibernate:
        case Instruction('C', 'l'):
            Xila.Software.Close(Smart_Hi_Fi_Handle);
            break;
        case Instruction('M', 'i'):
            Xila.Software.Minimize(Smart_Hi_Fi_Handle);
            Instance_Pointer->Save_Registry();
            break;
        case Instruction('K', 'N'):
            Xila.Dialog.Keyboard(Instance_Pointer->Device_Name, sizeof(Instance_Pointer->Device_Name));
            Instance_Pointer->Next_Login_Query = 0;
            Instance_Pointer->Send_Instruction('R', 'e');
            break;
        case Instruction('K', 'P'):
            Xila.Dialog.Keyboard(Instance_Pointer->Device_Password, sizeof(Instance_Pointer->Device_Password), true);
            Instance_Pointer->Next_Login_Query = 0;
            Instance_Pointer->Send_Instruction('R', 'e');
            break;
        case Instruction('S', 'c'):
            Instance_Pointer->Scan();
            break;
        case Instruction('M', 'u'):
            Instance_Pointer->Mute = !Instance_Pointer->Mute;
            Instance_Pointer->Set_Mute();
            Instance_Pointer->Send_Instruction('R', 'e');
            break;
        case Instruction('S', 'h'):
            Instance_Pointer->Power_Off();
            break;
        case Instruction('S', 'V'):
            Instance_Pointer->Set_Volume();
            Instance_Pointer->Send_Instruction('R', 'e');
            break;
        case Instruction('R', 'e'):
            if (Instance_Pointer->Next_Refresh_Query < Xila.Time.Milliseconds())
            {
                Instance_Pointer->Refresh_Interface();
                Instance_Pointer->Get_Volume();
                Instance_Pointer->Refresh_Interface();
                Instance_Pointer->Get_Mute();
                Instance_Pointer->Next_Refresh_Query = Xila.Time.Milliseconds() + 1000;
            }
            Instance_Pointer->Refresh_Interface();
            break;
        default:
            break;
        }
    }
}

void Smart_HiFi_Class::Refresh_Interface()
{
    Xila.Display.Set_Text(F("NAMEVAL_TXT"), Device_Name);
    Xila.Display.Set_Text(F("PASSVAL_TXT"), Device_Password);
    Xila.Display.Set_Value(F("VOLUME_SLI"), Volume);
    Xila.Display.Set_Value(F("MUTE_BUT"), Mute);
}

void Smart_HiFi_Class::Set_Variable(const void *Variable, uint8_t Type, uint8_t Adress, uint8_t Size)
{
    if (Type == Xila.Display.Variable_Long && Adress == 'V')
    {
        Volume = *(uint8_t *)Variable;
        Send_Instruction('S', 'V');
    }
}

void Smart_HiFi_Class::Power_Off()
{
    if (!Login())
    {
        return;
    }

    Set_Watchdog_Timeout(30000);

    HTTPClient HTTP_Client;

    memset(Temporary_String, '\0', sizeof(Temporary_String));

    strcpy(Temporary_String, "http://");
    strlcat(Temporary_String, Device_Name, sizeof(Temporary_String));
    strlcat(Temporary_String, "/set", sizeof(Temporary_String));


    HTTP_Client.begin(Temporary_String);
    HTTP_Client.addHeader("Content-Type", "application/x-www-form-urlencoded");

    if (HTTP_Client.POST(F("state=off")) > 0)
    {
        Xila.Dialog.Event(F("Failed to connect to device."), Xila.Error);
    }
    Set_Watchdog_Timeout();
    HTTP_Client.end();
}

void Smart_HiFi_Class::Set_Mute()
{
    if (!Login())
    {
        return;
    }

    Set_Watchdog_Timeout(30000);

    HTTPClient HTTP_Client;

    memset(Temporary_String, '\0', sizeof(Temporary_String));

    strcpy(Temporary_String, "http://");
    strlcat(Temporary_String, Device_Name, sizeof(Temporary_String));
    strlcat(Temporary_String, "/set", sizeof(Temporary_String));

    HTTP_Client.begin(Temporary_String);
    HTTP_Client.addHeader("Content-Type", "application/x-www-form-urlencoded");

    memset(Temporary_String, '\0', sizeof(Temporary_String));
    snprintf(Temporary_String, sizeof(Temporary_String), "volume=%i", Volume);

    if (Mute == true)
    {
        if (HTTP_Client.POST("mute=true") < 200)
        {
            Xila.Dialog.Event(F("Failed to connect to device."), Xila.Error);
        }
    }
    else
    {
        if (HTTP_Client.POST("mute=false") < 200)
        {
            Xila.Dialog.Event(F("Failed to connect to device."), Xila.Error);
        }
    }

    Set_Watchdog_Timeout();
    HTTP_Client.end();
}

bool Smart_HiFi_Class::Login()
{
    if (Next_Login_Query >= Xila.Time.Milliseconds() && Logged == true)
    {
        return Logged;
    }

    Set_Watchdog_Timeout(30000);

    if (Xila.WiFi.status() != WL_CONNECTED || strcmp(Device_Name, "") == 0)
    {
        Logged = false;
        return Logged;
    }

    HTTPClient HTTP_Client;
    HTTP_Client.setTimeout(300);

    memset(Temporary_String, '\0', sizeof(Temporary_String));

    strcpy(Temporary_String, "http://");
    strlcat(Temporary_String, Device_Name, sizeof(Temporary_String));
    strlcat(Temporary_String, "/get", sizeof(Temporary_String));

    HTTP_Client.begin(Temporary_String);
    HTTP_Client.addHeader("Content-Type", "application/x-www-form-urlencoded");

    memset(Temporary_String, '\0', sizeof(Temporary_String));
    strcpy(Temporary_String, "password=");
    strlcat(Temporary_String, Device_Password, sizeof(Temporary_String));

    if (HTTP_Client.POST(Temporary_String) > 0)
    {
        if (HTTP_Client.getString() == "true")
        {
            Logged = true;
        }
        else
        {
            Logged = false;
        }
    }
    Set_Watchdog_Timeout();
    Next_Login_Query = Xila.Time.Milliseconds() + 30000;
    HTTP_Client.end();
    return Logged;
}

void Smart_HiFi_Class::Scan()
{
    if (!Login())
    {
        return;
    }

    /*
    for (uint8_t i = 0; i <= 255; i++)
    {
        HTTPClient HTTP_Client;

        if (i )
    }*/
}

void Smart_HiFi_Class::Get_Volume()
{
    if (!Login())
    {
        return;
    }

    Set_Watchdog_Timeout(30000);
    HTTPClient HTTP_Client;
    HTTP_Client.setTimeout(300);

    memset(Temporary_String, '\0', sizeof(Temporary_String));
    strcpy(Temporary_String, "http://");
    strlcat(Temporary_String, Device_Name, sizeof(Temporary_String));
    strlcat(Temporary_String, "/get", sizeof(Temporary_String));
    HTTP_Client.begin(Temporary_String);
    HTTP_Client.addHeader("Content-Type", "application/x-www-form-urlencoded");
    if (HTTP_Client.POST(F("volume=0")) > 0)
    {
        Volume = (uint8_t)HTTP_Client.getString().toInt();
    }
    Set_Watchdog_Timeout();
    HTTP_Client.end();
    Xila.Task.Delay(100);
}

void Smart_HiFi_Class::Get_Mute()
{
    if (!Login())
    {

        return;
    }

    Set_Watchdog_Timeout(30000);
    HTTPClient HTTP_Client;
    HTTP_Client.setTimeout(300);

    memset(Temporary_String, '\0', sizeof(Temporary_String));

    strcpy(Temporary_String, "http://");
    strlcat(Temporary_String, Device_Name, sizeof(Temporary_String));
    strlcat(Temporary_String, "/get", sizeof(Temporary_String));

    HTTP_Client.begin(Temporary_String);
    HTTP_Client.addHeader("Content-Type", "application/x-www-form-urlencoded");

    if (HTTP_Client.POST(F("mute=0")) > 0)
    {
        if (HTTP_Client.getString() == "true")
        {
            Mute = true;
        }
        else
        {
            Mute = false;
        }
    }
    Set_Watchdog_Timeout();
    HTTP_Client.end();
    Xila.Task.Delay(100);
}

void Smart_HiFi_Class::Set_Volume()
{
    if (!Login())
    {
        return;
    }

    Set_Watchdog_Timeout(30000);

    HTTPClient HTTP_Client;

    memset(Temporary_String, '\0', sizeof(Temporary_String));

    strcpy(Temporary_String, "http://");
    strlcat(Temporary_String, Device_Name, sizeof(Temporary_String));
    strlcat(Temporary_String, "/set", sizeof(Temporary_String));

    HTTP_Client.begin(Temporary_String);
    HTTP_Client.addHeader("Content-Type", "application/x-www-form-urlencoded");

    memset(Temporary_String, '\0', sizeof(Temporary_String));
    snprintf(Temporary_String, sizeof(Temporary_String), "volume=%i", Volume);
    if (HTTP_Client.POST(Temporary_String) < 200)
    {
        Xila.Dialog.Event(F("Failed to connect to device."), Xila.Error);
    }
    Set_Watchdog_Timeout();
    HTTP_Client.end();
}

bool Smart_HiFi_Class::Load_Registry()
{
    File Temporary_File = Xila.Drive.Open(Smart_Hi_Fi_Registry_Path);
    DynamicJsonDocument Smart_Hi_Fi_Registry(512);
    memset(Device_Name, '\0', sizeof(Device_Name));
    memset(Device_Password, '\0', sizeof(Device_Password));
    if (deserializeJson(Smart_Hi_Fi_Registry, Temporary_File) != DeserializationError::Ok)
    {
        Temporary_File.close();
        return false;
    }
    Temporary_File.close();
    if (strcmp("Smart Hi-Fi", Smart_Hi_Fi_Registry["Registry"] | "") != 0)
    {
        return false;
    }
    strlcpy(Device_Name, Smart_Hi_Fi_Registry["Device Name"] | "", sizeof(Device_Name));
    strlcpy(Device_Password, Smart_Hi_Fi_Registry["Device Password"] | "", sizeof(Device_Password));
    return true;
}

bool Smart_HiFi_Class::Save_Registry()
{
    if (!Xila.Drive.Exists(Smart_Hi_Fi_Directory_Path))
    {
        Xila.Drive.Make_Directory(Smart_Hi_Fi_Directory_Path);
    }

    File Temporary_File = Xila.Drive.Open(Smart_Hi_Fi_Registry_Path, FILE_WRITE);
    DynamicJsonDocument Smart_Hi_Fi_Registry(512);

    Smart_Hi_Fi_Registry["Registry"] = "Smart Hi-Fi";

    Smart_Hi_Fi_Registry["Device Name"] = Device_Name;
    Smart_Hi_Fi_Registry["Device Password"] = Device_Password;

    if (serializeJson(Smart_Hi_Fi_Registry, Temporary_File) == 0)
    {
        Temporary_File.close();
        return false;
    }
    Temporary_File.close();
    return true;
}