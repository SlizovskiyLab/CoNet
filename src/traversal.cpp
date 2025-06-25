/* Traverse the graph based on earliest colocalizations */
#include "../include/traversal.h"
#include "../include/graph.h"
#include "../include/Timepoint.h"
#include <map>
#include <tuple>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <queue>

// This function traverses the graph and collects colocalization timepoints for each ARG and MGE pair.
// It uses an adjacency map to represent the graph structure, allowing for efficient traversal of nodes and their neighbors based on the defined edges.
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


// The function builds a timeline of colocalizations for each individual, ARG, and MGE pair, allowing for further analysis of colocalization patterns over time.
// This allows for efficient tracking of colocalization events for each individual across different ARG and MGE pairs.
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



/********************************************  Traverse by Time 1 (not considering patients) ******************************************/
// This version considers only the first occurrence of colocalization between ARG and MGE
// It builds a timeline of colocalizations for each ARG and MGE pair, without individual
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


// This function performs a BFS-like traversal starting from the first occurrence node
// It explores the graph in a forward-in-time manner, collecting colocalization timepoints for the ARG and MGE pair.
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

// This function builds a timeline of colocalizations for each ARG and MGE pair
// It first finds the first occurrence of colocalization between ARG and MGE, then performs a BFS traversal starting from each first occurrence node to collect all colocalization timepoints.
void traverseTempGraph(const Graph& graph, std::unordered_map<Node, std::unordered_set<Node>>& adjacency,
                     std::map<std::pair<int, int>, Node>& firstOccurrence, std::map<std::pair<int, int>, std::set<Timepoint>>& colocalizationsByTime){
    findFirstOccurrence(graph, adjacency, firstOccurrence);
    for (const auto& [key, startNode] : firstOccurrence) {
        bfsTemporal(startNode, adjacency, colocalizationsByTime);
    }
}



/************************************************  Traverse by Time 2  **********************************************/
// This version considers individuals and their first occurrence of colocalization
// It builds a timeline of colocalizations for each individual, ARG, and MGE pair
void findFirstOccurrenceByInd(
    const Graph& graph,
    std::unordered_map<Node, std::unordered_set<Node>>& adjacency,
    std::map<std::tuple<int, int, int>, Node>& firstOccurrenceByInd // This will store the first occurrence of the ARG-MGE pair for the individual (e.g{[key], srcNode(ARG)} )
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



// This function performs a BFS-like traversal starting from the first occurrence node
// It explores the graph in a forward-in-time manner, collecting colocalization timepoints for the specified individual, ARG, and MGE pair.
void temporalTimelineTraversal(
    const Node& start,
    const std::unordered_map<Node, std::unordered_set<Node>>& adjacency,
    const std::map<std::pair<Node, Node>, std::vector<Edge>>& edgeMap,
    int ind, int arg, int mge,
    std::map<std::tuple<int, int, int>, std::set<Timepoint>>& colocalizationTimelineByInd
) {
    std::queue<Node> q;
    std::unordered_set<Node> visited;

    q.push(start);
    visited.insert(start);

    auto key = std::make_tuple(ind, arg, mge);
    colocalizationTimelineByInd[key].insert(start.timepoint);

    while (!q.empty()) {
        Node curr = q.front();
        q.pop();

        for (const Node& neighbor : adjacency.at(curr)) {
            if (neighbor.timepoint < curr.timepoint) continue;
            if (visited.count(neighbor)) continue;

            auto it = edgeMap.find({curr, neighbor});
            if (it == edgeMap.end()) continue;

            for (const Edge& edge : it->second) {
                if (!edge.isColo || !edge.individuals.count(ind)) continue;

                int this_arg = curr.isARG ? curr.id : neighbor.id;
                int this_mge = curr.isARG ? neighbor.id : curr.id;

                if (this_arg == arg && this_mge == mge) {
                    colocalizationTimelineByInd[key].insert(neighbor.timepoint);
                    visited.insert(neighbor);
                    q.push(neighbor);
                    break; // one valid edge is enough
                }
            }
        }
    }
}

// Builds an edge map for efficient lookup of edges between nodes.
// The adjacency map is used to represent the graph structure, allowing for efficient traversal of nodes and their neighbors based on the defined edges.
// The function iterates over the first occurrences of each ARG-MGE pair and performs a BFS traversal starting from each first occurrence node. During the traversal, it collects colocalization timepoints for the specified individual, ARG, and MGE pair, ensuring that only valid colocalizations are recorded.
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
        temporalTimelineTraversal(startNode, adjacency, edgeMap, ind, arg, mge, colocalizationTimelineByInd);
    }
}



