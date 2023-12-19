#include "Node.h"

#include "PriorityQueue.h"

#include <iostream>
#include <string>
#include <unordered_set>

Node::Node(int id, double x, double y) {
    this->id = id;
    this->x = x;
    this->y = y;
}

double Node::addNeighbour(Node* neighbour) {
    neighbours.emplace_back(neighbour);
    double dist = Node::dist(this, neighbour);
    distances.emplace_back(dist);
    times.emplace_back(dist); // assume speed of 1 unit/s
    return dist;
}

void Node::addNeighbour(Node* neighbour, double dist, double time) {
    neighbours.emplace_back(neighbour);
    distances.emplace_back(dist);
    times.emplace_back(time);
}

void Node::removeNeighbour(Node* neighbour) {
    for (int i = 0; i < neighbours.size(); i++){
        if (neighbours[i] == neighbour) {
            neighbours.erase(neighbours.begin() + i);
            distances.erase(distances.begin() + i);
            break;
        }
    }
}

void Node::reset() {
    cost = NODE_MAX_VALUE;
    previousNode = nullptr;
}

int Node::getId() const {
    return id;
}

double Node::getCost() const {
    return cost;
}

void Node::setCost(const double cost) {
    this->cost = cost;
}

int Node::getLookupTableIdx() const {
    return lookupTableIdx;
}

void Node::setLookupTableIdx(int i) {
    lookupTableIdx = i;
}

const std::vector<Node*>& Node::getNeighbours() const {
    return neighbours;
}

const std::vector<double>& Node::getDistances() const {
    return distances;
}

void Node::setPreviousNode(Node* previousNode) {
    this->previousNode = previousNode;
}

const Node* Node::getPreviousNode() const {
    return previousNode;
}

const std::deque<Node*> Node::pathTo(Node* destination) {
    double finalCost = distTo(destination);
    return assemblePath(this, destination);
}

double Node::distTo(const Node* destination) {
    PriorityQueue<Node*, double> pq;
    
    cost = 0;
    pq.put(this, 0);

    double randomNum = getRand();

    while (previousRandNum == randomNum) {
        randomNum = getRand();
    }
    previousRandNum = randomNum;

    this->resetNum = randomNum;

    std::unordered_set<Node*> seen;
    seen.emplace(this);

    while (!pq.empty()) {
        Node* currentNode = pq.get();

        if (currentNode == destination) {
            break;
        }

        for (int i = 0; i < currentNode->neighbours.size(); i++) {
            Node* neighbourNode = currentNode->neighbours[i];
            double neighbourDist = currentNode->distances[i];
            if (neighbourNode->resetNum == NULL || neighbourNode->resetNum != randomNum) {
                neighbourNode->reset();
                neighbourNode->resetNum = randomNum;
            }
            
            // slows it down...
            // if (seen.find(neighbourNode) == seen.end()) {
            //     seen.emplace(neighbourNode);
            //     neighbourNode->reset();
            // }
            double totalCostG = currentNode->cost + neighbourDist;
            if (neighbourNode->cost > totalCostG) {
                neighbourNode->cost = totalCostG;
                double totalCostF = totalCostG + dist(neighbourNode, destination);
                pq.put(neighbourNode, totalCostF);
                neighbourNode->previousNode = currentNode;
            }
        }
    }
    return destination->cost;
}

// assumes everything in toFind is unique
void Node::explore(const std::vector<Node*>& toFind) {
    PriorityQueue<Node*, double> pq;
    
    cost = 0;
    pq.put(this, 0);

    double randomNum = getRand();
    while (previousRandNum == randomNum) {
        randomNum = getRand();
    }
    previousRandNum = randomNum;

    this->resetNum = randomNum;

    std::unordered_set<Node*> nodesToFind;
    for (Node* n : toFind) {
        nodesToFind.emplace(n);
    }

    while (!pq.empty()) {
        Node* currentNode = pq.get();

        // if everything found, break
        auto it = nodesToFind.find(currentNode);
        if (it != nodesToFind.end()) {
            nodesToFind.erase(it);
        }
        if (nodesToFind.empty()) break;

        for (int i = 0; i < currentNode->neighbours.size(); i++) {
            Node* neighbourNode = currentNode->neighbours[i];
            double neighbourDist = currentNode->distances[i];
            if (neighbourNode->resetNum == NULL || neighbourNode->resetNum != randomNum) {
                neighbourNode->reset();
                neighbourNode->resetNum = randomNum;
            }
            double totalCost = currentNode->cost + neighbourDist;
            if (neighbourNode->cost > totalCost) {
                neighbourNode->cost = totalCost;
                pq.put(neighbourNode, totalCost);
                neighbourNode->previousNode = currentNode;
            }
        }
    }
}

double Node::dist(const Node* a, const Node* b) {
    if (Node::mode == NodeMode::XY) {
        return sqrt(pow((a->x - b->x), 2) + pow((a->y - b->y), 2));
    } 
    return sqrt(pow((a->x - b->x), 2) + pow((a->y - b->y), 2));
}

std::deque<Node*> Node::assemblePath(Node* from, Node* to) {
    std::deque<Node*> path;
    double finalCost = to->cost;
    if (finalCost < NODE_MAX_VALUE) {
        Node* n = to;
        while (n != from) {
            path.push_front(n);
            n = n->previousNode;
        }
        path.push_front(n);
        return path;
    }
    std::cout << "unable to find path between nodes " << from->getId() << " and " << to->id << std::endl;
    return path;
}

void Node::setNodeCoordinatesMode(NodeMode mode) {
    Node::mode = mode;
}

double Node::getRand() {
    static std::random_device rd;
    static std::mt19937 gen = std::mt19937(rd());
    static std::uniform_real_distribution<> dis = std::uniform_real_distribution<>(0.0, 1000.0);
    return dis(gen);
}

double Node::previousRandNum = NULL;
NodeMode Node::mode = NodeMode::XY;