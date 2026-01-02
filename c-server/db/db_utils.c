#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool DbStringToOid(const char *str, bson_oid_t *oid) {
    if (str == NULL || oid == NULL) {
        return false;
    }

    size_t len = strlen(str);
    if (len != 24) {
        return false;
    }

    if (!bson_oid_is_valid(str, len)) {
        return false;
    }

    bson_oid_init_from_string(oid, str);
    return true;
}

void DbOidToString(const bson_oid_t *oid, char *str) {
    bson_oid_to_string(oid, str);
}

void DbFreeMeeting(Meeting *meeting) {
    if (meeting == NULL) {
        return;
    }

    if (meeting->StudentIds != NULL) {
        free(meeting->StudentIds);
        meeting->StudentIds = NULL;
    }

    if (meeting->Minutes != NULL) {
        free(meeting->Minutes);
        meeting->Minutes = NULL;
    }

    meeting->StudentCount = 0;
}

void DbFreeMeetings(Meeting *meetings, int count) {
    if (meetings == NULL || count <= 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        DbFreeMeeting(&meetings[i]);
    }

    free(meetings);
}