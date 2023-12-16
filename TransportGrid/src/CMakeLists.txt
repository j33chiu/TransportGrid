cmake_minimum_required(VERSION 3.16)

include_directories(${PROJECT_SOURCE_DIR}/TransportGrid/include)
add_executable(transpoGrid main.cpp)


set(_INCL "${PROJECT_SOURCE_DIR}/TransportGrid/include")

target_sources(transpoGrid PRIVATE
    ${_INCL}/Graph.h
    ${_INCL}/Util.h
    ${_INCL}/Node.h
    ${_INCL}/Package.h
    ${_INCL}/PriorityQueue.h
    ${_INCL}/SolutionState.h
    ${_INCL}/Solver.h
    ${_INCL}/Truck.h
    Graph.cpp
    Util.cpp
    Node.cpp
    Package.cpp
    SolutionState.cpp
    Solver.cpp
    Truck.cpp
)