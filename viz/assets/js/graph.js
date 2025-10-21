// --- D3 Setup ---
const svg = d3.select("#graph");
const g = svg.append("g");
const tooltip = d3.select("#tooltip");

svg.call(d3.zoom().scaleExtent([0.2, 8])
    .on("zoom", (e) => g.attr("transform", e.transform)));

let originalData = {}; 
let currentGraphKey = "json/graph1.json"; 

const shapeMap = { circle: d3.symbolCircle, box: d3.symbolSquare, triangle: d3.symbolTriangle, diamond: d3.symbolDiamond, hexagon: d3.symbolCross, octagon: d3.symbolStar, parallelogram: d3.symbolWye, trapezium: d3.symbolWye };

// --- DATA LOADING & INITIALIZATION ---
function loadAndRenderGraph(fileKey) {
    currentGraphKey = fileKey;
    resetFilters(false);

    if (originalData[fileKey]) {
        populateFilters(originalData[fileKey]);
        applyFiltersAndDraw();
    } else {
        d3.json(fileKey).then(data => {
            originalData[fileKey] = data;
            populateFilters(data);
            applyFiltersAndDraw();
        }).catch(error => console.error("Error loading JSON:", error));
    }
}

function populateFilters(data) {
    const mgeGroupSelect = d3.select("#mgeGroupFilter");
    const nodeSource = (currentGraphKey.includes('graph1')) ? data.nodes.filter(n => !n.isARG) : data.nodes;
    
    const mgeGroups = [...new Set(nodeSource.map(n => n.mgeGroup).filter(g => g))].sort();

    mgeGroupSelect.selectAll("option.dynamic-option").remove();
    
    mgeGroups.forEach(group => {
        mgeGroupSelect.append("option")
            .attr("class", "dynamic-option")
            .attr("value", group)
            .text(group);
    });
}

function resetFilters(redraw = true) {
    d3.select("#diseaseFilter").property("value", "all");
    d3.select("#mgeGroupFilter").property("value", "all");
    d3.select("#timepointFilter").property("value", "all");
    d3.select("#argSearch").property("value", "");
    d3.select("#mgeSearch").property("value", ""); 
    d3.selectAll(".timepoint-checkbox").property("checked", true);
    if (redraw) {
        applyFiltersAndDraw();
    }
}

// --- CORE FILTERING LOGIC ---
function applyFiltersAndDraw() {
    if (!originalData[currentGraphKey]) return;

    let data = JSON.parse(JSON.stringify(originalData[currentGraphKey])); 

    const filters = {
        disease: d3.select("#diseaseFilter").property("value"),
        mgeGroup: d3.select("#mgeGroupFilter").property("value"),
        timepoints: Array.from(
            d3.selectAll(".timepoint-checkbox").nodes()
        )
        .filter(cb => cb.checked)
        .map(cb => cb.value),
        argSearchTerm: d3.select("#argSearch").property("value").trim().toLowerCase(),
        mgeSearchTerm: d3.select("#mgeSearch").property("value").trim().toLowerCase(),
    };

    let { nodes, links } = data;
    
    // --- FILTERING PIPELINE ---

    let strictlyFilteredNodeIds = getStrictlyFilteredNodeIds(nodes, links, filters);

    let seedNodeIds = getSeedNodeIds(nodes, filters, strictlyFilteredNodeIds);

    let finalVisibleNodeIds;
if (filters.mgeGroup !== 'all' || filters.argSearchTerm || filters.mgeSearchTerm) {
    if (seedNodeIds.size === 0) {
        finalVisibleNodeIds = new Set();
    } else {
        const neighborIds = getNeighborIds(links, seedNodeIds);
        const allowedNeighbors = [...neighborIds].filter(id => strictlyFilteredNodeIds.has(id));
        const allowedSeeds = [...seedNodeIds].filter(id => strictlyFilteredNodeIds.has(id));
        finalVisibleNodeIds = new Set([...allowedSeeds, ...allowedNeighbors]);
    }
    } else {
        finalVisibleNodeIds = strictlyFilteredNodeIds;
    }

    const finalNodes = nodes.filter(n => finalVisibleNodeIds.has(n.id));
    let finalLinks = links.filter(l => {
        const sourceId = typeof l.source === 'object' ? l.source.id : l.source;
        const targetId = typeof l.target === 'object' ? l.target.id : l.target;
        return finalVisibleNodeIds.has(sourceId) && finalVisibleNodeIds.has(targetId);
    });
        
    updateVisualization({ nodes: finalNodes, links: finalLinks });
}

function getStrictlyFilteredNodeIds(nodes, links, filters) {
    let visibleNodeIds = new Set(nodes.map(n => n.id));

    // Timepoint Filter
    if (filters.timepoints && filters.timepoints.length > 0) {
        const allowedIds = new Set(
            nodes
                .filter(node => filters.timepoints.includes(node.timepointCategory))
                .map(n => n.id)
        );
        visibleNodeIds = new Set([...visibleNodeIds].filter(id => allowedIds.has(id)));
    } else {
        visibleNodeIds.clear();
    }
    
    // Disease Filter
    if (filters.disease !== 'all') {
        let diseaseFilteredIds = new Set();
        if (currentGraphKey.includes('graph1')) {
            links.forEach(link => {
                if (link.isColo && link.diseases && link.diseases.includes(filters.disease)) {
                    diseaseFilteredIds.add(typeof link.source === 'object' ? link.source.id : link.source);
                    diseaseFilteredIds.add(typeof link.target === 'object' ? link.target.id : link.target);
                }
            });
        } else {
            nodes.forEach(node => {
                if (node.diseases && node.diseases.includes(filters.disease)) {
                    diseaseFilteredIds.add(node.id);
                }
            });
        }
        visibleNodeIds = new Set([...visibleNodeIds].filter(id => diseaseFilteredIds.has(id)));
    }
    
    return visibleNodeIds;
}

function getSeedNodeIds(nodes, filters, availableNodeIds) {
    let mgeGroupSeedIds = new Set();
    let searchSeedIds = new Set();

    const availableNodes = nodes.filter(n => availableNodeIds.has(n.id));

    if (filters.mgeGroup !== 'all') {
        availableNodes.forEach(n => {
            if (n.mgeGroup === filters.mgeGroup) {
                mgeGroupSeedIds.add(n.id);
            }
        });
    }

    const hasArgSearch = !!filters.argSearchTerm;
    const hasMgeSearch = !!filters.mgeSearchTerm;

    if (hasArgSearch || hasMgeSearch) {
        availableNodes.forEach(n => {
            const label = n.label ? n.label.toLowerCase() : "";
            if (hasArgSearch && n.isARG && label.includes(filters.argSearchTerm)) {
                searchSeedIds.add(n.id);
            }
            if (hasMgeSearch && !n.isARG && label.includes(filters.mgeSearchTerm)) {
                searchSeedIds.add(n.id);
            }
        });
    }

    const isMgeGroupFiltered = filters.mgeGroup !== 'all';
    const isSearchFiltered = hasArgSearch || hasMgeSearch;

    if (isMgeGroupFiltered && isSearchFiltered) {
        return new Set([...mgeGroupSeedIds].filter(id => searchSeedIds.has(id)));
    }
    if (isMgeGroupFiltered) {
        return mgeGroupSeedIds;
    }
    if (isSearchFiltered) {
        return searchSeedIds;
    }
    
    return new Set();
}

function getNeighborIds(links, seedNodeIds) {
    const neighborIds = new Set();
    links.forEach(link => {
        const sourceId = typeof link.source === 'object' ? link.source.id : link.source;
        const targetId = typeof link.target === 'object' ? link.target.id : link.target;
        if (seedNodeIds.has(sourceId)) neighborIds.add(targetId);
        if (seedNodeIds.has(targetId)) neighborIds.add(sourceId);
    });
    return neighborIds;
}


// --- D3 RENDERING ---
function updateVisualization(data) {
    g.selectAll("*").remove();
    if (!data.nodes.length) return;

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
    
    let simNodes = data.nodes.map(d => ({...d}));
    let simLinks = data.links.map(d => ({...d}));

    const link = g.selectAll("path.link")
        .data(simLinks, d => `${d.source.id}-${d.target.id}-${d.type}`).join("path")
        .attr("class", "link")
        .attr("stroke", d => d.color || "#999")
        .attr("marker-end", d => d.isColo ? null : `url(#arrow-${(d.color || "#999").replace("#","")})`)
        .attr("stroke-width", d => Math.max(1, d.penwidth || 1))
        .attr("stroke-dasharray", d => d.isColo ? null : "4 2");

    const node = g.selectAll("path.node")
        .data(simNodes, d => d.id).join("path")
        .attr("class", "node")
        .attr("d", d3.symbol().type(d => shapeMap[d.shape] || d3.symbolCircle).size(300))
        .attr("fill", d => d.color).attr("stroke", "grey").attr("stroke-width", 1.5)
        .call(d3.drag().on("start", dragstart).on("drag", dragged).on("end", dragend));

    node.append("title").text(d => d.label);

    const label = g.selectAll("text.label")
        .data(simNodes, d => d.id).join("text")
        .attr("class", "label")
        .attr("dy", -12)
        .text(d => d.label);

    g.selectAll("text.label").style("display", d3.select("#toggleLabels").property("checked") ? "block" : "none");

    const sim = d3.forceSimulation(simNodes)
        .force("link", d3.forceLink(simLinks).id(d => d.id).distance(d => d.isColo ? 40 : 60))
        .force("charge", d3.forceManyBody().strength(d => -(40 + (d.degree || 0) * 15)))
        .force("center", d3.forceCenter(500, 350))
        .force("collision", d3.forceCollide().radius(20))
        .force("x", d3.forceX(500).strength(0.05)).force("y", d3.forceY(350).strength(0.05))
        .on("tick", ticked);
    
    function ticked() {
        link.attr("d", d => linkArc(d));
        node.attr("transform", d => `translate(${d.x},${d.y})`);
        label.attr("x", d => d.x).attr("y", d => d.y);
    }
    
    function dragstart(event, d){ if(!event.active) sim.alphaTarget(0.3).restart(); d.fx=d.x; d.fy=d.y; }
    function dragged(event, d){ d.fx=event.x; d.fy=event.y; }
    function dragend(event, d){ if(!event.active) sim.alphaTarget(0); d.fx=null; d.fy=null; }

    updateLinkVisibility();
}

function linkArc(d) {
    const r = Math.hypot(d.target.x - d.source.x, d.target.y - d.source.y);
    return `M${d.source.x},${d.source.y}A${r},${r} 0 0,1 ${d.target.x},${d.target.y}`;
}

function updateLinkVisibility() {
    const showColo = d3.select("#toggleColo").property("checked");
    const showTemporal = d3.select("#toggleTemporal").property("checked");

    g.selectAll("path.link")
        .style("display", d => {
            if (d.isColo) {
                return showColo ? "inline" : "none";
            } else {
                return showTemporal ? "inline" : "none";
            }
        });
}

// --- EVENT LISTENERS ---
d3.select("#dataset").on("change", function() { loadAndRenderGraph(this.value); });
d3.select("#diseaseFilter").on("change", applyFiltersAndDraw);
d3.select("#mgeGroupFilter").on("change", applyFiltersAndDraw);
d3.select("#timepointFilter").on("change", applyFiltersAndDraw);
d3.select("#searchBtn").on("click", applyFiltersAndDraw);
d3.select("#resetBtn").on("click", resetFilters);
d3.select("#argSearch").on("keydown", event => { if (event.key === 'Enter') { applyFiltersAndDraw(); } });
d3.select("#mgeSearch").on("keydown", event => { if (event.key === 'Enter') { applyFiltersAndDraw(); } });
d3.select("#toggleLabels").on("change", () => g.selectAll("text.label").style("display", d3.select("#toggleLabels").property("checked") ? "block" : "none"));
d3.select("#toggleColo").on("change", updateLinkVisibility);
d3.select("#toggleTemporal").on("change", updateLinkVisibility);
d3.selectAll(".timepoint-checkbox").on("change", applyFiltersAndDraw);

const legendOverlay = document.getElementById("legendOverlay");
document.getElementById("toggleLegend").addEventListener("change", function() {
	if (this.checked) {
		legendOverlay.classList.add("visible");
	} else {
		legendOverlay.classList.remove("visible");
	}
});

// --- INITIAL LOAD ---
loadAndRenderGraph(currentGraphKey);
