#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "monitor_neighbors.h"

int globalMyID = 0;
//last time you heard from each node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
struct sockaddr_in globalNodeAddrs[256];
char *costfile;
FILE *logfile;
LSDB *myLSDB;



int main(int argc, char** argv)
{
    if(argc != 4)
    {
        fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
        exit(1);
    }


    //initialization: get this process's node ID, record what time it is,
    //and set up our sockaddr_in's for sending to the other nodes.
    globalMyID = atoi(argv[1]);
    int i;
    for(i=0;i<256;i++)
    {
        gettimeofday(&globalLastHeartbeat[i], 0);

        char tempaddr[100];
        sprintf(tempaddr, "10.1.1.%d", i);
        memset(&globalNodeAddrs[i], 0, sizeof(globalNodeAddrs[i]));
        globalNodeAddrs[i].sin_family = AF_INET;
        globalNodeAddrs[i].sin_port = htons(7777);
        inet_pton(AF_INET, tempaddr, &globalNodeAddrs[i].sin_addr);
    }

    myLSDB = new LSDB(globalMyID);
//    seqNum = 1;


    costfile = argv[2];
    //TODO: read and parse initial costs file. default to cost 1 if no entry for a node. file may be empty.
    FILE *file = fopen(costfile, "r");
    char buf[50];
    int err = 0;
    int neighborId, cost;
    while(fgets(buf, sizeof(buf), file) != NULL){
        err = sscanf(buf, "%d %d", &neighborId, &cost);
//        if (err==EOF){
//            DEBUG_PRINTF("%s", "NOTHING IN COST FILE");
//        }
//        myLSDB->neighbors[neighborId] = 1;
        myLSDB->costs[neighborId] = cost;
        myLSDB->updateGraphCost(globalMyID,neighborId,cost);
    }
    fclose(file);



    //socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
    if((globalSocketUDP=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }
    char myAddr[100];
    struct sockaddr_in bindAddr;
    sprintf(myAddr, "10.1.1.%d", globalMyID);
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(7777);
    inet_pton(AF_INET, myAddr, &bindAddr.sin_addr);
    if(bind(globalSocketUDP, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        close(globalSocketUDP);
        exit(1);
    }


    logfile = fopen(argv[3], "w");


    //start threads... feel free to add your own, and to remove the provided ones.
    pthread_t announcerThread;
    pthread_create(&announcerThread, 0, announceToNeighbors, (void*)0);
    pthread_t checkConnectionsThread;
    pthread_create(&checkConnectionsThread, 0, checkConnections, (void*)0);


    //good luck, have fun!
    listenForNeighbors();



}

