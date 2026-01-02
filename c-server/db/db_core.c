#include "db.h"
#include <stdio.h>
#include <stdlib.h>

void DbInit() {
    mongoc_init();
}

void DbCleanup() {
    mongoc_cleanup();
}

bool DbConnect(DatabaseContext *ctx) {
    char uriString[MAX_URI_LENGTH];
    char dbName[MAX_DB_NAME_LENGTH];
    bson_error_t error;
    mongoc_uri_t *uri = NULL;

    ctx->Client = NULL;
    ctx->Database = NULL;
    ctx->IsConnected = false;

    if (!DbLoadUriFromEnv(uriString, sizeof(uriString))) {
        fprintf(stderr, "[DB Err] Cannot load MONGODB_URI from .env\n");
        return false;
    }

    if (!DbLoadDbNameFromEnv(dbName, sizeof(dbName))) {
        fprintf(stderr, "[DB Err] Cannot load DB_NAME from .env\n");
        return false;
    }

    uri = mongoc_uri_new_with_error(uriString, &error);
    if (!uri) {
        fprintf(stderr, "[DB Err] Failed to load MONGODB_URI from .env\n");
        return false;
    }
    
    ctx->Client = mongoc_client_new_from_uri(uri);
    if (!ctx->Client) {
        fprintf(stderr, "[DB Err] Failed to create client\n");
        mongoc_uri_destroy(uri);
        return false;
    }

    mongoc_client_set_appname(ctx->Client, "MeetingSchedulerServer");

    ctx->Database = mongoc_client_get_database(ctx->Client, dbName);

    mongoc_uri_destroy(uri);

    if (!DbPing(ctx)) {
        fprintf(stderr, "[DB Err] Ping failed - connection may not be working\n");
        DbDisconnect(ctx);
        return false;
    }

    ctx->IsConnected = true;
    printf("[DB] Connected successfully to MongoDB!\n");


    return true;
}

void DbDisconnect(DatabaseContext *ctx) {
    if (ctx->Database != NULL) {
        mongoc_database_destroy(ctx->Database);
        ctx->Database = NULL;
    }

    if (ctx->Client != NULL) {
        mongoc_client_destroy(ctx->Client);
        ctx->Client = NULL;
    }

    ctx->IsConnected = false;
    printf("[DB] Disconnected from MongoDB\n");
}

bool DbPing(DatabaseContext *ctx) {
    if (ctx->Client == NULL) {
        return false;
    }

    bson_t *command = BCON_NEW("ping", BCON_INT32(1));
    bson_t reply;
    bson_error_t error;

    bool success = mongoc_client_command_simple(
        ctx->Client,
        "admin",
        command,
        NULL,
        &reply,
        &error
    );

    if (!success) {
        fprintf(stderr, "[DB Error] Ping failed: %s\n", error.message);
    }

    bson_destroy(&reply);
    bson_destroy(command);

    return success;
}