#pragma once

#include "Graph.h"
#include "Node.h"
#include "Package.h"

#include <unordered_set>

class Truck {

public:
    Truck(Node* startNode, Node* endNode);
    Truck(Node* startNode, Node* endNode, int peopleCap, double volumeCap);

    const int getNumPackages();
    std::vector<Package*> getPackages();
    void overwritePackages(std::vector<Package*> newPackages);

    void assignPackage(Package* package);

    Node* getStartNode();
    Node* getEndNode();

    const long double getCost();
    long double evaluateCost();

private:
    // counter for ID
    static int ID;

    int id;
    int peopleCap = 4;
    double volumeCap = 100;
    Node* startNode;
    Node* endNode;

    std::unordered_set<Package*> assignedPackages; // list of packages that must stay on this truck during solving. can move around in this truck's delivery order, but not to other trucks
    std::vector<Package*> packages;
    long double currentCost;

};