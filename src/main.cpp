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
#include "../include/graph_analysis.h"
// #include "../include/export_graph_json.h"   

/* Main entry point: parse arguments, load data, call functions */

namespace fs = std::filesystem;

fs::path data_file = "data/patientwise_colocalization_by_timepoint.csv";

// static bool ensure_parent_dir(const fs::path& p) {
//     std::error_code ec;
//     fs::path parent = p.parent_path();
//     if (parent.empty()) return true;
//     if (fs::exists(parent, ec)) return !ec;
//     return fs::create_directories(parent, ec);
// }

int main() {
    Graph g;
    std::map<int, std::string> patientToDiseaseMap;
    std::unordered_map<Node, std::unordered_set<Node>> adjacency;

    // parse the data file and construct the graph (true to exclude ARGs requiring SNP confirmation, true to exclude metals)
    parseData(data_file, g, patientToDiseaseMap, true, false); 
    addTemporalEdges(g);  
    buildAdjacency(g, adjacency);

    /******************************** Graph Statistics  ************************************/
    writeGraphStatisticsCSV(g, adjacency, "output/graph_statistics.csv");

    /******************************** Traversal of Graph  ************************************/
    std::map<std::pair<int, int>, std::multiset<Timepoint>> colocalizationTimeline;
    traverseAdjacency(g, adjacency, colocalizationTimeline);

    /******************************** Traversal of Graph  ************************************/
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationByIndividual;
    traverseGraph(g, colocalizationByIndividual);
    std::map<std::pair<int, int>, std::set<int>> globalPairToPatients;
    

    /********************************* Colocalizations by Timepoints ************************************/
    writeAllDiseases_TemporalDynamicsCounts(colocalizationByIndividual, patientToDiseaseMap);

    // // ------ Export D3 JSON --------
    // const fs::path jsonOut = "viz/graph.json";
    // if (!ensure_parent_dir(jsonOut)) {
    //     std::cerr << "Failed to create dir: " << jsonOut.parent_path() << "\n";
    //     return 1;
    // }
    // Graph coNet = g;
    // std::cerr << "[main] Graph has nodes=" << coNet.nodes.size()
    //       << " edges=" << coNet.edges.size() << "\n";
    // exportGraphToJsonSimple(coNet, jsonOut.string());


    analyzeColocalizations(g, adjacency);
    // analyzeColocalizationsCollectively(g, adjacency);

    /**************************************** Most Prominent Genes ***********************************/
    mostProminentEntities(g);

    /************************************* Graph Visualization ***********************************/
    Graph coNet = g;
    exportToDot(coNet, "conet.dot");
    exportParentTemporalGraphDot(coNet, "conet_parent_temporal.dot", true);

    Graph rCDI = filterGraphByDisease(g, "rCDI", patientToDiseaseMap);
    exportToDot(rCDI, "rcdi.dot");
    exportParentTemporalGraphDot(rCDI, "rcdi_parent_temporal.dot", true);
    
    Graph melanoma = filterGraphByDisease(g, "Melanoma", patientToDiseaseMap);
    exportToDot(melanoma, "melanoma.dot");
    exportParentTemporalGraphDot(melanoma, "melanoma_parent_temporal.dot", true);

    Graph mdrb = filterGraphByDisease(g, "MDRB", patientToDiseaseMap);
    exportToDot(mdrb, "mdrb.dot");
    exportParentTemporalGraphDot(mdrb, "mdrb_temporal.dot", true);

    // Graph sub = filterGraphByARGName(g, "A16S");
    // exportToDot(sub, "A16S_subgraph.dot");
    // exportParentTemporalGraphDot(sub, "A16S_subgraph_parent_temporal.dot", true);

    Graph sub2 = filterGraphByARGName(g, "CFX");
    exportToDot(sub2, "CFX.dot");
    exportParentTemporalGraphDot(sub2, "CFX_parent_temporal.dot", true);

    Graph sub3 = filterGraphByMGEGroup(g, "virus");
    exportToDot(sub3, "virus.dot");
    exportParentTemporalGraphDot(sub3, "virus_parent_temporal.dot", true);

    Graph sub4 = filterGraphByTimepoint(g, "donor");
    exportToDot(sub4, "donor.dot");
    exportParentTemporalGraphDot(sub4, "donor_parent_temporal.dot", true);

    return 0;

}






