#pragma once
#include <string>
#include "graph.h"


bool exportGraphToJsonSimple(const Graph& g, const std::string& outPathStr);
bool exportParentGraphToJson(const Graph& g, const std::string& outPathStr, bool showLabels=true);