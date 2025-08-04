#ifndef GRAPH_ANALYSIS_H
#define GRAPH_ANALYSIS_H
#include "graph.h"
#include <unordered_map>
#include <unordered_set>

void printGraphStatistics(const Graph& g, 
                          const std::unordered_map<Node, std::unordered_set<Node>>& adjacency);

void analyzeColocalizations(const Graph& g, 
                             const std::unordered_map<Node, std::unordered_set<Node>>& adjacency);

void analyzeColocalizationsCollectively(const Graph& g, 
                                          const std::unordered_map<Node, std::unordered_set<Node>>& adjacency);

void mostProminentEntities(const Graph& g);

#endif // GRAPH_ANALYSIS_H