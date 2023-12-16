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
        std::unordered_map<Truck*, long double>& initialRouteCostMap,
        Graph* graph);

    void setCurrentTemp(const long double currentTemp);
    long double getTotalDistance();
    long double getLongestRouteDistance();
    std::unordered_map<Truck*, std::vector<Package*>> getRouteMapCopy();
    std::unordered_map<Truck*, long double> getRouteCostMapCopy();

    SolutionStateResult createNeighbour();
    std::pair<long double, SolutionStateResult> createNeighbourTSP();
    std::pair<long double, SolutionStateResult> createNeighbourShuffle();

    // simulated annealing helper functions
    int getNodeIdBefore(const int idx, Truck* truck, std::vector<Package*>& packages);
    int getNodeIdAfter(const int idx, Truck* truck, std::vector<Package*>& packages);
    SolutionStateResult shouldAccept(const long double newCost, const long double currentCost);
    void recalculateLongestRoute();
    long double sanityCheckRoute(Truck* t, std::vector<Package*>& packages);

    Truck* chooseRandomTruck(int minPackages);
    Truck* chooseRandomTruck(int minPackages, Truck* excluded);
    std::pair<long double, SolutionStateResult> inverseSubroute();
    std::pair<long double, SolutionStateResult> swapPackages();
    std::pair<long double, SolutionStateResult> movePackage();
    std::pair<long double, SolutionStateResult> moveSubroute();
    std::pair<long double, SolutionStateResult> givePackage();
    std::pair<long double, SolutionStateResult> giveSubroute();
    std::pair<long double, SolutionStateResult> exchangePackages();
    std::pair<long double, SolutionStateResult> exchangeSubroutes();

    static bool isClose(long double a, long double b);

private:
    std::vector<Truck*> trucks;
    std::unordered_map<Truck*, std::vector<Package*>> routeMap;
    std::unordered_map<Truck*, long double> routeCostMap;

    long double totalDistance;
    long double longestRouteDistance;
    Truck* longestTruckRoute = nullptr;
    long double currentTemp;

    Graph* graph;

    // math things
    const static long double e;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;
    std::uniform_int_distribution<> truckDis;
};