function Set_Remote_Button(button_to_set) {

    var Selected_Remote = document.getElementsByName('remote');

    for (var i = 0, length = Selected_Remote.length; i < length; i++) {
        if (Selected_Remote[i].checked) {
            $.post("set", {
                remote: i,
                button: button_to_set,
            });
            break;
        }
    }
}