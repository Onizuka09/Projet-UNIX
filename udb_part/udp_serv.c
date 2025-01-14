#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "logger.h"
#include "common.h"
#include <time.h>
uint8_t buff[255] = {0};
int port = 0;
message rcv = {0};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        LOG_MESSAGE(DEBUG_LEVEL_WARNING, "Usage %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    port = atoi(argv[1]);

    // Initialize structs
    struct sockaddr_in servSockAddr = {0}, clientSockAddr = {0};
    socklen_t clientSockAddr_t = sizeof(clientSockAddr);
    int servSocket = 0;

    // Create server socket
    servSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (servSocket < 0) {
        LOG_MESSAGE(DEBUG_LEVEL_ERROR, "Error creating socket server, EXITING...\n");
        exit(EXIT_FAILURE);
    }

    // Setup sockaddr
    servSockAddr.sin_family = AF_INET;
    servSockAddr.sin_port = htons(port);
    servSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(servSocket, (struct sockaddr *)&servSockAddr, sizeof(servSockAddr)) < 0) {
        LOG_MESSAGE(DEBUG_LEVEL_ERROR, "Error binding, EXITING...\n");
        close(servSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server Listening on Port: %d...\n", port);

    while (1) {
        // Receive data
        int status = recvfrom(servSocket, &rcv, sizeof(rcv), 0, (struct sockaddr *)&clientSockAddr, &clientSockAddr_t);
        if (status < 0) {
            perror("recvfrom failed");
            continue;
        }

        // Log client information
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientSockAddr.sin_addr, clientIP, sizeof(clientIP));
        printf("Received message from %s:%d\n", clientIP, ntohs(clientSockAddr.sin_port));

        printf("Message received: %d\n", rcv.n);
        for (int i = 0; i < rcv.n; i++) {
            rcv.arr[i] = rand() % NMAX;
        }

        printf("Sending...\n");
        if (sendto(servSocket, &rcv, sizeof(rcv), 0, (struct sockaddr *)&clientSockAddr, clientSockAddr_t) < 0) {
            perror("SEND FAILED");
        }else{ 
          printf("send complete \n");
        }
    }

    close(servSocket);
    return 0;
}
