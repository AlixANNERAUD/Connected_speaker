function Set_Color()
{
    var Power_Off_Color = document.getElementById("Power_Off_Color_Input").value;
    var Power_On_Wireless_Station_Color = document.getElementById("Power_On_Wireless_Station").value;
    var Power_On_Wireless_Access_Point_Color = document.getElementById("Power_On_Wireless_Station").value;
    var Power_On_Wireless_Disabled_Color = document.getElementById("Power_On_Wireless_Disabled").value;

    $.post("get", {
        set_power_off_color: Power_Off_Color,
        set_power_on_wireless_station_color: Power_On_Wireless_Station_Color,
        set_power_on_wireless_access_point_color: Power_On_Wireless_Access_Point_Color,
        set_power_on_wireless_disabled_color: Power_On_Wireless_Disabled_Color
    });
}