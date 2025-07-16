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
#include "id_maps.h"
#include <algorithm>

/************************************************************************************************/
bool isPostFMT(Timepoint tp) {
    return toString(tp).find("post") != std::string::npos;
}

bool isPreFMT(Timepoint tp) {
    return toString(tp).find("pre") != std::string::npos;
}

bool isDonor(Timepoint tp) {
    return toString(tp).find("donor") != std::string::npos;
}

/********************************* Patientwise Colocalizations ********************************/
void getPatientwiseColocalizationsByCriteria(
    const Graph& graph,
    const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual,
    bool donorStatus,
    bool preFMTStatus,
    bool postFMTStatus,
    const std::string& label
) {
    std::map<std::tuple<int, int, int>, std::set<Timepoint>> filteredColocs;

    for (const auto& [tuple, tps] : colocalizationByIndividual) {
        bool hasDonor = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return tp == Timepoint::Donor;
        });
        bool hasPreFMT = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPreFMT(tp);
        });
        bool hasPostFMT = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPostFMT(tp);
        });

        // Match against provided pattern
        if ((hasDonor == donorStatus) && (hasPreFMT == preFMTStatus) && (hasPostFMT == postFMTStatus)) {
            filteredColocs.insert({tuple, tps});
        }
    }

    std::cout << "Colocalizations (" << label << "): " << filteredColocs.size() << "\n";
    getTopARGMGEPairsByFrequency(filteredColocs, 10);
}

/***************************************** Colocalizations ****************************************/
void getColocalizationsByCriteria(
    const Graph& graph,
    const std::map<std::pair<int, int>, std::multiset<Timepoint>>& colocalizationByTimepoint,
    bool donorStatus,
    bool preFMTStatus,
    bool postFMTStatus,
    const std::string& label
) {
    std::map<std::pair<int, int>, std::multiset<Timepoint>> filteredColocs;

    for (const auto& [pair, tps] : colocalizationByTimepoint) {
        bool hasDonor = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isDonor(tp);
        });
        bool hasPreFMT = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPreFMT(tp);
        });
        bool hasPostFMT = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) {
            return isPostFMT(tp);
        });

        // Match against provided pattern
        if ((hasDonor == donorStatus) && (hasPreFMT == preFMTStatus) && (hasPostFMT == postFMTStatus)) {
            filteredColocs.insert({pair, tps});
        }
    }

    std::cout << "Colocalizations (" << label << "): " << filteredColocs.size() << "\n";
    getTopARGMGEPairsByFrequencyGlobally(filteredColocs, 10, false);
}


/********************************* Prominent Colocalizations by Frequency ********************************/

// exclude donor timepoints - handle through a boolean parameter

void getTopARGMGEPairsByFrequencyGlobally(
    const std::map<std::pair<int, int>, std::multiset<Timepoint>>& colocalizations,int topN, bool excludeDonor) {
    std::map<std::pair<int, int>, int> countMap;
    // Count frequency of each (ARG, MGE) pair
    for (const auto& [pair, tps] : colocalizations) {
        for (const Timepoint& tp : tps) {
            countMap[pair]++;
        }
    }
    // Convert map to vector for sorting
    std::vector<std::pair<std::pair<int, int>, int>> freqList(countMap.begin(), countMap.end());
    std::sort(freqList.begin(), freqList.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    std::cout << "Top ARG–MGE pairs by frequency:\n";
    int countPrinted = 0;
    for (const auto& [pair, count] : freqList) {
        if (countPrinted >= topN) break;

        int argID = pair.first;
        int mgeID = pair.second;

        std::cout << "ARG: ";
        if (argIdMap.count(argID))
            std::cout << getARGName(argID) << " (" << getARGGroupName(argID) << ")";
        else
            std::cout << "Unknown ARG ID " << argID;
        std::cout << ", MGE: ";
        if (mgeIdMap.count(mgeID))
            std::cout << getMGEName(mgeID);
        else
            std::cout << "Unknown MGE ID " << mgeID;
        std::cout << ", Count: " << count << "\n";
        countPrinted++;
    }

    std::cout << "Total unique ARG–MGE pairs: " << freqList.size() << "\n";
}


void getTopARGMGEPairsByFrequency(
    const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizations, int topN) {
    std::map<std::pair<int, int>, int> countMap;

    for (const auto& [tuple, tps] : colocalizations) {
        int argID = std::get<1>(tuple);
        int mgeID = std::get<2>(tuple);
        countMap[{argID, mgeID}]++;
    }

    std::vector<std::pair<std::pair<int, int>, int>> freqList(countMap.begin(), countMap.end());
    std::sort(freqList.begin(), freqList.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::cout << "Top ARG–MGE pairs by frequency:\n";
    int countPrinted = 0;
    for (const auto& [pair, count] : freqList) {
        if (countPrinted >= topN) break;
        int argID = pair.first;
        int mgeID = pair.second;
        std::cout << "ARG: " ;
        if (argIdMap.count(argID)) std::cout <<  getARGName(argID) << " (" << getARGGroupName(argID) << ")";
        else std::cout << "Unknown ARG ID " << argID;
        std::cout << ", MGE: " ;
        if (mgeIdMap.count(mgeID)) std::cout << getMGEName(mgeID);
        else std::cout << "Unknown MGE ID " << mgeID;
        std::cout << ", Count: " << count << "\n";
        countPrinted++;
    }

    std::cout << "Total unique ARG–MGE pairs: " << freqList.size() << "\n";
}


/***************************************** Connected MGEs *********************************************/
void getConnectedMGEs(const Graph& graph, int argID) {
    std::unordered_set<int> connectedMGEs;

    for (const Edge& edge : graph.edges) {
        if (!edge.isColo) continue;
        if (edge.source.isARG && edge.source.id == argID && !edge.target.isARG) {
            connectedMGEs.insert(edge.target.id);
        } else if (edge.target.isARG && edge.target.id == argID && !edge.source.isARG) {
            connectedMGEs.insert(edge.source.id);
        }
    }

    std::cout << "ARG ID " << argID << " is connected to MGE IDs:\n";
    for (int mge : connectedMGEs) {
        std::cout << "  - MGE " << mge;
        if (mgeIdMap.count(mge)) std::cout << " (" << mgeIdMap.at(mge) << ")";
        std::cout << "\n";
    }
}