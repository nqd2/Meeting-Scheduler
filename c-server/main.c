#include <stdio.h>
#include <stdlib.h>
#include "db.h"

int main(void) {
    printf("=== TEST MONGODB CONNECTION ===\n");

    DbInit();

    DatabaseContext dbCtx;

    printf("Connecting to MongoDB...\n");
    bool connected = DbConnect(&dbCtx);

    if (connected) {
        printf("\n[SUCCESS] Connected successfully!\n");
        printf("Database Object: %p\n", (void*)dbCtx.Database);

        if (DbPing(&dbCtx)) {
            printf("[INFO] Ping Database successful.\n");
        } else {
            printf("[WARNING] Ping Database failed.\n");
        }

        DbDisconnect(&dbCtx);
        printf("[INFO] Disconnected.\n");
    } else {
        fprintf(stderr, "\n[ERROR] Failed to connect to MongoDB.\n");
        fprintf(stderr, "Please check the .env file and the connection string.\n");
    }

    DbCleanup();

    printf("=== END TEST ===\n");

    return connected ? 0 : 1;
}