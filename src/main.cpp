#include <iostream>
#include <filesystem>
#include "../include/Timepoint.h"
#include "../include/graph.h"
#include "../include/parser.h"
#include "../include/id_maps.h"
#include "../include/traversal.h"
#include "../include/query_engine.h"
#include "../include/export.h"

/* Main entry point: parse arguments, load data, call functions */

namespace fs = std::filesystem;

fs::path data_file = "/Users/suhashidesilva/Documents/Projects/CoNet/data/patientwise_colocalization_by_timepoint.csv";

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
    


    /******************************** Traversal based on time - approach 1 ************************************/
    // std::map<std::pair<int, int>, Node> firstOccurrence;
    // std::map<std::pair<int, int>, std::set<Timepoint>> colocalizationsByTime;

    // traverseTempGraph(g, adjacency, firstOccurrence, colocalizationsByTime);

    // for (const auto& entry : colocalizationsByTime) {
    //     int argId = entry.first.first;
    //     int mgeId = entry.first.second;
    //     std::cout << "ARG ID: " << argId << ", MGE ID: " << mgeId << "\n";
    //     for (const auto& tp : entry.second) {
    //         std::cout << "  - " << tp << "\n";
    //     }
    // }

    // std::cout << "First occurrence of colocalizations:\n";
    // for (const auto& entry : firstOccurrence) {
    //     int argId = entry.first.first;
    //     int mgeId = entry.first.second;
    //     std::cout << "ARG ID: " << argId << ", MGE ID: " << mgeId << "\n";
    //     std::cout << "  - " << entry.second.timepoint << "\n";
    // }


    /******************************** Traversal based on time - approach2 ************************************/
    std::map<std::tuple<int, int, int>, Node> firstOccurrenceByInd;
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationsTimelineByInd;

    traverseGraphByInd(g, adjacency, g.edges, firstOccurrenceByInd, colocalizationsTimelineByInd);

    // for (const auto& entry : colocalizationsByTime) {
    //     int argId = entry.first.first;
    //     int mgeId = entry.first.second;
    //     std::cout << "ARG ID: " << argId << ", MGE ID: " << mgeId << "\n";
    //     for (const auto& tp : entry.second) {
    //         std::cout << "  - " << tp << "\n";
    //     }
    // }

    // std::cout << "First occurrence of colocalizations:\n";
    // for (const auto& entry : firstOccurrence) {
    //     int argId = entry.first.first;
    //     int mgeId = entry.first.second;
    //     std::cout << "ARG ID: " << argId << ", MGE ID: " << mgeId << "\n";
    //     std::cout << "  - " << entry.second.timepoint << "\n";
    // }


    // /******************************** Query Engine  ************************************/
    std::cout << "Query Engine Results:\n";
    // getColocalizationsPostFMTOnly(g, colocalizationsByTime);
    // getColocalizationsPreFMTOnly(g, colocalizationTimeline);

    getColocalizationsByIndPostFMTOnly(g, colocalizationsTimelineByInd);
    getColocalizationsByIndPreFMTOnly(g, colocalizationsTimelineByInd);

    return 0;
}





















 /******************************** Traversal of Graph  ************************************/
    // std::map<std::pair<int, int>, std::set<Timepoint>> colocalizationTimeline;
    
    // traverseAdjacency(g, adjacency, colocalizationTimeline);
    // std::cout << "individual timeline" << "\n";
    // for (const auto& entry : colocalizationTimeline) {
    //     std::cout << "ARG ID: " << entry.first.first
    //               << ", MGE ID: " << entry.first.second << "\n";
    //     for (const auto& tp : entry.second) {
    //         std::cout << "  - " << tp << "\n";
    //     }
    // }

    // std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationByIndividual;
    // traverseGraph(g, colocalizationByIndividual);
    // std::cout << "colocalization by individual" << "\n";
    // for (const auto& entry : colocalizationByIndividual) {
    //     int individualId = std::get<0>(entry.first);
    //     int argId = std::get<1>(entry.first);
    //     int mgeId = std::get<2>(entry.first);
    //     std::cout << "Ind ID: " << individualId 
    //               << ", ARG ID: " << argId 
    //               << ", MGE ID: " << mgeId << "\n";
    //     for (const auto& tp : entry.second) {
    //         std::cout << "  - " << tp << "\n";
    //     }
    // }
    // std::cout << "Total nodes: " << g.nodes.size() << "\n";
    // std::cout << "Total edges: " << g.edges.size() << "\n";
    // std::cout << "Colocalization by individual size: " << colocalizationByIndividual.size() << "\n";
    // std::cout << "Colocalization timeline size: " << colocalizationTimeline.size() << "\n";

