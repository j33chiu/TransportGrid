#include "Util.h"

#include <iostream>
#include <fstream>
#include <string>

std::vector<int> splitInt(std::string str, std::string delimit) {
    size_t pos = 0;
    std::string token;
    std::vector<int> v;
    while ((pos = str.find(delimit)) != std::string::npos) {
        token = str.substr(0, pos);
        v.push_back(std::stoi(token));
        str.erase(0, pos + delimit.length());
    }
    v.push_back(std::stoi(str));
    return v;
}

Util::Util(Graph* graph, Solver* solver) {
    this->graph = graph;
    this->solver = solver;
}

void Util::readFakeMap() {
    graph->deleteGraph();

    // read files
    /*
    x_min,x_max,y_min,y_max
    start_node_id
    num_nodes
    node_id,node_x,node_y
    ........
    num rows here = num_nodes
    ........
    num_edges
    n1_id,n2_id
    ........
    num rows here = num_edges
    ........
    delivery_id1,delivery_id2......
    */

    std::ifstream file;
    file.open("generatedMap.txt");
    if (file.is_open()) {
        std::string line;

        std::getline(file, line);
        std::vector<int> rangeLine = splitInt(line, ",");
        xMin = rangeLine[0];
        xMax = rangeLine[1];
        yMin = rangeLine[2];
        yMax = rangeLine[3];

        std::getline(file, line);
        std::vector<int> startNodeLine = splitInt(line, ",");
        int startNodeId = startNodeLine[0];

        std::getline(file, line);
        std::vector<int> numNodesLine = splitInt(line, ",");
        numNodes = numNodesLine[0];
        for (int i = 0; i < numNodes; i++) {
            std::getline(file, line);
            std::vector<int> nodeLine = splitInt(line, ",");
            int nodeId = nodeLine[0];
            int nodeX = nodeLine[1];
            int nodeY = nodeLine[2];
            graph->createNode(nodeId, nodeX, nodeY);
        }

        std::getline(file, line);
        std::vector<int> numEdgesLine = splitInt(line, ",");
        int numEdges = numEdgesLine[0];
        for (int i = 0; i < numEdges; i++) {
            std::getline(file, line);
            std::vector<int> edgeLine = splitInt(line, ",");
            int node1Id = edgeLine[0];
            int node2Id = edgeLine[1];
            graph->add2WayEdge(node1Id, node2Id);
        }

        std::getline(file, line);
        std::vector<int> deliveriesLine = splitInt(line, ",");
        std::vector<Node*> deliveryNodes = graph->getNodesById(deliveriesLine);
        solver->setPackages(deliveryNodes);

        Node* startNode = graph->getNodeById(startNodeId);

        file.close();
    }
    else {
        std::cout << "generatedMap.txt file not opened" << std::endl;
    }
}

void Util::readRealMap() {
    graph->deleteGraph();

    // read files
    /*
    structure:
    # comments
    # more comments
    numnodes
    numedges
    ...
    nodes: (id, lat, long)
    ...
    edges: (source id, dest id, length (m), street type, speed lim (km/h), direction (0-uni, 1-bi))
    */
}

void Util::writeFakeMap() {

}

void Util::writeRealMap() {

}

void Util::readProblem() {
    // read files
    /*
    num_trucks
    ........
    startNodeId endNodeId people cap volume cap
    ........
    delivery_id1 delivery_id2......
    */

    std::ifstream file;
    file.open("problem.txt");
    if (file.is_open()) {
        std::string line;

        std::getline(file, line);
        std::vector<int> numTrucksLine = splitInt(line, " ");
        int numTrucks = numTrucksLine[0];
        for (int i = 0; i < numTrucks; i++) {
            std::getline(file, line);
            std::vector<int> truckLine = splitInt(line, " ");
            Node* truckStart = graph->getNodeById(truckLine[0]);
            Node* truckEnd = graph->getNodeById(truckLine[1]);
            int truckPeopleCap = truckLine[2];
            double truckVolCap = truckLine[3];
            solver->addTruck(truckStart, truckEnd, truckPeopleCap, truckVolCap);
        }

        std::getline(file, line);
        std::vector<int> deliveriesLine = splitInt(line, " ");
        std::vector<Node*> deliveryNodes = graph->getNodesById(deliveriesLine);
        solver->setPackages(deliveryNodes);

        // TODO: if the package is assigned to a truck, do it here

        file.close();
    }
    else {
        std::cout << "problem.txt file not opened" << std::endl;
    }
}

void Util::writeProblem() {

}