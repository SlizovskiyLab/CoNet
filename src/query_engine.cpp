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
#include <fstream>
#include <iostream>

std::string getMGEGroupName(int id);

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
    getTopARGMGEPairsByFrequency(filteredColocs);
}

/***************************************** Colocalizations ****************************************/

void getColocalizationsByCriteria(
    const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual,
    bool donorStatus,
    bool preFMTStatus,
    bool postFMTStatus,
    std::map<std::pair<int, int>, std::set<int>>& globalPairToPatients
) {
    // Step 1: Determine globally valid (ARG, MGE) pairs
    std::map<std::pair<int, int>, std::set<Timepoint>> globalTimepoints;

    for (const auto& [tuple, tps] : colocalizationByIndividual) {
        int argID = std::get<1>(tuple);
        int mgeID = std::get<2>(tuple);
        globalTimepoints[{argID, mgeID}].insert(tps.begin(), tps.end());
    }

    std::set<std::pair<int, int>> validPairs;
    for (const auto& [pair, tps] : globalTimepoints) {
        bool hasDonor   = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp){ return isDonor(tp); });
        bool hasPreFMT  = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp){ return isPreFMT(tp); });
        bool hasPostFMT = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp){ return isPostFMT(tp); });

        if (hasDonor == donorStatus &&
            hasPreFMT == preFMTStatus &&
            hasPostFMT == postFMTStatus) {
            validPairs.insert(pair);
        }
    }

    // Step 2: Count patients per valid (ARG, MGE) pair only if their individual timepoints match the scenario
    for (const auto& [tuple, tps] : colocalizationByIndividual) {
        int patientID = std::get<0>(tuple);
        int argID     = std::get<1>(tuple);
        int mgeID     = std::get<2>(tuple);
        std::pair<int, int> pair = {argID, mgeID};

        if (!validPairs.count(pair)) continue;

        bool hasDonor   = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) { return isDonor(tp); });
        bool hasPreFMT  = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) { return isPreFMT(tp); });
        bool hasPostFMT = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) { return isPostFMT(tp); });

        if (hasDonor == donorStatus &&
            hasPreFMT == preFMTStatus &&
            hasPostFMT == postFMTStatus) {
            globalPairToPatients[pair].insert(patientID);
        }
    }

    getTopARGMGEPairsByUniquePatients(globalPairToPatients, 10, "Filtered by scenario");
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

    std::cout << "\nTop colocalizations by unique patients (" << label << "):\n";
    int countPrinted = 0;
    for (const auto& [pair, count] : freqList) {
        if (topN > 0 && countPrinted >= topN) break;
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
    const std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizations,
    int topN // default: print all
) {
    std::map<std::pair<int, int>, int> countMap;

    // Count (ARG_ID, MGE_ID) occurrences
    for (const auto& [tuple, tps] : colocalizations) {
        int argID = std::get<0>(tuple); // Correct index: ARG is first in tuple
        int mgeID = std::get<1>(tuple); // MGE is second in tuple
        countMap[{argID, mgeID}]++;
    }

    // Convert to vector for sorting
    std::vector<std::pair<std::pair<int, int>, int>> freqList(countMap.begin(), countMap.end());
    std::sort(freqList.begin(), freqList.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::cout << "Top ARG–MGE pairs by frequency:\n";
    int countPrinted = 0;
    for (const auto& [pair, count] : freqList) {
        if (topN > 0 && countPrinted >= topN) break;

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
        if (topN > 0 && countPrinted >= topN) break;
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



static inline bool isPostBin1(Timepoint tp) { int v = static_cast<int>(tp); return v >= 1  && v <= 30; }
static inline bool isPostBin2(Timepoint tp) { int v = static_cast<int>(tp); return v >= 31 && v <= 60; }
static inline bool isPostBin3(Timepoint tp) { int v = static_cast<int>(tp); return v >= 61; }

// Highest bin wins if multiple present
static inline int postBinOf(const std::set<Timepoint>& s) {
    bool b1 = false, b2 = false, b3 = false;
    for (auto tp : s) {
        if (!b1 && isPostBin1(tp)) b1 = true;
        else if (!b2 && isPostBin2(tp)) b2 = true;
        else if (!b3 && isPostBin3(tp)) b3 = true;
        if (b1 && b2 && b3) break;
    }
    if (b3) return 3; // post_3 (61+)
    if (b2) return 2; // post_2 (31–60)
    if (b1) return 1; // post_1 (1–30)
    return 0;         // no post
}



// ---------- ARG,MGE, Donor/Pre/Post flags -> patient counts (disease-specific) ----------
void writeTemporalDynamicsCountsForDisease(
    const std::string& disease,
    std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual,
    const std::map<int, std::string>& patientToDiseaseMap
) {
    // (ARG, MGE, DonorFlag, PreFlag, PostFlag) -> patient count
    std::map<std::tuple<int,int,int,int,int>, int> comboCounts;
    
    for (const auto& [key, tps] : colocalizationByIndividual) {
        const int patientID = std::get<0>(key);
        const int argID     = std::get<1>(key);
        const int mgeID     = std::get<2>(key);

        auto it = patientToDiseaseMap.find(patientID);
        if (it == patientToDiseaseMap.end() || it->second != disease) continue;

        bool hasDonor   = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) { return isDonor(tp); });
        bool hasPreFMT  = std::any_of(tps.begin(), tps.end(), [](const Timepoint& tp) { return isPreFMT(tp); });
        const int postBin = postBinOf(tps); // 0 (none), 1 (1–30), 2 (31–60), 3 (61+)

        comboCounts[std::make_tuple(argID, mgeID, hasDonor, hasPreFMT, postBin)] += 1;
    }


    const std::string filename = "output/Disease/" + disease + "_colocalizations" + ".csv";
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }

    out << "ARG_ID,MGE_ID,Donor,Pre,Post,PatientCount\n";
    for (const auto& [k, cnt] : comboCounts) {
        int argID, mgeID, donor, pre, post;
        std::tie(argID, mgeID, donor, pre, post) = k;
        out << getARGName(argID) << ','
            << getMGEName(mgeID) << ','
            << donor << ','
            << pre   << ','
            << post  << ','
            << cnt   << '\n';
    }
    out.close();
    std::cout << "Data written to " << filename << "\n";
}


/* Optional: write a CSV for every disease present in the map */
void writeAllDiseases_TemporalDynamicsCounts(
    std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual,
    const std::map<int, std::string>& patientToDiseaseMap
) {
    std::set<std::string> diseases;
    for (const auto& [pid, dz] : patientToDiseaseMap) diseases.insert(dz);
    for (const auto& dz : diseases)
        writeTemporalDynamicsCountsForDisease(dz, colocalizationByIndividual, patientToDiseaseMap);
}



// -------- write per-MGE-group temporal dynamics --------
void writeTemporalDynamicsCountsForMGEGroup(
    const std::map<std::tuple<int,int,int>, std::set<Timepoint>>& colocalizationByIndividual
) {
    // Outer map: MGE group -> inner stats
    std::unordered_map<std::string,
        std::map<std::tuple<int,int,int,int,int>, int>> groupedCounts;

    for (const auto& [key, tps] : colocalizationByIndividual) {
        // const int patientID = std::get<0>(key);
        const int argID     = std::get<1>(key);
        const int mgeID     = std::get<2>(key);

        const std::string group = getMGEGroupName(mgeID);

        const bool hasDonor = std::any_of(tps.begin(), tps.end(),
                            [](Timepoint tp){ return tp == Timepoint::Donor; });
        const bool hasPre   = std::any_of(tps.begin(), tps.end(),
                            [](Timepoint tp){ return isPreFMT(tp); });
        const bool hasPost  = std::any_of(tps.begin(), tps.end(),
                            [](Timepoint tp){ return isPostFMT(tp); });

        auto& comboCounts = groupedCounts[group];
        comboCounts[std::make_tuple(argID, mgeID,
                                    hasDonor ? 1 : 0,
                                    hasPre   ? 1 : 0,
                                    hasPost  ? 1 : 0)] += 1;
    }

    // Write one CSV per MGE group
    for (auto& [group, comboCounts] : groupedCounts) {
        const std::string filename = "output/MGE_Group/" + group + ".csv";
        std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "Error opening file: " << filename << "\n";
            continue;
        }
        out << "ARG_ID,MGE_ID,Donor,Pre,Post,PatientCount\n";
        for (const auto& [k, cnt] : comboCounts) {
            int argID, mgeID, donor, pre, post;
            std::tie(argID, mgeID, donor, pre, post) = k;
            out << getARGName(argID) << ',' << getMGEName(mgeID) << ','
                << donor << ',' << pre << ',' << post << ','
                << cnt << '\n';
        }
        out.close();
        std::cout << "Data written to " << filename << "\n";
    }
}
