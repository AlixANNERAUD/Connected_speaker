var Volume_Slider = document.getElementById("VolumeSlider");
var Volume_Value = document.getElementById("");

Volume_Value.innerHTML = Volume_Slider.value;

Volume_Slider.oninput = function() {
    Volume_Slider.value = this.value;
    Volume_Value.innerHTML = this.value;   
}

$.ajaxSetup({timeout:1000});

function Login()
{
    var Password = $("#password_input").val();
    $.post("get", {
        password: Password
    }, function (data, status, xhr) {
        if (xhr.responseText = 'true')
        {
            document.getElementById('Modal').style.display='block';
        }
        else
        {
            document.location.href="\sound.html";
        }
        document.getElementById("Volume_Slider").value = parseInt(xhr.responseText, 10);
    });
}

function Set_Volume(Value)
{
    $.get("/?value=" + Value + "&");
    {Connection: close};
}
