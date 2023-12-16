#pragma once

#include "Node.h"

class Graph {
public:
    Graph();

    // add node
    Node* createNode(const int id, const long double x, const long double y);
    Node* createNode(const long double x, const long double y);

    // add bidirectional edge
    long double add2WayEdge(Node* node1, Node* node2);
    long double add2WayEdge(int node1Id, int node2Id);
    // add unidirectional edge
    long double add1WayEdge(Node* from, Node* to);
    long double add1WayEdge(int node1Id, int node2Id);

    // get shortest path to another node
    long double distBetween(Node* from, Node* to);
    long double distBetween(int fromId, int toId);
    const std::deque<Node*> pathBetween(Node* from, Node* to);
    const std::deque<Node*> pathBetween(int fromId, int toId);
    
    // get distances between all given given nodes, will add to costMap
    // use copy of vector
    void calculateDistanceTable(std::vector<Node*> relevantNodes);
    void explore(Node* startNode);
    // get lowest cost from cost table
    long double getCost(Node* from, Node* to);
    long double getCost(int fromId, int toId);

    Node* getNodeById(int id);
    std::vector<Node*> getNodesById(std::vector<int>& ids);

    void resetAllNodes();
    void deleteGraph();

    ~Graph();

    static bool isClose(long double a, long double b);

private:
    // default node
    Node defaultNode = Node(-1, 0, 0);

    // list of all nodes, only place in program that creates and destroys Node objects 
    // ensure new and delete used
    std::vector<Node*> nodesList;
    std::unordered_map<int, Node*> nodesMap;

    // cost map storing the cost to go from 1 node to another
    // keys are "<node 1 id>:<node 2 id>"
    const static std::string key(Node* n1, Node* n2);
    const static std::string key(int n1Id, int n2Id);
    std::unordered_map<std::string, long double> costMap;

    int node_id_count = 0;

    const static long double e;
};