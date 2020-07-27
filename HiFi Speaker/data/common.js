

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