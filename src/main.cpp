#include <iostream>
#include <filesystem>
#include "../include/Timepoint.h"
#include "../include/graph.h"
#include "../include/parser.h"
#include "../include/id_maps.h"
#include "../include/export.h"

/* Main entry point: parse arguments, load data, call functions */

namespace fs = std::filesystem;

fs::path data_file = "C:/Users/asake/OneDrive/Desktop/Homework/FMT/CoNet/data/patientwise_colocalization_by_timepoint.csv";

int main() {
    Graph g;
    parseData(data_file, g);
    addTemporalEdges(g);  


    std::unordered_map<Node, std::unordered_set<Node>> adjacency;
    buildAdjacency(g, adjacency);

    std::cout << "Graph constructed with " << g.nodes.size() << " nodes and " << g.edges.size() << " edges.\n";
    std::cout << "Adjacency list size: " << adjacency.size() << " nodes.\n";

    // Printing of Adjacency List
    std::cout << "Adjacency list" << "\n";
    for (const auto& i : adjacency) {
        std::cout << i.first << " -> ";
        for (const auto& neighbor : i.second) {
            std::cout << neighbor <<  " ";
        }
        std::cout << "\n";
    }
    exportToDot(g, "graph_output.dot", 700, 11000);

    // std::cout << "print nodes" << "\n";
    // for (const auto& node : g.nodes) {
    //     std::cout << node << "\n";
    // }
    // std::cout << "print edges" << "\n";
    // for (const auto& edge : g.edges) {
    //     std::cout << "Edge from " << edge.source << " to " << edge.target 
    //               << " | isColo: " << edge.isColo 
    //               << " | weight: " << edge.weight 
    //               << "\n";
    // }

    return 0;
}











/*
int main() {
    Graph g;

    Node n1{true, Timepoint::Pre, 1, true};
    Node n2{false, Timepoint::Day4, 3, false};
    g.nodes.insert(n1);
    g.nodes.insert(n2);

    Edge e1;
    e1.label = "colocalization";
    e1.individuals = {101, 102};

    Edge e2;
    e2.label = "temporal";
    e2.weight = 4;

    g.edges.insert(e1);
    g.edges.insert(e2);

    std::cout << "Graph has " << g.nodes.size() << " nodes and " << g.edges.size() << " edges.\n";
    return 0;
}
*/