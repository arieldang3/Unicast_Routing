//
// Created by Ariel Dang on 2022/10.
//

#ifndef TEST7_MONITOR_NEIGHBORS_H
#define TEST7_MONITOR_NEIGHBORS_H


// the main file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
// Same thing but Linux version:
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
// The following are for windows:
//#include <Winsock2.h>
//#include <ws2tcpip.h>
//#include <pthread.h>


#include "LSDB.h"
//#include "Graph.h"
#include "json.hpp"
using json = nlohmann::json;


extern int globalMyID;
//last time you heard from each node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
extern struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
extern int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
extern struct sockaddr_in globalNodeAddrs[256];

extern FILE *logfile;
extern LSDB *myLSDB;
//extern long seqNum;
//extern std::unordered_set<int> *allNeighbors;
#define TIME_OUT 1




//Yes, this is terrible. It's also terrible that, in Linux, a socket
//can't receive broadcast packets unless it's bound to INADDR_ANY,
//which we can't do in this assignment.
void hackyBroadcast(const char* buf, int length);
void listenForNeighbors();
void* announceToNeighbors(void* unusedParam);
void* checkConnections(void* unusedParam);


void handleALinkIsDown(long seqNum, int src, int theNeighbor, int cost);
void handleALinkIsUp(long seqNum, int src, int theNeighbor, int cost);



// Thread:
// keep sending heartbeat; if no longer neighbor: 1->0 in LSDB.neighbors


void sendLSPtoALLNeighbors(const char *LSP, int ReceivedfromThisNode);
int receiveLSPfromANEighbor();
void shareALLMyKnowledge(int thisNode);

void handleASendMsg(char *buf);

void log_unreachable_msg(int dest);
void log_received_msg(char *msg);
void log_forward_msg(int dest, int nexthop, char *msg);
void log_sending_msg(int dest, int nexthop, char *msg);
void handleASendMsg(char *buf, int bytesRecvd, int heardFrom);







#endif //TEST7_MONITOR_NEIGHBORS_H
