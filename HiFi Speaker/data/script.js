$(document).ready(function() {
    $("#Login").click(function{
        var User = $("#User").val();
        var Password = $("#Password").val();
        $.post("loggin", {
            user: User
            password: Password
        })
    })
});


setInterval(function Refresh() {
    var Request = new XMLHttpRequest();

    Request.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200)
        {
            document.getElementById("Volume_Slider").value = this.responseText;
            document.getElementById("Volume_Value").innerHTML = this.responseText;
        }
    };

    Request.open("GET", "refresh-volume", true);
    Request.send();

}, 500);

function Open_Sidebar() {
    document.getElementById("Sidebar").style.display = "block";
    document.getElementById("myOverlay").style.display = "block";
}

function Close_Sidebar() {
    document.getElementById("Sidebar").style.display = "none";
    document.getElementById("myOverlay").style.display = "none";
}

function openNav(id) {
    document.getElementById("nav01").style.display = "none";
    document.getElementById(id).style.display = "block";
}

var Volume_Slider = document.getElementById("VolumeSlider");
var Volume_Value = document.getElementById("");

Volume_Value.innerHTML = Volume_Slider.value;

Volume_Slider.oninput = function() {
    Volume_Slider.value = this.value;
    Volume_Value.innerHTML = this.value;   
}

$.ajaxSetup({timeout:1000});
function Set_Volume(Value)
{
    $.get("/?value=" + Value + "&");
    {Connection: close};
}
