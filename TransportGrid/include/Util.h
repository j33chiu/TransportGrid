#include <string>


#include "Graph.h"
#include "Solver.h"

class Util {
public:
    Util(Graph* graph, Solver* solver);

    void readFakeMap();
    void readRealMap();

    void writeFakeMap();
    void writeRealMap();

    void readFakeProblem();
    void readRealProblem();

    void writeFakeProblem();
    void writeRealProblem();

    // random graph generator settings
    void setXMin(int xMin);
    void setXMax(int xMax);
    void setYMin(int yMin);
    void setYMax(int yMax);
    void setNumNodes(int numNodes);
    void setNumNeighbours(int numNeighbours);
    void setNumDeliveries(int numDeliveries);

    const int getXMin();
    const int getXMax();
    const int getYMin();
    const int getYMax();
    const int getNumNodes();
    const int getNumNeighbours();
    const int getNumDeliveries();

    void generate();

private:
    Graph* graph;
    Solver* solver;

    int xMin = 0;
    int xMax = 350;
    int yMin = 0;
    int yMax = 350;
    int numNodes = 500;
    int numNeighbours = 4;
    int numDeliveries = 50;


};