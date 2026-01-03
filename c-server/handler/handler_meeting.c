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

void HandleListAppointments(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    if (session->Role != RoleTeacher) {
        response->Code = StatusForbidden;
        return;
    }

    int64_t date = atoll(request->Data);
    if (date <= 0) {
        date = time(NULL);
    }

    Meeting *meetings = NULL;
    int count = 0;

    if (!DbFindMeetingsByTeacher(ctx->Db, &session->UserId, date, &meetings, &count)) {
        response->Code = StatusServerError;
        return;
    }

    response->Code = StatusOk;
    char *ptr = response->Payload;
    size_t remaining = MAX_PAYLOAD_LENGTH;

    for (int i = 0; i < count && remaining > 100; i++) {
        char meetingIdStr[25];
        char slotIdStr[25];
        DbOidToString(&meetings[i].Id, meetingIdStr);
        DbOidToString(&meetings[i].SlotId, slotIdStr);

        int written = snprintf(ptr, remaining, "%s|%s|%lld|%d;",
                              meetingIdStr,
                              slotIdStr,
                              (long long)meetings[i].CreatedAt,
                              meetings[i].Type);
        ptr += written;
        remaining -= written;
    }

    if (meetings != NULL) {
        DbFreeMeetings(meetings, count);
    }
}

void HandleBookGroup(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    if (session->Role != RoleStudent) {
        response->Code = StatusForbidden;
        return;
    }

    char *parts[3] = {NULL, NULL, NULL};
    int count = ProtocolSplitData(request->Data, '|', parts, 3);

    if (count < 3) {
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

    char *memberIdsStr = parts[2];
    char *memberParts[10] = {NULL};
    int memberCount = ProtocolSplitData(memberIdsStr, ',', memberParts, 10);

    if (memberCount <= 0) {
        ProtocolFreeDataParts(parts, count);
        ProtocolFreeDataParts(memberParts, memberCount);
        return;
    }

    Meeting meeting;
    memset(&meeting, 0, sizeof(Meeting));
    bson_oid_copy(&slotId, &meeting.SlotId);
    bson_oid_copy(&teacherId, &meeting.TeacherId);
    meeting.StudentIds = (bson_oid_t*)malloc((memberCount + 1) * sizeof(bson_oid_t));
    meeting.StudentCount = memberCount + 1;

    bson_oid_copy(&session->UserId, &meeting.StudentIds[0]);
    for (int i = 0; i < memberCount; i++) {
        if (!DbStringToOid(memberParts[i], &meeting.StudentIds[i + 1])) {
            DbFreeMeeting(&meeting);
            ProtocolFreeDataParts(parts, count);
            ProtocolFreeDataParts(memberParts, memberCount);
            return;
        }
    }

    meeting.Type = MeetingTypeGroup;
    meeting.Status = MeetingStatusScheduled;
    meeting.Minutes = NULL;
    meeting.CreatedAt = time(NULL);

    bson_oid_t meetingId;
    if (!DbCreateMeeting(ctx->Db, &meeting, &meetingId)) {
        response->Code = StatusServerError;
        DbFreeMeeting(&meeting);
        ProtocolFreeDataParts(parts, count);
        ProtocolFreeDataParts(memberParts, memberCount);
        return;
    }

    char meetingIdStr[25];
    DbOidToString(&meetingId, meetingIdStr);

    response->Code = StatusOk;
    snprintf(response->Payload, MAX_PAYLOAD_LENGTH, "%s", meetingIdStr);

    DbFreeMeeting(&meeting);
    ProtocolFreeDataParts(parts, count);
    ProtocolFreeDataParts(memberParts, memberCount);
}

void HandleListWeekMeetings(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    if (session->Role != RoleStudent) {
        response->Code = StatusForbidden;
        return;
    }

    char *parts[2] = {NULL, NULL};
    int count = ProtocolSplitData(request->Data, '|', parts, 2);

    int year = 2024;
    int week = 1;

    if (count >= 1) {
        year = atoi(parts[0]);
    }
    if (count >= 2) {
        week = atoi(parts[1]);
    }

    if (year < 1970 || week < 1 || week > 53) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        year = tm_info->tm_year + 1900;
        week = (tm_info->tm_yday / 7) + 1;
    }

    Meeting *meetings = NULL;
    int meetingCount = 0;

    if (!DbFindMeetingsByStudentWeek(ctx->Db, &session->UserId, year, week, &meetings, &meetingCount)) {
        response->Code = StatusServerError;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    response->Code = StatusOk;
    char *ptr = response->Payload;
    size_t remaining = MAX_PAYLOAD_LENGTH;

    for (int i = 0; i < meetingCount && remaining > 100; i++) {
        char meetingIdStr[25];
        char slotIdStr[25];
        DbOidToString(&meetings[i].Id, meetingIdStr);
        DbOidToString(&meetings[i].SlotId, slotIdStr);

        int written = snprintf(ptr, remaining, "%s|%s|%lld|%d;",
                              meetingIdStr,
                              slotIdStr,
                              (long long)meetings[i].CreatedAt,
                              meetings[i].Type);
        ptr += written;
        remaining -= written;
    }

    if (meetings != NULL) {
        DbFreeMeetings(meetings, meetingCount);
    }

    ProtocolFreeDataParts(parts, count);
}