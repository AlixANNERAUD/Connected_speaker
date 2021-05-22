setInterval(function Refresh_Volume() {
    $.post("get", {volume: 0}, function(data, status, xhr) {
        var Volume = parseInt(xhr.responseText, 10);
        document.getElementById("Volume_Slider").value = Volume;
        document.getElementById("Volume_Label").innerHTML = Math.round(document.getElementById("Volume_Slider").value / 256 * 100) + ' %';
    });
}, 500);

function Set_Volume()
{
    var Volume = document.getElementById("Volume_Slider").value;
    $.post("set", {
        volume: Volume
    });
    document.getElementById("Volume_Label").innerHTML = Math.round(document.getElementById("Volume_Slider").value / 256 * 100) + ' %';
}
