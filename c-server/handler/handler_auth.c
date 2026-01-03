#include "handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void HandleRegister(HandlerContext *ctx, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    char *parts[2] = {NULL, NULL};
    int count = ProtocolSplitData(request->Data, '|', parts, 2);

    if (count < 2) {
        ProtocolFreeDataParts(parts, count);
        return;
    }

    const char *username = parts[0];
    const char *password = parts[1];

    if (DbUserExists(ctx->Db, username)) {
        response->Code = StatusConflict;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    User newUser;
    memset(&newUser, 0, sizeof(User));
    strncpy(newUser.Username, username, sizeof(newUser.Username) - 1);
    strncpy(newUser.PasswordHash, password, sizeof(newUser.PasswordHash) - 1);
    newUser.Role = RoleStudent;
    newUser.CreatedAt = time(NULL);

    bson_oid_t userId;
    if (!DbCreateUser(ctx->Db, &newUser, &userId)) {
        response->Code = StatusServerError;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    Session *session = SessionCreate(ctx->Sessions, &userId, newUser.Role);
    if (session == NULL) {
        response->Code = StatusServerError;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    response->Code = StatusOk;
    snprintf(response->Payload, MAX_PAYLOAD_LENGTH, "%s", session->Token);

    ProtocolFreeDataParts(parts, count);
}

void HandleLogin(HandlerContext *ctx, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    char *parts[2] = {NULL, NULL};
    int count = ProtocolSplitData(request->Data, '|', parts, 2);

    if (count < 2) {
        ProtocolFreeDataParts(parts, count);
        return;
    }

    const char *username = parts[0];
    const char *password = parts[1];

    User user;
    if (!DbFindUserByUsername(ctx->Db, username, &user)) {
        response->Code = StatusNotFound;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    if (strcmp(user.PasswordHash, password) != 0) {
        response->Code = StatusWrongPassword;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    Session *session = SessionCreate(ctx->Sessions, &user.Id, user.Role);
    if (session == NULL) {
        response->Code = StatusServerError;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    const char *roleStr = (user.Role == RoleTeacher) ? "TEACHER" : "STUDENT";
    response->Code = StatusOk;
    snprintf(response->Payload, MAX_PAYLOAD_LENGTH, "%s|%s", session->Token, roleStr);

    ProtocolFreeDataParts(parts, count);
}

void HandleLogout(HandlerContext *ctx, const Request *request, Response *response) {
    response->Code = StatusTokenInvalid;
    response->Payload[0] = '\0';

    if (!request->HasToken) {
        response->Code = StatusTokenMissing;
        return;
    }

    if (!SessionValidate(ctx->Sessions, request->Token)) {
        return;
    }

    SessionDestroy(ctx->Sessions, request->Token);
    response->Code = StatusOk;
}


