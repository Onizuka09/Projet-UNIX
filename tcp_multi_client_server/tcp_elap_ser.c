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

typedef struct
{
    pthread_t threads[MAX_CLIENTS];
    int numReq;
} service_status;

void handle_interrupt(int sig);
void *host_handler(void *socket_desc);

service_status ser_status = {
    .numReq = 0,
    .threads = {0}, // Initialize all pthread_t elements to 0.
};

int service_socket = 0;

int main()
{
    printf("========================================\n");
    printf("RUNNING ELAPSED TIME SERVICE \n");
    printf("========================================\n");
    signal(SIGINT, handle_interrupt);

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Initialize the server socket and Bind it
    service_socket = SafeInitSocketServer(PORT_ELAPSED_TIME_SERVICE);
    safeListen(service_socket);

    LOG_INFO("Service Elapsed Time started. Waiting for connections...");

    while (1)
    {
        // accept connections
        int host_socket = accept(service_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (host_socket < 0)
        {
            LOG_ERROR("Error accepting connection.");
            continue;
        }

        LOG_INFO("Connection accepted from HOST %s:%d",
                 inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_t client_thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = host_socket;
        pthread_mutex_lock(&status_lock);
        if (pthread_create(&ser_status.threads[ser_status.numReq], NULL, host_handler, (void *)new_sock) < 0)
        {
            LOG_ERROR("Could not create thread for client.");
            close(host_socket);
            free(new_sock);
        }
        else
        {
            ser_status.numReq++;
            LOG_INFO("Service Request handler, Request num: <%d>", ser_status.numReq);
        }
        pthread_mutex_unlock(&status_lock);
    }

    close(service_socket);
    return 0;
}

void *host_handler(void *socket_desc)
{
    int host_socket = *(int *)socket_desc;
    free(socket_desc); // Free the dynamically allocated socket descriptor
    Message message_send = {0};
    Message message_rcv = {0};
    LOG_INFO("Waiting ... ");
    int client_id =0;
    LOG_INFO("Handling Request num <%d> for client <%d>", ser_status.numReq, client_id);
    bool running = true;
    bool first = true;
    while (running)
    {
        safe_rcv(host_socket, service_socket, &message_rcv);
        client_id = message_rcv.id;
        if (message_rcv.serviceType == ELASPSED_TIME && first)
        {
            getElapsedTime(true, NULL);
            LOG_INFO("Client ID <%d>: Initialized Timer, waiting for user Request", client_id);    
            message_send.serviceType = ACK;
            strcpy(message_send.payload.listCatPayload.output, "Initialized Timer, waiting for user Request");


            safe_send(host_socket, service_socket, &message_send);
            
            first = false;
        }
        else if (message_rcv.serviceType == ELASPSED_TIME && !first)
        {
            LOG_INFO("Client ID <%d>: Timer Requested", client_id);    

            ElapsedTimePayload elapsed = {0};
            getElapsedTime(false, &elapsed.elapsed_time_s);
            message_send.serviceType = ELASPSED_TIME;
            message_send.payload.elapsedTimePayload = elapsed;
            safe_send(host_socket, service_socket, &message_send);
            // running = false ; 
        }
        else
        {
            running = false ; 
            LOG_WARNING("Client ID <%d>: Request to close service, closing ... ");
        }
    }
    LOG_INFO("Cliend ID <%d>, Request Handler termination.", client_id);
    close(host_socket);
    pthread_exit(NULL);
}

void handle_interrupt(int sig)
{
    LOG_ERROR("Interrupt signal received. Cleaning up...");
    LOG_INFO("Closing LIST DIR Service socket...");
    close(service_socket);
    exit(EXIT_SUCCESS);
}
