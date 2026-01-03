#include "handler.h"
#include <stdio.h>

void HandleRequest(HandlerContext *ctx, int clientFd, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    if (request->Type == CmdRegister) {
        HandleRegister(ctx, request, response);
        return;
    }

    if (request->Type == CmdLogin) {
        HandleLogin(ctx, request, response);
        return;
    }

    if (!request->HasToken) {
        response->Code = StatusTokenMissing;
        return;
    }

    Session *session = SessionFindByToken(ctx->Sessions, request->Token);
    if (session == NULL) {
        response->Code = StatusTokenInvalid;
        return;
    }

    switch (request->Type) {
        case CmdLogout:
            HandleLogout(ctx, request, response);
            break;
        case CmdAddSlot:
            HandleAddSlot(ctx, session, request, response);
            break;
        case CmdDeleteSlot:
            HandleDeleteSlot(ctx, session, request, response);
            break;
        case CmdListAppointments:
            HandleListAppointments(ctx, session, request, response);
            break;
        case CmdListFreeSlots:
            HandleListFreeSlots(ctx, session, request, response);
            break;
        case CmdBookIndividual:
            HandleBookIndividual(ctx, session, request, response);
            break;
        case CmdBookGroup:
            HandleBookGroup(ctx, session, request, response);
            break;
        case CmdCancelMeeting:
            HandleCancelMeeting(ctx, session, request, response);
            break;
        case CmdListWeekMeetings:
            HandleListWeekMeetings(ctx, session, request, response);
            break;
        case CmdAddMinutes:
            HandleAddMinutes(ctx, session, request, response);
            break;
        case CmdGetMinutes:
            HandleGetMinutes(ctx, session, request, response);
            break;
        default:
            response->Code = StatusBadRequest;
            break;
    }
}