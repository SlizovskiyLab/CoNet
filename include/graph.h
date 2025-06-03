#ifndef GRAPH_H
#define GRAPH_H

#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <tuple>
#include "Timepoint.h"

// ------------------ Node ------------------

struct Node {
    int id;
    bool isARG;
    Timepoint timepoint;
    bool requiresSNPConfirmation;

    bool operator<(const Node& other) const {
        return std::tie(id, timepoint, isARG) < std::tie(other.id, other.timepoint, other.isARG);
    }

    bool operator==(const Node& other) const {
        return id == other.id && isARG == other.isARG && timepoint == other.timepoint;
    }
};

// Hash function for Node to use in unordered_map/set
namespace std {
    template <>
    struct hash<Node> {
        std::size_t operator()(const Node& n) const {
            return hash<int>()(n.id) ^ (hash<bool>()(n.isARG) << 1) ^ (hash<int>()(static_cast<int>(n.timepoint)) << 2);
        }
    };
}

// ------------------ Edge ------------------

struct Edge {
    Node source;
    Node target;
    bool isColo;
    std::set<int> individuals;
    int weight = 0;
    std::unordered_map<int, std::unordered_set<int>> argToIndividuals;

    bool operator<(const Edge& other) const {
        return std::tie(source, target, isColo, weight) < std::tie(other.source, other.target, other.isColo, other.weight);
    }

    bool operator==(const Edge& other) const {
        return source == other.source &&
               target == other.target &&
               isColo == other.isColo &&
               weight == other.weight;
    }
};

// ------------------ Graph ------------------

struct Graph {
    std::set<Node> nodes;
    std::set<Edge> edges;
};

// Function declaration
void buildAdjacency(const Graph& g, std::unordered_map<Node, std::unordered_set<Node>>& adj);


#endif // GRAPH_H
