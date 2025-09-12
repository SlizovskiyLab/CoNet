// Javascript code for D3.js graph visualization 
const svg = d3.select("#graph");
const g = svg.append("g");

svg.call(d3.zoom().scaleExtent([0.2, 5])
  .on("zoom", (e) => g.attr("transform", e.transform)));

const shapeMap = { circle: d3.symbolCircle, box: d3.symbolSquare, triangle: d3.symbolTriangle, diamond: d3.symbolDiamond, hexagon: d3.symbolCross, octagon: d3.symbolStar, parallelogram: d3.symbolWye, trapezium: d3.symbolWye };
const arcScale = { licensing: 1.5, suit: 0.1 };

function linkArc(d, scale = 1) {
  const dx = d.target.x - d.source.x;
  const dy = d.target.y - d.source.y;
  const baseR = Math.hypot(dx, dy) * scale;
  const step = 20;
  const offset = (d.linknum - (d.linkcount - 1) / 2) * step;
  const r = baseR + offset;
  return `M${d.source.x},${d.source.y} A${r},${r} 0 0,1 ${d.target.x},${d.target.y}`;
}

let originalData;

function loadAndRenderGraph(file) {
  d3.json(file).then(data => {
    originalData = data;
    applyFiltersAndDraw();
  });
}

function applyFiltersAndDraw() {
  if (!originalData) return;

  const showColo = d3.select("#toggleColo").property("checked");
  const showTemporal = d3.select("#toggleTemporal").property("checked");

  const filteredLinks = originalData.links.filter(link => {
      if (link.isColo === true) return showColo;
      if (link.isColo === false) return showTemporal;
      return true;
  });
  
  const filteredData = { nodes: originalData.nodes, links: filteredLinks };

  updateVisualization(filteredData);
}

function updateVisualization(data) {
  g.selectAll("*").remove();

  const linkGroups = d3.groups(data.links, d => {
    const s = typeof d.source === "object" ? d.source.id : d.source;
    const t = typeof d.target === "object" ? d.target.id : d.target;
    return s + "-" + t;
  });
  linkGroups.forEach(([key, links]) => {
    links.forEach((l, i) => { l.linknum = i; l.linkcount = links.length; });
  });

  svg.select("defs").remove(); 
  const defs = svg.append("defs");
  const colors = [...new Set(data.links.map(d => d.color || "#999"))];
  
  colors.forEach(c => {
    defs.append("marker")
      .attr("id", `arrow-${c.replace("#","")}`)
      .attr("viewBox", "0 -5 10 10").attr("refX", 12).attr("refY", 0)
      .attr("markerWidth", 3).attr("markerHeight", 3).attr("orient", "auto")
      .attr("markerUnits", "strokeWidth").append("path")
      .attr("d", "M0,-5L10,0L0,5").attr("fill", c);
  });

  const link = g.selectAll("path.link")
    .data(data.links).join("path")
    .attr("class", "link")
    .attr("stroke", d => d.color || "#999")
    .attr("marker-end", d => d.isColo ? null : `url(#arrow-${(d.color || "#999").replace("#","")})`)
    .attr("stroke-width", d => Math.max(1, d.penwidth || 1))
    .attr("stroke-dasharray", d => d.isColo ? null : "4 2");

  const node = g.selectAll(".node")
    .data(data.nodes).join("path")
    .attr("class", "node")
    .attr("d", d3.symbol().type(d => shapeMap[d.shape] || d3.symbolCircle).size(300))
    .attr("fill", d => d.color).attr("stroke", "grey").attr("stroke-width", 1.5)
    .call(d3.drag().on("start", dragstart).on("drag", dragged).on("end", dragend));

  node.append("title").text(d => `${d.id}`);

  const label = g.selectAll("text.label")
    .data(data.nodes).join("text")
    .attr("class", "label").attr("dy", -12).text(d => d.label);

  g.selectAll("text.label").style("display", d3.select("#toggleLabels").property("checked") ? "block" : "none");

  const sim = d3.forceSimulation(data.nodes)
    .force("link", d3.forceLink(data.links).id(d => d.id).distance(d => d.isColo ? 40 : 50))
    .force("charge", d3.forceManyBody().strength(d => -(40 + (d.degree || 0) * 15)))
    .force("center", d3.forceCenter(500, 350))
    .force("collision", d3.forceCollide().radius(20))
    .force("x", d3.forceX(500).strength(0.05)).force("y", d3.forceY(350).strength(0.05))
    .on("tick", () => {
      link.attr("d", d => linkArc(d, arcScale[d.type] || 1));
      node.attr("transform", d => `translate(${d.x},${d.y})`);
      label.attr("x", d => d.x).attr("y", d => d.y);
    });

  function dragstart(e, d){ if(!e.active) sim.alphaTarget(0.3).restart(); d.fx=d.x; d.fy=d.y; }
  function dragged(e, d){ d.fx=e.x; d.fy=e.y; }
  function dragend(e, d){ if(!e.active) sim.alphaTarget(0); d.fx=null; d.fy=null; }
}

// default load
loadAndRenderGraph("json/melanoma.json");

d3.select("#dataset").on("change", function() {
  loadAndRenderGraph(this.value);
});

d3.select("#toggleLabels").on("change", function() {
  g.selectAll("text.label").style("display", this.checked ? "block" : "none");
});

d3.select("#toggleColo").on("change", applyFiltersAndDraw);
d3.select("#toggleTemporal").on("change", applyFiltersAndDraw);

document.getElementById("toggleLabels").addEventListener("change", function(e) {
  if (e.target.checked) {
    d3.selectAll("text.label").style("display", "block");
  } else {
    d3.selectAll("text.label").style("display", "none");
  }
})


// document.getElementById("collapseArrow").addEventListener("click", function() {
//   const menu = document.getElementById("menu");
//   if (menu.classList.contains("collapsed")) {
//     menu.classList.remove("collapsed");
//     this.textContent = "▼"; // down arrow when expanded
//   } else {
//     menu.classList.add("collapsed");
//     this.textContent = "▲"; // up arrow when collapsed
//   }
// });


// const tooltip = d3.select("#tooltip");

// const link = g.selectAll("path.link")
//   .data(data.links)
//   .join("path")
//   .attr("class", "link")
//   .attr("stroke", d => d.color || "#999")
//   .attr("marker-end", d => d.isColo ? null : `url(#arrow-${(d.color || "#999").replace("#","")})`)
//   .attr("stroke-width", d => Math.max(1, d.penwidth || 1))
//   .attr("stroke-dasharray", d => d.isColo ? null : "4 2")
//   .on("mouseover", (event, d) => {
//     if (d.type === "colocalization") {
//       tooltip.style("display", "block")
//              .html(`Individual count: <b>${d.individualCount}</b>`);
//     }
//   })
//   .on("mousemove", (event) => {
//     tooltip.style("left", (event.pageX + 10) + "px")
//            .style("top", (event.pageY + 10) + "px");
//   })
//   .on("mouseout", () => {
//     tooltip.style("display", "none");
//   });