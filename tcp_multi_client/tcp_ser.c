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
#define MAX_CLIENTS  10
#define ERROR_ACK "Error: Wrong password or username. Try again..."
#define ACK_ "Authenticate yourself"

pthread_mutex_t status_lock = PTHREAD_MUTEX_INITIALIZER;

const char *services_str =
    "******************************************************\n\r"
    "0- LISTS_DIR <path>    : lists directories\n\r"
    "1- CAT_DIR <filename>  : returns file content\n\r"
    "2- ELAPSED_TIME        : returns elapsed time\n\r"
    "3- DATE_TIME <path>    : returns current date & time\n\r"
    "4- END                 : exits program\n\r"
    "*******************************************************";

typedef struct
{
  bool authenticated;
  Services_e currService;
  pthread_t threads[MAX_CLIENTS]; //
  int connectedClients; 
} server_status;
server_status ser_status = {.authenticated = false,.connectedClients=0, };

int server_socket = 0;

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

void *client_handler(void *socket_desc);

int main()
{
  printf("========================================\n"); 
  LOG_INFO("RUNNING MONO SERVER MULTI CLIENTS "); 
  printf("========================================\n"); 
  signal(SIGINT, handle_interrupt);

  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  // Initialize the server socket
  server_socket = SafeInitSocket(PORT);
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
    LOG_INFO("Client handler assigned. Connected clients: %d", ser_status.connectedClients);
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

  Message message_send = {0};
  Message message_rcv = {0};
  server_status client_status = {.authenticated = false,.connectedClients=0};

  LOG_INFO("Client handler started.");

  // Authentication phase
  int trials = 0;
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
  }

  if (!client_status.authenticated)
  {
    LOG_ERROR("Client failed authentication after 3 attempts. Disconnecting...");
    __sync_fetch_and_sub(&ser_status.connectedClients , 1); 
    close(client_socket);
    pthread_exit(NULL);
  }

  // Service phase
  getElapsedTime(true, NULL);

  while (1)
  {
    LOG_INFO("Sending available services to client...");
    strcpy(message_send.payload.ackPayload.msgACK, services_str);
    message_send.serviceType = ACK;
    safe_send(client_socket, client_socket, &message_send);

    // Receive client's choice
    safe_rcv(client_socket, client_socket, &message_rcv);

    switch (message_rcv.serviceType)
    {
    case LISTS_DIR:
    {
      LOG_INFO("Handling LISTS_DIR service.");
      ListCatPayload list_dirs = {0};
      strcpy(list_dirs.directory_path, message_rcv.payload.listCatPayload.directory_path);
      ListDirectories(list_dirs.directory_path, list_dirs.output, sizeof(list_dirs.output));
      message_send.serviceType = LISTS_DIR;
      message_send.payload.listCatPayload = list_dirs;
      safe_send(client_socket, client_socket, &message_send);
      break;
    }
    case DATE_TIME:
    {
      LOG_INFO("Handling DATE_TIME service.");
      TimeDatePayload td = {0};
      getDateTime(td.time, sizeof(td.time), td.date, sizeof(td.date));
      message_send.serviceType = DATE_TIME;
      message_send.payload.timeDatePayload = td;
      safe_send(client_socket, client_socket, &message_send);
      break;
    }
    case CAT_DIR:
    {
      LOG_INFO("Handling CAT_DIR service.");
      ListCatPayload cat_file = {0};
      strcpy(cat_file.directory_path, message_rcv.payload.listCatPayload.directory_path);
      getFileContent(cat_file.directory_path, cat_file.output, sizeof(cat_file.output));
      message_send.serviceType = CAT_DIR;
      message_send.payload.listCatPayload = cat_file;
      safe_send(client_socket, client_socket, &message_send);
      break;
    }
    case ELASPSED_TIME:
    {
      LOG_INFO("Handling ELAPSED_TIME service.");
      ElapsedTimePayload elapsed = {0};
      getElapsedTime(false, &elapsed.elapsed_time_s);
      message_send.serviceType = ELASPSED_TIME;
      message_send.payload.elapsedTimePayload = elapsed;
      safe_send(client_socket, client_socket, &message_send);
      break;
    }
    case END:
    {
      LOG_INFO("Client requested termination.");
      __sync_fetch_and_sub(&ser_status.connectedClients , 1); 
      LOG_INFO("Connected Clients %d ", ser_status.connectedClients); 

      close(client_socket);
      pthread_exit(NULL);
    }
    default:
      LOG_WARNING("Unknown service type requested.");
      break;
    }
  }
}
