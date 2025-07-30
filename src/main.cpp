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
    std::map<int, std::string> patientToDiseaseMap;
    
    // parse the data file and construct the graph (true to exclude ARGs requiring SNP confirmation, true to exclude metals)
    parseData(data_file, g, patientToDiseaseMap, true, true); 
    addTemporalEdges(g);  

    Graph rCDI = filterGraphByDisease(g, "rCDI", patientToDiseaseMap);
    exportToDot(rCDI, "rcdi.dot");
    Graph melanoma = filterGraphByDisease(g, "Melanoma", patientToDiseaseMap);
    exportToDot(melanoma, "melanoma.dot");
    Graph mdrb = filterGraphByDisease(g, "MDRB", patientToDiseaseMap);
    exportToDot(mdrb, "mdrb.dot");

    std::unordered_map<Node, std::unordered_set<Node>> adjacency;
    buildAdjacency(g, adjacency);

    /******************************** Graph Statistics ************************************/
    std::cout << "Graph constructed with " << g.nodes.size() << " nodes and " << g.edges.size() << " edges.\n";
    std::cout << "ARGs: " << std::count_if(g.nodes.begin(), g.nodes.end(), [](const Node& n) { return n.isARG; }) << "\n";
    std::cout << "MGEs: " << std::count_if(g.nodes.begin(), g.nodes.end(), [](const Node& n) { return !n.isARG; }) << "\n";
    std::cout << "Total nodes: " << g.nodes.size() << "\n";
    std::cout << "Colocalization Edges: " << std::count_if(g.edges.begin(), g.edges.end(), [](const Edge& e) { return e.isColo; }) << "\n";
    std::cout << "Temporal Edges: " << std::count_if(g.edges.begin(), g.edges.end(), [](const Edge& e) { return !e.isColo; }) << "\n";
    std::cout << "Total edges: " << g.edges.size() << "\n";
    std::cout << "Adjacency list size: " << adjacency.size() << " nodes.\n";


    exportToDot(g, "graph_output.dot", false);
    Graph sub = filterGraphByARGName(g, "A16S");
    exportToDot(sub, "A16S_subgraph.dot");

 
    
     /******************************** Traversal of Graph  ************************************/
    std::map<std::pair<int, int>, std::multiset<Timepoint>> colocalizationTimeline;
    traverseAdjacency(g, adjacency, colocalizationTimeline);
    std::cout << "No of unique colocalizations: " << colocalizationTimeline.size() << "\n";

    /******************************** Traversal of Graph  ************************************/
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationByIndividual;
    traverseGraph(g, colocalizationByIndividual);

    std::map<std::pair<int, int>, std::set<int>> globalPairToPatients;

    /********************************* Colocalizations by Timepoints ************************************/
    std::cout << "Patientwise Colocalization dynamics over time:\n";

    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, false, false, true, "PostFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, false, true, false, "PreFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, false, false, "Donor Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, true, false, "Donor & PreFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, false, true, true, "PreFMT & PostFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, true, true, "PreFMT, Donor & PostFMT");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, false, true, "Donor & PostFMT Only");

    std::cout << "Colocalization dynamics over time:\n";
    // getColocalizationsByCriteria(colocalizationByIndividual, false, false, true, globalPairToPatients);
    // globalPairToPatients.clear(); // Clear the map for the next scenario
    // getColocalizationsByCriteria(colocalizationByIndividual, false, true, false, globalPairToPatients);
    // globalPairToPatients.clear(); // Clear the map for the next scenario
    // getColocalizationsByCriteria(colocalizationByIndividual, true, false, false, globalPairToPatients);
    // globalPairToPatients.clear(); // Clear the map for the next scenario
    // getColocalizationsByCriteria(colocalizationByIndividual, true, true, false, globalPairToPatients);
    // globalPairToPatients.clear(); // Clear the map for the next scenario
    // getColocalizationsByCriteria(colocalizationByIndividual, false, true, true, globalPairToPatients);
    // globalPairToPatients.clear(); // Clear the map for the next scenario
    // getColocalizationsByCriteria(colocalizationByIndividual, true, true, true, globalPairToPatients);
    // globalPairToPatients.clear(); // Clear the map for the next scenario
    // getColocalizationsByCriteria(colocalizationByIndividual, true, false, true, globalPairToPatients);
    // globalPairToPatients.clear(); // Clear the map for the next scenario


    /**************************************** Most Prominent Genes ***********************************/


    // std::vector<std::pair<int, int>> topARGs = getTopKEntities(g, true, static_cast<unsigned int>(10)); // Top 10 ARGs
    // std::cout << "Top ARGs:\n";
    // for (const auto& [id, count] : topARGs) {
    //     std::cout << "ARG: " << getARGName(id) << " (" << getARGGroupName(id) << "), Count: " << count << "\n";
    // }
    // std::vector<std::pair<int, int>> topMGEs = getTopKEntities(g, false, static_cast<unsigned int>(10)); // Top 5 MGEs
    // std::cout << "Top MGEs:\n";
    // for (const auto& [id, count] : topMGEs) {
    //     std::cout << "MGE: " << getMGEName(id) << ", Count: " << count << "\n";
    // }  

    getTopARGMGEPairsByFrequency(colocalizationByIndividual, 10); // Top 10 ARG-MGE pairs by frequency

    /************************************* Graph Visualization ***********************************/

    exportToDot(g, "graph_output.dot", true);
    Graph sub = filterGraphByARGName(g, "A16S");
    exportToDot(sub, "A16S_subgraph.dot");


    Graph sub2 = filterGraphByARGName(g, "CFX");
    exportToDot(sub2, "CFX.dot");
    Graph sub3 = filterGraphByMGEGroup(g, "virus");
    exportToDot(sub3, "virus.dot");
    Graph sub4 = filterGraphByTimepoint(g, "donor");
    exportToDot(sub4, "donor.dot");
    exportToDot(g, "graph.dot");



    return 0;

}






