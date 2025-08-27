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


static std::string getTimepointColor(const Timepoint& tp) {
    int timeValue = static_cast<int>(tp);
    if (timeValue == 1000) return "yellow";
    if (timeValue == 0)    return "red";
    if (timeValue > 0 && timeValue < 31) return "#99D2FF";
    if (timeValue > 30 && timeValue < 61) return "#4D9DFF";
    if (timeValue > 60)    return "#3A6EFF";
    return "green"; // fallback
}

auto timepointOrder = [](Timepoint tp) -> int {
    if (tp == Timepoint::Donor) return -1;       // Donor comes first
    if (tp == Timepoint::PreFMT) return 0;       // PreFMT next
    return static_cast<int>(tp);               
};

std::string getLabel(const Node& node) {
    std::string label = node.isARG ? getARGName(node.id) : getMGENameForLabel(node.id);
    label += "\n" + toString(node.timepoint);
    return label;
}


bool exportGraphToJsonSimple(const Graph& g, const std::string& outPathStr) {
    json j;
    j["nodes"] = json::array();
    j["links"] = json::array();

    std::unordered_set<Node> active_nodes;
    std::set<std::pair<Node, Node>> processedColoEdges;

    for (const Edge& edge : g.edges) {
        if (edge.source == edge.target) continue;

        active_nodes.insert(edge.source);
        active_nodes.insert(edge.target);

        std::string style;
        std::string color;
        double penwidth = 4.0;
        std::string type = "other";

        if (edge.isColo) {
            auto canon = std::minmax(edge.source, edge.target);
            if (processedColoEdges.count(canon)) continue;
            processedColoEdges.insert(canon);

            style = "solid";
            color = "#696969";
            type  = "colocalization";

            int count = static_cast<int>(edge.individuals.size());
            if (count > 1) penwidth = 4.0 + (count - 1) * 2.0;
            penwidth = std::min(10.0, penwidth);
        } 
        else if (!edge.isColo) {
            style = "dashed";
            type  = "temporal";

            int w = edge.weight;
            if (w > 1) penwidth = 4.0 + (w - 1) * 2.0;
            penwidth = std::min(10.0, penwidth);

            Timepoint src_tp = edge.source.timepoint;
            Timepoint tgt_tp = edge.target.timepoint;

            bool src_is_post = (src_tp != Timepoint::Donor && src_tp != Timepoint::PreFMT);
            bool tgt_is_post = (tgt_tp != Timepoint::Donor && tgt_tp != Timepoint::PreFMT);

            if (src_tp == Timepoint::Donor && tgt_tp == Timepoint::PreFMT)      color = "#006400";
            else if (src_tp == Timepoint::Donor && tgt_is_post)                 color = "#4B0082";
            else if (src_tp == Timepoint::PreFMT && tgt_is_post)                color = "orange";
            else                                                                color = "black";
        } 
        else {
            style = "solid";
            color = "#808080";
        }

        j["links"].push_back({
            {"source", getNodeName(edge.source)},
            {"target", getNodeName(edge.target)},
            {"individualCount", static_cast<int>(edge.individuals.size())},
            {"style", style},
            {"color", color},
            {"penwidth", penwidth},
            {"isColo", edge.isColo},
            {"type", type}
        });
    }

    for (const Node& n : active_nodes) {
        std::string shape;
        if (n.isARG) {
            shape = "circle";
        } else {
            std::string groupName = getMGEGroupName(n.id);
            shape = getMGEGroupShape(groupName);
        }

        j["nodes"].push_back({
            {"id",        getNodeName(n)},              // match DOT's nodeName
            {"label",     getLabel(n)},
            {"isARG",     n.isARG},
            {"timepoint", static_cast<int>(n.timepoint)},
            {"color",     getTimepointColor(n.timepoint)},
            {"shape",     shape}
        });
    }

    std::ofstream out(outPathStr);
    if (!out) {
        std::cerr << "[exportGraphToJsonSimple] Cannot open " << outPathStr << " for write\n";
        return false;
    }
    out << j.dump(2) << '\n';

    std::cerr << "[exportGraphToJsonSimple] Wrote nodes=" << j["nodes"].size()
              << " links=" << j["links"].size()
              << " to " << outPathStr << "\n";
    return true;
}


bool exportParentGraphToJson(const Graph& g, const std::string& outPathStr, bool showLabels) {
    json j;
    j["nodes"] = json::array();
    j["links"] = json::array();

    struct ParentNodeInfo {
        std::string name;
        Timepoint tp;
        int argId;
        int mgeId;
    };

    int colocCounter = 0;
    std::map<std::tuple<int,int,Timepoint>, std::string> uniqueParents;
    std::map<std::pair<int,int>, std::vector<ParentNodeInfo>> colocMap;

    // --- create parent nodes ---
    for (const Edge& edge : g.edges) {
        if (!edge.isColo) continue;  // only colocalization edges define parent nodes

        const Node& argNode = edge.source.isARG ? edge.source : edge.target;
        const Node& mgeNode = edge.source.isARG ? edge.target : edge.source;

        int argId = argNode.id;
        int mgeId = mgeNode.id;
        Timepoint tp = argNode.timepoint;

        auto key = std::make_tuple(argId, mgeId, tp);
        if (!uniqueParents.count(key)) {
            std::string parentName = "Parent_" + std::to_string(++colocCounter);
            uniqueParents[key] = parentName;

            std::string color = getTimepointColor(tp);
            std::string groupName = getMGEGroupName(mgeId);
            std::string shape = getMGEGroupShape(groupName);
            std::string label = showLabels
                ? (getARGName(argId) + "\n" + getMGENameForLabel(mgeId) + "\n" + toString(tp))
                : "";

            // write JSON node
            j["nodes"].push_back({
                {"id", parentName},
                {"label", label},
                {"argId", argId},
                {"mgeId", mgeId},
                {"timepoint", static_cast<int>(tp)},
                {"color", color},
                {"shape", shape}
            });
        }

        auto pairKey = std::make_pair(argId, mgeId);
        colocMap[pairKey].push_back({uniqueParents[key], tp, argId, mgeId});
    }

    // --- add temporal edges between parent nodes ---
    for (auto& entry : colocMap) {
        auto& parentNodes = entry.second;

        std::sort(parentNodes.begin(), parentNodes.end(),
                  [&](const ParentNodeInfo& a, const ParentNodeInfo& b) {
                      return timepointOrder(a.tp) < timepointOrder(b.tp);
                  });

        for (size_t i = 0; i + 1 < parentNodes.size(); ++i) {
            if (parentNodes[i].tp == parentNodes[i+1].tp ||
                parentNodes[i].name == parentNodes[i+1].name) {
                continue;
            }

            Timepoint src_tp = parentNodes[i].tp;
            Timepoint tgt_tp = parentNodes[i+1].tp;

            bool src_is_post = (src_tp != Timepoint::Donor && src_tp != Timepoint::PreFMT);
            bool tgt_is_post = (tgt_tp != Timepoint::Donor && tgt_tp != Timepoint::PreFMT);

            std::string color;
            if (src_tp == Timepoint::Donor && tgt_tp == Timepoint::PreFMT)      color = "#006400";
            else if (src_tp == Timepoint::Donor && tgt_is_post)                 color = "#4B0082";
            else if (src_tp == Timepoint::PreFMT && tgt_is_post)                color = "orange";
            else                                                                color = "black";

            j["links"].push_back({
                {"source", parentNodes[i].name},
                {"target", parentNodes[i+1].name},
                {"style", "dashed"},
                {"color", color},
                {"penwidth", 5.0},
                {"isColo", false},
                {"type", "temporal"}
            });
        }
    }

    std::ofstream out(outPathStr);
    if (!out) {
        std::cerr << "[exportParentGraphToJson] Cannot open " << outPathStr << " for write\n";
        return false;
    }
    out << j.dump(2) << '\n';

    std::cerr << "[exportParentGraphToJson] Wrote parent-nodes=" << j["nodes"].size()
              << " links=" << j["links"].size()
              << " to " << outPathStr << "\n";
    return true;
}
