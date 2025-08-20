// write_graph_json_main.cpp
#include <iostream>
#include "../include/graph.h"

// declare the function
bool exportGraphToJson(const Graph& g, const std::string& path);

int main() {
    // --- build a tiny sample graph (replace with your real graph) ---
    Graph g;

    Node a{42, true,  Timepoint::Donor,  false};
    Node b{42, true,  Timepoint::PreFMT, false};
    Node c{7,  false, Timepoint::PreFMT, true};

    g.nodes.insert(a);
    g.nodes.insert(b);
    g.nodes.insert(c);

    Edge e1{a, b, false, {}, 1}; // temporal
    Edge e2{b, c, true,  {}, 2}; // colocalization

    g.edges.insert(e1);
    g.edges.insert(e2);

    if (exportGraphToJson(g, "viz/graph.json")) {
        std::cout << "Wrote viz/graph.json\n";
    } else {
        std::cerr << "Failed writing viz/graph.json\n";
        return 1;
    }
    return 0;
}
