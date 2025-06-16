#ifndef EXPORT_H
#define EXPORT_H

#include <string>
#include "graph.h"

void exportToDot(const Graph& g, const std::string& filename, int max_nodes = 200, int max_edges = 500);


#endif