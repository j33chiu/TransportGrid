#include "Package.h"

Package::Package(Node* node) 
    : node{node}
{}

Package::Package(Node* node, int people, double volume)
    : node{node}, people{people}, volume{volume}
{}

const int Package::getId() const {
    return node->getId();
}

Node* Package::getNode() const {
    return node;
}

const int Package::getPeople() const {
    return people;
}

const double Package::getVolume() const {
    return volume;
}