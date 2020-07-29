function Set_WiFi()
{
    var SSID = document.getElementById("SSID_Input").value;
    var Password = document.getElementById("Password_Input").value;
    
    $.post("get", {
        set_wifi_ssid: SSID,
        set_wifi_password: Password
    });

}