#ifndef PARSER_H
#define PARSER_H

#include <string>
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
void parseData(const std::string& filename, Graph& graph);
void addTemporalEdges(Graph& graph);

#endif // PARSER_H