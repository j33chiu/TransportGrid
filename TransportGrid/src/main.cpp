#include <iostream>
#include <memory>
#include <iomanip>

#include "Util.h"
#include "Graph.h"
#include "Solver.h"

long double debug(std::vector<Package*> route, Truck* t) {
    long double cost = 0;
    Node* currentNode = t->getStartNode();
    for (Package* p : route) {
        cost += currentNode->distTo(p->getNode());
        currentNode = p->getNode();
    }
    cost += currentNode->distTo(t->getEndNode());
    return cost;
}

int main() {
    std::unique_ptr<Graph> g = std::make_unique<Graph>();
    std::unique_ptr<Solver> solver = std::make_unique<Solver>(g.get());

    // load map (graph info) and problem (deliveries, trucks info)
    Util util(g.get(), solver.get());
    util.readFakeMap();
    util.readProblem();
    solver->solve();


    Node* n1 = new Node(-1, 0, 0);
    Node* n2 = new Node(-2, 0, 0);
    Node* n3 = new Node(-3, 0, 0);
    Node* n4 = new Node(-4, 0, 0);
    std::vector<Node*> testVec;
    testVec.push_back(n1);
    testVec.push_back(n2);
    testVec.push_back(n3);
    testVec.push_back(n4);

    std::unordered_map<Node*, std::vector<Node*>> testMap;
    testMap[n1] = testVec;

    std::vector<Node*>& fromMap = testMap[n1];
    fromMap.clear();

    std::cout << testMap[n1].size() << std::endl;

    
    delete n1;
    delete n2;
    delete n3;
    delete n4;
    return 0;
}