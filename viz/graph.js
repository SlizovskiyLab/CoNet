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


const tooltip = d3.select("#tooltip");

const link = g.selectAll("path.link")
  .data(data.links)
  .join("path")
  .attr("class", "link")
  .attr("stroke", d => d.color || "#999")
  .attr("marker-end", d => d.isColo ? null : `url(#arrow-${(d.color || "#999").replace("#","")})`)
  .attr("stroke-width", d => Math.max(1, d.penwidth || 1))
  .attr("stroke-dasharray", d => d.isColo ? null : "4 2")
  .on("mouseover", (event, d) => {
    if (d.type === "colocalization") {
      tooltip.style("display", "block")
             .html(`Individual count: <b>${d.individualCount}</b>`);
    }
  })
  .on("mousemove", (event) => {
    tooltip.style("left", (event.pageX + 10) + "px")
           .style("top", (event.pageY + 10) + "px");
  })
  .on("mouseout", () => {
    tooltip.style("display", "none");
  });



