//
// Created by Ariel Dang on 2022/10.
//

#include "LSDB.h"
#include <stdio.h>
#include <string.h>


LSDB::LSDB(int myRouterId) {
    myId = myRouterId;
    costs[myRouterId] = 0;
    SPT[myRouterId] = myRouterId;
    myMsgSeqN = 0;
    globalMsgSeqN = 0;
    neighbors[myRouterId] = 1;
//    memset(SPT, -1, ROUTER_NUM);
//    my_knowledge_of_graph = new Graph();
    vertices = new std::unordered_set<int>();
    vertices->insert(myRouterId);
    edges = new std::unordered_map<int, std::unordered_set<int>>();
    for (int i = 0 ;i<ROUTER_NUM;i++) {
        for(int j = 0;j<ROUTER_NUM;j++){
            if (i==j){
                graphCostTable[i][j]=0;
                continue;
            }
            graphCostTable[i][j]=1;
        }
    }
}

LSDB::~LSDB() {
    delete vertices;
    delete edges;
}

//std::unordered_set<int> *LSDB::getAllNeighbors(int v) {
//    return my_knowledge_of_graph->getAllNeighbors(v);
//}


void LSDB::dropEdge(int v, int u) {
//    my_knowledge_of_graph->dropEdge(v,u);
    auto iter1 = edges->find(v);
    if (iter1 != edges->end()){
        iter1->second.erase(u);
    }
    auto iter2 = edges->find(u);
    if (iter2!=edges->end()){
        iter2->second.erase(v);
    }
    if (iter1->second.size()==0){
        vertices->erase(v);
    }
    if (iter2->second.size()==0){
        vertices->erase(u);
    }
//    if (edges->at(v).size()==0){
//        vertices->erase(v);
//    }
//    if (edges->at(u).size()==0){
//        vertices->erase(u);
//    }
}

void LSDB::addEdge(int v, int u) {
    vertices->insert(v);
    vertices->insert(u);
    auto iter1 = edges->find(v);
    if (iter1 == edges->end()){
        std::unordered_set<int> neighs {u};
        edges->insert({v,neighs});
    }
    else{
        iter1->second.insert(u);
    }
    auto iter2 = edges->find(u);
    if (iter2 == edges->end()){
        std::unordered_set<int> neighs {v};
        edges->insert({u,neighs});
    }
    else{
        iter1->second.insert(v);
    }
}

void LSDB::updateGraphCost(int v, int u, int cost) {
    graphCostTable[v][u]= cost;
    graphCostTable[u][v]= cost;
}

std::string LSDB::packageALLMyKowledge() {
    std::unordered_map<int, std::unordered_map<int,int> > *my_knowledge_of_graph = new std::unordered_map<int, std::unordered_map<int,int> >();
//    std::unordered_map<int, int> my_knowledge_of_cost;
    for (auto it : (*edges) ){
        std::unordered_map<int,int> outEFromThisNode = {};
        for (auto it2 : it.second){
            outEFromThisNode.insert({it2, graphCostTable[it.first][it2]});
        }
        my_knowledge_of_graph->insert({ it.first, outEFromThisNode });
    }

    json LSP = {{"src", myId}, {"my_knowledge_of_graph", *my_knowledge_of_graph}, {"SeqNum", myMsgSeqN}, {"GlobalSeqNum", globalMsgSeqN}};
    std::string LSP_dump = LSP.dump();

    delete my_knowledge_of_graph;

    return LSP_dump;
}

std::string LSDB::packageMyCurrentLSP() {
    std::unordered_map<int,int> myCurrentNeighbors={};
//    DEBUG_PRINT("1\n");
    for (auto it: edges->at(myId)){
        myCurrentNeighbors.insert({it,graphCostTable[myId][it]});
    }

    json LSP = {{"src", myId}, {"my_knowledge_of_graph", myCurrentNeighbors}, {"SeqNum", myMsgSeqN}, {"GlobalSeqNum", globalMsgSeqN}};
    std::string LSP_dump = LSP.dump();

    return LSP_dump;
}

void LSDB::increasemyMsgSeqN() {
    ++myMsgSeqN;
//    ++globalMsgSeqN;
}

void LSDB::increaseglobMsgSeq() {
    ++globalMsgSeqN;
}

void LSDB::runDijkstraAlgorithm() {
    std::unordered_set<int> *unknown = new std::unordered_set<int>(vertices->begin(), vertices->end());
    std::unordered_map<int, int> *dist = new std::unordered_map<int, int>();
    std::unordered_map<int, int> *prev = new std::unordered_map<int, int>();


    // TODO: (May cause problem)
    for (int i; i<ROUTER_NUM;i++){
        if (neighbors[i]==1){
            vertices->insert(i);
        }
    }

    // init
    for (auto it: (*vertices) ){
        dist->insert({it, INT_MAX});
        prev->insert({it,it});
        unknown->insert(it);
    }
//    DEBUG_PRINT("2\n");
    dist->at(myId)=0;
//    DEBUG_PRINT("DOUBLE2\n");

    // findShortestPaths
    while (unknown->size()!=0){
//        DEBUG_PRINT("HERE: the unknown0: will enter the findMinDistVertex\n");
        int minDis=findMinDistVertex(unknown, dist);
//            cout<<minDis->toString()<<endl;
//        DEBUG_PRINT("HERE: the unknown000: will enter the relaxOutGoingEdges\n");
        unknown->erase(minDis);
        relaxOutGoingEdges(minDis, dist, prev);

    }

    // store the spt:
    memset(SPT, -1, ROUTER_NUM);
    for (auto it : (*prev) ){
        SPT[it.first] = it.second;
    }


    delete dist;
    delete prev;
    delete unknown;
}

int LSDB::findMinDistVertex(std::unordered_set<int> *unknown, std::unordered_map<int, int> *dist) {
    int minDisV;
    int mindis=INT_MAX;
    for (auto v: *unknown){
//        DEBUG_PRINT("3\n");
//        DEBUG_PRINTF("HERE: the unknown: %d\n",v);
        if (dist->at(v) < mindis){
            minDisV=v;
//            DEBUG_PRINT("4\n");
//            DEBUG_PRINTF("HERE: the unknown2: %d\n",v);
            mindis=dist->at(v);
//            DEBUG_PRINTF("HERE: the unknown3: %d\n",v);
        }
    }

//    DEBUG_PRINTF("HERE: minDisV: %d\n",minDisV);

    return minDisV;
}

void LSDB::relaxOutGoingEdges(int v, std::unordered_map<int, int> *dist, std::unordered_map<int, int> *prev) {
    for (auto it : edges->at(v)){
//        DEBUG_PRINT("5\n");
//        DEBUG_PRINTF("for (auto it : edges->at(v)): %d\n",it);
//        DEBUG_PRINTF("dist->at(it) > dist->at(v) + graphCostTable[v][it]  ): %d\n",dist->at(it));
        if (dist->at(it) > dist->at(v) + graphCostTable[v][it]  ){
            dist->at(it) = dist->at(v) + graphCostTable[v][it];
            prev->at(it) = v;
        }
    }
}

int LSDB::getTheNextHop(int dest) {
    if (SPT[dest]==-1){
        return -1;
    }
    else{
        int next = myId;
        int cur = dest;
        while ( cur!=myId){
            next = cur;
            cur = SPT[next];
        }
        return next;
    }
}

void LSDB::handleALLK(json receivedALLk) {
//    DEBUG_PRINT("6\n");
    int thegsq = receivedALLk.at("GlobalSeqNum");
    int theseq = receivedALLk.at("SeqNum");
    int scr = receivedALLk.at("src");
//    if (thegsq<globalMsgSeqN){
//        return;
//    }
    if (theseq <= LSPSeqNumReceivedfromThisNode[scr] ){
        return;
    }

//    DEBUG_PRINT("7\n");
    auto receivedGraph = receivedALLk.at("my_knowledge_of_graph");
//    DEBUG_PRINTF("%s",receivedGraph);

    globalMsgSeqN = thegsq;
    LSPSeqNumReceivedfromThisNode[scr] = theseq;
    edges->clear();
    vertices->clear();

    for (int i =0; i<receivedGraph.size();i++){
//        DEBUG_PRINT("8\n");
        int temp = receivedGraph.at(i).at(0);
        auto temp2 = receivedGraph.at(i).at(1);
        vertices->insert(temp);
        std::unordered_set<int> outEFromThisNode = {};
        for (int j = 0; j<temp2.size(); j++){
//            DEBUG_PRINT("9\n");
            int tempcost = temp2.at(j).at(1);
            int u = temp2.at(j).at(0);
            graphCostTable[temp][u] = tempcost;
            graphCostTable[u][temp] = tempcost;
            outEFromThisNode.insert(u);
        }
        edges->insert({temp, outEFromThisNode});
    }

    runDijkstraAlgorithm();

}

void LSDB::handleACMK(json receivedALLk) {
//    DEBUG_PRINT("10\n");
    int thegsq = receivedALLk.at("GlobalSeqNum");
    int theseq = receivedALLk.at("SeqNum");
    int scr = receivedALLk.at("src");
    if (thegsq<globalMsgSeqN){ //"I have known this graph"
        return;
    }
    if (theseq <= LSPSeqNumReceivedfromThisNode[scr] ){
        return;
    }

//    DEBUG_PRINT("11\n");
    auto receivedGraph = receivedALLk.at("my_knowledge_of_graph");
//    DEBUG_PRINTF("%s", LSP_paskage);
    globalMsgSeqN = thegsq;
    LSPSeqNumReceivedfromThisNode[scr] = theseq;

    auto iter = edges->find(scr);
    if (iter==edges->end()){
        std::unordered_set<int> outEFromThisNode = {};
        for (int i =0; i<receivedGraph.size();i++){
//            DEBUG_PRINT("12\n");
            int nei = receivedGraph.at(i).at(0);
            int cos = receivedGraph.at(i).at(1);
            outEFromThisNode.insert(nei);
            graphCostTable[nei][scr]=  cos;
            graphCostTable[scr][nei]=  cos;
            auto iter2 = edges->find(nei);
            if (iter2==edges->end()){
                std::unordered_set<int> backwrad = {scr};
                edges->insert({nei, backwrad});
            }
            else{
                iter2->second.insert(scr);
            }
        }
        edges->insert({scr, outEFromThisNode});
    }
    else{
        std::unordered_set<int> tempPrev(iter->second.begin(), iter->second.end());
        for (int i : tempPrev){
            dropEdge(scr, i);
        }
        for (int i =0; i<receivedGraph.size();i++){
//            DEBUG_PRINT("13\n");
            int temp = receivedGraph.at(i).at(0);
            addEdge(scr,temp);
            int cos = receivedGraph.at(i).at(1);
            graphCostTable[temp][scr]=  cos;
            graphCostTable[scr][temp]=  cos;
        }
    }

    runDijkstraAlgorithm();
}




