#pragma once

#include <vector>
#include <deque>
#include <random>
#include <unordered_map>

const static double NODE_MAX_VALUE = 1000000000.0;

enum class NodeMode {
    XY,
    LATLONG
};

class Node {
public:
    Node(int id, double x, double y);

    // add a neighbour to the node
    double addNeighbour(Node* neighbour);
    void addNeighbour(Node* neighbour, double dist, double time);
    // remove a neighbour from the node
    void removeNeighbour(Node* neighbour);
    // reset the node (f, g, h values)
    void reset();

    // getters and setters:
    int getId() const;
    double getCost() const;
    void setCost(const double cost);
    void setResetHash(int resetHash);
    int getLookupTableIdx() const;
    void setLookupTableIdx(int i);

    const std::vector<Node*>& getNeighbours() const;
    const std::vector<double>& getDistances() const;
    void setPreviousNode(Node* previousNode);
    const Node* getPreviousNode() const;

    // get shortest path, distance to another node
    const std::deque<Node*> pathTo(Node* destination);
    double distTo(const Node* destination);
    // get distances to all given nodes
    void explore(const std::vector<Node*>& toFind);

    // constant function for distance between two nodes
    static double dist(const Node* a, const Node* b);

    // constant function for assembling path between nodes
    static std::deque<Node*> assemblePath(Node* from, Node* to); 

    static void setNodeCoordinatesMode(NodeMode mode);

protected:
    int id = -1;
    double x = 0;
    double y = 0;
    int lookupTableIdx = -1;
    static NodeMode mode;

private:
    double cost;
    double resetNum = NULL;

    double getRand();
    static double previousRandNum;

    // note that indices of these correspond to each other
    std::vector<Node*> neighbours;
    std::vector<double> distances;
    std::vector<double> times;      // if given speed limits

    // useful during graph traversal
    Node* previousNode;
};