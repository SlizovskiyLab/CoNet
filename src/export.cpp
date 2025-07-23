#include <fstream>
#include <string>
#include <unordered_set>
#include <set>
#include <utility>
#include <algorithm>
#include "graph.h"
#include "id_maps.h"
#include "export.h"

std::string getNodeName(const Node& node) {
    std::string name = "N_" + std::to_string(node.id);
    name += node.isARG ? "_ARG_" : "_MGE_";
    name += std::to_string(static_cast<int>(node.timepoint));
    return name;
}

std::string getNodeLabel(const Node& node) {
    std::string label = node.isARG ? getARGName(node.id) : getMGEName(node.id);
    label += "\\n" + toString(node.timepoint);
    return label;
}

std::string getTimepointColor(const Timepoint& tp) {
    int timeValue = static_cast<int>(tp);
    if (timeValue == 1000) return "yellow";
    if (timeValue == 0)  return "deepskyblue";
    return "green";
}

bool isTemporalEdge(const Edge& edge) {
    return edge.source.id == edge.target.id &&
           edge.source.isARG == edge.target.isARG &&
           edge.source.timepoint != edge.target.timepoint;
}

std::string getMGEGroupShape(const std::string& groupName) {
    if (groupName == "plasmid" || groupName == "Colicin_plasmid" || groupName == "Inc_plasmid") {
        return "diamond";
    }
    if (groupName == "prophage") {
        return "hexagon";
    }
    if (groupName == "virus") {
        return "triangle";
    }
    if (groupName == "ICE" || groupName == "ICEberg") {
        return "octagon";
    }
    if (groupName == "replicon") {
        return "parallelogram";
    }
    if (groupName == "likely IS/TE") {
        return "trapezium";
    }
    return "box"; // Default for UNCLASSIFIED or others
}

void exportToDot(const Graph& g, const std::string& filename, bool showLabels) {
    std::ofstream file(filename);
    file << "digraph G {\n";
    file << "  layout=sfdp;\n";
    file << "  node [style=filled];\n";

    std::unordered_set<Node> active_nodes;
    for (const Edge& edge : g.edges) {
        active_nodes.insert(edge.source);
        active_nodes.insert(edge.target);
    }

    for (const Node& node : active_nodes) {
        std::string nodeName = getNodeName(node);
        std::string label = showLabels ? getNodeLabel(node) : ""; 
        std::string color = getTimepointColor(node.timepoint);
        
        std::string shape;
        if (node.isARG) {
            shape = "circle";
        } else {
            std::string groupName = getMGEGroupName(node.id);
            shape = getMGEGroupShape(groupName);
        }

        file << "  " << nodeName
             << " [label=\"" << label << "\", shape=" << shape
             << ", fixedsize=true, width=0.5, height=0.5, fillcolor=" << color << "]\n";
    }

    std::set<std::pair<Node, Node>> processedColoEdges;

    for (const Edge& edge : g.edges) {
        std::string sourceName = getNodeName(edge.source);
        std::string targetName = getNodeName(edge.target);

        std::string color;
        std::string style;
        std::string extraAttributes;

        if (edge.isColo) {
            auto canonical_pair = std::minmax(edge.source, edge.target);
            if (processedColoEdges.count(canonical_pair)) {
                continue;
            }
            processedColoEdges.insert(canonical_pair);

            color = "\"#0000FF\"";
            style = "solid";
            extraAttributes = "dir=both";

        } else if (isTemporalEdge(edge)) {
            color = "\"#FF0000\"";
            style = "dashed";
        } else {
            color = "\"#808080\"";
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