#pragma once

#include "Package.h"
#include "Truck.h"
#include "Graph.h"

#include <random>

enum class SolutionStateResult {
    NEW_SOLN,
    SAME_COST,
    NO_NEW_SOLN,
    REDO
};

class SolutionState {

public:
    SolutionState(std::vector<Truck*>& trucks, 
        std::unordered_map<Truck*, std::vector<Package*>>& initialRouteMap, 
        std::unordered_map<Truck*, double>& initialRouteCostMap,
        Graph* graph,
        double wDist,
        double wTime);

    void setCurrentTemp(const double currentTemp);
    double getTotalDistance();
    double getLongestRouteDistance();
    std::unordered_map<Truck*, std::vector<Package*>> getRouteMapCopy();
    std::unordered_map<Truck*, double> getRouteCostMapCopy();

    SolutionStateResult createNeighbour();
    SolutionStateResult createNeighbourTSP();
    SolutionStateResult createNeighbourShuffle();

    // simulated annealing helper functions
    Node* getNodeBefore(const int idx, Truck* truck, const std::vector<Package*>& packages) const;
    Node* getNodeAfter(const int idx, Truck* truck, const std::vector<Package*>& packages) const;
    int getNodeIdBefore(const int idx, Truck* truck, const std::vector<Package*>& packages) const;
    int getNodeIdAfter(const int idx, Truck* truck, const std::vector<Package*>& packages) const;
    SolutionStateResult shouldAccept(const double newCost, const double currentCost);
    void recalculateLongestRoute();
    void updateOverallCost();
    double sanityCheckRoute(Truck* t, std::vector<Package*>& packages);

    Truck* chooseRandomTruck(int minPackages);
    Truck* chooseRandomTruck(int minPackages, Truck* excluded);
    SolutionStateResult inverseSubroute();
    SolutionStateResult swapPackages();
    SolutionStateResult movePackage();
    SolutionStateResult moveSubroute();
    SolutionStateResult givePackage();
    SolutionStateResult giveSubroute();
    SolutionStateResult exchangePackages();
    SolutionStateResult exchangeSubroutes();

    static bool isClose(double a, double b);

private:
    std::vector<Truck*> trucks;
    std::unordered_map<Truck*, std::vector<Package*>> routeMap;
    std::unordered_map<Truck*, double> routeCostMap;

    double wDist = 0.5;
    double wTime = 0.5;
    double totalDistance;
    double longestRouteDistance;
    double overallCost;
    Truck* longestTruckRoute = nullptr;
    double currentTemp;

    Graph* graph;

    // math things
    const static double e;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;
    std::uniform_int_distribution<> truckDis;
};