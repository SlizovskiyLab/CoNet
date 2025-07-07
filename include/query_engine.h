#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H
#include <map>
#include <set>
#include <string>
#include "graph.h"
#include "Timepoint.h"

enum class Presence { Absent = 0, Present = 1, Any = 2 };

bool isPostFMT(const std::string& tp);
bool isPreFMT(const std::string& tp);
bool isDonor(const std::string& tp);

void getPatientwiseColocalizationsByCriteria(
    const Graph& graph,
    const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual,
    Presence donorStatus,
    Presence preFMTStatus,
    Presence postFMTStatus,
    const std::string& label
);
void getColocalizationsByCriteria(
    const Graph& graph,
    const std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizationByTimepoint,
    Presence donorStatus,
    Presence preFMTStatus,
    Presence postFMTStatus,
    const std::string& label
);

void getTopARGMGEPairsByFrequencyGlobally(const std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizations, int topN = 10);
void getTopARGMGEPairsByFrequency(const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizations, int topN = 10);
void getConnectedMGE(const Graph& graph, const std::set<Edge>& edges, int mgeId, const std::string& mgeName);
#endif