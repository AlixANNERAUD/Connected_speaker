function Login()
{
    var Password = $("#password_input").val();
    $.post("get", {
        password: Password
    }, function (data, status, xhr) {
        if (xhr.responseText == 'true')
        {
            document.location.href="\\sound";

        }
        else if (xhr.responseText == 'failed')
        {
            document.getElementById('Failed').style.display='block';
        }
        else
        {
            document.getElementById('Wrong-Credentials').style.display='block';
        }
    });
}