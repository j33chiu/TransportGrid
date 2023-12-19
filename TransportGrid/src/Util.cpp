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

std::vector<std::string> splitByDelimiter(std::string str, std::string delimit) {
    size_t pos = 0;
    std::string token;
    std::vector<std::string> v;
    while ((pos = str.find(delimit)) != std::string::npos) {
        token = str.substr(0, pos);
        v.push_back(token);
        str.erase(0, pos + delimit.length());
    }
    v.push_back(str);
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
        std::vector<int> numNodesLine = splitInt(line, ",");
        numNodes = numNodesLine[0];
        Node::setNodeCoordinatesMode(NodeMode::XY);
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

    std::ifstream file;
    file.open("ontario.txt");
    if (file.is_open()) {
        std::string line;

        std::getline(file, line);
        while (line.at(0) == '#') {
            std::getline(file, line);
        }

        std::vector<int> numNodesLine = splitInt(line, ",");
        numNodes = numNodesLine[0];

        std::getline(file, line);
        std::vector<int> numEdgesLine = splitInt(line, ",");
        int numEdges = numEdgesLine[0];

        Node::setNodeCoordinatesMode(NodeMode::LATLONG);
        for (int i = 0; i < numNodes; i++) {
            std::getline(file, line);
            std::vector<std::string> nodeLine = splitByDelimiter(line, " ");
            int nodeId = std::stoi(nodeLine[0]);
            double latitude = std::stod(nodeLine[1]);
            double longitude = std::stod(nodeLine[2]);
            graph->createNode(nodeId, latitude, longitude);
        }

        for (int i = 0; i < numEdges; i++) {
            std::getline(file, line);
            std::vector<std::string> edgeLine = splitByDelimiter(line, " ");
            int sourceId = std::stoi(edgeLine[0]);
            int destId = std::stoi(edgeLine[1]);
            double dist = std::stod(edgeLine[2]);
            std::string streetType = edgeLine[3];
            int speedLimit = std::stoi(edgeLine[4]);
            bool bidirectional = std::stoi(edgeLine[5]) == 1;

            if (bidirectional) {
                graph->add2WayEdge(sourceId, destId, dist, speedLimit);
            } else {
                graph->add1WayEdge(sourceId, destId, dist, speedLimit);
            }
        }

        file.close();
    }
    else {
        std::cout << "generatedMap.txt file not opened" << std::endl;
    }
}

void Util::writeFakeMap() {

}

void Util::writeRealMap() {

}

void Util::readFakeProblem() {
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

        file.close();
    }
    else {
        std::cout << "problem.txt file not opened" << std::endl;
    }
}

void Util::readRealProblem() {
    // read files
    /*
    num_trucks
    ........
    truckName startLat startLong endLat endLong peopleCap volumeCap
    ........
    num_deliveries
    ........
    deliveryName lat long (optional: truckName)
    ........
    */

    std::ifstream file;
    file.open("realProblem.txt");
    if (file.is_open()) {
        std::string line;

        std::getline(file, line);
        std::vector<int> numTrucksLine = splitInt(line, " ");
        int numTrucks = numTrucksLine[0];
        for (int i = 0; i < numTrucks; i++) {
            std::getline(file, line);
            std::vector<std::string> truckLine = splitByDelimiter(line, " ");
            std::string truckName = truckLine[0];
            double startLat = std::stod(truckLine[1]);
            double startLong = std::stod(truckLine[2]);
            double endLat = std::stod(truckLine[3]);
            double endLong = std::stod(truckLine[4]);
            int truckPeopleCap = std::stoi(truckLine[5]);
            double truckVolCap = std::stod(truckLine[6]);
                        
            Node* truckStart = graph->findClosestNodeTo(startLat, startLong);
            Node* truckEnd = graph->findClosestNodeTo(endLat, endLong);

            Truck* t = solver->addTruck(truckStart, truckEnd, truckPeopleCap, truckVolCap);
            t->setName(truckName);
        }

        solver->clearPackages();
        std::getline(file, line);
        std::vector<int> numDeliveriesLine = splitInt(line, " ");
        int numDeliveries = numDeliveriesLine[0];
        for (int i = 0; i < numDeliveries; i++) {
            std::getline(file, line);
            std::vector<std::string> deliveryLine = splitByDelimiter(line, " ");
            std::string deliveryName = deliveryLine[0];
            double latitude = std::stod(deliveryLine[1]);
            double longitude = std::stod(deliveryLine[2]);

            Node* deliveryNode = graph->findClosestNodeTo(latitude, longitude);
            Package* package = solver->addPackage(deliveryNode);
            if (deliveryLine.size() > 3) {
                // package is assigned
                std::string assignedTruck = deliveryLine[3];
                for (Truck* t : solver->getTrucks()) {
                    if (t->getName() == assignedTruck) {
                        t->assignPackage(package);
                        package->setAssignedTruckId(t->getId());
                        break;
                    }
                }
            }
        }

        file.close();
    }
    else {
        std::cout << "problem.txt file not opened" << std::endl;
    }
}

void Util::writeFakeProblem() {

}

void Util::writeRealProblem() {

}