#include "../include/parser.h"
#include "../include/id_maps.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <algorithm>
#include <map>

/* Read input files (CSV), extract (individual, ARG, MGE, timepoint) data */

void addEdge(Graph& graph, const Node& src, const Node& tgt, bool isColo, int patientID) {
    Edge edge = {src, tgt, isColo};
    if (isColo && patientID != -1) {
        edge.individuals.insert(patientID);
    }

    auto it = graph.edges.find(edge);
    if (it != graph.edges.end() && isColo && patientID != -1) {
        const_cast<Edge&>(*it).individuals.insert(patientID);
    } else {
        graph.edges.insert(edge);
        if (isColo) {
            // Ensure reverse edge is also inserted for bidirectional colocalization
            Edge reverseEdge = {tgt, src, true};
            if (patientID != -1) reverseEdge.individuals.insert(patientID);
            graph.edges.insert(reverseEdge);
        }
    }
}

// This function reads a CSV file containing patient data and constructs a graph.
// It extracts ARG and MGE labels, maps them to IDs, and creates nodes and edges
void parseData(const std::filesystem::path& filename, Graph& graph, bool includeSNPConfirmationARGs, bool excludeMetals) {
    std::ifstream infile(filename);
    std::string line;
    std::vector<std::string> headers;
    bool isHeader = true;

    std::unordered_map<std::string, Timepoint> columnToTimepoint;

    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (isHeader) {
            headers = tokens;
            for (const std::string& col : headers) {
                if (col == "Donor") columnToTimepoint[col] = Timepoint::Donor;
                else if (col == "PreFMT") columnToTimepoint[col] = Timepoint::PreFMT;
                else if (col.rfind("PostFMT_", 0) == 0) {
                    std::string day = col.substr(8);
                    columnToTimepoint[col] = static_cast<Timepoint>(std::stoi(day));
                }
            }
            isHeader = false;
            continue;
        }

        int patientID = std::stoi(tokens[0]);
        std::string argLabel = tokens[2];
        std::string mgeLabel = tokens[3];

        int argID = -1, mgeID = -1;
        for (const auto& [id, name] : argIdMap)
            if (name == argLabel) { argID = id; break; }
        for (const auto& [id, name] : mgeIdMap)
            if (name == mgeLabel) { mgeID = id; break; }

        if (argID == -1 || mgeID == -1) continue;

        for (size_t i = 4; i < tokens.size(); ++i) {
            std::string colName = headers[i];
            if (columnToTimepoint.count(colName) && ((tokens[i] == "1")||(tokens[i] == "2"))) {
                Timepoint tp = columnToTimepoint[colName];

                bool requiresSNPConfirmation = false;

                // Check if this ARG is marked as requiring SNP confirmation
                auto it = argIDSNPConfirmation.find(argID);
                if (it != argIDSNPConfirmation.end()) {
                    requiresSNPConfirmation = it->second;
                }
                std::string argResistance = "Drugs";
                // If user wants to EXCLUDE SNP-confirmed ARGs, skip them
                if (includeSNPConfirmationARGs && requiresSNPConfirmation)
                    continue;
                
                // Determine ARG resistance type (e.g., Drugs, Metals, Biocides)
                auto resistanceIt = argResistanceMap.find(argID);
                if (resistanceIt != argResistanceMap.end()) {
                    argResistance = resistanceIt->second;
                }
                // Skip metals and biocides if requested
                if (excludeMetals && argResistance != "Drugs") {
                    continue;
                }

                Node argNode = {argID, true, tp, requiresSNPConfirmation};
                Node mgeNode = {mgeID, false, tp, false};

                graph.nodes.insert(argNode);
                graph.nodes.insert(mgeNode);

                addEdge(graph, argNode, mgeNode, true, patientID);
            }
        }
    }
}



/**
 * This function adds patient-specific temporal edges between nodes.
 * It creates directed edges ONLY between chronologically adjacent timepoints for the same gene within the same patient.
 * @param graph The graph to which temporal edges will be added.
 */
void addTemporalEdges(Graph& graph) {
    // Reconstruct which nodes belong to which patient from the colocalization edges.
    std::map<int, std::set<Node>> nodesByPatient;
    for (const auto& edge : graph.edges) {
        if (!edge.isColo) continue;
        for (int patientID : edge.individuals) {
            nodesByPatient[patientID].insert(edge.source);
            nodesByPatient[patientID].insert(edge.target);
        }
    }

    // For each patient, create their temporal edges.
    for (const auto& [patientID, nodeSet] : nodesByPatient) {
        // Group nodes for this specific patient by their ID and type (ARG/MGE).
        std::unordered_map<std::pair<int, bool>, std::vector<Node>> groupedNodes;
        for (const Node& node : nodeSet) {
            groupedNodes[{node.id, node.isARG}].push_back(node);
        }

        // For each gene group within this patient, sort by time and create chronological edges.
        for (auto const& [key, nodeGroup] : groupedNodes) {
            auto nodes = nodeGroup; // Make a mutable copy

            std::sort(nodes.begin(), nodes.end());

            // Create edges between consecutive timepoints.
            for (size_t i = 0; i < nodes.size() - 1; ++i) {
                const Node& sourceNode = nodes[i];
                const Node& targetNode = nodes[i + 1];
                // Add a non-colocalization edge
                addEdge(graph, sourceNode, targetNode, false, -1);
            }
        }
    }
}
