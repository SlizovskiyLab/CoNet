#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include "graph.h"
#include <string>

// Public functions: filter subgraphs by ARG or MGE name
Graph filterGraphByARGName(const Graph& g, const std::string& argName);
Graph filterGraphByMGEName(const Graph& g, const std::string& mgeName);

#endif // GRAPH_UTILS_H
