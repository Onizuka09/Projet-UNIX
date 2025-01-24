/*
 * Client side Using UDP,
 * Created 1/14/2025, Moktar SELLAMI
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "logger.h"
#include <string.h>
#include <time.h>

#include "common.h"

uint8_t buff[255] = {0};
int port = 0;
const char *hostanme;
int tmpn = 0;
message msg_send = {0};

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    LOG_MESSAGE(DEBUG_LEVEL_WARNING, "Usage %s <hostanme> <port> \n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // init random number generator
  srand(time(NULL));

  port = atoi(argv[2]);
  hostanme = argv[1];

  // inititalizing structs
  struct sockaddr_in servSockAddr = {0}, clientSockAddr = {0};
  socklen_t clientSockAddr_t = {0};
  int servSocket = 0;
  // create server socket
  servSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (servSocket < 0)
  {
    LOG_MESSAGE(DEBUG_LEVEL_ERROR, "Error creating socket server, EXITING ... \n");
    exit(EXIT_FAILURE);
  }
  // setup sockaddr
  servSockAddr.sin_family = AF_INET;
  servSockAddr.sin_port = htons(port);
  servSockAddr.sin_addr.s_addr = inet_addr(hostanme);

  printf(" Connecting to %s on Port: %d ... \n", hostanme, port);
  uint8_t BUFFER[] = "Hello form client ";
  printf("Genrating a random number ...: \n");
  tmpn = rand() % NMAX;

  printf("Generated number %d\n", tmpn); 
  msg_send.n = tmpn;
  if (sendto(servSocket, &msg_send, sizeof(msg_send), 0, (struct sockaddr *)&servSockAddr, sizeof(servSockAddr)) < 0)
  {
    perror("SEND FAILED ");
  }

  printf("Send complete \n");

  int status = recvfrom(servSocket, &msg_send, sizeof(msg_send), 0, (struct sockaddr *)&clientSockAddr, &clientSockAddr_t);
  if (status < 0)
  {
    LOG_ERROR("Error while Recieving from message \n");
  }
  printf(" Message received %d\n", msg_send.n);
  for (int i = 0; i < msg_send.n; i++)
  {
    printf("msges received %d \n", msg_send.arr[i]);
  }

  return 0;
}
