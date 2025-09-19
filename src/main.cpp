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
#include "../include/export_graph_json.h" 
#include "../include/config_loader.h"  

/* Main entry point: parse arguments, load data, call functions */

namespace fs = std::filesystem;

fs::path data_file;
fs::path interaction_json_path;
fs::path parent_json_path;


int main() {
    try {
        Config cfg = loadConfig("config/paths.json");
        data_file = fs::path(cfg.input_data_path);
        interaction_json_path = fs::path(cfg.viz_interaction);
        parent_json_path = fs::path(cfg.viz_parent);

    } catch (const std::exception& e) {
        std::cerr << "Config error: " << e.what() << "\n";
        return 1;
    }
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
    writeAllDiseasesTemporalDynamicsCounts(colocalizationByIndividual, patientToDiseaseMap);
    writeTemporalDynamicsCountsForMGEGroup(colocalizationByIndividual);

    exportColocalizations(g, colocalizationByIndividual);

    // /************************************* Graph Visualization ***********************************/

    Graph coNet = g;
    exportGraphToJsonSimple(coNet, interaction_json_path);
    exportParentGraphToJson(coNet, parent_json_path);

    // exportToDot(coNet, "conet.dot");
    // exportParentTemporalGraphDot(coNet, "conet_parent_temporal.dot", true);

    // Graph rCDI = filterGraphByDisease(g, "rCDI", patientToDiseaseMap);
    // exportToDot(rCDI, "rcdi.dot");
    // exportParentTemporalGraphDot(rCDI, "rcdi_parent_temporal.dot", true);
    
    // Graph melanoma = filterGraphByDisease(g, "Melanoma", patientToDiseaseMap);
    // exportToDot(melanoma, "melanoma.dot", false);
    // exportParentTemporalGraphDot(melanoma, "melanoma_parent_temporal.dot", true);

    // Graph mdrb = filterGraphByDisease(g, "MDRB", patientToDiseaseMap);
    // exportToDot(mdrb, "mdrb.dot", false);
    // exportParentTemporalGraphDot(mdrb, "mdrb_temporal.dot", true);

    // Graph sub = filterGraphByARGName(g, "A16S");
    // exportToDot(sub, "A16S_subgraph.dot");
    // exportParentTemporalGraphDot(sub, "A16S_subgraph_parent_temporal.dot", true);

    // Graph sub2 = filterGraphByARGName(g, "ERMB");
    // exportToDot(sub2, "ERMB.dot", false);
    // exportParentTemporalGraphDot(sub2, "CFX_parent_temporal.dot", false);

    // Graph sub3 = filterGraphByMGEGroup(g, "virus");
    // exportToDot(sub3, "virus.dot");
    // exportParentTemporalGraphDot(sub3, "virus_parent_temporal.dot", true);

    // Graph sub4 = filterGraphByTimepoint(g, "donor");
    // exportToDot(sub4, "donor.dot");
    // exportParentTemporalGraphDot(sub4, "donor_parent_temporal.dot", true);

    return 0;

}






