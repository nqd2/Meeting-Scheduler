#include "handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void HandleAddSlot(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
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

    int64_t startTime = atoll(parts[0]);
    int64_t endTime = atoll(parts[1]);

    if (startTime >= endTime) {
        ProtocolFreeDataParts(parts, count);
        return;
    }

    if (DbCheckSlotOverlap(ctx->Db, &session->UserId, startTime, endTime)) {
        response->Code = StatusConflict;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    Slot newSlot;
    memset(&newSlot, 0, sizeof(Slot));
    bson_oid_copy(&session->UserId, &newSlot.TeacherId);
    newSlot.StartTime = startTime;
    newSlot.EndTime = endTime;
    newSlot.IsBooked = false;
    newSlot.CreatedAt = time(NULL);

    bson_oid_t slotId;
    if (!DbCreateSlot(ctx->Db, &newSlot, &slotId)) {
        response->Code = StatusServerError;
        ProtocolFreeDataParts(parts, count);
        return;
    }

    response->Code = StatusOk;
    ProtocolFreeDataParts(parts, count);
}

void HandleDeleteSlot(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    if (session->Role != RoleTeacher) {
        response->Code = StatusForbidden;
        return;
    }

    bson_oid_t slotId;
    if (!DbStringToOid(request->Data, &slotId)) {
        return;
    }

    if (!DbDeleteSlot(ctx->Db, &slotId)) {
        response->Code = StatusServerError;
        return;
    }

    response->Code = StatusOk;
}

void HandleListFreeSlots(HandlerContext *ctx, const Session *session, const Request *request, Response *response) {
    response->Code = StatusBadRequest;
    response->Payload[0] = '\0';

    bson_oid_t teacherId;
    if (!DbStringToOid(request->Data, &teacherId)) {
        bson_oid_copy(&session->UserId, &teacherId);
    }

    Slot *slots = NULL;
    int count = 0;

    if (!DbFindFreeSlots(ctx->Db, &teacherId, &slots, &count)) {
        response->Code = StatusServerError;
        return;
    }

    response->Code = StatusOk;
    char *ptr = response->Payload;
    size_t remaining = MAX_PAYLOAD_LENGTH;

    for (int i = 0; i < count && remaining > 50; i++) {
        char slotIdStr[25];
        DbOidToString(&slots[i].Id, slotIdStr);

        int written = snprintf(ptr, remaining, "%s|%lld|%lld;",
                              slotIdStr,
                              (long long)slots[i].StartTime,
                              (long long)slots[i].EndTime);
        ptr += written;
        remaining -= written;
    }

    if (slots != NULL) {
        free(slots);
    }
}