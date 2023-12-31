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
    bool canRemove(Package* package) const;
    bool canRemove(const std::vector<Package*>& package) const;

    void setName(std::string name);
    std::string getName() const;
    int getId() const;

    Node* getStartNode();
    Node* getEndNode();

    const double getCost();
    double evaluateCost();

private:
    // counter for ID
    static int ID;

    int id;
    std::string truckName;
    int peopleCap = 4;
    double volumeCap = 100;
    Node* startNode;
    Node* endNode;

    std::unordered_set<Package*> assignedPackages; // list of packages that must stay on this truck during solving. can move around in this truck's delivery order, but not to other trucks
    std::vector<Package*> packages;
    double currentCost;

};