#pragma once

#include <vector>
#include <deque>
#include <random>
#include <unordered_map>

const static long double NODE_MAX_VALUE = 1000000000.0;
class Node {
public:
    Node(int id, long double x, long double y);

    // add a neighbour to the node
    long double addNeighbour(Node* neighbour);
    void addNeighbour(Node* neighbour, const long double dist);
    // remove a neighbour from the node
    void removeNeighbour(Node* neighbour);
    // reset the node (f, g, h values)
    void reset();

    // getters and setters:
    const int getId();
    const long double getCost();
    void setCost(const long double cost);
    void setResetHash(int resetHash);

    const std::vector<Node*>& getNeighbours();
    const std::vector<long double>& getDistances();
    void setPreviousNode(Node* previousNode);
    const Node* getPreviousNode();

    // get shortest path, distance to another node
    const std::deque<Node*> pathTo(Node* destination);
    long double distTo(const Node* destination);
    // get distances to all given nodes
    void explore(const std::vector<Node*>& toFind);

    // constant function for distance between two nodes
    const static long double dist(const Node* a, const Node* b);
    const static std::deque<Node*> assemblePath(Node* from, Node* to); 

protected:
    int id = -1;
    long double x = 0;
    long double y = 0;

private:
    long double cost;
    long double resetNum = NULL;

    long double getRand();
    static long double previousRandNum;

    // note that indices of these correspond to each other
    std::vector<Node*> neighbours;
    std::vector<long double> distances;

    // useful during graph traversal
    Node* previousNode;
};