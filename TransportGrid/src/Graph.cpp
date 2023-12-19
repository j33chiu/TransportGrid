#include "Graph.h"

#include "PriorityQueue.h"

#include <string>
#include <unordered_set>
#include <iostream>
#include <iomanip>
#include <queue>

Graph::Graph() {

}

Node* Graph::createNode(const int id, const double x, const double y) {
    if (nodesMap.find(id) != nodesMap.end()) {
        // cannot create node as one already exists with the same id
        return nodesMap[id];
    }

    Node* n = new Node(id, x, y);
    nodesList.push_back(n);
    nodesMap[id] = n;
    return n;
}

Node* Graph::createNode(const double x, const double y) {
    return createNode(node_id_count++, x, y);
}

double Graph::add2WayEdge(Node* node1, Node* node2) {
    double dist = -1;
    if (std::find(node1->getNeighbours().begin(), node1->getNeighbours().end(), node2) == node1->getNeighbours().end()) {
        dist = node1->addNeighbour(node2);
    }
    if (std::find(node2->getNeighbours().begin(), node2->getNeighbours().end(), node1) == node2->getNeighbours().end()) {
        dist = node2->addNeighbour(node1);
    }
    return dist;
}

double Graph::add2WayEdge(int node1Id, int node2Id) {
    return add2WayEdge(nodesMap[node1Id], nodesMap[node2Id]);
}

void Graph::add2WayEdge(int node1Id, int node2Id, double dist, int speedLimit) {
    // speed limit is km/h
    // dist is m
    Node* node1 = nodesMap[node1Id];
    Node* node2 = nodesMap[node2Id];
    double time = ((dist / 1000.0) / speedLimit) * 3600.0; // seconds
    if (std::find(node1->getNeighbours().begin(), node1->getNeighbours().end(), node2) == node1->getNeighbours().end()) {
        node1->addNeighbour(node2, dist, time);
    }
    if (std::find(node2->getNeighbours().begin(), node2->getNeighbours().end(), node1) == node2->getNeighbours().end()) {
        node2->addNeighbour(node1, dist, time);
    }
}

double Graph::add1WayEdge(Node* from, Node* to) {
    return from->addNeighbour(to);
}

double Graph::add1WayEdge(int node1Id, int node2Id) {
    return add1WayEdge(nodesMap[node1Id], nodesMap[node2Id]);
}

void Graph::add1WayEdge(int sourceId, int destId, double dist, int speedLimit) {
    // speed limit is km/h
    // dist is m
    Node* source = nodesMap[sourceId];
    Node* dest = nodesMap[destId];
    if (std::find(source->getNeighbours().begin(), source->getNeighbours().end(), dest) == source->getNeighbours().end()) {
        double time = ((dist / 1000.0) / speedLimit) * 3600.0; // seconds
        source->addNeighbour(dest, dist, time);
    }
}

Node* Graph::findClosestNodeTo(double a, double b) {
    double shortestDist = NODE_MAX_VALUE;
    Node* closestNode = nullptr;

    Node tempNode(-1, a, b);
    for (Node* n : nodesList) {
        double dist = Node::dist(n, &tempNode);
        if (dist < shortestDist) {
            shortestDist = dist;
            closestNode = n;
        }
    }
    return closestNode;
}

double Graph::distBetween(Node* from, Node* to) {
    return from->distTo(to);
}

double Graph::distBetween(int fromId, int toId) {
    try {
        return distBetween(nodesMap.at(fromId), nodesMap.at(toId));
    } catch (const std::out_of_range& err) {
        std::cout << "given node ids for pathBetween do not exist in the graph" << std::endl;
        return -1;
    }
}

const std::deque<Node*> Graph::pathBetween(Node* from, Node* to) {
    double cost = distBetween(from, to);
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

    for (int i = 0; i < relevantNodes.size(); i++) {
        relevantNodes[i]->setLookupTableIdx(i);
    }
    costTable = std::vector<std::vector<double>>(relevantNodes.size(), std::vector<double>(relevantNodes.size(), -1));

    while (!relevantNodes.empty()) {
        Node* n1 = relevantNodes[0];
        n1->explore(relevantNodes);
        for (Node* n2 : relevantNodes) {
            int n1Id = n1->getId();
            int n2Id = n2->getId();
            double cost = n2->getCost();
            costTable[n1->getLookupTableIdx()][n2->getLookupTableIdx()] = cost;
            costTable[n2->getLookupTableIdx()][n1->getLookupTableIdx()] = cost;
        }
        relevantNodes.erase(relevantNodes.begin() + 0);
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
        const std::vector<double>& distances = currentNode->getDistances();
        for (int i = 0; i < neighbours.size(); i++) {
            Node* neighbourNode = neighbours[i];
            double neighbourDist = distances[i];
            double totalCost = currentNode->getCost() + neighbourDist;
            if (neighbourNode->getCost() > totalCost) {
                neighbourNode->setCost(totalCost);
                pq.push(neighbourNode);
                neighbourNode->setPreviousNode(currentNode);
            }
        }
    }
}

double Graph::getCost(Node* from, Node* to) const {
    try {
        //return getCost(from->getId(), to->getId());
        return costTable[from->getLookupTableIdx()][to->getLookupTableIdx()];
    } catch (const std::out_of_range& err) {
        std::cout << "unable to find node with id" << std::endl;
        return -1;
    }
}

double Graph::getCost(int fromId, int toId) const {
    try {
        return getCost(getNodeById(fromId), getNodeById(toId));
    } catch (const std::out_of_range& err) {
        std::cout << "unable to find node with id" << std::endl;
        return -1;
    }
}

Node* Graph::getNodeById(int id) const {
    try {
        return nodesMap.at(id);
    } catch(const std::out_of_range& err) {
        std::cout << "unable to find node with id " << id << std::endl;
        return nullptr;
    }
}

std::vector<Node*> Graph::getNodesById(std::vector<int>& ids) const {
    std::vector<Node*> out;
    for (int id : ids) {
        try {
            out.push_back(nodesMap.at(id));
        } catch(const std::out_of_range& err) {
            std::cout << "unable to find node with id " << id << std::endl;
            out.push_back(nullptr);
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

bool Graph::isClose(double a, double b) {
    return std::fabs(a - b) < e;
}

const double Graph::e = 10e-9;