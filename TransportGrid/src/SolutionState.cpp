#include "SolutionState.h"

#include <iostream>
#include <iomanip>

SolutionState::SolutionState(std::vector<Truck*>& trucks, 
        std::unordered_map<Truck*, std::vector<Package*>>& initialRouteMap, 
        std::unordered_map<Truck*, long double>& initialRouteCostMap,
        Graph* graph) 
{
    long double initialDist = 0;
    long double initialLongest = 0;
    for (Truck* t : trucks) {
        routeMap[t] = std::vector<Package*>(initialRouteMap[t]);
        long double c = initialRouteCostMap[t];
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
    this->graph = graph;

    gen = std::mt19937(rd());
    dis = std::uniform_real_distribution<>(0.0, 1.0);
    truckDis = std::uniform_int_distribution<>(0, trucks.size() - 1);
}

void SolutionState::setCurrentTemp(const long double currentTemp) {
    this->currentTemp = currentTemp;
}

long double SolutionState::getTotalDistance() {
    return totalDistance;
}

long double SolutionState::getLongestRouteDistance() {
    return longestRouteDistance;
}

std::unordered_map<Truck*, std::vector<Package*>> SolutionState::getRouteMapCopy() {
    return routeMap;
}

std::unordered_map<Truck*, long double> SolutionState::getRouteCostMapCopy() {
    return routeCostMap;
}

SolutionStateResult SolutionState::createNeighbour() {
    std::vector<int> choiceTypes = {0, 1};
    std::uniform_int_distribution<> choiceTypesDist(0, choiceTypes.size() - 1);
    std::pair<long double, SolutionStateResult> result(-1, SolutionStateResult::REDO);
    while (result.second == SolutionStateResult::REDO) {
        int choiceType = choiceTypesDist(gen);
        switch (choiceType) {
            case 0:
                // allow switching/moving packages
                result = createNeighbourShuffle();
                break;
            case 1:
            default:
                // just TSP 
                result = createNeighbourTSP();
                break;
        }
    }
    return result.second;
}

std::pair<long double, SolutionStateResult> SolutionState::createNeighbourShuffle() {
    std::vector<int> choices = {0};
    std::uniform_int_distribution<> choiceDist(0, choices.size() - 1);
    int choice = choiceDist(gen);
    std::pair<long double, SolutionStateResult> result(-1, SolutionStateResult::REDO);
    switch(choice) {
        case 0:
        default:
            break;
    }
    return result;
}

std::pair<long double, SolutionStateResult> SolutionState::createNeighbourTSP() {
    std::vector<int> choices = {0, 1, 2};
    std::uniform_int_distribution<> choiceDist(0, choices.size() - 1);
    int choice = choiceDist(gen);
    std::pair<long double, SolutionStateResult> result(-1, SolutionStateResult::REDO);
    switch(choice) {
        case 0:
            result = inverseSubroute();
            break;
        case 1:
            result = swapPackages();
            break;
        case 2:
            result = movePackage();
            break;
        default:
            result = inverseSubroute();
            break;
    }
    return result;
}


// simulated annealing helper functions
int SolutionState::getNodeIdBefore(const int idx, Truck* truck, std::vector<Package*>& packages) {
    int beforeIdx = idx - 1;
    if (beforeIdx < 0) return truck->getStartNode()->getId();
    return packages[beforeIdx]->getId();
}

int SolutionState::getNodeIdAfter(const int idx, Truck* truck, std::vector<Package*>& packages) {
    int afterIdx = idx + 1;
    if (afterIdx >= packages.size()) return truck->getEndNode()->getId();
    return packages[afterIdx]->getId();
}

SolutionStateResult SolutionState::shouldAccept(const long double newCost, const long double currentCost) {
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
        long double cost = routeCostMap[t];
        if (cost > longestRouteDistance) {
            longestRouteDistance = cost;
            longestTruckRoute = t;
        }
    }
}

long double SolutionState::sanityCheckRoute(Truck* t, std::vector<Package*>& packages) {
    long double cost = 0;
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

std::pair<long double, SolutionStateResult> SolutionState::inverseSubroute() {
    Truck* t = chooseRandomTruck(2);
    if (t == nullptr) return std::pair<long double, SolutionStateResult>(0, SolutionStateResult::REDO);

    std::vector<Package*>& route = routeMap[t];
    long double currentRouteCost = routeCostMap[t];

    std::uniform_int_distribution<> randInt(0, route.size() - 1);
    int n1 = randInt(gen);
    int n2 = randInt(gen);
    while (n1 == n2) {
        n2 = randInt(gen);
    }
    if (n1 > n2) {
        std::swap(n1, n2);
    }
    int n1Id = route[n1]->getId();
    int n2Id = route[n2]->getId();
    int beforeN1NodeId = getNodeIdBefore(n1, t, route);
    int afterN2NodeId = getNodeIdAfter(n2, t, route);

    // TODO: current implementation assumes reversed route is the same cost. this is only true if the graph is ALL bidirectional
    long double newRouteCost = currentRouteCost - graph->getCost(beforeN1NodeId, n1Id) - graph->getCost(n2Id, afterN2NodeId);
    newRouteCost += graph->getCost(beforeN1NodeId, n2Id) + graph->getCost(n1Id, afterN2NodeId);

    SolutionStateResult result = shouldAccept(newRouteCost, currentRouteCost);
    if (result == SolutionStateResult::NEW_SOLN || result == SolutionStateResult::SAME_COST) {
        // accepted
        std::reverse(route.begin() + n1, route.begin() + n2 + 1);
        
        // long double testCost = sanityCheckRoute(t, route);
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
        return std::pair<long double, SolutionStateResult>(newRouteCost, result);
    }
    // not accepted
    return std::pair<long double, SolutionStateResult>(currentRouteCost, result);
}

std::pair<long double, SolutionStateResult> SolutionState::swapPackages() {
    Truck* t = chooseRandomTruck(2);
    if (t == nullptr) return std::pair<long double, SolutionStateResult>(0, SolutionStateResult::REDO);

    std::vector<Package*>& route = routeMap[t];
    long double currentRouteCost = routeCostMap[t];

    std::uniform_int_distribution<> randInt(0, route.size() - 1);
    int n1 = randInt(gen);
    int n2 = randInt(gen);
    while (n1 == n2) {
        n2 = randInt(gen);
    }
    if (n1 > n2) {
        std::swap(n1, n2);
    }

    long double newRouteCost = currentRouteCost;

    int n1Id = route[n1]->getId();
    int n2Id = route[n2]->getId();
    int nodeIdBeforeN1 = getNodeIdBefore(n1, t, route);
    int nodeIdAfterN1 = getNodeIdAfter(n1, t, route);
    int nodeIdBeforeN2 = getNodeIdBefore(n2, t, route);
    int nodeIdAfterN2 = getNodeIdAfter(n2, t, route);
    
    newRouteCost -= graph->getCost(nodeIdBeforeN1, n1Id);
    newRouteCost -= graph->getCost(n1Id, nodeIdAfterN1);
    newRouteCost -= graph->getCost(nodeIdBeforeN2, n2Id);
    newRouteCost -= graph->getCost(n2Id, nodeIdAfterN2);
    if (nodeIdAfterN1 == n2Id) nodeIdAfterN1 = n1Id;
    if (nodeIdBeforeN2 == n1Id) nodeIdBeforeN2 = n2Id;
    newRouteCost += graph->getCost(nodeIdBeforeN1, n2Id);
    newRouteCost += graph->getCost(n2Id, nodeIdAfterN1);
    newRouteCost += graph->getCost(nodeIdBeforeN2, n1Id);
    newRouteCost += graph->getCost(n1Id, nodeIdAfterN2);


    SolutionStateResult result = shouldAccept(newRouteCost, currentRouteCost);
    if (result == SolutionStateResult::NEW_SOLN || result == SolutionStateResult::SAME_COST) {
        // accepted       
        std::swap(route[n1], route[n2]);

        // long double testCost = sanityCheckRoute(t, route);
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
        return std::pair<long double, SolutionStateResult>(newRouteCost, result);
    }
    // not accepted
    return std::pair<long double, SolutionStateResult>(currentRouteCost, result);
}

std::pair<long double, SolutionStateResult> SolutionState::movePackage() {
    Truck* t = chooseRandomTruck(2);
    if (t == nullptr) return std::pair<long double, SolutionStateResult>(0, SolutionStateResult::REDO);

    std::vector<Package*>& route = routeMap[t];
    long double currentRouteCost = routeCostMap[t];

    std::uniform_int_distribution<> randInt(0, route.size() - 1);
    int n1 = randInt(gen);
    int n2 = randInt(gen);
    while (n1 == n2) {
        n2 = randInt(gen);
    }

    int n1Id = route[n1]->getId();
    int n2Id = route[n2]->getId();
    int nodeIdBeforeN1 = getNodeIdBefore(n1, t, route);
    int nodeIdAfterN1 = getNodeIdAfter(n1, t, route);
    int nodeIdBeforeN2 = getNodeIdBefore(n2, t, route);
    int nodeIdAfterN2 = getNodeIdAfter(n2, t, route);

    long double newRouteCost = currentRouteCost;
    newRouteCost -= graph->getCost(nodeIdBeforeN1, n1Id);
    newRouteCost -= graph->getCost(n1Id, nodeIdAfterN1);
    newRouteCost += graph->getCost(nodeIdBeforeN1, nodeIdAfterN1);

    if (n1 > n2) {
        newRouteCost += graph->getCost(nodeIdBeforeN2, n1Id);
        newRouteCost += graph->getCost(n1Id, n2Id);
        newRouteCost -= graph->getCost(nodeIdBeforeN2, n2Id);
    } else {
        newRouteCost += graph->getCost(n2Id, n1Id);
        newRouteCost += graph->getCost(n1Id, nodeIdAfterN2);
        newRouteCost -= graph->getCost(n2Id, nodeIdAfterN2);
    }

    SolutionStateResult result = shouldAccept(newRouteCost, currentRouteCost);
    if (result == SolutionStateResult::NEW_SOLN || result == SolutionStateResult::SAME_COST) {
        // accepted  
        if (n1 < n2) {
            std::rotate(route.begin() + n1, route.begin() + n1 + 1, route.begin() + n2 + 1);
        } else {
            std::rotate(route.begin() + n2, route.begin() + n1, route.begin() + n1 + 1);
        }

        // long double testCost = sanityCheckRoute(t, route);
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
        return std::pair<long double, SolutionStateResult>(newRouteCost, result);
    }
    // not accepted
    return std::pair<long double, SolutionStateResult>(currentRouteCost, result);
}

std::pair<long double, SolutionStateResult> SolutionState::moveSubroute() {
    Truck* t = chooseRandomTruck(3);

    return std::pair<long double, SolutionStateResult>(0, SolutionStateResult::REDO);
}

std::pair<long double, SolutionStateResult> SolutionState::givePackage() {

    return std::pair<long double, SolutionStateResult>(0, SolutionStateResult::REDO);
}

std::pair<long double, SolutionStateResult> SolutionState::giveSubroute() {

    return std::pair<long double, SolutionStateResult>(0, SolutionStateResult::REDO);
}

std::pair<long double, SolutionStateResult> SolutionState::exchangePackages() {

    return std::pair<long double, SolutionStateResult>(0, SolutionStateResult::REDO);
}

std::pair<long double, SolutionStateResult> SolutionState::exchangeSubroutes() {

    return std::pair<long double, SolutionStateResult>(0, SolutionStateResult::REDO);
}

bool SolutionState::isClose(long double a, long double b) {
    return std::fabs(a - b) < e;
}

const long double SolutionState::e = 10e-9;