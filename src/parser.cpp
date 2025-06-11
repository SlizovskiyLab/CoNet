#include "../include/parser.h"
#include "../include/id_maps.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

/* Read input files (CSV), extract (individual, ARG, MGE, timepoint) data */

void parseData(const std::filesystem::path& filename, Graph& graph) {
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
            if (columnToTimepoint.count(colName) && tokens[i] == "1") {
                Timepoint tp = columnToTimepoint[colName];

                Node argNode = {argID, true, tp, false};
                Node mgeNode = {mgeID, false, tp, false};

                graph.nodes.insert(argNode);
                graph.nodes.insert(mgeNode);

                auto existing = std::find_if(graph.edges.begin(), graph.edges.end(), [&](const Edge& e) {
                    return e.isColo && e.source == argNode && e.target == mgeNode;
                });

                if (existing != graph.edges.end()) {
                    const_cast<Edge&>(*existing).individuals.insert(patientID);
                } else {
                    Edge edge = {argNode, mgeNode, true, {patientID}};
                    graph.edges.insert(edge);
                }
            }
        }
    }
}

void addTemporalEdges(Graph& graph) {
    std::unordered_map<std::pair<int, bool>, std::vector<Node>> groupedNodes;

    for (const Node& node : graph.nodes) {
        groupedNodes[{node.id, node.isARG}].push_back(node);
    }

    for (const auto& [key, nodeGroup] : groupedNodes) {
        for (size_t i = 0; i < nodeGroup.size(); ++i) {
            for (size_t j = i + 1; j < nodeGroup.size(); ++j) {
                const Node& a = nodeGroup[i];
                const Node& b = nodeGroup[j];
                if (a.timepoint != b.timepoint) {
                    graph.edges.insert({a, b, false});
                }
            }
        }
    }
}




