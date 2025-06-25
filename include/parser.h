#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <filesystem>
#include "graph.h"

namespace std {
    template <>
    struct hash<std::pair<int, bool>> {
        std::size_t operator()(const std::pair<int, bool>& p) const {
            return std::hash<int>()(p.first) ^ (std::hash<bool>()(p.second) << 1);
        }
    };
}


// Declares the function that will be defined in parser.cpp
void parseData(const std::filesystem::path& filename, Graph& graph);
void addEdge(Graph& graph, const Node& src, const Node& tgt, bool isColo, int patientID = -1);
void addTemporalEdges(Graph& graph);

#endif // PARSER_H