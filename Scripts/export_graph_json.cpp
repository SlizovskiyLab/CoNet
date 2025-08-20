// export_graph_json.cpp
#include <fstream>
#include <string>
#include <filesystem>
#include <algorithm>
#include "../include/graph.h"
#include "../include/id_maps.h"     // getARGName, getMGENameForLabel, getMGEGroupName
#include "../include/export.h"      // toString(Timepoint) if you need it elsewhere
#include "../external/json.hpp"
#include "../include/graph.h"
#include "../include/Timepoint.h"

using nlohmann::json;
namespace fs = std::filesystem;

/* ---------- Helpers (mirroring your DOT logic) ---------- */

// // Pretty label bucket: Donor, PreFMT, PostFMT (any PostFMT_***)
// static inline const char* tpPretty(Timepoint tp) {
//     if (tp == Timepoint::Donor)  return "Donor";
//     if (tp == Timepoint::PreFMT) return "PreFMT";
//     // Treat anything else as PostFMT
//     return "PostFMT";
// }

// // Unique key piece for IDs (do NOT collapse)
// static inline std::string tpKey(Timepoint tp) {
//     return std::to_string(static_cast<int>(tp)); // stable + unique
// }

// // Node.id used in JSON (unique, stable)
// static inline std::string d3NodeId(const Node& n) {
//     return std::to_string(n.id) + "_" + (n.isARG ? "ARG" : "MGE") + "_" + tpKey(n.timepoint);
// }

// // Same color buckets you use for DOT
// static inline std::string tpColorHex(const Timepoint& tp) {
//     const int v = static_cast<int>(tp);
//     if (v == 1000) return "#ffff00";         // "yellow"
//     if (v == 0)    return "#ff0000";         // "red"
//     if (v > 0 && v < 31)  return "#99D2FF";
//     if (v > 30 && v < 61) return "#4D9DFF";
//     if (v > 60)           return "#3A6EFF";
//     return "#00ff00"; // "green" fallback
// }

// static inline bool isTemporalEdge(const Edge& e) {
//     return e.source.id == e.target.id &&
//            e.source.isARG == e.target.isARG &&
//            e.source.timepoint != e.target.timepoint;
// }

// // Shapes for MGEs (same mapping as your DOT)
// static inline std::string mgeShape(const std::string& groupName) {
//     if (groupName == "plasmid" || groupName == "Colicin_plasmid" || groupName == "Inc_plasmid") return "diamond";
//     if (groupName == "prophage")   return "hexagon";
//     if (groupName == "virus")      return "triangle";
//     if (groupName == "ICE" || groupName == "ICEberg") return "octagon";
//     if (groupName == "replicon")   return "parallelogram";
//     if (groupName == "likely IS/TE") return "trapezium";
//     return "box";
// }

// static bool ensure_parent_dir(const fs::path& p) {
//     std::error_code ec;
//     auto parent = p.parent_path();
//     if (parent.empty()) return true;
//     if (fs::exists(parent, ec)) return !ec;
//     return fs::create_directories(parent, ec);
// }

// bool exportGraphToJson(const Graph& g, const std::string& outPathStr) {
//     fs::path outPath(outPathStr);
//     if (!ensure_parent_dir(outPath)) {
//         std::cerr << "[exportGraphToJson] Failed to create parent dir for " << outPath << "\n";
//         return false;
//     }

//     // 1) Collect "active" nodes (those that appear in at least one edge)
//     std::unordered_set<Node> activeNodes;
//     activeNodes.reserve(g.edges.size() * 2 + 16);
//     for (const Edge& e : g.edges) {
//         activeNodes.insert(e.source);
//         activeNodes.insert(e.target);
//     }

//     // Early out if graph is empty
//     if (activeNodes.empty()) {
//         json empty;
//         empty["nodes"] = json::array();
//         empty["links"] = json::array();
//         std::ofstream out(outPath);
//         if (!out) {
//             std::cerr << "[exportGraphToJson] Cannot open " << outPath << " for write.\n";
//             return false;
//         }
//         out << empty.dump(2);
//         return true;
//     }

//     // 2) Deterministic order: copy to vector and sort using Node::operator<
//     std::vector<Node> nodesVec(activeNodes.begin(), activeNodes.end());
//     std::sort(nodesVec.begin(), nodesVec.end());

//     // 3) Build node records + a quick lookup from Node->id string
//     json j;
//     j["nodes"] = json::array();
//     j["links"] = json::array();

//     // Optional: stable numeric index map if you ever want indices
//     std::unordered_map<std::string, int> idToIndex;
//     idToIndex.reserve(nodesVec.size());

//     for (size_t i = 0; i < nodesVec.size(); ++i) {
//         const Node& n = nodesVec[i];

//         const std::string idStr   = d3NodeId(n);                   // unique string for D3
//         const std::string typeStr = n.isARG ? "ARG" : "MGE";
//         const std::string tpDisp  = tpPretty(n.timepoint);
//         const std::string color   = tpColorHex(n.timepoint);

//         std::string label = n.isARG ? getARGName(n.id) : getMGENameForLabel(n.id);
//         label += "\n";
//         label += tpDisp;

//         std::string shape = "circle";
//         std::string mgeGroup;
//         if (!n.isARG) {
//             mgeGroup = getMGEGroupName(n.id);
//             shape = mgeShape(mgeGroup);
//         }

//         idToIndex[idStr] = static_cast<int>(i);

//         j["nodes"].push_back({
//             {"id", idStr},                                   // primary key used by D3's forceLink().id()
//             {"index", static_cast<int>(i)},                  // optional numeric index (handy for debugging)
//             {"origId", n.id},                                // your original integer id
//             {"type", typeStr},                               // "ARG" | "MGE"
//             {"tp", tpDisp},                                  // "Donor" | "PreFMT" | "PostFMT" | ...
//             {"tpValue", static_cast<int>(n.timepoint)},      // numeric value for scales or sorting
//             {"label", label},                                // pretty label
//             {"color", color},                                // hex color for convenience
//             {"shape", shape},                                // front-end can map shapes
//             {"requiresSNPConfirmation", n.requiresSNPConfirmation},
//             {"groupName", n.isARG ? "" : mgeGroup}           // optional group for MGEs
//         });
//     }

//     // 4) Links
//     // - Colocalization: treat as undirected; dedupe using canonical (min,max) id pair
//     // - Temporal: directed; style/dash/color reflect your DOT logic
//     auto clampPen = [](double v) {
//         return std::max(1.0, std::min(10.0, v));
//     };

//     std::set<std::pair<std::string, std::string>> emittedUndirected;

//     for (const Edge& e : g.edges) {
//         const std::string s = d3NodeId(e.source);
//         const std::string t = d3NodeId(e.target);

//         // Skip edges pointing to nodes that weren't emitted, just in case
//         if (!idToIndex.count(s) || !idToIndex.count(t)) continue;

//         if (e.isColo) {
//             // undirected canonicalization by id string
//             auto canon = std::minmax(s, t);
//             if (emittedUndirected.count(canon)) continue;
//             emittedUndirected.insert(canon);

//             const int count = static_cast<int>(e.individuals.size());
//             const double pen = clampPen(4.0 + std::max(0, count - 1) * 2.0);

//             j["links"].push_back({
//                 {"source", canon.first},     // string IDs (D3: .id(d=>d.id))
//                 {"target", canon.second},
//                 {"type", "colocalization"},
//                 {"style", "solid"},
//                 {"color", "#696969"},
//                 {"penwidth", pen},
//                 {"count", count},
//                 {"dir", "both"}              // hint for UI if you want to render no arrows
//             });

//         } else if (isTemporalEdge(e)) {
//             const int w = e.weight;
//             const double pen = clampPen(4.0 + std::max(0, w - 1) * 2.0);

//             const Timepoint src = e.source.timepoint;
//             const Timepoint tgt = e.target.timepoint;

//             const bool src_is_post = (src != Timepoint::Donor && src != Timepoint::PreFMT);
//             const bool tgt_is_post = (tgt != Timepoint::Donor && tgt != Timepoint::PreFMT);

//             std::string color = "#000000";
//             if (src == Timepoint::Donor && tgt == Timepoint::PreFMT)       color = "#006400"; // donor->pre
//             else if (src == Timepoint::Donor && tgt_is_post)               color = "#4B0082"; // donor->post
//             else if (src == Timepoint::PreFMT && tgt_is_post)              color = "orange";  // pre->post
//             // else keep black

//             j["links"].push_back({
//                 {"source", s},
//                 {"target", t},
//                 {"type", "temporal"},
//                 {"style", "dashed"},
//                 {"color", color},
//                 {"penwidth", pen},
//                 {"weight", w}
//             });

//         } else {
//             // Fallback: treat as directed, neutral styling
//             j["links"].push_back({
//                 {"source", s},
//                 {"target", t},
//                 {"type", "other"},
//                 {"style", "solid"},
//                 {"color", "#808080"},
//                 {"penwidth", 4.0},
//                 {"weight", e.weight}
//             });
//         }
//     }

//     // 5) Write file
//     std::ofstream out(outPath);
//     if (!out) {
//         std::cerr << "[exportGraphToJson] Cannot open " << outPath << " for write.\n";
//         return false;
//     }
//     out << j.dump(2) << '\n';
//     return true;
// }



// Minimal, self-contained ID for D3 (unique per {id, isARG, timepoint})
static std::string makeNodeId(const Node& n) {
    return "N_" + std::to_string(n.id) +
           (n.isARG ? "_ARG_" : "_MGE_") + "_" +
           std::to_string(static_cast<int>(n.timepoint));
}

// Very simple exporter: nodes = all graph nodes, links = all edges
// No styling, no deduping, no parent dirs—just write the JSON.
bool exportGraphToJsonSimple(const Graph& g, const std::string& outPathStr) {
    json j;
    j["nodes"] = json::array();
    j["links"] = json::array();

    // Emit ALL nodes so the file isn't blank when there are no edges yet
    for (const Node& n : g.nodes) {
        j["nodes"].push_back({
            {"id",         makeNodeId(n)},                     // D3 key
            {"origId",     n.id},                              // optional, for debugging
            {"isARG",      n.isARG},                           // optional
            {"timepoint",  static_cast<int>(n.timepoint)}      // optional
        });
    }

    // Emit links with string IDs that match nodes[i].id
    for (const Edge& e : g.edges) {
        j["links"].push_back({
            {"source", makeNodeId(e.source)},
            {"target", makeNodeId(e.target)}
        });
    }

    std::ofstream out(outPathStr);
    if (!out) {
        std::cerr << "[exportGraphToJsonSimple] Cannot open " << outPathStr << " for write.\n";
        return false;
    }
    out << j.dump(2) << '\n';
    return true;
}
