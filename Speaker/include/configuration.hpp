#ifndef CONFIGURATION_HPP_INCLUDED
#define CONFIGURATION_HPP_INCLUDED

// Pinout

#define Default_Device_Name "SmartHiFi"
#define Default_Device_Password ""

#define Potentiometer_Pin 33

#define Green_LED_Pin 32
#define Red_LED_Pin 21
#define Blue_LED_Pin 14

#define Up_Pin 16
#define Down_Pin 17

#define Infrared_Receiver_Pin GPIO_NUM_15

#define Power_Pin 13

#define Web_Server_Port 80

#define Loggin_Timeout 300000

#define Maximum_Logged_Clients 5

// Volume

#define Volume_Step 16

#define Maximum_Remotes 5


#define LED_Frequency 60

#define WiFi_Timeout 10000
#define Wakeup_Timeout 3000

#define LED_Fade_Delay 4

#define Led_Registry_Path "/led"
#define Remote_Registry_Path "/remote"
#define WiFi_Registry_Path "/wifi"
#define Device_Registry_Path "/device"

#endif