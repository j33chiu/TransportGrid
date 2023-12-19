#include "Truck.h"

int Truck::ID = 0;

Truck::Truck(Node* startNode, Node* endNode) 
    : startNode{startNode}, endNode{endNode}, id{ID++} 
{}

Truck::Truck(Node* startNode, Node* endNode, int peopleCap, double volumeCap)
    : startNode{startNode}, endNode{endNode}, peopleCap{peopleCap}, volumeCap{volumeCap}, id{ID++} 
{}

const int Truck::getNumPackages() {
    return packages.size();
}

std::vector<Package*> Truck::getPackages() {
    return packages;
}

void Truck::overwritePackages(std::vector<Package*> newPackages) {
    packages = newPackages;
}

void Truck::assignPackage(Package* package) {
    assignedPackages.emplace(package);
    // add to packages if not in already
    if (std::find(packages.begin(), packages.end(), package) == packages.end()) {
        packages.push_back(package);
    }
}

bool Truck::canRemove(Package* package) const {
    return assignedPackages.find(package) == assignedPackages.end();
}

bool Truck::canRemove(const std::vector<Package*>& packages) const {
    for (Package* p : packages) {
        if (assignedPackages.find(p) != assignedPackages.end()) {
            return false;
        }
    }
    return true;
}

void Truck::setName(std::string name) {
    this->truckName = name;
}

std::string Truck::getName() const {
    return truckName;
}

int Truck::getId() const {
    return id;
}

Node* Truck::getStartNode() {
    return startNode;
}

Node* Truck::getEndNode() {
    return endNode;
}

const double Truck::getCost() {
    return currentCost;
}

double Truck::evaluateCost() {
    double cost = 0;
    Node* currentNode = startNode;
    for (Package* p : packages) {
        cost += currentNode->distTo(p->getNode());
        currentNode = p->getNode();
    }
    cost += currentNode->distTo(endNode);
    currentCost = cost;
    return currentCost;
}