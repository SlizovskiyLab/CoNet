#include <fstream>
#include <string>
#include <unordered_set>
#include "graph.h"
#include "id_maps.h"

// Generates a unique node identifier for Graphviz from node properties
std::string getNodeName(const Node& node) {
    std::string name = "N_" + std::to_string(node.id);
    name += node.isARG ? "_ARG_" : "_MGE_";
    name += std::to_string(static_cast<int>(node.timepoint));
    return name;
}

// Determines the color used to fill the node based on its timepoint
std::string getTimepointColor(const Timepoint& tp) {
    int timeValue = static_cast<int>(tp);
    if (timeValue == -1) return "yellow";   // Donor
    if (timeValue == 0)  return "blue";     // PreFMT
    return "green";                          // PostFMT
}

// Determines whether an edge is a temporal edge between same ARG/MGE across timepoints
bool isTemporalEdge(const Edge& edge) {
    return edge.source.id == edge.target.id &&
           edge.source.isARG == edge.target.isARG &&
           edge.source.timepoint != edge.target.timepoint;
}

// Main export function to write the graph structure into a .dot file
void exportToDot(const Graph& g, const std::string& filename, int max_nodes, int max_edges) {
    std::ofstream file(filename);
    file << "graph G {\n";
    file << "  layout=circo;\n";
    file << "  node [style=filled];\n";

    int node_count = 0;
    std::unordered_set<Node> included_nodes;
    for (const Node& node : g.nodes) {
        if (node_count++ >= max_nodes) break;

        std::string nodeName = getNodeName(node);
        included_nodes.insert(node);
        std::string color = getTimepointColor(node.timepoint);

        std::string shape = node.isARG ? "circle" : "box";

        file << "  " << nodeName
             << " [label=\"\", shape=" << shape
             << ", fixedsize=true, width=0.5, height=0.5, fillcolor=" << color << "]\n";
    }

    int edge_count = 0;
    for (const Edge& edge : g.edges) {
        if (edge_count++ >= max_edges) break;

        if (included_nodes.count(edge.source) == 0 || included_nodes.count(edge.target) == 0) continue;

        std::string sourceName = getNodeName(edge.source);
        std::string targetName = getNodeName(edge.target);

        std::string color;
        std::string style;

        if (isTemporalEdge(edge)) {
            color = "red";
            style = "dashed";
        } else if (edge.isColo) {
            color = "blue";
            style = "solid";
        } else {
            color = "gray";
            style = "solid";
        }

        file << "  " << sourceName
             << " -- " << targetName
             << " [style=" << style << ", color=" << color << "]\n";
    }

    file << "}\n";
}
