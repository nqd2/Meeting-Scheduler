#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static mongoc_collection_t* GetMeetingsCollection(const DatabaseContext *ctx) {
    return mongoc_client_get_collection(
        ctx->Client,
        mongoc_database_get_name(ctx->Database),
        COLLECTION_MEETINGS
    );
}

static const char* MeetingTypeToString(MeetingType type) {
    return type == MeetingTypeGroup ? "GROUP" : "INDIVIDUAL";
}

static MeetingType StringToMeetingType(const char *str) {
    if (strcmp(str, "GROUP") == 0) return MeetingTypeGroup;
    return MeetingTypeIndividual;
}

static const char* MeetingStatusToString(MeetingStatus status) {
    switch (status) {
        case MeetingStatusScheduled: return "SCHEDULED";
        case MeetingStatusCompleted: return "COMPLETED";
        case MeetingStatusCancelled: return "CANCELLED";
        default: return "SCHEDULED";
    }
}

static MeetingStatus StringToMeetingStatus(const char *str) {
    if (strcmp(str, "COMPLETED") == 0) return MeetingStatusCompleted;
    if (strcmp(str, "CANCELLED") == 0) return MeetingStatusCancelled;
    return MeetingStatusScheduled;
}

static bool ParseMeetingFromBson(const bson_t *doc, Meeting *outMeeting) {
    bson_iter_t iter;

    if (!bson_iter_init(&iter, doc)) {
        return false;
    }

    memset(outMeeting, 0, sizeof(Meeting));
    outMeeting->StudentIds = NULL;
    outMeeting->StudentCount = 0;
    outMeeting->Minutes = NULL;

    while (bson_iter_next(&iter)) {
        const char *key = bson_iter_key(&iter);

        if (strcmp(key, "_id") == 0 && BSON_ITER_HOLDS_OID(&iter)) {
            bson_oid_copy(bson_iter_oid(&iter), &outMeeting->Id);
        }
        else if (strcmp(key, "slot_id") == 0 && BSON_ITER_HOLDS_OID(&iter)) {
            bson_oid_copy(bson_iter_oid(&iter), &outMeeting->SlotId);
        }
        else if (strcmp(key, "teacher_id") == 0 && BSON_ITER_HOLDS_OID(&iter)) {
            bson_oid_copy(bson_iter_oid(&iter), &outMeeting->TeacherId);
        }
        else if (strcmp(key, "student_ids") == 0 && BSON_ITER_HOLDS_ARRAY(&iter)) {
            bson_iter_t arrayIter;
            bson_iter_recurse(&iter, &arrayIter);

            int count = 0;
            while (bson_iter_next(&arrayIter)) {
                if (BSON_ITER_HOLDS_OID(&arrayIter)) {
                    count++;
                }
            }

            if (count > 0) {
                outMeeting->StudentIds = (bson_oid_t*)malloc(count * sizeof(bson_oid_t));
                outMeeting->StudentCount = count;

                bson_iter_recurse(&iter, &arrayIter);
                int index = 0;
                while (bson_iter_next(&arrayIter) && index < count) {
                    if (BSON_ITER_HOLDS_OID(&arrayIter)) {
                        bson_oid_copy(bson_iter_oid(&arrayIter), &outMeeting->StudentIds[index]);
                        index++;
                    }
                }
            }
        }
        else if (strcmp(key, "meeting_type") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
            outMeeting->Type = StringToMeetingType(bson_iter_utf8(&iter, NULL));
        }
        else if (strcmp(key, "status") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
            outMeeting->Status = StringToMeetingStatus(bson_iter_utf8(&iter, NULL));
        }
        else if (strcmp(key, "minutes") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
            uint32_t len;
            const char *val = bson_iter_utf8(&iter, &len);
            outMeeting->Minutes = (char*)malloc(len + 1);
            if (outMeeting->Minutes != NULL) {
                memcpy(outMeeting->Minutes, val, len);
                outMeeting->Minutes[len] = '\0';
            }
        }
        else if (strcmp(key, "created_at") == 0 && BSON_ITER_HOLDS_INT64(&iter)) {
            outMeeting->CreatedAt = bson_iter_int64(&iter);
        }
    }

    return true;
}

bool DbCreateMeeting(DatabaseContext *ctx, const Meeting *meeting, bson_oid_t *outId) {
    mongoc_collection_t *collection = GetMeetingsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_oid_t oid;
    bson_oid_init(&oid, NULL);

    bson_t *doc = bson_new();
    BSON_APPEND_OID(doc, "_id", &oid);
    BSON_APPEND_OID(doc, "slot_id", &meeting->SlotId);
    BSON_APPEND_OID(doc, "teacher_id", &meeting->TeacherId);

    bson_t studentArray;
    BSON_APPEND_ARRAY_BEGIN(doc, "student_ids", &studentArray);
    for (int i = 0; i < meeting->StudentCount; i++) {
        char key[16];
        snprintf(key, sizeof(key), "%d", i);
        BSON_APPEND_OID(&studentArray, key, &meeting->StudentIds[i]);
    }
    bson_append_array_end(doc, &studentArray);

    BSON_APPEND_UTF8(doc, "meeting_type", MeetingTypeToString(meeting->Type));
    BSON_APPEND_UTF8(doc, "status", MeetingStatusToString(meeting->Status));

    if (meeting->Minutes != NULL) {
        BSON_APPEND_UTF8(doc, "minutes", meeting->Minutes);
    } else {
        BSON_APPEND_UTF8(doc, "minutes", "");
    }

    BSON_APPEND_INT64(doc, "created_at", meeting->CreatedAt);

    bson_error_t error;
    bool success = mongoc_collection_insert_one(collection, doc, NULL, NULL, &error);

    if (!success) {
        fprintf(stderr, "[DB Error] Insert meeting failed: %s\n", error.message);
    } else if (outId != NULL) {
        bson_oid_copy(&oid, outId);
    }

    bson_destroy(doc);
    mongoc_collection_destroy(collection);

    return success;
}

bool DbFindMeetingsByTeacher(DatabaseContext *ctx, const bson_oid_t *teacherId,
                             int64_t date, Meeting **outMeetings, int *outCount) {
    mongoc_collection_t *collection = GetMeetingsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    int64_t startOfDay = date;
    int64_t endOfDay = date + 86400;

    bson_t *filter = BCON_NEW(
        "teacher_id", BCON_OID(teacherId),
        "created_at", "{",
            "$gte", BCON_INT64(startOfDay),
            "$lt", BCON_INT64(endOfDay),
        "}"
    );

    bson_error_t error;
    int64_t count = mongoc_collection_count_documents(
        collection, filter, NULL, NULL, NULL, &error
    );

    if (count <= 0) {
        *outMeetings = NULL;
        *outCount = 0;
        bson_destroy(filter);
        mongoc_collection_destroy(collection);
        return true;
    }

    *outMeetings = (Meeting*)malloc(count * sizeof(Meeting));
    if (*outMeetings == NULL) {
        bson_destroy(filter);
        mongoc_collection_destroy(collection);
        return false;
    }

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
        collection, filter, NULL, NULL
    );

    const bson_t *doc;
    int index = 0;

    while (mongoc_cursor_next(cursor, &doc) && index < count) {
        ParseMeetingFromBson(doc, &(*outMeetings)[index]);
        index++;
    }

    *outCount = index;

    mongoc_cursor_destroy(cursor);
    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return true;
}

bool DbFindMeetingsByStudentWeek(DatabaseContext *ctx, const bson_oid_t *studentId,
                                 int year, int week, Meeting **outMeetings, int *outCount) {
    mongoc_collection_t *collection = GetMeetingsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    int64_t startOfWeek = ((year - 1970) * 365 + (week - 1) * 7) * 86400;
    int64_t endOfWeek = startOfWeek + 7 * 86400;

    bson_t *filter = BCON_NEW(
        "student_ids", BCON_OID(studentId),
        "created_at", "{",
            "$gte", BCON_INT64(startOfWeek),
            "$lt", BCON_INT64(endOfWeek),
        "}"
    );

    bson_error_t error;
    int64_t count = mongoc_collection_count_documents(
        collection, filter, NULL, NULL, NULL, &error
    );

    if (count <= 0) {
        *outMeetings = NULL;
        *outCount = 0;
        bson_destroy(filter);
        mongoc_collection_destroy(collection);
        return true;
    }

    *outMeetings = (Meeting*)malloc(count * sizeof(Meeting));
    if (*outMeetings == NULL) {
        bson_destroy(filter);
        mongoc_collection_destroy(collection);
        return false;
    }

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
        collection, filter, NULL, NULL
    );

    const bson_t *doc;
    int index = 0;

    while (mongoc_cursor_next(cursor, &doc) && index < count) {
        ParseMeetingFromBson(doc, &(*outMeetings)[index]);
        index++;
    }

    *outCount = index;

    mongoc_cursor_destroy(cursor);
    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return true;
}

bool DbUpdateMeetingStatus(DatabaseContext *ctx, const bson_oid_t *meetingId,
                           MeetingStatus status) {
    mongoc_collection_t *collection = GetMeetingsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW("_id", BCON_OID(meetingId));
    bson_t *update = BCON_NEW(
        "$set", "{",
            "status", BCON_UTF8(MeetingStatusToString(status)),
        "}"
    );

    bson_error_t error;
    bool success = mongoc_collection_update_one(
        collection, filter, update, NULL, NULL, &error
    );

    if (!success) {
        fprintf(stderr, "[DB Error] Update meeting status failed: %s\n", error.message);
    }

    bson_destroy(update);
    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return success;
}

bool DbUpdateMeetingMinutes(DatabaseContext *ctx, const bson_oid_t *meetingId,
                            const char *minutesContent) {
    mongoc_collection_t *collection = GetMeetingsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW("_id", BCON_OID(meetingId));
    bson_t *update = BCON_NEW(
        "$set", "{",
            "minutes", BCON_UTF8(minutesContent),
        "}"
    );

    bson_error_t error;
    bool success = mongoc_collection_update_one(
        collection, filter, update, NULL, NULL, &error
    );

    if (!success) {
        fprintf(stderr, "[DB Error] Update meeting minutes failed: %s\n", error.message);
    }

    bson_destroy(update);
    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return success;
}

bool DbGetMeetingMinutes(DatabaseContext *ctx, const bson_oid_t *meetingId,
                         char **outMinutes) {
    mongoc_collection_t *collection = GetMeetingsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW("_id", BCON_OID(meetingId));

    bson_t *opts = BCON_NEW(
        "projection", "{",
            "minutes", BCON_INT32(1),
        "}"
    );

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
        collection, filter, opts, NULL
    );

    const bson_t *doc;
    bool found = false;

    if (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        if (bson_iter_init_find(&iter, doc, "minutes") && BSON_ITER_HOLDS_UTF8(&iter)) {
            uint32_t len;
            const char *val = bson_iter_utf8(&iter, &len);
            *outMinutes = (char*)malloc(len + 1);
            if (*outMinutes != NULL) {
                memcpy(*outMinutes, val, len);
                (*outMinutes)[len] = '\0';
                found = true;
            }
        }
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(opts);
    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return found;
}

