#include "SolutionState.h"

#include <iostream>
#include <iomanip>
#include <vector>

SolutionState::SolutionState(std::vector<Truck*>& trucks, 
        std::unordered_map<Truck*, std::vector<Package*>>& initialRouteMap, 
        std::unordered_map<Truck*, double>& initialRouteCostMap,
        Graph* graph,
        double wDist,
        double wTime) 
{
    double initialDist = 0;
    double initialLongest = 0;
    for (Truck* t : trucks) {
        routeMap[t] = std::vector<Package*>(initialRouteMap[t]);
        double c = initialRouteCostMap[t];
        routeCostMap[t] = c;
        initialDist += c;
        if (c > initialLongest) {
            initialLongest = c;
            longestTruckRoute = t;
        }
    }
    this->trucks = std::vector<Truck*>(trucks);
    totalDistance = initialDist;
    longestRouteDistance = initialLongest;
    this->wDist = wDist;
    this->wTime = wTime;
    updateOverallCost();
    this->graph = graph;

    gen = std::mt19937(rd());
    dis = std::uniform_real_distribution<>(0.0, 1.0);
    truckDis = std::uniform_int_distribution<>(0, trucks.size() - 1);
}

void SolutionState::setCurrentTemp(const double currentTemp) {
    this->currentTemp = currentTemp;
}

double SolutionState::getTotalDistance() {
    return totalDistance;
}

double SolutionState::getLongestRouteDistance() {
    return longestRouteDistance;
}

std::unordered_map<Truck*, std::vector<Package*>> SolutionState::getRouteMapCopy() {
    return routeMap;
}

std::unordered_map<Truck*, double> SolutionState::getRouteCostMapCopy() {
    return routeCostMap;
}

SolutionStateResult SolutionState::createNeighbour() {
    static std::vector<int> choiceTypes = {0, 1};
    static std::uniform_int_distribution<> choiceTypesDist(0, choiceTypes.size() - 1);
    SolutionStateResult result = SolutionStateResult::REDO;
    while (result == SolutionStateResult::REDO) {
        int choiceType = choiceTypesDist(gen);
        if (choiceType == 0) {
            result = createNeighbourShuffle();
        } else {
            result = createNeighbourTSP();
        }
    }
    return result;
}

SolutionStateResult SolutionState::createNeighbourShuffle() {
    static std::vector<int> choices = {0, 1};
    static std::uniform_int_distribution<> choiceDist(0, choices.size() - 1);
    int choice = choiceDist(gen);
    SolutionStateResult result = SolutionStateResult::REDO;
    if (choice == 0) {
        result = givePackage();
    } else if (choice == 1) {
        result = giveSubroute();
    }
    return result;
}

SolutionStateResult SolutionState::createNeighbourTSP() {
    static std::vector<int> choices = {0, 1, 2, 3};
    static std::uniform_int_distribution<> choiceDist(0, choices.size() - 1);
    int choice = choiceDist(gen);
    SolutionStateResult result = SolutionStateResult::REDO;
    if (choice == 0) {
        result = inverseSubroute();
    } else if (choice == 1) {
        result = swapPackages();
    } else if (choice == 2) {
        result = movePackage();
    } else if (choice == 3) {
        result = moveSubroute();
    }
    return result;
}

// simulated annealing helper functions
Node* SolutionState::getNodeBefore(const int idx, Truck* truck, const std::vector<Package*>& packages) const {
    int beforeIdx = idx - 1;
    if (beforeIdx < 0) return truck->getStartNode();
    return packages[beforeIdx]->getNode();
}

Node* SolutionState::getNodeAfter(const int idx, Truck* truck, const std::vector<Package*>& packages) const {
    int afterIdx = idx + 1;
    if (afterIdx >= packages.size()) return truck->getEndNode();
    return packages[afterIdx]->getNode();
}

int SolutionState::getNodeIdBefore(const int idx, Truck* truck, const std::vector<Package*>& packages) const {
    int beforeIdx = idx - 1;
    if (beforeIdx < 0) return truck->getStartNode()->getId();
    return packages[beforeIdx]->getId();
}

int SolutionState::getNodeIdAfter(const int idx, Truck* truck, const std::vector<Package*>& packages) const {
    int afterIdx = idx + 1;
    if (afterIdx >= packages.size()) return truck->getEndNode()->getId();
    return packages[afterIdx]->getId();
}

SolutionStateResult SolutionState::shouldAccept(const double newCost, const double currentCost) {
    if (isClose(newCost, currentCost)) {
        return SolutionStateResult::SAME_COST;
    } else if (newCost < currentCost) {
        return SolutionStateResult::NEW_SOLN;
    } else {
        if (dis(gen) <= exp(((1.0/newCost) - (1/currentCost)) / currentTemp)) {
            return SolutionStateResult::NEW_SOLN;
        }
        return SolutionStateResult::NO_NEW_SOLN;
    }
}

void SolutionState::recalculateLongestRoute() {
    longestRouteDistance = 0;
    longestTruckRoute = nullptr;
    for (Truck* t : trucks) {
        double cost = routeCostMap[t];
        if (cost > longestRouteDistance) {
            longestRouteDistance = cost;
            longestTruckRoute = t;
        }
    }
}

void SolutionState::updateOverallCost() {
    overallCost = (longestRouteDistance * wTime) + (totalDistance * wDist);
}

double SolutionState::sanityCheckRoute(Truck* t, std::vector<Package*>& packages) {
    double cost = 0;
    Node* currentNode = t->getStartNode();
    for (Package* p : packages) {
        cost += currentNode->distTo(p->getNode());
        currentNode = p->getNode();
    }
    cost += currentNode->distTo(t->getEndNode());
    return cost;
}

Truck* SolutionState::chooseRandomTruck(int minPackages) {
    Truck* out = trucks[truckDis(gen)];
    int i = 0;
    while ((routeMap[out].size() < minPackages)) {
        if (i == trucks.size() * 2) return nullptr;
        out = trucks[truckDis(gen)];
        i++;
    }
    return out;
}

Truck* SolutionState::chooseRandomTruck(int minPackages, Truck* excluded) {
    Truck* out = trucks[truckDis(gen)];
    int i = 0;
    while (excluded == out || routeMap[out].size() < minPackages) {
        if (i == trucks.size() * 2) return nullptr;
        out = trucks[truckDis(gen)];
        i++;
    }
    return out;
}

SolutionStateResult SolutionState::inverseSubroute() {
    Truck* t = chooseRandomTruck(2);
    if (t == nullptr) return SolutionStateResult::REDO;

    std::vector<Package*>& route = routeMap[t];
    double currentRouteCost = routeCostMap[t];

    std::uniform_int_distribution<> randInt(0, route.size() - 1);
    int n1 = randInt(gen);
    int n2 = randInt(gen);
    while (n1 == n2) {
        n2 = randInt(gen);
    }
    if (n1 > n2) {
        std::swap(n1, n2);
    }
    Node* n1Node = route[n1]->getNode();
    Node* n2Node = route[n2]->getNode();
    Node* beforeN1Node = getNodeBefore(n1, t, route);
    Node* afterN2Node = getNodeAfter(n2, t, route);

    // TODO: current implementation assumes reversed route is the same cost. this is only true if the graph is ALL bidirectional
    double newRouteCost = currentRouteCost - graph->getCost(beforeN1Node, n1Node) - graph->getCost(n2Node, afterN2Node);
    newRouteCost += graph->getCost(beforeN1Node, n2Node) + graph->getCost(n1Node, afterN2Node);

    SolutionStateResult result = shouldAccept(newRouteCost, currentRouteCost);
    if (result == SolutionStateResult::NEW_SOLN || result == SolutionStateResult::SAME_COST) {
        // accepted
        std::reverse(route.begin() + n1, route.begin() + n2 + 1);
        
        // double testCost = sanityCheckRoute(t, route);
        // if (!isClose(testCost, newRouteCost)) {
        //     std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        //                 << "inverse wrong cost " << newRouteCost << " vs " << testCost << std::endl;
        // }

        totalDistance = totalDistance - currentRouteCost + newRouteCost;
        routeCostMap[t] = newRouteCost;
        if (newRouteCost > longestRouteDistance) {
            longestRouteDistance = newRouteCost;
            longestTruckRoute = t;
        } else if (newRouteCost < longestRouteDistance && longestTruckRoute == t) {
            recalculateLongestRoute();
        }
        updateOverallCost();
        return result;
    }
    // not accepted
    return result;
}

// TODO: this is almost 2x slower than python
SolutionStateResult SolutionState::swapPackages() {
    Truck* t = chooseRandomTruck(2);
    if (t == nullptr) return SolutionStateResult::REDO;

    std::vector<Package*>& route = routeMap[t];
    double currentRouteCost = routeCostMap[t];

    std::uniform_int_distribution<> randInt(0, route.size() - 1);
    int n1 = randInt(gen);
    int n2 = randInt(gen);
    while (n1 == n2) {
        n2 = randInt(gen);
    }
    if (n1 > n2) {
        std::swap(n1, n2);
    }

    double newRouteCost = currentRouteCost;

    Node* n1Node = route[n1]->getNode();
    Node* n2Node = route[n2]->getNode();
    Node* nodeBeforeN1 = getNodeBefore(n1, t, route);
    Node* nodeAfterN1 = getNodeAfter(n1, t, route);
    Node* nodeBeforeN2 = getNodeBefore(n2, t, route);
    Node* nodeAfterN2 = getNodeAfter(n2, t, route);
    
    newRouteCost -= graph->getCost(nodeBeforeN1, n1Node);
    newRouteCost -= graph->getCost(n1Node, nodeAfterN1);
    newRouteCost -= graph->getCost(nodeBeforeN2, n2Node);
    newRouteCost -= graph->getCost(n2Node, nodeAfterN2);
    if (nodeAfterN1 == n2Node) nodeAfterN1 = n1Node;
    if (nodeBeforeN2 == n1Node) nodeBeforeN2 = n2Node;
    newRouteCost += graph->getCost(nodeBeforeN1, n2Node);
    newRouteCost += graph->getCost(n2Node, nodeAfterN1);
    newRouteCost += graph->getCost(nodeBeforeN2, n1Node);
    newRouteCost += graph->getCost(n1Node, nodeAfterN2);


    SolutionStateResult result = shouldAccept(newRouteCost, currentRouteCost);
    if (result == SolutionStateResult::NEW_SOLN || result == SolutionStateResult::SAME_COST) {
        // accepted       
        std::swap(route[n1], route[n2]);

        // double testCost = sanityCheckRoute(t, route);
        // if (!isClose(testCost, newRouteCost)) {
        //     std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        //                 << "swap wrong cost " << newRouteCost << " vs " << testCost << std::endl;
        //     newRouteCost = testCost;
        // }

        totalDistance = totalDistance - currentRouteCost + newRouteCost;
        routeCostMap[t] = newRouteCost;
        if (newRouteCost > longestRouteDistance) {
            longestRouteDistance = newRouteCost;
            longestTruckRoute = t;
        } else if (newRouteCost < longestRouteDistance && longestTruckRoute == t) {
            recalculateLongestRoute();
        }
        updateOverallCost();
        return result;
    }
    // not accepted
    return result;
}
// TODO: slower than python
SolutionStateResult SolutionState::movePackage() {
    Truck* t = chooseRandomTruck(2);
    if (t == nullptr) return SolutionStateResult::REDO;

    std::vector<Package*>& route = routeMap[t];
    double currentRouteCost = routeCostMap[t];

    std::uniform_int_distribution<> randInt(0, route.size() - 1);
    int n1 = randInt(gen);
    int n2 = randInt(gen);
    while (n1 == n2) {
        n2 = randInt(gen);
    }

    Node* n1Node = route[n1]->getNode();
    Node* n2Node = route[n2]->getNode();
    Node* nodeBeforeN1 = getNodeBefore(n1, t, route);
    Node* nodeAfterN1 = getNodeAfter(n1, t, route);
    Node* nodeBeforeN2 = getNodeBefore(n2, t, route);
    Node* nodeAfterN2 = getNodeAfter(n2, t, route);

    double newRouteCost = currentRouteCost;
    newRouteCost -= graph->getCost(nodeBeforeN1, n1Node);
    newRouteCost -= graph->getCost(n1Node, nodeAfterN1);
    newRouteCost += graph->getCost(nodeBeforeN1, nodeAfterN1);

    if (n1 > n2) {
        newRouteCost += graph->getCost(nodeBeforeN2, n1Node);
        newRouteCost += graph->getCost(n1Node, n2Node);
        newRouteCost -= graph->getCost(nodeBeforeN2, n2Node);
    } else {
        newRouteCost += graph->getCost(n2Node, n1Node);
        newRouteCost += graph->getCost(n1Node, nodeAfterN2);
        newRouteCost -= graph->getCost(n2Node, nodeAfterN2);
    }

    SolutionStateResult result = shouldAccept(newRouteCost, currentRouteCost);
    if (result == SolutionStateResult::NEW_SOLN || result == SolutionStateResult::SAME_COST) {
        // accepted  
        if (n1 < n2) {
            std::rotate(route.begin() + n1, route.begin() + n1 + 1, route.begin() + n2 + 1);
        } else {
            std::rotate(route.begin() + n2, route.begin() + n1, route.begin() + n1 + 1);
        }

        // double testCost = sanityCheckRoute(t, route);
        // if (!isClose(testCost, newRouteCost)) {
        //     std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        //                 << "move wrong cost " << newRouteCost << " vs " << testCost << std::endl;
        //     newRouteCost = testCost;
        // }

        totalDistance = totalDistance - currentRouteCost + newRouteCost;
        routeCostMap[t] = newRouteCost;
        if (newRouteCost > longestRouteDistance) {
            longestRouteDistance = newRouteCost;
            longestTruckRoute = t;
        } else if (newRouteCost < longestRouteDistance && longestTruckRoute == t) {
            recalculateLongestRoute();
        }
        updateOverallCost();
        return newRouteCost, result;
    }
    // not accepted
    return result;
}

SolutionStateResult SolutionState::moveSubroute() {
    Truck* t = chooseRandomTruck(4);
    if (t == nullptr) return SolutionStateResult::REDO;

    std::vector<Package*>& route = routeMap[t];
    double currentRouteCost = routeCostMap[t];

    std::uniform_int_distribution<> randInt(0, route.size() - 1);
    int n1 = randInt(gen);
    int n2 = randInt(gen);
    while ((n1 == n2) || (std::abs(n2 - n1) >= route.size() - 2)) {
        n1 = randInt(gen);
        n2 = randInt(gen);
    }
    if (n1 > n2) {
        std::swap(n1, n2);
    }
    int n3 = randInt(gen);
    while (n3 >= n1 && n3 <= n2 + 1) {
        n3 = randInt(gen);
    }

    double newRouteCost = currentRouteCost;

    Node* n1Node = route[n1]->getNode();
    Node* n2Node = route[n2]->getNode();
    Node* n3Node = route[n3]->getNode();
    Node* nodeBeforeN1 = getNodeBefore(n1, t, route);
    Node* nodeAfterN2 = getNodeAfter(n2, t, route);
    Node* nodeBeforeN3 = getNodeBefore(n3, t, route);

    newRouteCost -= graph->getCost(nodeBeforeN1, n1Node);
    newRouteCost -= graph->getCost(n2Node, nodeAfterN2);
    newRouteCost += graph->getCost(nodeBeforeN1, nodeAfterN2);

    newRouteCost -= graph->getCost(nodeBeforeN3, n3Node);
    newRouteCost += graph->getCost(nodeBeforeN3, n1Node);
    newRouteCost += graph->getCost(n2Node, n3Node);

    SolutionStateResult result = shouldAccept(newRouteCost, currentRouteCost);
    if (result == SolutionStateResult::NEW_SOLN || result == SolutionStateResult::SAME_COST) {
        // accepted  
        if (n3 < n1) {
            std::rotate(route.begin() + n3, route.begin() + n1, route.begin() + n2 + 1);
        } else {
            std::rotate(route.begin() + n1, route.begin() + n2 + 1, route.begin() + n3);
        }

        // double testCost = sanityCheckRoute(t, route);
        // if (!isClose(testCost, newRouteCost)) {
        //     std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        //                 << "move wrong cost " << newRouteCost << " vs " << testCost << std::endl;
        //     newRouteCost = testCost;
        // }

        totalDistance = totalDistance - currentRouteCost + newRouteCost;
        routeCostMap[t] = newRouteCost;
        if (newRouteCost > longestRouteDistance) {
            longestRouteDistance = newRouteCost;
            longestTruckRoute = t;
        } else if (newRouteCost < longestRouteDistance && longestTruckRoute == t) {
            recalculateLongestRoute();
        }
        updateOverallCost();
        return result;
    }
    // not accepted
    return result;
}



SolutionStateResult SolutionState::givePackage() {
    Truck* t1 = chooseRandomTruck(1); // truck to give from
    Truck* t2 = chooseRandomTruck(0); // truck to receive
    while (t2 == t1) {
        t2 = chooseRandomTruck(0);
    }
    if (t1 == nullptr || t2 == nullptr) return SolutionStateResult::REDO;

    double currentOverallCost = overallCost;

    std::vector<Package*>& giverRoute = routeMap[t1];
    double currentGiverRouteCost = routeCostMap[t1];
    std::vector<Package*>& receiverRoute = routeMap[t2];
    double currentReceiverRouteCost = routeCostMap[t2];

    std::uniform_int_distribution<> randIntGiver(0, giverRoute.size() - 1);
    int n1 = randIntGiver(gen);
    std::uniform_int_distribution<> randIntReceiver(0, receiverRoute.size());
    int n2 = randIntReceiver(gen);
    
    double newGiverRouteCost = currentGiverRouteCost;
    double newReceiverRouteCost = currentReceiverRouteCost;

    Node* n1Node = giverRoute[n1]->getNode();
    Node* n2Node = nullptr;
    if (n2 == receiverRoute.size()) {
        n2Node = t2->getEndNode();
    } else {
        n2Node = receiverRoute[n2]->getNode();
    }
    Node* nodeBeforeN1 = getNodeBefore(n1, t1, giverRoute);
    Node* nodeAfterN1 = getNodeAfter(n1, t1, giverRoute);
    Node* nodeBeforeN2 = getNodeBefore(n2, t2, receiverRoute);

    newGiverRouteCost -= graph->getCost(nodeBeforeN1, n1Node);
    newGiverRouteCost -= graph->getCost(n1Node, nodeAfterN1);
    newGiverRouteCost += graph->getCost(nodeBeforeN1, nodeAfterN1);

    newReceiverRouteCost -= graph->getCost(nodeBeforeN2, n2Node);
    newReceiverRouteCost += graph->getCost(nodeBeforeN2, n1Node);
    newReceiverRouteCost += graph->getCost(n1Node, n2Node);

    double newTotalDistance = totalDistance - currentGiverRouteCost - currentReceiverRouteCost;
    newTotalDistance += newGiverRouteCost + newReceiverRouteCost;

    double newLongestDistance = 0;
    Truck* newLongestTruckRoute = nullptr;
    for (Truck* truck : trucks) {
        double truckDist = routeCostMap[truck];
        if (truck == t1) truckDist = newGiverRouteCost;
        if (truck == t2) truckDist = newReceiverRouteCost;
        if (truckDist >= newLongestDistance) {
            newLongestDistance = truckDist;
            newLongestTruckRoute = truck;
        }
    }
    double newOverallCost = (newTotalDistance * wDist) + (newLongestDistance * wTime);

    SolutionStateResult result = shouldAccept(newOverallCost, currentOverallCost);
    if (result == SolutionStateResult::NEW_SOLN || result == SolutionStateResult::SAME_COST) {
        // accepted  
        if (n2 == receiverRoute.size()) {
            receiverRoute.push_back(giverRoute[n1]);
        } else {
            receiverRoute.insert(receiverRoute.begin() + n2, giverRoute[n1]);
        }
        giverRoute.erase(giverRoute.begin() + n1);

        // double testCost1 = sanityCheckRoute(t1, giverRoute);
        // double testCost2 = sanityCheckRoute(t2, receiverRoute);
        // if (!isClose(testCost1, newGiverRouteCost)) {
        //     std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        //                 << "give node wrong giver cost " << newGiverRouteCost << " vs " << testCost1 << std::endl;
        //     newGiverRouteCost = testCost1;
        // }
        // if (!isClose(testCost2, newReceiverRouteCost)) {
        //     std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        //                 << "give node wrong receiver cost " << newReceiverRouteCost << " vs " << testCost2 << std::endl;
        //     newReceiverRouteCost = testCost2;
        // }

        totalDistance = newTotalDistance;
        routeCostMap[t1] = newGiverRouteCost;
        routeCostMap[t2] = newReceiverRouteCost;
        longestRouteDistance = newLongestDistance;
        longestTruckRoute = newLongestTruckRoute;
        updateOverallCost();
        return result;
    }
    // not accepted
    return result;
}

SolutionStateResult SolutionState::giveSubroute() {
    Truck* t1 = chooseRandomTruck(2); // truck to give from
    Truck* t2 = chooseRandomTruck(0); // truck to receive
    while (t2 == t1) {
        t2 = chooseRandomTruck(0);
    }
    if (t1 == nullptr || t2 == nullptr) return SolutionStateResult::REDO;

    double currentOverallCost = overallCost;

    std::vector<Package*>& giverRoute = routeMap[t1];
    double currentGiverRouteCost = routeCostMap[t1];
    std::vector<Package*>& receiverRoute = routeMap[t2];
    double currentReceiverRouteCost = routeCostMap[t2];

    std::uniform_int_distribution<> randIntGiver(0, giverRoute.size() - 1);
    int n1 = randIntGiver(gen);
    int n2 = randIntGiver(gen);
    while (n2 == n1) {
        n2 = randIntGiver(gen);
    }
    if (n1 > n2) {
        std::swap(n1, n2);
    }
    std::uniform_int_distribution<> randIntReceiver(0, receiverRoute.size());
    int n3 = randIntReceiver(gen);
    
    double newGiverRouteCost = currentGiverRouteCost;
    double newReceiverRouteCost = currentReceiverRouteCost;

    Node* n1Node = giverRoute[n1]->getNode();
    Node* n2Node = giverRoute[n2]->getNode();
    Node* n3Node = nullptr;
    if (n3 == receiverRoute.size()) {
        n3Node = t2->getEndNode();
    } else {
        n3Node = receiverRoute[n3]->getNode();
    }
    Node* nodeBeforeN1 = getNodeBefore(n1, t1, giverRoute);
    Node* nodeAfterN2 = getNodeAfter(n2, t1, giverRoute);
    Node* nodeBeforeN3 = getNodeBefore(n3, t2, receiverRoute);

    // TODO: this is SLOW, almost halves the number of SA iterations possible
    double subrouteCost = 0;
    for (int i = n1; i < n2; i++) {
        subrouteCost += graph->getCost(giverRoute[i]->getId(), giverRoute[i + 1]->getId());
    }

    newGiverRouteCost -= graph->getCost(nodeBeforeN1, n1Node);
    newGiverRouteCost -= graph->getCost(n2Node, nodeAfterN2);
    newGiverRouteCost -= subrouteCost;
    newGiverRouteCost += graph->getCost(nodeBeforeN1, nodeAfterN2);

    newReceiverRouteCost -= graph->getCost(nodeBeforeN3, n3Node);
    newReceiverRouteCost += graph->getCost(nodeBeforeN3, n1Node);
    newReceiverRouteCost += graph->getCost(n2Node, n3Node);
    newReceiverRouteCost += subrouteCost;

    double newTotalDistance = totalDistance - currentGiverRouteCost - currentReceiverRouteCost;
    newTotalDistance += newGiverRouteCost + newReceiverRouteCost;

    double newLongestDistance = 0;
    Truck* newLongestTruckRoute = nullptr;
    for (Truck* truck : trucks) {
        double truckDist = routeCostMap[truck];
        if (truck == t1) truckDist = newGiverRouteCost;
        if (truck == t2) truckDist = newReceiverRouteCost;
        if (truckDist >= newLongestDistance) {
            newLongestDistance = truckDist;
            newLongestTruckRoute = truck;
        }
    }
    double newOverallCost = (newTotalDistance * wDist) + (newLongestDistance * wTime);

    SolutionStateResult result = shouldAccept(newOverallCost, currentOverallCost);
    if (result == SolutionStateResult::NEW_SOLN || result == SolutionStateResult::SAME_COST) {
        // accepted  
        if (n3 == receiverRoute.size()) {
            receiverRoute.push_back(giverRoute[n1]);
            receiverRoute.insert(receiverRoute.end(), giverRoute.begin() + n1 + 1, giverRoute.begin() + n2 + 1);
        } else {
            receiverRoute.insert(receiverRoute.begin() + n3, giverRoute.begin() + n1, giverRoute.begin() + n2 + 1);
        }
        giverRoute.erase(giverRoute.begin() + n1, giverRoute.begin() + n2 + 1);

        // double testCost1 = sanityCheckRoute(t1, giverRoute);
        // double testCost2 = sanityCheckRoute(t2, receiverRoute);
        // if (!isClose(testCost1, newGiverRouteCost)) {
        //     std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        //                 << "give node wrong giver cost " << newGiverRouteCost << " vs " << testCost1 << std::endl;
        //     newGiverRouteCost = testCost1;
        // }
        // if (!isClose(testCost2, newReceiverRouteCost)) {
        //     std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        //                 << "give node wrong receiver cost " << newReceiverRouteCost << " vs " << testCost2 << std::endl;
        //     newReceiverRouteCost = testCost2;
        // }

        totalDistance = newTotalDistance;
        routeCostMap[t1] = newGiverRouteCost;
        routeCostMap[t2] = newReceiverRouteCost;
        longestRouteDistance = newLongestDistance;
        longestTruckRoute = newLongestTruckRoute;
        updateOverallCost();
        return result;
    }
    // not accepted
    return result;
}

SolutionStateResult SolutionState::exchangePackages() {

    return SolutionStateResult::REDO;
}

SolutionStateResult SolutionState::exchangeSubroutes() {

    return SolutionStateResult::REDO;
}

bool SolutionState::isClose(double a, double b) {
    return std::fabs(a - b) < e;
}

const double SolutionState::e = 10e-9;