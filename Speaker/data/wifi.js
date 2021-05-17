function Set_WiFi()
{
    var SSID = document.getElementById("SSID_Input").value;
    var Password = document.getElementById("Password_Input").value;   
    $.post("set", {
        wifi_ssid: SSID,
        wifi_password: Password
    });

}