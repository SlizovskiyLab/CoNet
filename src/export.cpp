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
    std::string label = node.isARG ? getARGName(node.id) : getMGENameForLabel(node.id);
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

            color = "\"#0000FF\""; // Blue for colocalization
            style = "solid";
            extraAttributes = "dir=both";

        } else if (isTemporalEdge(edge)) {
            style = "dashed"; // Style is always dashed for temporal edges
            
            Timepoint src_tp = edge.source.timepoint;
            Timepoint tgt_tp = edge.target.timepoint;

            bool src_is_post = (src_tp != Timepoint::Donor && src_tp != Timepoint::PreFMT);
            bool tgt_is_post = (tgt_tp != Timepoint::Donor && tgt_tp != Timepoint::PreFMT);

            if (src_tp == Timepoint::Donor && tgt_tp == Timepoint::PreFMT) {
                color = "\"orange\"";      // Donor -> Pre
            } else if (src_tp == Timepoint::Donor && tgt_is_post) {
                color = "\"purple\"";      // Donor -> Post
            } else if (src_tp == Timepoint::PreFMT && tgt_is_post) {
                color = "\"brown\"";        // Pre -> Post
            } else if (src_is_post && tgt_is_post) {
                color = "\"#FF0000\"";     // Post -> Post (original red)
            } else {
                color = "\"#FF0000\"";     // Fallback to red for any other temporal case
            }
            
        } else {
            // This case should ideally not be hit with current logic
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
