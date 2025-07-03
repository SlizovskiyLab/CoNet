#include <fstream>
#include <string>
#include <unordered_set>
#include "graph.h"
#include "id_maps.h"

std::string getNodeName(const Node& node) {
    std::string name = "N_" + std::to_string(node.id);
    name += node.isARG ? "_ARG_" : "_MGE_";
    name += std::to_string(static_cast<int>(node.timepoint));
    return name;
}

std::string getNodeLabel(const Node& node) {
    std::string label = node.isARG ? getARGName(node.id) : getMGEName(node.id);
    label += "\\n" + toString(node.timepoint);  // \n becomes actual newline in Graphviz
    return label;
}

std::string getTimepointColor(const Timepoint& tp) {
    int timeValue = static_cast<int>(tp);
    if (timeValue == 1000) return "yellow";  // Donor
    if (timeValue == 0)  return "blue";      // PreFMT
    return "green";                          // PostFMT
}

bool isTemporalEdge(const Edge& edge) {
    return edge.source.id == edge.target.id &&
           edge.source.isARG == edge.target.isARG &&
           edge.source.timepoint != edge.target.timepoint;
}

void exportToDot(const Graph& g, const std::string& filename) {
    std::ofstream file(filename);
    file << "digraph G {\n";
    file << "  layout=sfdp;\n";
    file << "  node [style=filled];\n";

    // Get all nodes referenced in edges
    std::unordered_set<Node> active_nodes;
    for (const Edge& edge : g.edges) {
        active_nodes.insert(edge.source);
        active_nodes.insert(edge.target);
    }

    for (const Node& node : active_nodes) {
        std::string nodeName = getNodeName(node);
        std::string label = getNodeLabel(node);
        std::string color = getTimepointColor(node.timepoint);
        std::string shape = node.isARG ? "circle" : "box";

        file << "  " << nodeName
             << " [label=\"" << label << "\", shape=" << shape
             << ", fixedsize=true, width=0.5, height=0.5, fillcolor=" << color << "]\n";
    }

    for (const Edge& edge : g.edges) {
        std::string sourceName = getNodeName(edge.source);
        std::string targetName = getNodeName(edge.target);

        std::string color;
        std::string style;
        std::string extraAttributes;

        if (isTemporalEdge(edge)) {
            color = "red";
            style = "dashed";
        } else if (edge.isColo) {
            color = "blue";
            style = "solid";
            extraAttributes = "dir=both";
        } else {
            color = "gray";
            style = "solid";
        }

        file << "  " << sourceName << " -> " << targetName
             << " [style=" << style
             << ", color=" << color
             << ", arrowsize=0.3";

        if (!extraAttributes.empty()) {
            file << ", " << extraAttributes;
        }

        file << "]\n";
    }

    file << "}\n";
}
