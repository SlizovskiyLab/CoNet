#include <iostream>

/* Main entry point: parse arguments, load data, call functions */

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

/*
int main() {
    Graph g;

    Node n1{true, Timepoint::Pre, 1, true};
    Node n2{false, Timepoint::Day4, 3, false};
    g.nodes.insert(n1);
    g.nodes.insert(n2);

    Edge e1;
    e1.label = "colocalization";
    e1.individuals = {101, 102};

    Edge e2;
    e2.label = "temporal";
    e2.weight = 4;

    g.edges.insert(e1);
    g.edges.insert(e2);

    std::cout << "Graph has " << g.nodes.size() << " nodes and " << g.edges.size() << " edges.\n";
    return 0;
}
*/