#pragma once
#include <string>
#include <map>
#include "graph.h"

bool exportGraphToJsonSimple(const Graph& g, const std::string& outPathStr, const std::map<int, std::string>& patientToDiseaseMap);

bool exportParentGraphToJson(const Graph& g, const std::string& outPathStr, const std::map<int, std::string>& patientToDiseaseMap, bool showLabels = true);