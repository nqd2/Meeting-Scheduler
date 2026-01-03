#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

bool TcpServerInit(TcpServer *server, int port) {
    server->Port = port;
    server->IsRunning = false;

    server->ServerFd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->ServerFd < 0) {
        perror("[NET] socket() failed");
        return false;
    }

    int opt = 1;
    if (setsockopt(server->ServerFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[NET] setsockopt() failed");
        close(server->ServerFd);
        return false;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server->ServerFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[NET] bind() failed");
        close(server->ServerFd);
        return false;
    }

    if (listen(server->ServerFd, MAX_CLIENTS) < 0) {
        perror("[NET] listen() failed");
        close(server->ServerFd);
        return false;
    }

    printf("[NET] Server initialized on port %d\n", port);
    return true;
}

bool TcpServerStart(TcpServer *server, ClientHandler handler, void *userData) {
    server->IsRunning = true;

    fd_set masterSet, readSet;
    FD_ZERO(&masterSet);
    FD_SET(server->ServerFd, &masterSet);
    int maxFd = server->ServerFd;

    printf("[NET] Server started, waiting for connections...\n");

    while (server->IsRunning) {
        readSet = masterSet;

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(maxFd + 1, &readSet, NULL, NULL, &timeout);

        if (activity < 0) {
            if (errno == EINTR) continue;
            perror("[NET] select() failed");
            break;
        }

        if (activity == 0) {
            continue;
        }

        if (FD_ISSET(server->ServerFd, &readSet)) {
            struct sockaddr_in clientAddr;
            socklen_t addrLen = sizeof(clientAddr);

            int clientFd = accept(server->ServerFd, (struct sockaddr*)&clientAddr, &addrLen);
            if (clientFd < 0) {
                perror("[NET] accept() failed");
            } else {
                printf("[NET] New connection from %s:%d (fd=%d)\n",
                       inet_ntoa(clientAddr.sin_addr),
                       ntohs(clientAddr.sin_port),
                       clientFd);

                FD_SET(clientFd, &masterSet);
                if (clientFd > maxFd) {
                    maxFd = clientFd;
                }
            }
        }

        for (int fd = 0; fd <= maxFd; fd++) {
            if (fd == server->ServerFd) continue;
            if (!FD_ISSET(fd, &readSet)) continue;

            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));

            ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

            if (bytesRead <= 0) {
                if (bytesRead == 0) {
                    printf("[NET] Client fd=%d disconnected\n", fd);
                } else {
                    perror("[NET] recv() failed");
                }
                close(fd);
                FD_CLR(fd, &masterSet);
            } else {
                buffer[bytesRead] = '\0';
                printf("[NET] Received from fd=%d: %s", fd, buffer);

                if (handler != NULL) {
                    handler(fd, buffer, userData);
                }
            }
        }
    }

    return true;
}

void TcpServerStop(TcpServer *server) {
    server->IsRunning = false;
    if (server->ServerFd >= 0) {
        close(server->ServerFd);
        server->ServerFd = -1;
    }
    printf("[NET] Server stopped\n");
}

bool TcpServerSend(int clientFd, const char *message) {
    if (clientFd < 0 || message == NULL) {
        return false;
    }

    size_t len = strlen(message);
    ssize_t sent = send(clientFd, message, len, 0);

    if (sent < 0) {
        perror("[NET] send() failed");
        return false;
    }

    return true;
}

void TcpServerCloseClient(int clientFd) {
    if (clientFd >= 0) {
        close(clientFd);
    }
}