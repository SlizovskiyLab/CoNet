#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <set>
#include "graph.h"
#include "Timepoint.h"
#include "query_engine.h"
#include <algorithm>

/*************************************** PostFMT Only *************************************/
bool isPostFMT(Timepoint tp) {
    return toString(tp).find("post") != std::string::npos;
}

void getColocalizationsPostFMTOnly(const Graph& graph, std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizationByTimepoint) {
    std::map<std::pair<int, int>, std::set<Timepoint>> colocalizationsPostFMTOnly;
    for (const auto& [pair, tps] : colocalizationByTimepoint) {
        bool onlyPostFMT = std::all_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPostFMT(tp);
        });

        if (onlyPostFMT) {
            colocalizationsPostFMTOnly.insert({pair, tps});
            // std::cout << "ARG " << pair.first << ", MGE " << pair.second << " only in Post-FMT\n";
        }
    }
    std::cout << "Colocalizations Post-FMT Only size: " << colocalizationsPostFMTOnly.size() << "\n";
}


void getColocalizationsByIndPostFMTOnly(const Graph& graph, std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual) {
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationsPostFMTOnly;
    for (const auto& [tuple, tps] : colocalizationByIndividual) {
        // Check if all timepoints are post-FMT
        bool onlyPostFMT = std::all_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPostFMT(tp);
        });
        if (onlyPostFMT) {
            colocalizationsPostFMTOnly.insert({tuple, tps});
            // std::cout << "IND " << std::get<0>(tuple) << ", ARG " << std::get<1>(tuple) << ", MGE " << std::get<2>(tuple) << " only in Post-FMT\n";
        }
    }
    std::cout << "Colocalizations Post-FMT Only size: " << colocalizationsPostFMTOnly.size() << "\n";
    // std::cout << "Colocalizations Post-FMT Only: " << "\n";
    // for (const auto& [tuple, tps] : colocalizationsPostFMTOnly) {
    //     std::cout << "IND " << std::get<0>(tuple) << ", ARG " << std::get<1>(tuple) << ", MGE " << std::get<2>(tuple) << ": ";
    //     for (const auto& tp : tps) {
    //         std::cout << toString(tp) << " ";
    //     }
    //     std::cout << "\n";
    // }
}

/***************************************** PreFMT Only *********************************************/
bool isPreFMT(Timepoint tp) {
    return toString(tp).find("pre") != std::string::npos;
}

void getColocalizationsPreFMTOnly(const Graph& graph, std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizationByTimepoint) {
    std::map<std::pair<int, int>, std::set<Timepoint>> colocalizationsPreFMTOnly;
    for (const auto& [pair, tps] : colocalizationByTimepoint) {
        // Check if all timepoints are pre-FMT
        bool onlyPreFMT = std::all_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPreFMT(tp);
        });
        if (onlyPreFMT) {
            colocalizationsPreFMTOnly.insert({pair, tps});
            // std::cout << "ARG " << pair.first << ", MGE " << pair.second << " only in Pre-FMT\n";
        }
    }
    std::cout << "Colocalizations Pre-FMT Only size: " << colocalizationsPreFMTOnly.size() << "\n";
}

void getColocalizationsByIndPreFMTOnly(const Graph& graph, std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual) {
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationsPreFMTOnly;
    for (const auto& [tuple, tps] : colocalizationByIndividual) {
        // Check if all timepoints are pre-FMT
        bool onlyPreFMT = std::all_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPreFMT(tp);
        });
        if (onlyPreFMT) {
            colocalizationsPreFMTOnly.insert({tuple, tps});
            // std::cout << "IND " << std::get<0>(tuple) << "ARG " << std::get<1>(tuple) << ", MGE " << std::get<2>(tuple) << " only in Pre-FMT\n";
        }
    }
    std::cout << "Colocalizations Pre-FMT Only size: " << colocalizationsPreFMTOnly.size() << "\n";
    // std::cout << "Colocalizations Pre-FMT Only: " << "\n";
    // for (const auto& [tuple, tps] : colocalizationsPreFMTOnly) {
    //     std::cout << "IND " << std::get<0>(tuple) << ", ARG " << std::get<1>(tuple) << ", MGE " << std::get<2>(tuple) << ": ";
    //     for (const auto& tp : tps) {
    //         std::cout << toString(tp) << " ";
    //     }
    //     std::cout << "\n";
    // }
}



/***************************************** PreFMT & PostFMT both *********************************************/
void getColocalizationsByIndPreFMTAndPostFMT(const Graph& graph, std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual) {
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationsByIndPreFMTAndPostFMT;
    for (const auto& [tuple, tps] : colocalizationByIndividual) {
        // Check if timepoints contain both pre-FMT and post-FMT
        bool hasPreFMT = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPreFMT(tp);
        });
        bool hasPostFMT = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPostFMT(tp);
        });

        if (hasPreFMT && hasPostFMT) {
            colocalizationsByIndPreFMTAndPostFMT.insert({tuple, tps});
        }
    }
    std::cout << "Colocalizations Pre-FMT & Post-FMT size: " << colocalizationsByIndPreFMTAndPostFMT.size() << "\n";
}

/***************************************** Donor & PostFMT both *********************************************/
void getColocalizationsByIndDonorAndPostFMT(const Graph& graph, std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual) {
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> colocalizationsByIndDonorAndPostFMT;
    for (const auto& [tuple, tps] : colocalizationByIndividual) {
        // Check if timepoints contain donor and post-FMT
        bool hasDonor = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return tp == Timepoint::Donor;
        });
        bool hasPostFMT = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPostFMT(tp);
        });

        if (hasDonor && hasPostFMT) {
            colocalizationsByIndDonorAndPostFMT.insert({tuple, tps});
        }
    }
    std::cout << "Colocalizations Donor & Post-FMT size: " << colocalizationsByIndDonorAndPostFMT.size() << "\n";
}