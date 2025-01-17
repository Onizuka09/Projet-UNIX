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
#include "common.h"
#include "logger.h"
#include "network.h"
#include <signal.h>
#define ERROR_ACK "Error wrong password or username, try again ... "
#define ACK_ "Ahenticate you self  your self "
const char *services_str =
    "******************************************************\n\r"
    "0- LISTS_DIR <path>    : lists directories\n\r"
    "1- CAT_DIR <filename>  : returns file content\n\r"
    "2- ELASPSED_TIME       : returns elapsed time\n\r"
    "3- DATE_TIME <path>    : returns Current date & time\n\r"
    "4- END                 : exits program \n\r"
    "*******************************************************";
typedef struct
{
  bool authenticated;
  Services_e currService;
} server_status;
int service_fd= 0 ;
int sock = 0  ; 
server_status SerStaus_t;
Message messageSend_t = {0};
Message messageRcv_t = {0};
char buff[1024];
char buff2[2048];
void handle_interup(){ 
  LOG_ERROR ("Running Interrupt handler  ....");

  LOG_INFO ("Closing sockets ....");
  close(service_fd);
  close(sock);
  exit(EXIT_FAILURE); 
}
int main(){
  // signal(SIGPIPE, SIG_IGN);
  signal(SIGINT,handle_interup); 
  int ntrial = 0;
  struct sockaddr_in client_addr = {0};
  socklen_t client_addr_len = sizeof(client_addr);
   service_fd;
   sock = SafeInitSocket(PORT);
  safeListen(sock);
  service_fd = accept(sock, (struct sockaddr *)&client_addr, &client_addr_len);
  // while (1)
  // {

  if (service_fd > 0)
  {
    LOG_INFO("Client with addr %s \n", inet_ntoa(client_addr.sin_addr));
    strcpy(messageSend_t.payload.ackPayload.msgACK, ACK_);
    messageSend_t.serviceType = ACK;
    safe_send(service_fd, sock, &messageSend_t);
    do
    {
      safe_rcv(service_fd, sock, &messageRcv_t);

      if (messageRcv_t.serviceType == AUTH)
      {
        AuthPayload authpay = messageRcv_t.payload.authPayload;
        bool authenticated = Authenticate(authpay.username, authpay.password);

        if (authenticated)
        {
          LOG_INFO("Authenticated successfully.\n");

          strcpy(messageSend_t.payload.ackPayload.msgACK, "1"); // Success code
          messageSend_t.serviceType = ACK;
          safe_send(service_fd, sock, &messageSend_t);

          SerStaus_t.authenticated = true;
          break; // Exit loop on success
        }
        else
        {
          LOG_ERROR("Error: Wrong username or password. Try again...\n");

          strcpy(messageSend_t.payload.ackPayload.msgACK, ERROR_ACK); // Error code
          messageSend_t.serviceType = ACK;
          safe_send(service_fd, sock, &messageSend_t);

          SerStaus_t.authenticated = false;
          ntrial++; // Increment trial count
        }
      }
    } while (ntrial < 3 && !SerStaus_t.authenticated); // Stop after 3 attempts or success
  }
  // enable time elapsed
  getElapsedTime(true, NULL);
  while (1)
  {
    LOG_INFO("Server Routine");
    strcpy(messageSend_t.payload.ackPayload.msgACK, services_str); // Error code
    messageSend_t.serviceType = ACK;
    safe_send(service_fd, sock, &messageSend_t);
    // recieve choice
    safe_rcv(service_fd, sock, &messageRcv_t);
    switch (messageRcv_t.serviceType)
    {
    case LISTS_DIR:
      /* code */
      LOG_INFO("Servcie Listing dirs ");
      messageSend_t.serviceType = LISTS_DIR;
      ListCatPayload listDirs = {0};
      strcpy(listDirs.directory_path, messageRcv_t.payload.listCatPayload.directory_path);

      ListDirectories(listDirs.directory_path, listDirs.output, sizeof(listDirs.output));
      
      printf("path %s\n ", listDirs.directory_path);
      printf("out %s\n ", listDirs.output);


      messageSend_t.payload.listCatPayload = listDirs;

      // messageSend_t.payload.listCatPayload = listDirs;
      safe_send(service_fd, sock, &messageSend_t);
      // get output

      break;
    case DATE_TIME:
      /* code */
      LOG_INFO("Service Date TIME  ");
      messageSend_t.serviceType = DATE_TIME;
      TimeDatePayload td = {0};
      // strcpy(td.directory_path, messageRcv_t.payload.listCatPayload.directory_path);
      getDateTime(td.time, sizeof(td.time), td.date, sizeof(td.date));
      printf("date %s\n ", td.time);
      printf("Time %s\n ", td.date);
      messageSend_t.payload.timeDatePayload= td;

      safe_send(service_fd, sock, &messageSend_t);

      break;
    case CAT_DIR:
      /* code */
      LOG_INFO("Service Files Contents ");
      messageSend_t.serviceType = CAT_DIR;
      ListCatPayload catf = {0};
      strcpy(catf.directory_path, messageRcv_t.payload.listCatPayload.directory_path);
      getFileContent(catf.directory_path, catf.output, sizeof(catf.output));
      printf("path %s\n ", catf.directory_path);
      printf("out %s\n ", catf.output);
      messageSend_t.payload.listCatPayload = catf;

      safe_send(service_fd, sock, &messageSend_t);

      break;

    case ELASPSED_TIME:
      /* code */
      LOG_INFO("Service Elapsed time  ");
      messageSend_t.serviceType = ELASPSED_TIME;
      ElapsedTimePayload elp={0} ;
      getElapsedTime(false,&elp.elapsed_time_s ); 
      messageSend_t.payload.elapsedTimePayload =elp;
      
      safe_send(service_fd, sock, &messageSend_t);


      break;

    case END:
      /* code */
      SerStaus_t.currService= END; 
      LOG_INFO("Exiting ... ");
      close(service_fd);
      close(sock);
      exit(EXIT_SUCCESS);
      break;

    default:
      break;
    }
  }
  close(service_fd);
  close(sock);
  return 0;
}
