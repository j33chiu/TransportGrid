#include "Solver.h"
#include "SolutionState.h"

#include <iostream>
#include <chrono>

Solver::Solver(Graph* graph) {
    this->graph = graph;
}

std::vector<Package*> Solver::setPackages(std::vector<Node*> packageNodes) {
    clearPackages();
    for (Node* n : packageNodes) {
        packages.push_back(new Package(n));
    }
    return packages;
}

std::vector<Package*> Solver::setPackages(std::vector<Node*> packageNodes, std::vector<int> people, std::vector<double> volume) {
    clearPackages();
    for (int i = 0; i < packageNodes.size(); i++) {
        packages.push_back(new Package(packageNodes[i], people[i], volume[i]));
    }
    return packages;
}

std::vector<Package*> Solver::getPackages() {
    return packages;
}

void Solver::clearPackages() {
    for (Package* p : packages) {
        delete p;
    }
    packages.clear();
    //std::cout << "deleted packages" << std::endl;
}

Truck* Solver::addTruck(Node* startNode, Node* endNode, int peopleCap, double volumeCap) {
    Truck* t = new Truck(startNode, endNode, peopleCap, volumeCap);
    trucks.push_back(t);
    return t;
}

std::vector<Truck*> Solver::getTrucks() {
    return trucks;
}

void Solver::clearTrucks() {
    for (Truck* t: trucks) {
        delete t;
    }
    trucks.clear();
    //std::cout << "deleted trucks" << std::endl;
}

long double Solver::solve() {
    // pre calculate important costs
    std::cout << "performing graph analysis" << std::endl;
    std::vector<Node*> importantNodes;
    for (Package* p : packages) {
        importantNodes.push_back(p->getNode());
    }
    for (Truck* t : trucks) {
        importantNodes.push_back(t->getStartNode());
        importantNodes.push_back(t->getEndNode());
    }
    graph->calculateDistanceTable(importantNodes);

    std::cout << "searching..." << std::endl;
    // setup data storage for SA
    std::unordered_map<Truck*, std::vector<Package*>> initialRouteMap;
    std::unordered_map<Truck*, long double> initialRouteCostMap;

    // simulated annealing iterations:
    long double bestCost = MAX_COST;
    long double bestTotalDist = MAX_COST;
    long double bestLongestDist = MAX_COST;
    std::unordered_map<Truck*, std::vector<Package*>> bestRouteMap;
    std::unordered_map<Truck*, long double> bestRouteCostMap;

    for (int i = 0; i < saIters; i++) {
        // perform initial clustering
        initialRouteMap.clear();
        initialRouteCostMap.clear();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, trucks.size() - 1);
        for (Package* p : packages) {
            // randomly assign
            // TODO: if a package has a designated truck, assign it here
            int ti = dis(gen);
            initialRouteMap[trucks[ti]].push_back(p);
        }

        // set initial solution from random clustering and evaluate costs
        for (Truck* t : trucks) {
            t->overwritePackages(initialRouteMap[t]);
            initialRouteCostMap[t] = t->evaluateCost();
        }

        // create solution state with initial solution
        SolutionState state(trucks, initialRouteMap, initialRouteCostMap, graph);

        // simulated annealing here:
        long double a = 0.99;
        long double currentTemp = saInitTemp;

        int repeated = 0;
        int repeatedCost = 0;

        std::chrono::steady_clock::time_point saIterStart = std::chrono::steady_clock::now();
        int saIterElapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - saIterStart).count();
        int debugIters = 0;
        while ((repeated < saStopCond1) && (repeatedCost < saStopCond2) && (saIterElapsed < saMaxTime)) {
            state.setCurrentTemp(currentTemp);
            SolutionStateResult result = state.createNeighbour();

            switch(result) {
                case SolutionStateResult::NEW_SOLN:
                    repeated = 0;
                    repeatedCost = 0;
                    break;
                case SolutionStateResult::SAME_COST:
                    repeated = 0;
                    repeatedCost++;
                    break;
                case SolutionStateResult::NO_NEW_SOLN:
                default:
                    repeated++;
                    repeatedCost++;
                    break;
            }

            // decrement current temp
            currentTemp *= a;
            // time
            saIterElapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - saIterStart).count();
            debugIters++;
        }
        std::cout << "Finished simulated annealing iteration " << i + 1 << " in " << saIterElapsed << " seconds with " << debugIters << " iterations" << std::endl;

        // compare result with previous iterations
        long double newTotalDist = state.getTotalDistance();
        long double newLongestDist = state.getLongestRouteDistance();
        long double newCost = (wDist * newTotalDist) + (wTime * newLongestDist);
        if (newCost < bestCost) {
            bestRouteMap = state.getRouteMapCopy();
            bestRouteCostMap = state.getRouteCostMapCopy();
            bestCost = newCost;
            bestTotalDist = newTotalDist;
            bestLongestDist = newLongestDist;
        }
    }

    // write packages and costs back to trucks
    long double assertLongestDist = 0;
    long double sum = 0;
    for (Truck* t : trucks) {
        t->overwritePackages(bestRouteMap[t]);
        long double assertDist = t->evaluateCost();
        sum += assertDist;
        if (!isClose(assertDist, bestRouteCostMap[t])) {
            std::cout << "actual route dist not equal to expected route dist, likely error in sa: " << assertDist << ", " << bestRouteCostMap[t] << std::endl;
        }
        if (assertDist > assertLongestDist) {
            assertLongestDist = assertDist;
        }
    }
    if (!isClose(sum, bestTotalDist)) {
        std::cout << "actual total dist not equal to expected total dist, likely error in sa: " << sum << ", " << bestTotalDist << std::endl;
    }
    if (!isClose(assertLongestDist, bestLongestDist)) {
        std::cout << "actual longest dist not equal to expected longest dist, likely error in sa: " << assertLongestDist << ", " << bestLongestDist << std::endl;
    }

    long double finalCost = (wDist * sum) + (wTime * assertLongestDist);
    std::cout << "Final cost: " << finalCost << std::endl;
    std::cout << "Final total distance: " << bestTotalDist << std::endl;
    std::cout << "Final longest route: " << bestLongestDist << std::endl;


    return finalCost ;
}

bool Solver::isClose(long double a, long double b) {
    return std::fabs(a - b) < e;
}

const long double Solver::e = 10e-9;

Solver::~Solver() {
    clearPackages();
    clearTrucks();
}
