// --- D3 Setup ---
const svg = d3.select("#graph");
const g = svg.append("g");
const tooltip = d3.select("#tooltip");

svg.call(d3.zoom().scaleExtent([0.2, 8])
    .on("zoom", (e) => g.attr("transform", e.transform)));

// --- DATA MAPPINGS ---
const TIMEPOINT_CATEGORIES = {
    "donor": "donor",
    "pre": "pre",
    "post1": { category: "post", range: [1, 30] },
    "post2": { category: "post", range: [31, 60] },
    "post3": { category: "post", range: [61, Infinity] }
};

const shapeMap = { circle: d3.symbolCircle, box: d3.symbolSquare, triangle: d3.symbolTriangle, diamond: d3.symbolDiamond, hexagon: d3.symbolCross, octagon: d3.symbolStar, parallelogram: d3.symbolWye, trapezium: d3.symbolWye };

// --- GLOBAL STATE ---
let originalData = {}; 
let currentGraphKey = "json/graph1.json"; 

// --- DATA LOADING & INITIALIZATION ---
function loadAndRenderGraph(fileKey) {
    currentGraphKey = fileKey;
    if (originalData[fileKey]) {
        populateFilters(originalData[fileKey]);
        applyFiltersAndDraw();
    } else {
        d3.json(fileKey).then(data => {
            originalData[fileKey] = data;
            populateFilters(data);
            applyFiltersAndDraw();
        });
    }
}

function populateFilters(data) {
    const mgeGroupSelect = d3.select("#mgeGroupFilter");
    const nodeSource = (currentGraphKey.includes('graph1')) ? data.nodes.filter(n => !n.isARG) : data.nodes;
    const groupKey = (currentGraphKey.includes('graph1')) ? 'group' : 'mgeGroup';
    
    const mgeGroups = [...new Set(nodeSource.map(n => n[groupKey]))].sort();

    mgeGroupSelect.selectAll("option.dynamic-option").remove();
    
    mgeGroups.forEach(group => {
        if (group) {
            mgeGroupSelect.append("option")
                .attr("class", "dynamic-option")
                .attr("value", group)
                .text(group);
        }
    });
}

function resetFilters() {
    d3.select("#diseaseFilter").property("value", "all");
    d3.select("#mgeGroupFilter").property("value", "all");
    d3.select("#timepointFilter").property("value", "all");
    d3.select("#searchBox").property("value", "");
    applyFiltersAndDraw();
}


// --- CORE FILTERING LOGIC ---
function applyFiltersAndDraw() {
    if (!originalData[currentGraphKey]) return;

    let data = JSON.parse(JSON.stringify(originalData[currentGraphKey])); 

    const filters = {
        disease: d3.select("#diseaseFilter").property("value"),
        mgeGroup: d3.select("#mgeGroupFilter").property("value"),
        timepoint: d3.select("#timepointFilter").property("value"),
        searchTerm: d3.select("#searchBox").property("value").trim().toLowerCase(),
        showColo: d3.select("#toggleColo").property("checked"),
        showTemporal: d3.select("#toggleTemporal").property("checked")
    };

    let { nodes, links } = data;
    
    ({ nodes, links } = filterByProperties({ nodes, links }, filters));
    
    if (filters.searchTerm) {
        ({ nodes, links } = filterBySearch({ nodes, links }, filters.searchTerm));
    }
    
    links = links.filter(link => (link.isColo && filters.showColo) || (!link.isColo && filters.showTemporal));
    
    updateVisualization({ nodes, links });
}

function filterByProperties({ nodes, links }, filters) {
    let visibleNodeIds = new Set(nodes.map(n => n.id));

    // Timepoint Filter
    if (filters.timepoint !== 'all') {
        const timepointInfo = TIMEPOINT_CATEGORIES[filters.timepoint];
        const timepointFilteredIds = new Set(
            nodes.filter(node => {
                if (node.timepointCategory === timepointInfo) return true;
                if (timepointInfo.category && node.timepointCategory === timepointInfo.category) {
                    return node.timepoint >= timepointInfo.range[0] && node.timepoint <= timepointInfo.range[1];
                }
                return false;
            }).map(n => n.id)
        );
        visibleNodeIds = new Set([...visibleNodeIds].filter(id => timepointFilteredIds.has(id)));
    }
    
    // MGE Group Filter
    if (filters.mgeGroup !== 'all') {
        const groupKey = currentGraphKey.includes('graph1') ? 'group' : 'mgeGroup';
        const mgeFilteredIds = new Set(nodes.filter(n => n[groupKey] === filters.mgeGroup).map(n => n.id));

        if (currentGraphKey.includes('graph1')) {
            // In graph1, we also need to keep the ARGs connected to these MGEs
            const connectedArgIds = new Set();
            links.forEach(link => {
                if (link.isColo) {
                    const sourceId = typeof link.source === 'object' ? link.source.id : link.source;
                    const targetId = typeof link.target === 'object' ? link.target.id : link.target;
                    if (mgeFilteredIds.has(sourceId)) connectedArgIds.add(targetId);
                    if (mgeFilteredIds.has(targetId)) connectedArgIds.add(sourceId);
                }
            });
            const allVisible = new Set([...mgeFilteredIds, ...connectedArgIds]);
            visibleNodeIds = new Set([...visibleNodeIds].filter(id => allVisible.has(id)));
        } else {
             visibleNodeIds = new Set([...visibleNodeIds].filter(id => mgeFilteredIds.has(id)));
        }
    }
    
    // Disease Filter
    if (filters.disease !== 'all') {
        let diseaseFilteredIds = new Set();
        if (currentGraphKey.includes('graph1')) { // Disease is on links
            links.forEach(link => {
                if (link.isColo && link.diseases && link.diseases.includes(filters.disease)) {
                    diseaseFilteredIds.add(typeof link.source === 'object' ? link.source.id : link.source);
                    diseaseFilteredIds.add(typeof link.target === 'object' ? link.target.id : link.target);
                }
            });
        } else { // Disease is on nodes for graph2
            nodes.forEach(node => {
                if (node.diseases && node.diseases.includes(filters.disease)) {
                    diseaseFilteredIds.add(node.id);
                }
            });
        }
        visibleNodeIds = new Set([...visibleNodeIds].filter(id => diseaseFilteredIds.has(id)));
    }
    
    const finalNodes = nodes.filter(n => visibleNodeIds.has(n.id));
    const finalLinks = links.filter(l => {
        const sourceId = typeof l.source === 'object' ? l.source.id : l.source;
        const targetId = typeof l.target === 'object' ? l.target.id : l.target;
        return visibleNodeIds.has(sourceId) && visibleNodeIds.has(targetId);
    });

    return { nodes: finalNodes, links: finalLinks };
}

function filterBySearch({ nodes, links }, searchTerm) {
    if (!searchTerm) return { nodes, links };
    
    const nameKey = currentGraphKey.includes('graph1') ? 'name' : 'label';
    const seedNodeIds = new Set(
        nodes.filter(n => n[nameKey] && n[nameKey].toLowerCase().includes(searchTerm)).map(n => n.id)
    );

    if (seedNodeIds.size === 0) return { nodes: [], links: [] };

    const neighborIds = new Set();
    links.forEach(link => {
        const sourceId = typeof link.source === 'object' ? link.source.id : link.source;
        const targetId = typeof link.target === 'object' ? link.target.id : link.target;
        if (seedNodeIds.has(sourceId)) neighborIds.add(targetId);
        if (seedNodeIds.has(targetId)) neighborIds.add(sourceId);
    });
    
    const visibleNodeIds = new Set([...seedNodeIds, ...neighborIds]);
    
    const finalNodes = nodes.filter(n => visibleNodeIds.has(n.id));
    const finalLinks = links.filter(l => {
        const sourceId = typeof l.source === 'object' ? l.source.id : l.source;
        const targetId = typeof l.target === 'object' ? l.target.id : l.target;
        return visibleNodeIds.has(sourceId) && visibleNodeIds.has(targetId);
    });
    
    return { nodes: finalNodes, links: finalLinks };
}


// --- D3 RENDERING ---
function updateVisualization(data) {
    g.selectAll("*").remove();
    if (!data.nodes.length) return;

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

    node.append("title").text(d => d.label.replace(/\\n/g, '\n'));

    const label = g.selectAll("text.label")
        .data(simNodes, d => d.id).join("text")
        .attr("class", "label")
        .attr("dy", -12)
        .text(d => d.name || d.label.split('\n')[0]);

    g.selectAll("text.label").style("display", d3.select("#toggleLabels").property("checked") ? "block" : "none");

    const sim = d3.forceSimulation(simNodes)
        .force("link", d3.forceLink(simLinks).id(d => d.id).distance(d => d.isColo ? 70 : 130).strength(0.5))
        .force("charge", d3.forceManyBody().strength(-500))
        .force("center", d3.forceCenter(500, 350))
        .force("collision", d3.forceCollide().radius(30))
        .on("tick", ticked);
    
    function ticked() {
        link.attr("d", d => linkArc(d));
        node.attr("transform", d => `translate(${d.x},${d.y})`);
        label.attr("x", d => d.x).attr("y", d => d.y);
    }
    
    function dragstart(event, d){ if(!event.active) sim.alphaTarget(0.3).restart(); d.fx=d.x; d.fy=d.y; }
    function dragged(event, d){ d.fx=event.x; d.fy=event.y; }
    function dragend(event, d){ if(!event.active) sim.alphaTarget(0); d.fx=null; d.fy=null; }
}

function linkArc(d) {
    const r = Math.hypot(d.target.x - d.source.x, d.target.y - d.source.y);
    return `M${d.source.x},${d.source.y}A${r},${r} 0 0,1 ${d.target.x},${d.target.y}`;
}

// --- EVENT LISTENERS ---
d3.select("#dataset").on("change", function() { loadAndRenderGraph(this.value); });
d3.select("#diseaseFilter").on("change", applyFiltersAndDraw);
d3.select("#mgeGroupFilter").on("change", applyFiltersAndDraw);
d3.select("#timepointFilter").on("change", applyFiltersAndDraw);
d3.select("#searchBtn").on("click", applyFiltersAndDraw);
d3.select("#resetBtn").on("click", resetFilters);
d3.select("#searchBox").on("keydown", event => { if (event.key === 'Enter') { applyFiltersAndDraw(); } });
d3.select("#toggleLabels").on("change", () => g.selectAll("text.label").style("display", d3.select("#toggleLabels").property("checked") ? "block" : "none"));
d3.select("#toggleColo").on("change", applyFiltersAndDraw);
d3.select("#toggleTemporal").on("change", applyFiltersAndDraw);


// --- INITIAL LOAD ---
loadAndRenderGraph(currentGraphKey);
