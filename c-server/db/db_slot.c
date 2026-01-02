#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static mongoc_collection_t* GetSlotsCollection(const DatabaseContext *ctx) {
    return mongoc_client_get_collection(
        ctx->Client,
        mongoc_database_get_name(ctx->Database),
        COLLECTION_SLOTS
    );
}

static bool ParseSlotFromBson(const bson_t *doc, Slot *outSlot) {
    bson_iter_t iter;

    if (!bson_iter_init(&iter, doc)) {
        return false;
    }

    memset(outSlot, 0, sizeof(Slot));

    while (bson_iter_next(&iter)) {
        const char *key = bson_iter_key(&iter);

        if (strcmp(key, "_id") == 0 && BSON_ITER_HOLDS_OID(&iter)) {
            bson_oid_copy(bson_iter_oid(&iter), &outSlot->Id);
        }
        else if (strcmp(key, "teacher_id") == 0 && BSON_ITER_HOLDS_OID(&iter)) {
            bson_oid_copy(bson_iter_oid(&iter), &outSlot->TeacherId);
        }
        else if (strcmp(key, "start_time") == 0 && BSON_ITER_HOLDS_INT64(&iter)) {
            outSlot->StartTime = bson_iter_int64(&iter);
        }
        else if (strcmp(key, "end_time") == 0 && BSON_ITER_HOLDS_INT64(&iter)) {
            outSlot->EndTime = bson_iter_int64(&iter);
        }
        else if (strcmp(key, "is_booked") == 0 && BSON_ITER_HOLDS_BOOL(&iter)) {
            outSlot->IsBooked = bson_iter_bool(&iter);
        }
        else if (strcmp(key, "created_at") == 0 && BSON_ITER_HOLDS_INT64(&iter)) {
            outSlot->CreatedAt = bson_iter_int64(&iter);
        }
    }

    return true;
}

bool DbCreateSlot(DatabaseContext *ctx, const Slot *slot, bson_oid_t *outId) {
    mongoc_collection_t *collection = GetSlotsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_oid_t oid;
    bson_oid_init(&oid, NULL);

    bson_t *doc = bson_new();
    BSON_APPEND_OID(doc, "_id", &oid);
    BSON_APPEND_OID(doc, "teacher_id", &slot->TeacherId);
    BSON_APPEND_INT64(doc, "start_time", slot->StartTime);
    BSON_APPEND_INT64(doc, "end_time", slot->EndTime);
    BSON_APPEND_BOOL(doc, "is_booked", slot->IsBooked);
    BSON_APPEND_INT64(doc, "created_at", slot->CreatedAt);

    bson_error_t error;
    bool success = mongoc_collection_insert_one(collection, doc, NULL, NULL, &error);

    if (!success) {
        fprintf(stderr, "[DB Error] Insert slot failed: %s\n", error.message);
    } else if (outId != NULL) {
        bson_oid_copy(&oid, outId);
    }

    bson_destroy(doc);
    mongoc_collection_destroy(collection);

    return success;
}

bool DbFindFreeSlots(DatabaseContext *ctx, const bson_oid_t *teacherId,
                     Slot **outSlots, int *outCount) {
    mongoc_collection_t *collection = GetSlotsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW(
        "teacher_id", BCON_OID(teacherId),
        "is_booked", BCON_BOOL(false)
    );

    bson_error_t error;
    int64_t count = mongoc_collection_count_documents(
        collection, filter, NULL, NULL, NULL, &error
    );

    if (count <= 0) {
        *outSlots = NULL;
        *outCount = 0;
        bson_destroy(filter);
        mongoc_collection_destroy(collection);
        return true;
    }

    *outSlots = (Slot*)malloc(count * sizeof(Slot));
    if (*outSlots == NULL) {
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
        ParseSlotFromBson(doc, &(*outSlots)[index]);
        index++;
    }

    *outCount = index;

    mongoc_cursor_destroy(cursor);
    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return true;
}

bool DbUpdateSlotBooked(DatabaseContext *ctx, const bson_oid_t *slotId, bool isBooked) {
    mongoc_collection_t *collection = GetSlotsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW("_id", BCON_OID(slotId));

    bson_t *update = BCON_NEW(
        "$set", "{",
            "is_booked", BCON_BOOL(isBooked),
        "}"
    );

    bson_error_t error;
    bool success = mongoc_collection_update_one(
        collection, filter, update, NULL, NULL, &error
    );

    if (!success) {
        fprintf(stderr, "[DB Error] Update slot failed: %s\n", error.message);
    }

    bson_destroy(update);
    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return success;
}

bool DbDeleteSlot(DatabaseContext *ctx, const bson_oid_t *slotId) {
    mongoc_collection_t *collection = GetSlotsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW("_id", BCON_OID(slotId));

    bson_error_t error;
    bool success = mongoc_collection_delete_one(
        collection, filter, NULL, NULL, &error
    );

    if (!success) {
        fprintf(stderr, "[DB Error] Delete slot failed: %s\n", error.message);
    }

    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return success;
}

bool DbCheckSlotOverlap(DatabaseContext *ctx, const bson_oid_t *teacherId,
                        int64_t startTime, int64_t endTime) {
    mongoc_collection_t *collection = GetSlotsCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW(
        "teacher_id", BCON_OID(teacherId),
        "start_time", "{", "$lt", BCON_INT64(endTime), "}",
        "end_time", "{", "$gt", BCON_INT64(startTime), "}"
    );

    bson_error_t error;
    int64_t count = mongoc_collection_count_documents(
        collection, filter, NULL, NULL, NULL, &error
    );

    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return count > 0;
}

