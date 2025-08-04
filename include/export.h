#ifndef EXPORT_H
#define EXPORT_H

#include <string>
#include "graph.h"

void exportToDot(const Graph& g, const std::string& filename, bool showLabels = true);
void exportParentTemporalGraphDot(const Graph& g, const std::string& filename, bool showLabels=true);
#endif