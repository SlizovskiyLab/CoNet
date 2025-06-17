/* Traverse the graph based on earliest colocalizations */
#include <map>
#include <tuple>
#include <set>
#include "traversal.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

void traverseAdjacency(const Graph& graph, const std::unordered_map<Node, std::unordered_set<Node>>& adjacency, 
    std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizationTimeline) {
    for (const auto& node : graph.nodes) {
        // Skip if node is not an ARG
        if (!node.isARG) continue;

        for (const auto& neighbor : adjacency.at(node)) {
            // Skip if neighbor is not an MGE
            if (neighbor.isARG) continue;

            // ARG to MGE, record colocalization
            int argId = node.id;
            int mgeId = neighbor.id;
            Timepoint tp = node.timepoint;

            colocalizationTimeline[{argId, mgeId}].insert(tp);
        }
    }
}

void traverseGraph(const Graph& graph, 
    std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationByIndividual) {
    // std::unordered_map<Node, std::unordered_set<Node>> adjacency;

    for (const auto& edge : graph.edges) {
        if (!edge.isColo) continue;

        int arg_id = edge.source.isARG ? edge.source.id : edge.target.id;
        int mge_id = edge.source.isARG ? edge.target.id : edge.source.id;
        Timepoint tp = edge.source.timepoint; // or target.timepoint

        for (int ind_id : edge.individuals) {
            auto key = std::make_tuple(ind_id, arg_id, mge_id);
            colocalizationByIndividual[key].insert(tp);
        }
    }
}



/********************************************  Traverse by Time 1 ******************************************/
void findFirstOccurrence(const Graph& graph, std::unordered_map<Node, std::unordered_set<Node>>& adjacency,
                     std::map<std::pair<int, int>, Node>& firstOccurrence){
    for (const auto& edge : graph.edges) {
        if (!edge.isColo) continue;

        int arg = edge.source.isARG ? edge.source.id : edge.target.id;
        int mge = edge.source.isARG ? edge.target.id : edge.source.id;
        Node src = edge.source.isARG ? edge.source : edge.target;

        auto key = std::make_pair(arg, mge);

        if (!firstOccurrence.count(key) || src.timepoint < firstOccurrence[key].timepoint)
            firstOccurrence[key] = src;
    }
    // std::cout << "First occurrences: " << firstOccurrence.size() << "\n";
}


void bfsTemporal(const Node& start, const std::unordered_map<Node, std::unordered_set<Node>>& adjacency, std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizationTimeline){
    std::queue<Node> q;
    std::unordered_set<Node> visited;

    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        Node curr = q.front();
        q.pop();

        for (const Node& neighbor : adjacency.at(curr)) {
            if (neighbor.timepoint < curr.timepoint) continue; // Enforce forward-in-time
            if (visited.count(neighbor)) continue;

            visited.insert(neighbor);
            q.push(neighbor);

            if (curr.isARG != neighbor.isARG) {
                int arg = curr.isARG ? curr.id : neighbor.id;
                int mge = curr.isARG ? neighbor.id : curr.id;

                colocalizationTimeline[{arg, mge}].insert(neighbor.timepoint);
            }
        }
    }
}

void traverseTempGraph(const Graph& graph, std::unordered_map<Node, std::unordered_set<Node>>& adjacency,
                     std::map<std::pair<int, int>, Node>& firstOccurrence, std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizationsByTime){
    findFirstOccurrence(graph, adjacency, firstOccurrence);
    for (const auto& [key, startNode] : firstOccurrence) {
        bfsTemporal(startNode, adjacency, colocalizationsByTime);
    }
}



/************************************************  Traverse by Time 2  **********************************************/
void findFirstOccurrenceByInd(
    const Graph& graph,
    std::unordered_map<Node, std::unordered_set<Node>>& adjacency,
    std::map<std::tuple<int, int, int>, Node>& firstOccurrenceByInd // store the first occurrence of the ARG-MGE pair for the individual (e.g{[key], srcNode(ARG)})
) {
    for (const auto& edge : graph.edges) {
        if (!edge.isColo) continue;
        if (edge.source.isARG == edge.target.isARG) {
            std::cerr << "Warning: Invalid ARG-MGE edge with same type nodes (ID " << edge.source.id << ", " << edge.target.id << ")\n";
            continue;
        }
        
        int arg = edge.source.isARG ? edge.source.id : edge.target.id;
        int mge = edge.source.isARG ? edge.target.id : edge.source.id;
        Node src = edge.source.isARG ? edge.source : edge.target;

        for (int ind : edge.individuals) {
            auto key = std::make_tuple(ind, arg, mge);

            if (!firstOccurrenceByInd.count(key) || static_cast<int>(src.timepoint) < static_cast<int>(firstOccurrenceByInd[key].timepoint)) {
                firstOccurrenceByInd[key] = src;
            }
        }
    }
    std::cout << "First occurrences by individual found: " << firstOccurrenceByInd.size() << "\n";
}

void bfsTemporalByInd(
    const Node& start,
    const std::unordered_map<Node, std::unordered_set<Node>>& adjacency,
    const std::map<std::pair<Node, Node>, std::vector<Edge>>& edgeMap,
    int ind,
    std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationTimelineByInd
) {
    std::queue<Node> q;
    std::unordered_set<Node> visited;

    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        Node curr = q.front();
        q.pop();

        for (const Node& neighbor : adjacency.at(curr)) {
            //  only forward-in-time traversal
            if (neighbor.timepoint < curr.timepoint) continue;
            if (visited.count(neighbor)) continue;

            auto iterator = edgeMap.find({curr, neighbor});
            if (iterator == edgeMap.end()) continue;

            bool validStep = false;

            for (const Edge& edge : iterator->second) {
                if (!edge.individuals.count(ind)) continue;

                validStep = true; // Individual is involved in this edge

                if (edge.isColo && curr.isARG != neighbor.isARG) {
                    int arg = curr.isARG ? curr.id : neighbor.id;
                    int mge = curr.isARG ? neighbor.id : curr.id;
                    colocalizationTimelineByInd[{ind, arg, mge}].insert(neighbor.timepoint);
                }

                break; // One valid edge is enough
            }

            if (!validStep) continue;

            visited.insert(neighbor);
            q.push(neighbor);
        }
    }
}

void traverseGraphByInd(const Graph& graph, std::unordered_map<Node, std::unordered_set<Node>>& adjacency, const std::set<Edge>& edges,
                     std::map<std::tuple<int, int, int>, Node>& firstOccurrenceByInd, std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationTimelineByInd) {
    findFirstOccurrenceByInd(graph, adjacency, firstOccurrenceByInd);
    std::map<std::pair<Node, Node>, std::vector<Edge>> edgeMap;
    for (const auto& edge : graph.edges) {
        edgeMap[{edge.source, edge.target}].push_back(edge);
        edgeMap[{edge.target, edge.source}].push_back(edge); // undirected lookup
    }
    for (const auto& [key, startNode] : firstOccurrenceByInd) {
        auto [ind, arg, mge] = key;
        bfsTemporalByInd(startNode, adjacency, edgeMap, ind, colocalizationTimelineByInd);
    }
}



