function Set_Remote_Button(button_to_set)
{
    $.post("set", {
        code: button_to_set,
    });

}