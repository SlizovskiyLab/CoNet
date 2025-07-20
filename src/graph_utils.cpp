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

Graph filterGraphByTimepoint(const Graph& g, const std::string& timepointCategory) {
    Graph subgraph;

    auto matchesCategory = [&](Timepoint tp) {
        if (timepointCategory == "donor") {
            return tp == Timepoint::Donor;
        }
        if (timepointCategory == "pre") {
            return tp == Timepoint::PreFMT;
        }
        if (timepointCategory == "post") {
            return tp != Timepoint::Donor && tp != Timepoint::PreFMT;
        }
        return false;
    };

    for (const auto& edge : g.edges) {
        if (matchesCategory(edge.source.timepoint) && matchesCategory(edge.target.timepoint)) {
            subgraph.nodes.insert(edge.source);
            subgraph.nodes.insert(edge.target);
            subgraph.edges.insert(edge);
        }
    }

    if (subgraph.nodes.empty()) {
        std::cerr << "Warning: No nodes found for timepoint category '" << timepointCategory 
                  << "'. The resulting graph will be empty.\n";
    }

    return subgraph;
}

Graph filterGraphByARGAndMGENames(const Graph& g, const std::string& argName, const std::string& mgeName) {
    int argID = getARGId(argName);
    if (argID == -1) {
        std::cerr << "ARG not found: " << argName << "\n";
        return {};
    }

    int mgeID = getMGEId(mgeName);
    if (mgeID == -1) {
        std::cerr << "MGE not found: " << mgeName << "\n";
        return {};
    }

    Graph subgraph;
    std::unordered_set<Node> relevant_nodes;

    // Find all colocalization edges between the specific ARG and MGE
    for (const Edge& edge : g.edges) {
        if (!edge.isColo) continue;

        bool arg_is_source = edge.source.isARG && edge.source.id == argID;
        bool mge_is_target = !edge.target.isARG && edge.target.id == mgeID;

        bool mge_is_source = !edge.source.isARG && edge.source.id == mgeID;
        bool arg_is_target = edge.target.isARG && edge.target.id == argID;

        if ((arg_is_source && mge_is_target) || (mge_is_source && arg_is_target)) {
            subgraph.edges.insert(edge);
            relevant_nodes.insert(edge.source);
            relevant_nodes.insert(edge.target);
        }
    }

    // Add all temporal edges that connect the relevant nodes.
    for (const Edge& edge : g.edges) {
        if (edge.isColo) continue;

        // Check if a temporal edge connects two nodes that we've already identified as relevant
        if (relevant_nodes.count(edge.source) && relevant_nodes.count(edge.target)) {
            subgraph.edges.insert(edge);
        }
    }
    
    // Add all the collected nodes to the subgraph
    for(const auto& node : relevant_nodes) {
        subgraph.nodes.insert(node);
    }

    return subgraph;
}