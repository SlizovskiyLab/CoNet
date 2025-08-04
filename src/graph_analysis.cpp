#include "../include/graph_analysis.h"
#include "../include/graph.h"
#include "../include/traversal.h"
#include "../include/query_engine.h"
#include "../include/export.h"
#include "../include/id_maps.h"
#include <iostream>
#include <map>
#include <set>
#include <tuple>
#include <algorithm>

void printGraphStatistics(const Graph& g, 
                          const std::unordered_map<Node, std::unordered_set<Node>>& adjacency) {
    std::cout << "Graph constructed with " << g.nodes.size() 
              << " nodes and " << g.edges.size() << " edges.\n";
    std::cout << "ARGs: " << std::count_if(g.nodes.begin(), g.nodes.end(), 
                  [](const Node& n) { return n.isARG; }) << "\n";
    std::cout << "MGEs: " << std::count_if(g.nodes.begin(), g.nodes.end(), 
                  [](const Node& n) { return !n.isARG; }) << "\n";
    std::cout << "Total nodes: " << g.nodes.size() << "\n";
    std::cout << "Colocalization Edges: " << std::count_if(g.edges.begin(), g.edges.end(), 
                  [](const Edge& e) { return e.isColo; }) << "\n";
    std::cout << "Temporal Edges: " << std::count_if(g.edges.begin(), g.edges.end(), 
                  [](const Edge& e) { return !e.isColo; }) << "\n";
    std::cout << "Total edges: " << g.edges.size() << "\n";
    std::cout << "Adjacency list size: " << adjacency.size() << " nodes.\n";
}




void analyzeColocalizations(const Graph& g, 
                            const std::unordered_map<Node, std::unordered_set<Node>>& adjacency) {

    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationByIndividual;
    traverseGraph(g, colocalizationByIndividual);

    std::cout << "Patientwise Colocalization dynamics over time:\n";
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, false, false, true, "PostFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, false, true, false, "PreFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, false, false, "Donor Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, true, false, "Donor & PreFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, false, true, true, "PreFMT & PostFMT Only");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, true, true, "PreFMT, Donor & PostFMT");
    getPatientwiseColocalizationsByCriteria(g, colocalizationByIndividual, true, false, true, "Donor & PostFMT Only");
}


void analyzeColocalizationsCollectively(const Graph& g, 
                                         const std::unordered_map<Node, std::unordered_set<Node>>& adjacency) {
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationByIndividual;
    traverseGraph(g, colocalizationByIndividual);
    std::map<std::pair<int, int>, std::set<int>> globalPairToPatients;
    std::cout << "Colocalization dynamics over time:\n";
    getColocalizationsByCriteria(colocalizationByIndividual, false, false, true, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario
    getColocalizationsByCriteria(colocalizationByIndividual, false, true, false, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario
    getColocalizationsByCriteria(colocalizationByIndividual, true, false, false, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario
    getColocalizationsByCriteria(colocalizationByIndividual, true, true, false, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario
    getColocalizationsByCriteria(colocalizationByIndividual, false, true, true, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario
    getColocalizationsByCriteria(colocalizationByIndividual, true, true, true, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario
    getColocalizationsByCriteria(colocalizationByIndividual, true, false, true, globalPairToPatients);
    globalPairToPatients.clear(); // Clear the map for the next scenario
}



void mostProminentEntities(const Graph& g) {
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationByIndividual;
    traverseGraph(g, colocalizationByIndividual);

    std::cout << "Most Prominent ARGs and MGEs:\n";
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
    getTopARGMGEPairsByFrequency(colocalizationByIndividual, 10); // Top 10 ARG-MGE pairs by frequency

}