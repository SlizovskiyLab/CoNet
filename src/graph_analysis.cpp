#include "../include/graph_analysis.h"
#include "../include/graph.h"
#include "../include/traversal.h"
#include "../include/query_engine.h"
#include "../include/export.h"
#include "../include/id_maps.h"
#include "../include/Timepoint.h"
#include <iostream>
#include <map>
#include <set>
#include <tuple>
#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <tuple>
#include <string>
#include <fstream>
#include <iostream>



void writeGraphStatisticsCSV(
    const Graph& g,
    const std::unordered_map<Node, std::unordered_set<Node>>& adjacency,
    const std::string& filename
) {
    // Precompute stats once
    const size_t total_nodes = g.nodes.size();
    const size_t total_edges = g.edges.size();
    const size_t arg_count = std::count_if(g.nodes.begin(), g.nodes.end(),
                                           [](const Node& n){ return n.isARG; });
    const size_t mge_count = total_nodes - arg_count;
    const size_t colo_edges = std::count_if(g.edges.begin(), g.edges.end(),
                                            [](const Edge& e){ return e.isColo; });
    const size_t temporal_edges = total_edges - colo_edges;
    const size_t adjacency_nodes = adjacency.size();

    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }

    // Header
    out << "TotalNodes,TotalEdges,ARGs,MGEs,ColocalizationEdges,TemporalEdges,AdjacencyNodes\n";
    // Row
    out << total_nodes << ','
        << total_edges << ','
        << arg_count << ','
        << mge_count << ','
        << colo_edges << ','
        << temporal_edges << ','
        << adjacency_nodes << '\n';

    out.close();
    std::cout << "Graph statistics written to " << filename << "\n";
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



