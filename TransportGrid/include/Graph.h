#pragma once

#include "Node.h"

class Graph {
public:
    Graph();

    // add node
    Node* createNode(const int id, const double x, const double y);
    Node* createNode(const double x, const double y);

    // add bidirectional edge
    double add2WayEdge(Node* node1, Node* node2);
    double add2WayEdge(int node1Id, int node2Id);
    void add2WayEdge(int node1Id, int node2Id, double distance, int speedlimit);
    // add unidirectional edge
    double add1WayEdge(Node* from, Node* to);
    double add1WayEdge(int node1Id, int node2Id);
    void add1WayEdge(int sourceId, int destId, double distance, int speedlimit);

    Node* findClosestNodeTo(double a, double b);

    // get shortest path to another node
    double distBetween(Node* from, Node* to);
    double distBetween(int fromId, int toId);
    const std::deque<Node*> pathBetween(Node* from, Node* to);
    const std::deque<Node*> pathBetween(int fromId, int toId);
    
    // get distances between all given given nodes, will add to costMap
    // use copy of vector
    void calculateDistanceTable(std::vector<Node*> relevantNodes);
    void explore(Node* startNode);
    // get lowest cost from cost table
    double getCost(Node* from, Node* to) const;
    double getCost(int fromId, int toId) const;

    Node* getNodeById(int id) const;
    std::vector<Node*> getNodesById(std::vector<int>& ids) const;

    void resetAllNodes();
    void deleteGraph();

    ~Graph();

    static bool isClose(double a, double b);

private:
    // default node
    const Node defaultNode = Node(-1, 0, 0);

    // list of all nodes, only place in program that creates and destroys Node objects 
    // ensure new and delete used
    std::vector<Node*> nodesList;
    std::unordered_map<int, Node*> nodesMap;

    // cost map storing the cost to go from 1 node to another
    // keys are "<node 1 id>:<node 2 id>"
    const static std::string key(Node* n1, Node* n2);
    const static std::string key(int n1Id, int n2Id);
    std::vector<std::vector<double>> costTable;

    int node_id_count = 0;

    const static double e;
};