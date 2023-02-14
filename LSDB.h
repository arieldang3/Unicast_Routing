//
// Created by Ariel Dang on 2022/10.
//

#ifndef TEST7_LSDB_H
#define TEST7_LSDB_H

//#include "Graph.h"
//#include "LSP.h"
#include "json.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <climits>



#define DEBUG 1

#define DEBUG_PRINTF(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "node%d:%s:%d:%s(): " fmt "\n", 555, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define DEBUG_PRINT(text) \
        do { if (DEBUG) fprintf(stderr, "node%d:%s:%d:%s(): " text "\n", 555, __FILE__, \
                                __LINE__, __func__); } while (0)

using json = nlohmann::json;

#define ROUTER_NUM 256
/**
 * Local info knows by a router;
 * Describes the entire network;
 * all routers should know the same info about the network;
 * Input: LSA received from other routers;
 * OUTPUT: a shortest path tree to all routers;
 */
class LSDB{
private:
    /*
     * //globalMsgSeqN<- the #version of the graph; if I have known this graph, no need to change
     */
    int myId, myMsgSeqN, globalMsgSeqN;


//    Graph *my_knowledge_of_graph;
    std::unordered_set<int> *vertices{};
    std::unordered_map<int, std::unordered_set<int> > *edges; //{scr: <dest1, dest2, ...>}
    int graphCostTable[ROUTER_NUM][ROUTER_NUM];

    int SPT[ROUTER_NUM] = {-1}; //<- by this shortest path tree, can know the "next-hop"
//    std::unordered_map<int, double> *costs;

    // DijkstraAlgorithm
    int findMinDistVertex(std::unordered_set<int> *unknown, std::unordered_map<int, int> *dist);
    void relaxOutGoingEdges(int v, std::unordered_map<int, int> *dist, std::unordered_map<int, int> *prev);
//    void findShortestPaths(int v);



public:
    LSDB(int myRouterId);
    ~LSDB();

    // for this project only; (since we know there's fixed number of routers)
    // better (if no order): to store as a set/map
    int neighbors[ROUTER_NUM] = {0}; // 1 isNeighbor;
    int costs[ROUTER_NUM] = {1}; // parse the cost file; defualt: 1

    int LSPSeqNumReceivedfromThisNode[ROUTER_NUM] = {0}; //before update: check LSP.seq

//    std::unordered_set<int> *getAllNeighbors(int v);
    void addEdge(int v, int u);
    void dropEdge(int v, int u);

    void updateGraphCost(int v, int u, int cost);
    std::string packageALLMyKowledge();
    std::string packageMyCurrentLSP();
    void increasemyMsgSeqN();
    void increaseglobMsgSeq();
    void setglobalMsgSeqN(int i);
    void runDijkstraAlgorithm();

    int getTheNextHop(int dest);

    void handleALLK(json receivedALLk);
    void handleACMK(json receivedALLk);



};

#endif //TEST7_LSDB_H
