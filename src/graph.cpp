#include <iostream>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>

/* Define the graph data structure and utilities (e.g., adding nodes/edges) */

// Timepoint enum
enum class Timepoint {
    Donor = -1,
    Pre = 0,
    Day4 = 4,
    Day7 = 7,
    Day14 = 14
};

// Node definition
struct Node {
    bool isARG;        			          // True for ARG, false for MGE
    Timepoint timepoint;         		  // -1, 0, 4, 7, 14
    int id;				                  // Unique identifier
    bool requiresSNPConfirmation;

    // Define operator< for use in std::set
    bool operator<(const Node& other) const {
        return id < other.id;
    }
};


// Edge definition
struct Edge {
    bool isColoc;                     // True for "colocalization", false for "temporal"
    std::set<int> individuals;       // For colocalization edges
    int weight = 0;                  // For temporal edges

};


// Graph definition
struct Graph {
    std::set<Node> nodes;
    std::set<Edge> edges;
};
