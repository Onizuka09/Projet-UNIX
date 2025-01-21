#include "network.h"
#include "logger.c"
void safe_send(int service_fd, int sock, Message *message)
{

    if (send(service_fd, message, sizeof(*message), 0) < 0)
    {
        LOG_ERROR("Send failed");
        close(service_fd);
        if (sock != -1)
            close(sock);
        exit(EXIT_FAILURE);
    }
}

void safe_rcv(int service_fd, int sock, Message *message)
{
    if (recv(service_fd, message, sizeof(*message), 0) < 0)
    {
        LOG_ERROR("Receive failed");
        close(service_fd);
        if (sock != -1)
            close(sock);
        exit(EXIT_FAILURE);
    }
}
int SafeInitSocketClient(int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        exit(EXIT_FAILURE);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int server_conn = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (server_conn < 0)
    {
        LOG_CRITICAL("Couldn't connect to server <%d>", port);
        return -1;
    }
    return sock;
}
int SafeInitSocketServer(int p)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        exit(EXIT_FAILURE);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(p);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        LOG_ERROR("Binding ERRO");
        exit(EXIT_FAILURE);
    }
    return sock;
}
void safeListen(int fd)
{
    if (listen(fd, 10) < 0)
    {
        LOG_ERROR("Binding ERRO");
        exit(EXIT_FAILURE);
    }
}