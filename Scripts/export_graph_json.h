#pragma once
#include <string>
#include "graph.h"



// static inline const char* tpPretty(Timepoint tp);
// static inline std::string tpKey(Timepoint tp);
// static inline std::string d3NodeId(const Node& n);
// static inline std::string tpColorHex(const Timepoint& tp);
// static inline bool isTemporalEdge(const Edge& e);
// static inline std::string mgeShape(const std::string& groupName);
// static bool ensure_parent_dir(const fs::path& p);

static std::string makeNodeId(const Node& n);
bool exportGraphToJsonSimple(const Graph& g, const std::string& outPathStr);
