function w3_open() {
    document.getElementById("Sidebar").style.display = "block";
    document.getElementById("myOverlay").style.display = "block";
}

function w3_close() {
    document.getElementById("Sidebar").style.display = "none";
    document.getElementById("myOverlay").style.display = "none";
}

openNav("nav01");
function openNav(id) {
    document.getElementById("nav01").style.display = "none";
    document.getElementById(id).style.display = "block";
}