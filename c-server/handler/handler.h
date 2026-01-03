#ifndef C_SERVER_HANDLER_H
#define C_SERVER_HANDLER_H

#include "../protocol/protocol.h"
#include "../session/session.h"
#include "../db/db.h"

typedef struct {
    DatabaseContext *Db;
    SessionManager *Sessions;
} HandlerContext;

void HandleRequest(HandlerContext *ctx, int clientFd, const Request *request, Response *response);

void HandleRegister(HandlerContext *ctx, const Request *request, Response *response);

void HandleLogin(HandlerContext *ctx, const Request *request, Response *response);

void HandleLogout(HandlerContext *ctx, const Request *request, Response *response);

void HandleAddSlot(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

void HandleDeleteSlot(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

void HandleListAppointments(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

void HandleAddMinutes(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

void HandleGetMinutes(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

void HandleBookIndividual(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

void HandleBookGroup(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

void HandleCancelMeeting(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

void HandleListWeekMeetings(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

void HandleListFreeSlots(HandlerContext *ctx, const Session *session, const Request *request, Response *response);

#endif