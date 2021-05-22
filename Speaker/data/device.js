function Set_Device_Name()
{
    var Device_Name = document.getElementById("Device_Name_Input").value;  
    $.post("set", {
        device_name: Device_Name,
    });
}

function Set_Device_Password()
{
    var Device_Passsword = document.getElementById("Device_Password_Input").value;
    $.post("set", {
        device_password: Device_Passsword
    })
}

function Set_State(Button)
{
    if (Button == 'Shutdown_Button')
    {
        $.post("set", {
            state: 'off'
        })
    }
    else if (Button == 'Disconnect_Button')
    {
        $.post("set", {
            state: 'disconnect'
        })
    }

}