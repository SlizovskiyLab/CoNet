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
    const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual,
    bool donorStatus,
    bool preFMTStatus,
    bool postFMTStatus,
    std::map<std::pair<int, int>, std::set<int>>& globalPairToPatients
) {
    std::map<std::pair<int, int>, std::set<Timepoint>> globalTimepoints;

    // Step 1: Accumulate all timepoints per (ARG, MGE) pair across patients
    for (const auto& [tuple, tps] : colocalizationByIndividual) {
        int argID     = std::get<1>(tuple);
        int mgeID     = std::get<2>(tuple);
        auto& timeSet = globalTimepoints[{argID, mgeID}];
        timeSet.insert(tps.begin(), tps.end());
    }

    // Step 2: Determine valid (ARG, MGE) pairs matching the scenario
    std::set<std::pair<int, int>> validPairs;
    for (const auto& [pair, tps] : globalTimepoints) {
        bool hasDonor   = std::any_of(tps.begin(), tps.end(), [](Timepoint tp) { return isDonor(tp); });
        bool hasPreFMT  = std::any_of(tps.begin(), tps.end(), [](Timepoint tp) { return isPreFMT(tp); });
        bool hasPostFMT = std::any_of(tps.begin(), tps.end(), [](Timepoint tp) { return isPostFMT(tp); });

        if ((hasDonor == donorStatus) &&
            (hasPreFMT == preFMTStatus) &&
            (hasPostFMT == postFMTStatus)) {
            validPairs.insert(pair);  // keep only pairs that match the timepoint pattern
        }
    }

    // Step 3: For valid (ARG, MGE) pairs, collect unique patient IDs
    for (const auto& [tuple, tps] : colocalizationByIndividual) {
        int patientID = std::get<0>(tuple);
        int argID     = std::get<1>(tuple);
        int mgeID     = std::get<2>(tuple);
        std::pair<int, int> pair = {argID, mgeID};

        if (validPairs.count(pair)) {
            // Count patient if scenario includes POST_FMT and patient has POST_FMT timepoint
            if (postFMTStatus) {
                if (std::any_of(tps.begin(), tps.end(), [](Timepoint tp) { return isPostFMT(tp); })) {
                    globalPairToPatients[pair].insert(patientID);
                }
            } else {
                // For donor-only, preFMT-only, or other combinations: count patient regardless of timepoint
                globalPairToPatients[pair].insert(patientID);
            }
        }
    }
    getTopARGMGEPairsByUniquePatients(globalPairToPatients, 10, "Aggregated by scenario");
}

void getTopARGMGEPairsByUniquePatients(
    const std::map<std::pair<int, int>, std::set<int>>& globalPairToPatients,
    int topN,
    const std::string& label
) {
    std::vector<std::pair<std::pair<int, int>, int>> freqList;
    for (const auto& [pair, patientSet] : globalPairToPatients) {
        freqList.emplace_back(pair, patientSet.size());
    }

    std::sort(freqList.begin(), freqList.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::cout << "\nTop ARG–MGE pairs by unique patients (" << label << "):\n";
    int countPrinted = 0;
    for (const auto& [pair, count] : freqList) {
        if (countPrinted++ >= topN) break;
        int argID = pair.first;
        int mgeID = pair.second;

        std::cout << "ARG: ";
        if (argIdMap.count(argID)) std::cout << getARGName(argID) << " (" << getARGGroupName(argID) << ")";
        else std::cout << "Unknown ARG ID " << argID;

        std::cout << ", MGE: ";
        if (mgeIdMap.count(mgeID)) std::cout << getMGEName(mgeID);
        else std::cout << "Unknown MGE ID " << mgeID;

        std::cout << ",Patients: " << count << "\n";
    }

    std::cout << "Total unique colocalizations: " << freqList.size() << "\n";
}

/********************************* Prominent Colocalizations by Frequency ********************************/
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


void getTopARGMGEPairsByFrequencyWODonor(
    const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizations, int topN) {
    
    std::map<std::pair<int, int>, int> countMap;

    for (const auto& [tuple, tps] : colocalizations) {
        // Skip if all timepoints are DONOR
        bool hasNonDonor = false;
        for (const auto& tp : tps) {
            if (tp != Timepoint::Donor) {
                hasNonDonor = true;
                break;
            }
        }
        if (!hasNonDonor) continue;

        int argID = std::get<1>(tuple);
        int mgeID = std::get<2>(tuple);
        countMap[{argID, mgeID}]++;
    }

    std::vector<std::pair<std::pair<int, int>, int>> freqList(countMap.begin(), countMap.end());
    std::sort(freqList.begin(), freqList.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::cout << "Top ARG–MGE pairs by frequency (excluding donor-only entries):\n";
    int countPrinted = 0;
    for (const auto& [pair, count] : freqList) {
        if (countPrinted >= topN) break;
        int argID = pair.first;
        int mgeID = pair.second;
        std::cout << "ARG: ";
        if (argIdMap.count(argID)) std::cout << getARGName(argID) << " (" << getARGGroupName(argID) << ")";
        else std::cout << "Unknown ARG ID " << argID;
        std::cout << ", MGE: ";
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





