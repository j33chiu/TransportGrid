# pragma once

#include "Graph.h"
#include "Package.h"
#include "Truck.h"

class Solver {

public:
    Solver(Graph* graph);

    // take copy of deliveries and save into packages
    std::vector<Package*> setPackages(std::vector<Node*> packageNodes);
    std::vector<Package*> setPackages(std::vector<Node*> packageNodes, std::vector<int> people, std::vector<double> volume);
    std::vector<Package*> getPackages();
    void clearPackages();

    // take truck info and create new objects
    Truck* addTruck(Node* startNode, Node* endNode, int peopleCap = 4, double volumeCap = 100.0f);
    std::vector<Truck*> getTrucks();
    void clearTrucks();

    // simulated annealing
    double solve();

    ~Solver();

    const static int MAX_COST = 99999999999;
    static bool isClose(double a, double b);

private:
    // stores list of packages
    std::vector<Package*> packages;
    std::vector<Truck*> trucks;

    // store graph instance
    Graph* graph;

    // solver parameters
    // cost function
    double wDist = 0.5;
    double wTime = 0.5;
    // simulated annealing config
    int saInitTemp = 10000;
    int saStopCond1 = 20000;
    int saStopCond2 = 150000;
    int saIters = 1;
    int saMaxTime = 120; // in seconds

    const static double e;
    

};