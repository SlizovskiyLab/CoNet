#include "graph_utils.h"
#include "id_maps.h"
#include <iostream>

Graph filterGraphByARGName(const Graph& g, const std::string& argName) {
    int argID = getARGId(argName);
    if (argID == -1) {
        std::cerr << "ARG not found: " << argName << "\n";
        return {};
    }

    Graph subgraph;

    for (const Edge& edge : g.edges) {
        if (!edge.isColo) continue;

        // Check if either side is the ARG of interest
        if ((edge.source.isARG && edge.source.id == argID) ||
            (edge.target.isARG && edge.target.id == argID)) {
            
            // Always insert both nodes and the edge
            subgraph.nodes.insert(edge.source);
            subgraph.nodes.insert(edge.target);
            subgraph.edges.insert(edge);
        }
    }

    return subgraph;
}

Graph filterGraphByMGEName(const Graph& g, const std::string& mgeName) {
    int mgeID = getMGEId(mgeName);
    if (mgeID == -1) {
        std::cerr << "MGE not found: " << mgeName << "\n";
        return {};
    }

    Graph subgraph;

    for (const Edge& edge : g.edges) {
        if (!edge.isColo) continue;

        // Check if either side is the MGE of interest
        if ((!edge.source.isARG && edge.source.id == mgeID) ||
            (!edge.target.isARG && edge.target.id == mgeID)) {

            // Always insert both nodes and the edge
            subgraph.nodes.insert(edge.source);
            subgraph.nodes.insert(edge.target);
            subgraph.edges.insert(edge);
        }
    }

    return subgraph;
}

Graph filterGraphByMGEGroup(const Graph& g, const std::string& groupName) {
    std::unordered_set<int> mgeIDsInGroup;
    for (const auto& [id, name] : mgeGroupMap) {
        if (name == groupName) {
            mgeIDsInGroup.insert(id);
        }
    }

    if (mgeIDsInGroup.empty()) {
        std::cerr << "No MGEs found for group: " << groupName << "\n";
        return {};
    }

    Graph subgraph;
    for (const Edge& edge : g.edges) {
        if (!edge.isColo) continue;

        bool sourceInGroup = !edge.source.isARG && mgeIDsInGroup.count(edge.source.id);
        bool targetInGroup = !edge.target.isARG && mgeIDsInGroup.count(edge.target.id);

        if (sourceInGroup || targetInGroup) {
            subgraph.nodes.insert(edge.source);
            subgraph.nodes.insert(edge.target);
            subgraph.edges.insert(edge);
        }
    }

    return subgraph;
}