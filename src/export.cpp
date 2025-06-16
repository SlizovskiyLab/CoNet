#include <fstream>  // For file output
#include <string>   // For string manipulation
#include "graph.h"   
#include "id_maps.h" 

// Generates a Graphviz node name string for internal use
std::string getNodeName(const Node& node) {
    std::string name = "N_" + std::to_string(node.id);
    if (node.isARG) {
        name += "_ARG_";
    } else {
        name += "_MGE_";
    }
    name += std::to_string(static_cast<int>(node.timepoint));
    return name;
}

// Returns a human-readable label for a given node based on its ID and type
std::string getNodeLabel(const Node& node) {
    std::string label;
    if (node.isARG) {
        if (argIdMap.find(node.id) != argIdMap.end()) {
            label = argIdMap.at(node.id);
        } else {
            label = std::to_string(node.id);
        }
    } else {
        if (mgeIdMap.find(node.id) != mgeIdMap.end()) {
            label = mgeIdMap.at(node.id);
        } else {
            label = std::to_string(node.id);
        }
    }
    label += " (" + std::to_string(static_cast<int>(node.timepoint)) + ")";
    return label;
}

// Returns a color string for a node based on its timepoint
std::string getTimepointColor(const Timepoint& tp) {
    int timeValue = static_cast<int>(tp);
    if (timeValue == -1) {
        return "yellow"; // Donor
    } else if (timeValue == 0) {
        return "blue"; // PreFMT
    } else {
        return "green"; // PostFMT
    }
}

// Checks if an edge is temporal (same ARG or MGE)
bool isTemporalEdge(const Edge& edge) {
    return edge.source.id == edge.target.id && edge.source.isARG == edge.target.isARG;
}

// Writes the graph to a .dot file for Graphviz
void exportToDot(const Graph& g, const std::string& filename, int max_nodes = 200, int max_edges = 500) {
    std::ofstream file(filename);
    file << "graph G {\n";        // undirected
    file << "  layout=circo;\n";  // circular layout

    int node_count = 0;
    for (const Node& node : g.nodes) {
        if (node_count++ >= max_nodes) break;  // limit nodes

        std::string nodeName = getNodeName(node);
        std::string nodeLabel = getNodeLabel(node);
        std::string color = getTimepointColor(node.timepoint);

        if (node.isARG) {
            // Draw ARGs as true circles
            file << "  " << nodeName
                 << " [label=\"" << nodeLabel << "\", shape=circle, fixedsize=true, width=0.5, fillcolor=" << color << "]\n";
        } else {
            // Draw MGEs as true squares
            file << "  " << nodeName
                 << " [label=\"" << nodeLabel << "\", shape=square, fixedsize=true, width=0.5, fillcolor=" << color << "]\n";
        }
    }

    int edge_count = 0;
    for (const Edge& edge : g.edges) {
        if (edge_count++ >= max_edges) break;  // limit edges

        std::string sourceName = getNodeName(edge.source);
        std::string targetName = getNodeName(edge.target);
        std::string style;

        // edge is temporal or colocalization
        if (isTemporalEdge(edge)) {
            style = "dotted";
        } else {
            style = "bold";
        }

        // Determine color
        std::string color;
        if (edge.isColo) {
            color = "black";
        } else {
            color = "gray";
        }

        // edge definition in DOT format without label
        file << "  " << sourceName
             << " -- " << targetName
             << " [style=" << style
             << ", color=" << color
             << "]\n";
    }

    file << "}\n"; // Close DOT graph
}
