#include <iostream>
#include <filesystem>
#include "../include/Timepoint.h"
#include "../include/graph.h"
#include "../include/parser.h"
#include "../include/id_maps.h"
#include "../include/traversal.h"
#include "../include/query_engine.h"
#include "../include/export.h"
#include "../include/graph_utils.h"

/* Main entry point: parse arguments, load data, call functions */

namespace fs = std::filesystem;

fs::path data_file = "data/patientwise_colocalization_by_timepoint.csv";


int main() {
    Graph g;
    // parse the data file and construct the graph (true to exclude ARGs requiring SNP confirmation, true to exclude metals)
    parseData(data_file, g, true, false); 
    addTemporalEdges(g);  

    std::unordered_map<Node, std::unordered_set<Node>> adjacency;
    buildAdjacency(g, adjacency);

    std::cout << "Graph constructed with " << g.nodes.size() << " nodes and " << g.edges.size() << " edges.\n";
    std::cout << "Adjacency list size: " << adjacency.size() << " nodes.\n";

    
     /******************************** Traversal of Graph  ************************************/
    std::map<std::pair<int, int>, std::multiset<Timepoint>> colocalizationTimeline;
    traverseAdjacency(g, adjacency, colocalizationTimeline);


    /******************************** Traversal of Graph  ************************************/
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationByIndividual;
    traverseGraph(g, colocalizationByIndividual);

    std::map<std::pair<int, int>, std::set<int>> globalPairToPatients;

    /*********************************** Query Engine  **************************************/
    std::cout << "Colocalization dynamics over time:\n";

    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, false, false, true, "PostFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, false, true, false, "PreFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, false, true, "PostFMT & Donor Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, false, true, true, "PreFMT & PostFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, false, false, "Donor Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, true, false, "PreFMT & Donor Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, true, true, "PreFMT, Donor & PostFMT");

    getColocalizationsByCriteria(colocalizationByIndividual, false, false, true, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario
    getColocalizationsByCriteria(colocalizationByIndividual, false, true, false, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario

    getColocalizationsByCriteria(colocalizationByIndividual, true, false, true, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario
    getColocalizationsByCriteria(colocalizationByIndividual, false, true, true, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario



    std::vector<std::pair<int, int>> topARGs = getTopKEntities(g, true, static_cast<unsigned int>(10)); // Top 10 ARGs
    std::cout << "Top ARGs:\n";
    for (const auto& [id, count] : topARGs) {
        std::cout << "ARG: " << getARGName(id) << " (" << getARGGroupName(id) << "), Count: " << count << "\n";
    }
    std::vector<std::pair<int, int>> topMGEs = getTopKEntities(g, false, static_cast<unsigned int>(10)); // Top 5 MGEs
    std::cout << "Top MGEs:\n";
    for (const auto& [id, count] : topMGEs) {
        std::cout << "MGE: " << getMGEName(id) << ", Count: " << count << "\n";
    }  

    getTopARGMGEPairsByFrequencyWODonor(colocalizationByIndividual, 10); // Top 10 ARG-MGE pairs by frequency

    Graph sub2 = filterGraphByARGName(g, "ANT3-DPRIME");
    exportToDot(sub2, "ANT3-DPRIME_subgraph.dot");

    return 0;

}






