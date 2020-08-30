document.onload = function () {
    $.post("get", { device_name: 0 }, function (data, status, xhr) {
        document.getElementById("Device_Name").value = xhr.responseText;
    });
    $.post("get", { state: 0 }, function (data, status, xhr) {
        document.getElementById("State_Button").style.color = xhr.responseText;
    });
}

function Open_State_Menu()
{
    
}

function Open_Sidebar() {
    document.getElementById("Sidebar").style.display = "block";
    document.getElementById("Overlay").style.display = "block";
}

function Close_Sidebar() {
    document.getElementById("Sidebar").style.display = "none";
    document.getElementById("Overlay").style.display = "none";
}

function openNav(id) {
    document.getElementById("nav01").style.display = "none";
    document.getElementById(id).style.display = "block";
}