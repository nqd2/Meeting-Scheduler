#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static mongoc_collection_t* GetUsersCollection(const DatabaseContext *ctx) {
    return mongoc_client_get_collection(
        ctx->Client,
        mongoc_database_get_name(ctx->Database),
        COLLECTION_USERS
    );
}

static const char* UserRoleToString(const UserRole role) {
    return role == RoleTeacher ? "TEACHER" : "STUDENT";
}

static UserRole StringToUserRole(const char *str) {
    if (strcmp(str, "TEACHER") == 0) {
        return RoleTeacher;
    }
    return RoleStudent;
}

bool DbCreateUser(DatabaseContext *ctx, const User *user, bson_oid_t *outId) {
    mongoc_collection_t *collection = GetUsersCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_oid_t oid;
    bson_oid_init(&oid, NULL);

    bson_t *doc = bson_new();
    BSON_APPEND_OID(doc, "_id", &oid);
    BSON_APPEND_UTF8(doc, "username", user->Username);
    BSON_APPEND_UTF8(doc, "password", user->PasswordHash);
    BSON_APPEND_UTF8(doc, "full_name", user->FullName);
    BSON_APPEND_UTF8(doc, "role", UserRoleToString(user->Role));
    BSON_APPEND_INT64(doc, "created_at", user->CreatedAt);

    bson_error_t error;
    bool success = mongoc_collection_insert_one(
        collection,
        doc,
        NULL,
        NULL,
        &error
    );

    if (!success) {
        fprintf(stderr, "[DB Error] Insert user failed: %s\n", error.message);
    } else if (outId != NULL) {
        bson_oid_copy(&oid, outId);
    }

    bson_destroy(doc);
    mongoc_collection_destroy(collection);

    return success;
}

static bool ParseUserFromBson(const bson_t *doc, User *outUser) {
    bson_iter_t iter;

    if (!bson_iter_init(&iter, doc)) {
        return false;
    }

    memset(outUser, 0, sizeof(User));

    while (bson_iter_next(&iter)) {
        const char *key = bson_iter_key(&iter);

        if (strcmp(key, "_id") == 0 && BSON_ITER_HOLDS_OID(&iter)) {
            bson_oid_copy(bson_iter_oid(&iter), &outUser->Id);
        }
        else if (strcmp(key, "username") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
            const char *val = bson_iter_utf8(&iter, NULL);
            strncpy(outUser->Username, val, sizeof(outUser->Username) - 1);
        }
        else if (strcmp(key, "password") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
            const char *val = bson_iter_utf8(&iter, NULL);
            strncpy(outUser->PasswordHash, val, sizeof(outUser->PasswordHash) - 1);
        }
        else if (strcmp(key, "full_name") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
            const char *val = bson_iter_utf8(&iter, NULL);
            strncpy(outUser->FullName, val, sizeof(outUser->FullName) - 1);
        }
        else if (strcmp(key, "role") == 0 && BSON_ITER_HOLDS_UTF8(&iter)) {
            const char *val = bson_iter_utf8(&iter, NULL);
            outUser->Role = StringToUserRole(val);
        }
        else if (strcmp(key, "created_at") == 0 && BSON_ITER_HOLDS_INT64(&iter)) {
            outUser->CreatedAt = bson_iter_int64(&iter);
        }
    }

    return true;
}


bool DbFindUserByUsername(DatabaseContext *ctx, const char *username, User *outUser) {
    mongoc_collection_t *collection = GetUsersCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW("username", BCON_UTF8(username));

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
        collection,
        filter,
        NULL,
        NULL
    );

    const bson_t *doc;
    bool found = false;

    if (mongoc_cursor_next(cursor, &doc)) {
        found = ParseUserFromBson(doc, outUser);
    }

    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error)) {
        fprintf(stderr, "[DB Error] Find user failed: %s\n", error.message);
        found = false;
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return found;
}

bool DbFindUserById(DatabaseContext *ctx, const bson_oid_t *id, User *outUser) {
    mongoc_collection_t *collection = GetUsersCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW("_id", BCON_OID(id));

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
        collection, filter, NULL, NULL
    );

    const bson_t *doc;
    bool found = false;

    if (mongoc_cursor_next(cursor, &doc)) {
        found = ParseUserFromBson(doc, outUser);
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return found;
}

bool DbUserExists(DatabaseContext *ctx, const char *username) {
    mongoc_collection_t *collection = GetUsersCollection(ctx);
    if (collection == NULL) {
        return false;
    }

    bson_t *filter = BCON_NEW("username", BCON_UTF8(username));
    bson_error_t error;

    const int64_t count = mongoc_collection_count_documents(
        collection,
        filter,
        NULL,
        NULL,
        NULL,
        &error
    );

    bson_destroy(filter);
    mongoc_collection_destroy(collection);

    return count > 0;
}