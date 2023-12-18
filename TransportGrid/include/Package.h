#pragma once

#include "Node.h"

class Package {
public: 
    Package(Node* node);
    Package(Node* node, int people, double volume);

    const int getId() const;
    Node* getNode() const;
    const int getPeople() const;
    const double getVolume() const;

private:
    Node* node;
    double volume = 0.0;
    int people = 0;

};