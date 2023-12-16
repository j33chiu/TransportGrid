#include "Graph.h"

#include "PriorityQueue.h"

#include <string>
#include <unordered_set>
#include <iostream>
#include <iomanip>
#include <queue>

Graph::Graph() {

}

Node* Graph::createNode(const int id, const long double x, const long double y) {
    if (nodesMap.find(id) != nodesMap.end()) {
        // cannot create node as one already exists with the same id
        return nodesMap[id];
    }

    Node* n = new Node(id, x, y);
    nodesList.push_back(n);
    nodesMap[id] = n;
    return n;
}

Node* Graph::createNode(const long double x, const long double y) {
    return createNode(node_id_count++, x, y);
}

long double Graph::add2WayEdge(Node* node1, Node* node2) {
    long double dist = -1;
    if (std::find(node1->getNeighbours().begin(), node1->getNeighbours().end(), node2) == node1->getNeighbours().end()) {
        dist = node1->addNeighbour(node2);
    }
    if (std::find(node2->getNeighbours().begin(), node2->getNeighbours().end(), node1) == node2->getNeighbours().end()) {
        dist = node2->addNeighbour(node1);
    }
    return dist;
}

long double Graph::add2WayEdge(int node1Id, int node2Id) {
    return add2WayEdge(nodesMap[node1Id], nodesMap[node2Id]);
}

long double Graph::add1WayEdge(Node* from, Node* to) {
    return from->addNeighbour(to);
}

long double Graph::add1WayEdge(int node1Id, int node2Id) {
    return add1WayEdge(nodesMap[node1Id], nodesMap[node2Id]);
}

long double Graph::distBetween(Node* from, Node* to) {
    return from->distTo(to);
}

long double Graph::distBetween(int fromId, int toId) {
    try {
        return distBetween(nodesMap.at(fromId), nodesMap.at(toId));
    } catch (const std::out_of_range& err) {
        std::cout << "given node ids for pathBetween do not exist in the graph" << std::endl;
        return -1;
    }
}

const std::deque<Node*> Graph::pathBetween(Node* from, Node* to) {
    long double cost = distBetween(from, to);
    return Node::assemblePath(from, to);
}

const std::deque<Node*> Graph::pathBetween(int fromId, int toId) {
    try {
        return pathBetween(nodesMap.at(fromId), nodesMap.at(toId));
    } catch (const std::out_of_range& err) {
        std::cout << "given node ids for pathBetween do not exist in the graph" << std::endl;
        return std::deque<Node*>();
    }
}

void Graph::calculateDistanceTable(std::vector<Node*> relevantNodes) {
    std::sort(relevantNodes.begin(), relevantNodes.end());
    relevantNodes.erase(std::unique(relevantNodes.begin(), relevantNodes.end()), relevantNodes.end());

    int relevantNodesOriginalSize = relevantNodes.size();

    while (!relevantNodes.empty()) {
        Node* n1 = relevantNodes[0];
        n1->explore(relevantNodes);
        for (Node* n2 : relevantNodes) {
            int n1Id = n1->getId();
            int n2Id = n2->getId();
            long double cost = n2->getCost();
            costMap[key(n1Id, n2Id)] = cost;
            costMap[key(n2Id, n1Id)] = cost;
        }
        relevantNodes.erase(relevantNodes.begin() + 0);
    }

    long testSum = 0;
    for (long i = relevantNodesOriginalSize; i >= 1; i--) {
        testSum += (i*2);
        testSum--;
    }
    if (costMap.size() != testSum) {
        std::cout << "cost map issue" << std::endl;
    }
}

void Graph::explore(Node* startNode) {
    resetAllNodes();

    auto compare = [](Node* a, Node* b) {return a->getCost() > b->getCost();};
    std::priority_queue<Node*, std::vector<Node*>, decltype(compare)> pq(compare);

    startNode->setCost(0);
    pq.push(startNode);

    while (!pq.empty()) {
        Node* currentNode = pq.top();
        pq.pop();
        const std::vector<Node*>& neighbours = currentNode->getNeighbours();
        const std::vector<long double>& distances = currentNode->getDistances();
        for (int i = 0; i < neighbours.size(); i++) {
            Node* neighbourNode = neighbours[i];
            long double neighbourDist = distances[i];
            long double totalCost = currentNode->getCost() + neighbourDist;
            if (neighbourNode->getCost() > totalCost) {
                neighbourNode->setCost(totalCost);
                pq.push(neighbourNode);
                neighbourNode->setPreviousNode(currentNode);
            }
        }
    }
}

long double Graph::getCost(Node* from, Node* to) {
    try {
        return costMap.at(key(from->getId(), to->getId()));
    } catch (const std::out_of_range& err) {
        resetAllNodes();
        long double cost = distBetween(from, to);
        costMap[key(from->getId(), to->getId())] = cost;
        return cost;
    }
}

long double Graph::getCost(int fromId, int toId) {
    try {
        return getCost(nodesMap[fromId], nodesMap[toId]);
    } catch(const std::out_of_range& err) {
        std::cout << "unable to find node with id" << std::endl;
        return -1;
    }
}

Node* Graph::getNodeById(int id) {
    try {
        return nodesMap.at(id);
    } catch(const std::out_of_range& err) {
        std::cout << "unable to find node with id " << id << std::endl;
        return &defaultNode;
    }
}

std::vector<Node*> Graph::getNodesById(std::vector<int>& ids) {
    std::vector<Node*> out;
    for (int id : ids) {
        try {
            out.push_back(nodesMap.at(id));
        } catch(const std::out_of_range& err) {
            std::cout << "unable to find node with id " << id << std::endl;
            out.push_back(&defaultNode);
        }
    }
    return out;
}

void Graph::resetAllNodes() {
    for (Node* n : nodesList) {
        n->reset();
    }
}

void Graph::deleteGraph() {
    for (Node* n : nodesList) {
        delete n;
    }
    nodesList.clear();
    nodesMap.clear();
    costMap.clear();
    node_id_count = 0;
    //std::cout << "deleted graph" << std::endl;
}

const std::string Graph::key(Node* n1, Node* n2) {
    return key(n1->getId(), n2->getId());
}

const std::string Graph::key(int n1Id, int n2Id) {
    return std::to_string(n1Id) + ":" + std::to_string(n2Id);
}

Graph::~Graph() {
    deleteGraph();
}

bool Graph::isClose(long double a, long double b) {
    return std::fabs(a - b) < e;
}

const long double Graph::e = 10e-9;