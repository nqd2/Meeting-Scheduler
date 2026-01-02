#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool loadEnvFromFile(const char *key, char *buffer, size_t bufferSize) {
    FILE *file = fopen(".env", "r");
    if (file ==NULL) {
        return false;
    }

    char line[1024];
    size_t keyLen = strlen(key);
    bool found = false;

    while (fgets(line, sizeof(line), file) != NULL) {
        // empty lines or comments
        if (line[0] == '\n' || line[0] == '#' || line[0] == '\0') {
            continue;
        }

        // find '='
        char *equalSign = strchr(line, '=');
        if (equalSign == NULL) {
            continue;
        }

        const size_t lineKeyLen = equalSign - line;

        if (lineKeyLen == keyLen && strncmp(line, key, lineKeyLen) == 0) {
            char *value = equalSign + 1;

            size_t valueLen = strlen(value);
            while (valueLen > 0 && (value[valueLen-1] == '\n' || value[valueLen-1] == '\r')) {
                value[valueLen-1] = '\0';
                valueLen--;
            }

            strncpy(buffer, value, bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
            found = true;
            break;
        }
    }

    fclose(file);
    return found;
}

bool DbLoadUriFromEnv(char *uriBuffer, size_t bufferSize) {
    const char *env = getenv("MONGODB_URI");
    if (env != NULL) {
        strncpy(uriBuffer, env, bufferSize - 1);
        uriBuffer[bufferSize - 1] = '\0';
        return true;
    }
    return loadEnvFromFile("MONGODB_URI", uriBuffer, bufferSize);
}

bool DbLoadDbNameFromEnv(char *dbNameBuffer, size_t bufferSize) {
    const char *env = getenv("DB_NAME");
    if (env != NULL) {
        strncpy(dbNameBuffer, env, bufferSize - 1);
        dbNameBuffer[bufferSize - 1] = '\0';
        return true;
    }
    return loadEnvFromFile("DB_NAME", dbNameBuffer, bufferSize);
}
