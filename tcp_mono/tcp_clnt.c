#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "logger.h"
#include "common.h"
#include "network.h"

#define Authentication_success "1"

int ntrial = 0;
Message messageSend_t = {0};
Message messageRcv_t = {0};

int main()
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    exit(EXIT_FAILURE);

  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  int server_conn = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

  if (server_conn < 0)
  {
    printf("Couldn't connect to server \n");
    exit(EXIT_FAILURE);
  }
  else
  {
    safe_rcv(sock, -1, &messageRcv_t);
    if (messageRcv_t.serviceType == ACK)
    {
      printf("ACK: %s \n", messageRcv_t.payload.ackPayload.msgACK);
    }
  }
  do
  {
    AuthPayload Authpay = {0};
    char pass[64];
    char user[64];

    printf("Enter user: ");
    scanf("%s", user); // Limit input size
    printf("Enter pass: ");
    scanf("%s", pass); // Limit input size
    printf("\n");

    LOG_INFO("Authenticating %s : %s ... ", user, pass);

    strcpy(Authpay.username, user);
    strcpy(Authpay.password, pass);

    messageSend_t.serviceType = AUTH;
    messageSend_t.payload.authPayload = Authpay;

    safe_send(sock, -1, &messageSend_t);
    safe_rcv(sock, -1, &messageRcv_t);

    if (messageRcv_t.serviceType == ACK)
    {
      if (strcmp(messageRcv_t.payload.ackPayload.msgACK, "1") == 0)
      {
        LOG_INFO("Success\n");
        break; // Exit loop on success
      }
      else
      {
        LOG_ERROR("Error: Authentication failed. Please try again.");
      }
    }
    ntrial++;
  } while (ntrial < 3); // Infinite loop until successful authentication
  LOG_INFO("Authentication Success ");
  int selection = 0;
  bool running = true;
  while (running)
  {
    safe_rcv(sock, -1, &messageRcv_t);
    if (messageRcv_t.serviceType == ACK)
    {
      printf("%s\n", messageRcv_t.payload.ackPayload.msgACK);
      printf("$ select a choice: ");
      scanf("%d", &selection);
      printf("you have selected choise %d : %d \n", selection , (Services_e)selection);
      switch ((Services_e)selection)
      {
      case LISTS_DIR:
        /* code */
        LOG_INFO("Listing dirs ");
        messageSend_t.serviceType = LISTS_DIR;
        printf("Enter Path: ");
        char path[50];
        scanf("%s", path);
        ListCatPayload listDirs;
        strcpy(listDirs.directory_path, path);
        messageSend_t.payload.listCatPayload = listDirs;
        safe_send(sock, -1, &messageSend_t);
        // get output
        printf("$ Output: \n");
        
        safe_rcv(sock, -1, &messageRcv_t);
        if (messageRcv_t.serviceType == LISTS_DIR)
        {
          printf("%s \n", messageRcv_t.payload.listCatPayload.output);
          printf("PATH %s \n", messageRcv_t.payload.listCatPayload.directory_path);
        
        }
        break;
        case DATE_TIME:
        /* code */
        LOG_INFO("getting Date Time  ");
        messageSend_t.serviceType =DATE_TIME;
      
        TimeDatePayload td;
        messageSend_t.payload.timeDatePayload=td;
        safe_send(sock, -1, &messageSend_t);
        // get output
        printf("$ Output: \n");
        safe_rcv(sock, -1, &messageRcv_t);
        if (messageRcv_t.serviceType == DATE_TIME)
        {
          printf("\tDATE:  %s \n", messageRcv_t.payload.timeDatePayload.time);
          printf("\tTIME:  %s \n", messageRcv_t.payload.timeDatePayload.date);
        
        }
        break;

      case CAT_DIR:
        /* code */
        LOG_INFO("Printing Files Contents ");

        messageSend_t.serviceType = CAT_DIR;
        printf("Enter File name: ");
        char fpath[50];
        scanf("%s", fpath);
        ListCatPayload catFile;
        strcpy(catFile.directory_path, fpath);
        messageSend_t.payload.listCatPayload = catFile;
        safe_send(sock, -1, &messageSend_t);

        printf("$ Output: \n");
        safe_rcv(sock, -1, &messageRcv_t);
        if (messageRcv_t.serviceType == CAT_DIR)
        {
          printf("PATH %s \n", messageRcv_t.payload.listCatPayload.directory_path);
          printf("%s \n", messageRcv_t.payload.listCatPayload.output);
          
        }
        break;

      case ELASPSED_TIME:
        /* code */
        LOG_INFO("Elapsed time dirs ");
        messageSend_t.serviceType = ELASPSED_TIME;
        safe_send(sock, -1, &messageSend_t);

        printf("$ Output: \n");
        safe_rcv(sock, -1, &messageRcv_t);
        if (messageRcv_t.serviceType == ELASPSED_TIME)
        {
          printf("\tElapsed time : %f \n", messageRcv_t.payload.elapsedTimePayload.elapsed_time_s);
        }

        break;

      case END:
        /* code */
        messageSend_t.serviceType = END;
        safe_send(sock, -1, &messageSend_t);
        LOG_INFO("Service Exiting ....");
        running = false;
        break;

      default:
        break;
      }
    }
  }

  LOG_INFO("DONE .... ");
  close(sock);
  return 0;
}
