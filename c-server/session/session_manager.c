#include "session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void SessionManagerInit(SessionManager *manager) {
    memset(manager, 0, sizeof(SessionManager));
    manager->Count = 0;
}

void SessionGenerateToken(char *outToken, size_t tokenLength) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static bool seeded = false;
    
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    for (size_t i = 0; i < tokenLength; i++) {
        int index = rand() % (sizeof(charset) - 1);
        outToken[i] = charset[index];
    }
    outToken[tokenLength] = '\0';
}

Session* SessionCreate(SessionManager *manager, const bson_oid_t *userId, UserRole role) {
    SessionCleanupExpired(manager);

    if (manager->Count >= MAX_SESSIONS) {
        fprintf(stderr, "[Session] Max sessions reached\n");
        return NULL;
    }

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!manager->Sessions[i].IsActive) {
            Session *session = &manager->Sessions[i];
            
            SessionGenerateToken(session->Token, TOKEN_LENGTH);
            bson_oid_copy(userId, &session->UserId);
            session->Role = role;
            session->CreatedAt = time(NULL);
            session->LastActivity = session->CreatedAt;
            session->IsActive = true;
            
            manager->Count++;
            return session;
        }
    }

    return NULL;
}

Session* SessionFindByToken(SessionManager *manager, const char *token) {
    if (token == NULL || token[0] == '\0') {
        return NULL;
    }

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->Sessions[i].IsActive && 
            strcmp(manager->Sessions[i].Token, token) == 0) {
            manager->Sessions[i].LastActivity = time(NULL);
            return &manager->Sessions[i];
        }
    }

    return NULL;
}

bool SessionValidate(SessionManager *manager, const char *token) {
    Session *session = SessionFindByToken(manager, token);
    if (session == NULL) {
        return false;
    }

    int64_t now = time(NULL);
    if (now - session->LastActivity > SESSION_TIMEOUT_SECONDS) {
        SessionDestroy(manager, token);
        return false;
    }

    return true;
}

void SessionDestroy(SessionManager *manager, const char *token) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->Sessions[i].IsActive && 
            strcmp(manager->Sessions[i].Token, token) == 0) {
            memset(&manager->Sessions[i], 0, sizeof(Session));
            manager->Count--;
            return;
        }
    }
}

void SessionCleanupExpired(SessionManager *manager) {
    int64_t now = time(NULL);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->Sessions[i].IsActive) {
            if (now - manager->Sessions[i].LastActivity > SESSION_TIMEOUT_SECONDS) {
                memset(&manager->Sessions[i], 0, sizeof(Session));
                manager->Count--;
            }
        }
    }
}