#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "db/db.h"
#include "net/net.h"
#include "protocol/protocol.h"
#include "session/session.h"
#include "handler/handler.h"

static TcpServer gServer;
static DatabaseContext gDb;
static SessionManager gSessions;
static HandlerContext gHandlerCtx;

void OnClientMessage(int clientFd, const char *message, void *userData) {
    (void)userData;

    Request request;
    Response response;

    if (!ProtocolParseRequest(message, &request)) {
        response.Code = StatusBadRequest;
        response.Payload[0] = '\0';
    } else {
        HandleRequest(&gHandlerCtx, clientFd, &request, &response);
    }

    char responseBuffer[MAX_PAYLOAD_LENGTH + 32];
    ProtocolBuildResponse(&response, responseBuffer, sizeof(responseBuffer));

    TcpServerSend(clientFd, responseBuffer);
    printf("[MAIN] Sent response: %s", responseBuffer);
}

void SignalHandler(int sig) {
    (void)sig;
    printf("\n[MAIN] Shutting down...\n");
    TcpServerStop(&gServer);
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    printf("=== Meeting Scheduler Server ===\n\n");

    DbInit();

    if (!DbConnect(&gDb)) {
        fprintf(stderr, "[MAIN] Failed to connect to database\n");
        DbCleanup();
        return 1;
    }

    SessionManagerInit(&gSessions);

    gHandlerCtx.Db = &gDb;
    gHandlerCtx.Sessions = &gSessions;

    if (!TcpServerInit(&gServer, port)) {
        fprintf(stderr, "[MAIN] Failed to initialize server\n");
        DbDisconnect(&gDb);
        DbCleanup();
        return 1;
    }

    TcpServerStart(&gServer, OnClientMessage, NULL);

    DbDisconnect(&gDb);
    DbCleanup();

    printf("[MAIN] Server shutdown complete\n");
    return 0;
}