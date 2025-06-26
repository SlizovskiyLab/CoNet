#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H
#include <map>
#include <set>
#include <string>
#include "graph.h"
#include "Timepoint.h"

bool isPostFMT(const std::string& tp);
void getColocalizationsPostFMTOnly(const Graph& graph, std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizationByTimepoint);
void getColocalizationsByIndPostFMTOnly(const Graph& graph, std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual);

bool isPreFMT(const std::string& tp);
void getColocalizationsPreFMTOnly(const Graph& graph, std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizationByTimepoint);
void getColocalizationsByIndPreFMTOnly(const Graph& graph, std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual);

void getColocalizationsByIndDonorAndPostFMT(const Graph& graph, std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual);
void getColocalizationsByIndPreFMTAndPostFMT(const Graph& graph, std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual);

void printConnectedMGE(const Graph& graph, const std::set<Edge>& edges, int mgeId, const std::string& mgeName);
#endif