#ifndef C_SERVER_NET_H
#define C_SERVER_NET_H

#include <stdbool.h>
#include <stddef.h>

#define DEFAULT_PORT 8080
#define MAX_CLIENTS 50
#define BUFFER_SIZE 4096

typedef struct {
    int ServerFd;
    int Port;
    bool IsRunning;
} TcpServer;

typedef void (*ClientHandler)(int clientFd, const char *message, void *userData);

bool TcpServerInit(TcpServer *server, int port);

bool TcpServerStart(TcpServer *server, ClientHandler handler, void *userData);

void TcpServerStop(TcpServer *server);

bool TcpServerSend(int clientFd, const char *message);

void TcpServerCloseClient(int clientFd);

#endif