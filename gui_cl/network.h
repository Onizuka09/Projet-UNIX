#ifndef NETWORK_H
#define NETWORK_H
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "common.h"

int SafeInitSocketServer(int port);
int SafeInitSocketClient(int port);
void safeListen(int queueSize); 
void safe_send(int service_fd, int sock, Message* message); 

void safe_rcv(int service_fd, int sock, Message* message); 


#endif 