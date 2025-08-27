document.getElementById("toggleLabels").addEventListener("change", function(e) {
  if (e.target.checked) {
    d3.selectAll("text.label").style("display", "block");
  } else {
    d3.selectAll("text.label").style("display", "none");
  }
})

document.getElementById("collapseArrow").addEventListener("click", function() {
  const menu = document.getElementById("menu");
  if (menu.classList.contains("collapsed")) {
    menu.classList.remove("collapsed");
    this.textContent = "▼"; // down arrow when expanded
  } else {
    menu.classList.add("collapsed");
    this.textContent = "▲"; // up arrow when collapsed
  }
});


