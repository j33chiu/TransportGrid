#include "Package.h"

Package::Package(Node* node) 
    : node{node}
{}

Package::Package(Node* node, int people, double volume)
    : node{node}, people{people}, volume{volume}
{}

const int Package::getId() {
    return node->getId();
}

Node* Package::getNode() {
    return node;
}

const int Package::getPeople() {
    return people;
}

const double Package::getVolume() {
    return volume;
}