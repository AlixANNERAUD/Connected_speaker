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

function Reduce_Volume()
{
    var Request = new XMLHttpRequest();
}
