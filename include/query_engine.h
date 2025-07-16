#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H
#include <map>
#include <set>
#include <string>
#include "graph.h"
#include "Timepoint.h"

bool isPostFMT(const std::string& tp);
bool isPreFMT(const std::string& tp);
bool isDonor(const std::string& tp);

void getPatientwiseColocalizationsByCriteria(
    const Graph& graph,
    const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual,
    bool donorStatus,
    bool preFMTStatus,
    bool postFMTStatus,
    const std::string& label
);
void getColocalizationsByCriteria(
    const Graph& graph,
    const std::map<std::pair<int, int>, std::multiset<Timepoint>>& colocalizationByTimepoint,
    bool donorStatus,
    bool preFMTStatus,
    bool postFMTStatus,
    const std::string& label
);

void getTopARGMGEPairsByFrequencyGlobally(const std::map<std::pair<int, int>, std::multiset<Timepoint>>& colocalizations, int topN = 10, bool excludeDonor = false);
void getTopARGMGEPairsByFrequency(const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizations, int topN = 10);
void getConnectedMGE(const Graph& graph, const std::set<Edge>& edges, int mgeId, const std::string& mgeName);
#endif