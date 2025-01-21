#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include "common.h"
#include "logger.h"
#include "network.h"
#define MAX_CLIENTS 10
#define ERROR_ACK "Error: Wrong password or username. Try again..."
#define ACK_ "Authenticate yourself"

pthread_mutex_t status_lock = PTHREAD_MUTEX_INITIALIZER;

const char *services_str =
    "******************************************************\n\r"
    "0- LISTS_DIR <path>    : lists directories\n\r"
    "1- CAT_DIR <filename>  : returns file content\n\r"
    "2- ELAPSED_TIME        : returns elapsed time\n\r"
    "3- DATE_TIME    : returns current date & time\n\r"
    "4- END                 : exits program\n\r"
    "*******************************************************";
typedef struct
{
  Services_e currService;
  // Message message_send;
  // Message message_rcv;
  int id;
  bool authenticated;
} client_state;

typedef struct
{
  Services_e currService;
  pthread_t threads[MAX_CLIENTS];
  // client_state ConnectedClientStruct[MAX_CLIENTS];
  int connectedClients;
} server_status;

server_status ser_status = {
    // .authenticated = false,
    .currService = INIT, // Replace `SOME_SERVICE` with an appropriate value from `Services_e`.
    .connectedClients = 0,
    .threads = {0}, // Initialize all pthread_t elements to 0.
                    // .ConnectedClientStruct = {
                    //     [0 ... MAX_CLIENTS - 1] = {                     // Initialize all `client_state` elements
                    //                                .currService = INIT, // Replace `SOME_SERVICE` with a default value.
                    //                                .message_send = {0}, // Assuming `Message` has its own initializer.
                    //                                .message_rcv = {0},  // Assuming `Message` has its own initializer.
                    //                                .id = -1,            // Set default IDs to -1 or another value.
                    //                                .authenticated = false,
                    //                                .free = true }}
};
int ConnectToRemoteService(int p);
int server_socket = 0;
void handle_interrupt(int sig);

void *client_handler(void *socket_desc);

int main()
{
  printf("========================================\n");
  printf("RUNNING MULTI SERVER MULTI CLIENTS \n");
  printf("========================================\n");
  signal(SIGINT, handle_interrupt);

  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  // Initialize the server socket
  server_socket = SafeInitSocketServer(PORT);
  safeListen(server_socket);

  LOG_INFO("Server started. Waiting for connections...");

  while (1)
  {
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket < 0)
    {
      LOG_ERROR("Error accepting connection.");
      continue;
    }

    LOG_INFO("Connection accepted from %s:%d",
             inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    pthread_t client_thread;
    int *new_sock = malloc(sizeof(int));
    *new_sock = client_socket;
    pthread_mutex_lock(&status_lock);
    if (pthread_create(&ser_status.threads[ser_status.connectedClients], NULL, client_handler, (void *)new_sock) < 0)
    {
      LOG_ERROR("Could not create thread for client.");
      close(client_socket);
      free(new_sock);
    }
    else
    {
      ser_status.connectedClients++;
      LOG_INFO("Client handler assigned. Connected clients: <%d>", ser_status.connectedClients);
    }
    pthread_mutex_unlock(&status_lock);
  }

  close(server_socket);
  return 0;
}

void *client_handler(void *socket_desc)
{
  int client_socket = *(int *)socket_desc;
  free(socket_desc); // Free the dynamically allocated socket descriptor
  int client_id = ser_status.connectedClients;
  Message message_send = {0};
  Message message_rcv = {0};
  client_state client_status = {
      .authenticated = false,
      .id = client_id,
      .currService = AUTH,
  };

  LOG_INFO("Client handler started: ID  <%d> .", client_id);

  // Authentication phase
  int trials = 0;
  int elapSock = 0 ; 
  bool status_elap_serv_status = false;
  strcpy(message_send.payload.ackPayload.msgACK, ACK_);
  message_send.serviceType = ACK;

  safe_send(client_socket, client_socket, &message_send);

  while (trials < 3 && !client_status.authenticated)
  {
    safe_rcv(client_socket, client_socket, &message_rcv);

    if (message_rcv.serviceType == AUTH)
    {
      AuthPayload auth_payload = message_rcv.payload.authPayload;
      client_status.authenticated = Authenticate(auth_payload.username, auth_payload.password);

      if (client_status.authenticated)
      {
        LOG_INFO("Client authenticated successfully.");
        strcpy(message_send.payload.ackPayload.msgACK, "1"); // Success
      }
      else
      {
        LOG_ERROR("Authentication failed.");
        strcpy(message_send.payload.ackPayload.msgACK, ERROR_ACK); // Failure
        trials++;
      }
      safe_send(client_socket, client_socket, &message_send);
    }
    else if (message_rcv.serviceType == END)
    {
      LOG_ERROR("Client id <%d> disconnected  ...", client_id);
      __sync_fetch_and_sub(&ser_status.connectedClients, 1);
      close(client_socket);
      pthread_exit(NULL);
    }
  }

  if (!client_status.authenticated)
  {
    LOG_ERROR("Client failed authentication after 3 attempts. Disconnecting...");
    __sync_fetch_and_sub(&ser_status.connectedClients, 1);
    close(client_socket);
    pthread_exit(NULL);
  }

  // Invoke service elapse time ...
  elapSock = SafeInitSocketClient(PORT_ELAPSED_TIME_SERVICE);
  if (elapSock >= 0)
  {

    message_send.serviceType = ELASPSED_TIME;
    message_send.id = client_id;

    safe_send(elapSock, server_socket, &message_send);

    // Receive client's choice
    safe_rcv(elapSock, server_socket, &message_rcv);
    if (message_rcv.serviceType == ACK)
    {
      LOG_INFO("Client ID <%d>: %s", client_id, message_rcv.payload.ackPayload.msgACK);
    }
    else
    {
      status_elap_serv_status = false;
      LOG_ERROR("Error initializing the Elpsed time ");
    }
  }
  else
  {
    status_elap_serv_status = false;
  }
  status_elap_serv_status = true;

  // getElapsedTime(true, NULL);
  int remote_sock = 0;
  while (1)
  {
    message_send.id = client_id;
    message_rcv.id = client_id;
    remote_sock = -1;
    LOG_INFO("Sending available services to client...");
    strcpy(message_send.payload.ackPayload.msgACK, services_str);
    message_send.serviceType = ACK;
    safe_send(client_socket, server_socket, &message_send);

    // Receive client's choice
    safe_rcv(client_socket, server_socket, &message_rcv);

    switch (message_rcv.serviceType)
    {
    case LISTS_DIR:
    {
      LOG_INFO("Calling LISTS_DIR service.");
      printf("Client ID <%d> Requested List dir: %s\n", client_id, message_rcv.payload.listCatPayload.directory_path);
      // connect to remote service
      remote_sock = SafeInitSocketClient(PORT_LIST_DIR_SERVICE);
      if (remote_sock < 0)
      {
        LOG_ERROR("Unable to connect to remote service LIST DIR %d disrigrading client request ", PORT_LIST_DIR_SERVICE);
        strcpy(message_send.payload.listCatPayload.output, "Error ...");
      }
      else
      {

        safe_send(remote_sock, server_socket, &message_rcv);

        safe_rcv(remote_sock, server_socket, &message_send);
        close(remote_sock);
      }
      // send to client output if valid
      printf("OUTPUT %s\n", message_send.payload.listCatPayload.output);
      printf("PATH %s\n", message_send.payload.listCatPayload.directory_path);
      safe_send(client_socket, server_socket, &message_send);

      break;
    }
    case DATE_TIME:
    {
      LOG_INFO("Calling DATE_TIME service.");

      printf("Client ID <%d> Requested List dir: %s\n", client_id, message_rcv.payload.listCatPayload.directory_path);
      // connect to remote service
      message_rcv.id = client_id;

      remote_sock = SafeInitSocketClient(PORT_TIME_DATE_SERVICE);
      if (remote_sock < 0)
      {
        LOG_ERROR("Unable to connect to remote service TIME_DATE_SERVICER %d disrigrading client request ", PORT_TIME_DATE_SERVICE);
        message_send.serviceType = ACK;

        strcpy(message_send.payload.ackPayload.msgACK, "Host unable to Reach remote service.");
        safe_send(client_socket, server_socket, &message_send);
      }
      else
      {
        safe_send(remote_sock, server_socket, &message_rcv);

        safe_rcv(remote_sock, server_socket, &message_send);
        printf("time %s\n", message_send.payload.timeDatePayload.time);
        printf("date %s\n", message_send.payload.timeDatePayload.date);
        safe_send(client_socket, server_socket, &message_send);
      }
      // send to client output if valid
      close(remote_sock);

      break;
    }
    case CAT_DIR:
    {
      LOG_INFO("Calling CAT_DIR  service.");
      printf("Client ID <%d> Requested List dir: %s\n", client_id, message_rcv.payload.listCatPayload.directory_path);
      // connect to remote service
      message_rcv.id = client_id;

      remote_sock = SafeInitSocketClient(PORT_CAT_CONTENT_SERVICE);
      if (remote_sock < 0)
      {
        LOG_ERROR("Unable to connect to remote service CAT FILE CONTENTS %d disrigrading client request ", PORT_CAT_CONTENT_SERVICE);
        strcpy(message_send.payload.listCatPayload.output, "Error ...");
      }
      else
      {
        safe_send(remote_sock, server_socket, &message_rcv);

        safe_rcv(remote_sock, server_socket, &message_send);
        close(remote_sock);
      }
      // send to client output if valid
      printf("OUTPUT %s\n", message_send.payload.listCatPayload.output);
      printf("PATH %s\n", message_send.payload.listCatPayload.directory_path);
      safe_send(client_socket, server_socket, &message_send);
      break;
    }
    case ELASPSED_TIME:
    {
      LOG_INFO("Calling ELAPSED_TIME service.");
      if (status_elap_serv_status)
      {
        message_send.id = client_id;

        message_send.serviceType=ELASPSED_TIME;
        safe_send(elapSock, server_socket, &message_send);
        // Receive client's choice
        safe_rcv(elapSock, server_socket, &message_rcv);
        if (message_rcv.serviceType == ELASPSED_TIME)
        {
          printf("Elaspsed time %f\n", message_rcv.payload.elapsedTimePayload.elapsed_time_s);
          safe_send(client_socket, server_socket, &message_rcv);
        }
        else
        {
          LOG_ERROR("ERROR CLient ID <%d>:  Unkown data type return Elapsed Time", client_id);
          message_send.serviceType = ACK;
          strcpy(message_send.payload.listCatPayload.output, "Error return type");
          safe_send(client_socket, server_socket, &message_send);
        }
      }
      else
      {
        message_send.serviceType = ACK;
        strcpy(message_send.payload.listCatPayload.output, "ELAPSED Time Service is DOWN ...");
        safe_send(client_socket, server_socket, &message_send);
      }
      break;
    }
    case END:
    {
      LOG_INFO("Client requested termination.");
      __sync_fetch_and_sub(&ser_status.connectedClients, 1);
      LOG_INFO("Connected Clients <%d>", ser_status.connectedClients);
      // send to elap sock to close
      message_send.serviceType = END;
      safe_send(elapSock, server_socket, &message_send);

      close(elapSock);
      close(client_socket);
      pthread_exit(NULL);
      break;
    }
    default:
      LOG_WARNING("Unknown service type requested., EXITING ");
      __sync_fetch_and_sub(&ser_status.connectedClients, 1);
      // send to elap sock to close
      message_send.serviceType = END;
      safe_send(elapSock, server_socket, &message_send);

      close(elapSock);
      close(client_socket);

      pthread_exit(NULL);
      break;
    }
  }
}
void handle_interrupt(int sig)
{
  LOG_ERROR("Interrupt signal received. Cleaning up...");
  // join all active threads
  for (int i = 0; i < ser_status.connectedClients; i++)
  {
    pthread_join(ser_status.threads[i], NULL);
  }

  LOG_INFO("Closing server socket...");
  close(server_socket);
  exit(EXIT_SUCCESS);
}
