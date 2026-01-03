#include "handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void HandleBookIndividual(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    if (session->Role != RoleStudent) {
        response->Code = StatusForbidden;
        return;
    }

    char *parts[2] = {NULL, NULL};
    int count = ProtocolSplitData(request->Data, '|', parts, 2);

    if (count < 2) {
        ProtocolFreeDataParts(parts, count);
        return;
    }

    bson_oid_t teacherId, slotId;
    if (!DbStringToOid(parts[0], &teacherId) || !DbStringToOid(parts[1], &slotId)) {
        ProtocolFreeDataParts(parts, count);
        return;
    }

    if (!DbUpdateSlotBooked(ctx->Db, &slotId, true)) {
        response->Code = StatusConflict;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    Meeting meeting;
    memset(&meeting, 0, sizeof(Meeting));
    bson_oid_copy(&slotId, &meeting.SlotId);
    bson_oid_copy(&teacherId, &meeting.TeacherId);
    meeting.StudentIds = (bson_oid_t*)malloc(sizeof(bson_oid_t));
    bson_oid_copy(&session->UserId, &meeting.StudentIds[0]);
    meeting.StudentCount = 1;
    meeting.Type = MeetingTypeIndividual;
    meeting.Status = MeetingStatusScheduled;
    meeting.Minutes = NULL;
    meeting.CreatedAt = time(NULL);

    bson_oid_t meetingId;
    if (!DbCreateMeeting(ctx->Db, &meeting, &meetingId)) {
        response->Code = StatusServerError;
        DbFreeMeeting(&meeting);
        ProtocolFreeDataParts(parts, count);
        return;
    }

    char meetingIdStr[25];
    DbOidToString(&meetingId, meetingIdStr);

    response->Code = StatusOk;
    snprintf(response->Payload, MAX_PAYLOAD_LENGTH, "%s", meetingIdStr);

    DbFreeMeeting(&meeting);
    ProtocolFreeDataParts(parts, count);
}

void HandleCancelMeeting(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    bson_oid_t meetingId;
    if (!DbStringToOid(request->Data, &meetingId)) {
        return;
    }

    if (!DbUpdateMeetingStatus(ctx->Db, &meetingId, MeetingStatusCancelled)) {
        response->Code = StatusServerError;
        return;
    }

    response->Code = StatusOk;
}

void HandleAddMinutes(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    if (session->Role != RoleTeacher) {
        response->Code = StatusForbidden;
        return;
    }

    char *parts[2] = {NULL, NULL};
    int count = ProtocolSplitData(request->Data, '|', parts, 2);

    if (count < 2) {
        ProtocolFreeDataParts(parts, count);
        return;
    }

    bson_oid_t meetingId;
    if (!DbStringToOid(parts[0], &meetingId)) {
        ProtocolFreeDataParts(parts, count);
        return;
    }

    if (!DbUpdateMeetingMinutes(ctx->Db, &meetingId, parts[1])) {
        response->Code = StatusServerError;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    response->Code = StatusOk;
    ProtocolFreeDataParts(parts, count);
}

void HandleGetMinutes(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    bson_oid_t meetingId;
    if (!DbStringToOid(request->Data, &meetingId)) {
        return;
    }

    char *minutes = NULL;
    if (!DbGetMeetingMinutes(ctx->Db, &meetingId, &minutes)) {
        response->Code = StatusNotFound;
        return;
    }

    response->Code = StatusOk;
    if (minutes != NULL) {
        strncpy(response->Payload, minutes, MAX_PAYLOAD_LENGTH - 1);
        free(minutes);
    }
}

