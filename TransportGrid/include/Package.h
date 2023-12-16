#pragma once

#include "Node.h"

class Package {
public: 
    Package(Node* node);
    Package(Node* node, int people, double volume);

    const int getId();
    Node* getNode();
    const int getPeople();
    const double getVolume();

private:
    Node* node;
    double volume = 0.0;
    int people = 0;

};