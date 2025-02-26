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
#include <signal.h>
#include <Gooey/gooey.h>
#include "ui.h"
#include <pthread.h> // For threading support

#define Authentication_success "1"

extern bool status_Auth_btn;

extern GooeyWindow Authwin;
extern GooeyWindow SerWin;
GooeySignal signal1, guiUpdateSignal;

int ntrial;
ntrial = 0;
int id = 0;
int sock = 0;

Message messageSend_t = {0};
Message messageRcv_t = {0};

void handle_interrupt(int sig);


void signal_callback(void *context, void *data)
{
  LOG_INFO("Recieved signal, Executing the signal handlers");

  GooeyWindow_MakeVisible(&Authwin, false);
  GooeyWindow_MakeVisible(&SerWin, true);
}
void *client_network_thread(void *arg);

int main()
{
  signal(SIGINT, handle_interrupt);
  pthread_t net_thread;
  pthread_create(&net_thread, NULL, client_network_thread, NULL);

  // Start the GUI in the main thread
  Gooey_Init(GLFW);
  Authwin = GooeyWindow_Create("GUI client", 600, 900, true);
  Create_authentification_layout();
  SerWin = GooeyWindow_CreateChild("GUI client", 600, 900, false);
  create_services_layout();
  signal1 = GooeySignal_Create();
  GooeySignal_Link(&signal1, signal_callback, NULL);
  GooeySignal guiUpdateSignal = GooeySignal_Create();

  pthread_join(net_thread, NULL);
  GooeyWindow_Run(2, &Authwin, &SerWin);

  // Wait for the network thread to finish (if necessary)

  GooeyWindow_Cleanup(2, &Authwin, &SerWin);
  return 0;
}

void *client_network_thread(void *arg)
{

  // GooeyWindow_Cleanup(1, &win);
  sock = socket(AF_INET, SOCK_STREAM, 0);
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
    // Create_authentification_layout();

    if (status_Auth_btn)
    {

      AuthPayload Authpay = {0};
      char *pass;
      char *user;
      //
      user = getUserName();
      pass = getPasswd();

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
          LOG_ERROR("Error: Authentication failed. Please try again. %d", ntrial);
        }
      }
      status_Auth_btn = false;
      ntrial++;
    }
  } while (ntrial < 3); // Infinite loop until successful authentication
  if (ntrial >= 3)
  {
    LOG_ERROR("Excceeded MAX Trails 3, Exiting ...");
    close(sock);
    exit(EXIT_FAILURE);
  }

  GooeySignal_Emit(&signal1, "world");

  LOG_INFO("Authentication Success ");
  int selection = 0;
  bool running = true;
  while (running)
  {
    safe_rcv(sock, -1, &messageRcv_t);
    if (messageRcv_t.serviceType == ACK)
    {
      id = messageRcv_t.id;
      set_ConnectionStatus(true);

      setUserID(id);
      printf("My ID <%d>\n%s\n", id, messageRcv_t.payload.ackPayload.msgACK);
      printf("$ select a choice: ");
      scanf("%d", &selection);
      printf("you have selected choise %d : %d \n", selection, (Services_e)selection);
      messageSend_t.id = id;
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
        messageSend_t.serviceType = DATE_TIME;

        TimeDatePayload td = {0};
        messageSend_t.payload.timeDatePayload = td;
        safe_send(sock, -1, &messageSend_t);
        // get output
        printf("$ Output: \n");
        safe_rcv(sock, -1, &messageRcv_t);
        if (messageRcv_t.serviceType == DATE_TIME)
        {
          printf("\tDATE:  %s \n", messageRcv_t.payload.timeDatePayload.time);
          printf("\tTIME:  %s \n", messageRcv_t.payload.timeDatePayload.date);
        }
        else if (messageRcv_t.serviceType == ACK)
        {
          printf("<ERROR>: %s\n", messageRcv_t.payload.ackPayload.msgACK);
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
        else if (messageRcv_t.serviceType == ACK)
        {
          printf("<ERROR>: %s\n", messageRcv_t.payload.ackPayload.msgACK);
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
        LOG_ERROR("Wrong Service, Avialable services [0...4]");

        break;
      }
    }
  }

  LOG_INFO("DONE .... ");
  close(sock);
  return NULL;
}
void handle_interrupt(int sig)
{
  LOG_ERROR("Interrupt signal received. Cleaning up...");
  // join all active threads
  messageSend_t.serviceType = END;
  safe_send(sock, -1, &messageSend_t);
  GooeyWindow_Cleanup(2, &Authwin, &SerWin);

  LOG_INFO("Closing client socket socket...");
  close(sock);
  exit(EXIT_SUCCESS);
}