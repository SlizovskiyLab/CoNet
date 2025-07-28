#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include "graph.h"
#include <string>
#include <unordered_set>

Graph filterGraphByARGName(const Graph& g, const std::string& argName);
Graph filterGraphByMGEName(const Graph& g, const std::string& mgeName);
Graph filterGraphByMGEGroup(const Graph& g, const std::string& groupName);
Graph filterGraphByTimepoint(const Graph& g, const std::string& timepointCategory);

#endif