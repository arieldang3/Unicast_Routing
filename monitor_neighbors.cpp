//
// Created by Ariel Dang on 2022/10.
//

#include "monitor_neighbors.h"


void handleALinkIsDown();

void handleALinkIsUp(long num, int i);

//Yes, this is terrible. It's also terrible that, in Linux, a socket
//can't receive broadcast packets unless it's bound to INADDR_ANY,
//which we can't do in this assignment.
void hackyBroadcast(const char* buf, int length)
{
    int i;
    for(i=0;i<256;i++)
        if(i != globalMyID) //(although with a real broadcast you would also get the packet yourself)
            sendto(globalSocketUDP, buf, length, 0,
                   (struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
}

void* announceToNeighbors(void* unusedParam)
{
    struct timespec sleepFor;
    sleepFor.tv_sec = 0;
    sleepFor.tv_nsec = 300 * 1000 * 1000; //300 ms
    while(1)
    {
        hackyBroadcast("HEREIAM", 4);
        nanosleep(&sleepFor, 0);
    }
}

/**
 * keeps heartbeating
 * @param unusedParam
 * @return
 */
void* checkConnections(void* unusedParam){
    struct timespec sleepFor;
    sleepFor.tv_sec = 0;
    sleepFor.tv_nsec = 500 * 1000 * 1000; //500 ms

    int i;
    int yesChanged = 0;

    while(1)
    {
        yesChanged = 0;
        for(i=0;i<256;i++){
            if (i==globalMyID){
                continue;
            }
            struct timeval currentTime;
            gettimeofday(&currentTime, 0);
            long timeInterval = currentTime.tv_sec - globalLastHeartbeat[i].tv_sec;
            if (timeInterval>TIME_OUT){
                if (myLSDB->neighbors[i]==0){ //Correct!
                    continue;
                }
                else{ //1->0; failed
                    myLSDB->neighbors[i]=0;
                    myLSDB->dropEdge(globalMyID, i);
                    yesChanged= 1;
                }
            }
            else{
                if (myLSDB->neighbors[i]==1){ //Correct!
                    continue;
                }
                else{ //0->1; reconnected
                    myLSDB->neighbors[i]=1;
                    myLSDB->addEdge(globalMyID, i);
                    myLSDB->increasemyMsgSeqN();
                    shareALLMyKnowledge(i);
                    yesChanged= 1;
                }
            }
        }
        if (yesChanged){
            // Announce to ALL Neighbors
            myLSDB->increasemyMsgSeqN();
            myLSDB->increaseglobMsgSeq();
            std::string LSP_paskage = "ACMK"+ myLSDB->packageMyCurrentLSP(); //"ACMK" <- "A change in my knowledge"
            const char *LSP_p =LSP_paskage.c_str();
            sendLSPtoALLNeighbors(LSP_p, globalMyID);

            // Change Local info
            myLSDB->runDijkstraAlgorithm();
        }


        nanosleep(&sleepFor, 0);
    }
}

void shareALLMyKnowledge(int thisNode){
//    DEBUG_PRINTF("%s", LSP_paskage);
    std::string LSP_paskage = "ALLK"+ myLSDB->packageALLMyKowledge();
    const char *LSP_p =LSP_paskage.c_str();
    sendto(globalSocketUDP, LSP_p, strlen(LSP_p) + 1, 0,
            (struct sockaddr*) & globalNodeAddrs[thisNode], sizeof(globalNodeAddrs[thisNode]));

}


void sendLSPtoALLNeighbors(const char *LSP, int ReceivedfromThisNode){
    /*
     * method 1
     */
    int i;
    for (i=0;i<256;i++){
        if (myLSDB->neighbors[i]==1 && i!=ReceivedfromThisNode && i!= globalMyID){
            sendto(globalSocketUDP, LSP, strlen(LSP) + 1, 0,
                   (struct sockaddr*) & globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
        }
    }
    /*
     * method 2
     */
//    allNeighbors = myLSDB->getAllNeighbors(globalMyID);
//    for (auto it = allNeighbors->begin(); it != allNeighbors->end(); it++) {
//        sendto(globalSocketUDP, LSP, strlen(LSP) + 1, 0,
//                   (struct sockaddr*) & globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
//    }

}



void listenForNeighbors()
{
    char fromAddr[100];
    struct sockaddr_in theirAddr;
    socklen_t theirAddrLen;
    char recvBuf[1000];

    int bytesRecvd;
    while(1)
    {
        theirAddrLen = sizeof(theirAddr);
        if ((bytesRecvd = recvfrom(globalSocketUDP, recvBuf, 1000 , 0,
                                   (struct sockaddr*)&theirAddr, &theirAddrLen)) == -1)
        {
            perror("connectivity listener: recvfrom failed");
            exit(1);
        }

        inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);

        short int heardFrom = -1;
        if(strstr(fromAddr, "10.1.1."))
        {
            heardFrom = atoi(
                    strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);

            //TODO: this node can consider heardFrom to be directly connected to it; do any such logic now.
//            myLSDB->neighbors[heardFrom]=1;

            // Did this in checkConnections;

            //record that we heard from heardFrom just now.
            gettimeofday(&globalLastHeartbeat[heardFrom], 0);
        }

        //Is it a packet from the manager? (see mp2 specification for more details)
        //send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
        if(!strncmp(recvBuf, "send", 4))
        {
            //TODO send the requested message to the requested destination node
            handleASendMsg(recvBuf,bytesRecvd, heardFrom);


        }
            //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
        else if(!strncmp(recvBuf, "cost", 4))
        {
            //TODO record the cost change (remember, the link might currently be down! in that case,
            //this is the new cost you should treat it as having once it comes back up.)


        }

//        else if (!strncmp(recvBuf, "HERE", 4)){
//
//        }
        else if (!strncmp(recvBuf, "ACMK", 4)){
            char theMsg[5000];
            memset(theMsg, '\0', sizeof(theMsg));
            strcpy(theMsg, recvBuf+4);
            json theknowledgeReceived=json::parse(theMsg);
            myLSDB->handleACMK(theknowledgeReceived);
        }

        else if (!strncmp(recvBuf, "ALLK", 4)){
            char theMsg[5000];
            memset(theMsg, '\0', sizeof(theMsg));
            strcpy(theMsg, recvBuf+4);
            json theknowledgeReceived=json::parse(theMsg);
            myLSDB->handleALLK(theknowledgeReceived);

        }

        //TODO now check for the various types of packets you use in your own protocol
        //else if(!strncmp(recvBuf, "your other message types", ))
        // ...
    }
    //(should never reach here)
    close(globalSocketUDP);
}

//send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
void handleASendMsg(char *buf, int bytesRecvd, int heardFrom){
    short destID;
    std::memcpy(&destID, buf+4, 2);
    char theMsg[200];
    memset(theMsg, '\0', sizeof(theMsg));
    strcpy(theMsg, buf+6);
    destID = ntohs(destID);
    int nextHop = myLSDB->getTheNextHop(destID);
    if (nextHop==-1){
        log_unreachable_msg(destID);
    }
    else{
        if (nextHop==globalMyID){
            log_received_msg(theMsg);
        }
        else{
            sendto(globalSocketUDP, buf, bytesRecvd, 0,
                   (struct sockaddr*)&globalNodeAddrs[nextHop], sizeof(globalNodeAddrs[nextHop]));
            if (heardFrom != -1) {
                log_forward_msg(destID, nextHop, theMsg);
            } else {
                log_sending_msg(destID, nextHop, theMsg);
            }
        }

    }

}


void log_unreachable_msg(int dest){
    char buffer[500];
    memset(buffer, '\0', sizeof buffer);
    sprintf(buffer, "unreachable dest %d\n", dest);
    fwrite(buffer, sizeof(char), strlen(buffer), logfile);
}

void log_received_msg(char *msg){
    char buffer[500];
    memset(buffer, '\0', sizeof buffer);
    sprintf(buffer, "receive packet message %s\n", msg);
    fwrite(buffer, sizeof(char), strlen(buffer), logfile);
}

void log_forward_msg(int dest, int nexthop, char *msg){
    char buffer[500];
    memset(buffer, '\0', sizeof buffer);
    sprintf(buffer, "forward packet dest %d nexthop %d message %s\n", dest, nexthop, msg);
    fwrite(buffer, sizeof(char), strlen(buffer), logfile);
}

void log_sending_msg(int dest, int nexthop, char *msg){
    char buffer[500];
    memset(buffer, '\0', sizeof buffer);
    sprintf(buffer, "sending packet dest %d nexthop %d message %s\n", dest, nexthop, msg);
    fwrite(buffer, sizeof(char), strlen(buffer), logfile);
}