setInterval(function Refresh_Volume() {

    $.post("get", {get_volume: 0}, function(data, status, xhr) {
        document.getElementById("Volume_Slider").value = parseInt(xhr.responseText, 10);
    }
    );

}, 500);

function Set_Volume()
{
    var Volume = document.getElementById("Volume_Slider").value;
    $.post("get", {
        set_volume: Volume
    });
}
