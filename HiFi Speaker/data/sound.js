setInterval(function Refresh() {
    var Request = new XMLHttpRequest();
    Request.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200)
        {
            document.getElementById("Volume_Slider").value = this.value;
        }
    };
    $.post("get", {get_volume});
}, 500);

function Set_Volume()
{
    var Volume = document.getElementById("Volume_Slider").value;
    $.post("get", {
        set_volume: Volume
    });
}

function Increase_Volume()
{
    if (document.getElementById("Volume_Slider").value < 33)
    {
        document.getElementById("Volume_Slider").value++;
    }
    Set_Volume();
}

function Reduce_Volume()
{
    if (document.getElementById("Volume_Slider").value >= 0)
    {
        document.getElementById("Volume_Slider").value--;
    }
    Set_Volume();
}
