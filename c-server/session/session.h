#ifndef C_SERVER_SESSION_H
#define C_SERVER_SESSION_H

#include <stdbool.h>
#include <stdint.h>
#include <mongoc/mongoc.h>
#include "../db/db.h"

#define MAX_SESSIONS 100
#define TOKEN_LENGTH 32
#define SESSION_TIMEOUT_SECONDS 3600

typedef struct {
    char Token[TOKEN_LENGTH + 1];
    bson_oid_t UserId;
    UserRole Role;
    int64_t CreatedAt;
    int64_t LastActivity;
    bool IsActive;
} Session;

typedef struct {
    Session Sessions[MAX_SESSIONS];
    int Count;
} SessionManager;

void SessionManagerInit(SessionManager *manager);

Session* SessionCreate(SessionManager *manager, const bson_oid_t *userId, UserRole role);

Session* SessionFindByToken(SessionManager *manager, const char *token);

bool SessionValidate(SessionManager *manager, const char *token);

void SessionDestroy(SessionManager *manager, const char *token);

void SessionCleanupExpired(SessionManager *manager);

void SessionGenerateToken(char *outToken, size_t tokenLength);

#endif