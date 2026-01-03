#include "protocol.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

CommandType ProtocolStringToCommand(const char *cmdStr) {
    if (strcmp(cmdStr, "REGISTER") == 0) return CmdRegister;
    if (strcmp(cmdStr, "LOGIN") == 0) return CmdLogin;
    if (strcmp(cmdStr, "LOGOUT") == 0) return CmdLogout;
    if (strcmp(cmdStr, "ADD_SLOT") == 0) return CmdAddSlot;
    if (strcmp(cmdStr, "DELETE_SLOT") == 0) return CmdDeleteSlot;
    if (strcmp(cmdStr, "LIST_APPOINTMENTS") == 0) return CmdListAppointments;
    if (strcmp(cmdStr, "ADD_MINUTES") == 0) return CmdAddMinutes;
    if (strcmp(cmdStr, "GET_MINUTES") == 0) return CmdGetMinutes;
    if (strcmp(cmdStr, "BOOK_INDIVIDUAL") == 0) return CmdBookIndividual;
    if (strcmp(cmdStr, "BOOK_GROUP") == 0) return CmdBookGroup;
    if (strcmp(cmdStr, "CANCEL_MEETING") == 0) return CmdCancelMeeting;
    if (strcmp(cmdStr, "LIST_WEEK_MEETINGS") == 0) return CmdListWeekMeetings;
    if (strcmp(cmdStr, "LIST_FREE_SLOTS") == 0) return CmdListFreeSlots;
    return CmdUnknown;
}

bool ProtocolParseRequest(const char *rawMessage, Request *outRequest) {
    if (rawMessage == NULL || outRequest == NULL) {
        return false;
    }

    memset(outRequest, 0, sizeof(Request));
    outRequest->Type = CmdUnknown;
    outRequest->HasToken = false;

    char buffer[MAX_DATA_LENGTH + MAX_TOKEN_LENGTH + MAX_COMMAND_LENGTH + 10];
    strncpy(buffer, rawMessage, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    size_t len = strlen(buffer);
    while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
        buffer[len-1] = '\0';
        len--;
    }

    char *saveptr;
    const char *commandStr = strtok_r(buffer, " ", &saveptr);
    if (commandStr == NULL) {
        return false;
    }

    outRequest->Type = ProtocolStringToCommand(commandStr);
    if (outRequest->Type == CmdUnknown) {
        return false;
    }

    if (outRequest->Type == CmdRegister || outRequest->Type == CmdLogin) {
        char *data = strtok_r(NULL, "", &saveptr);
        if (data != NULL) {
            strncpy(outRequest->Data, data, MAX_DATA_LENGTH - 1);
        }
        return true;
    }

    char *token = strtok_r(NULL, " ", &saveptr);
    if (token != NULL) {
        strncpy(outRequest->Token, token, MAX_TOKEN_LENGTH - 1);
        outRequest->HasToken = true;
    }

    char *data = strtok_r(NULL, "", &saveptr);
    if (data != NULL) {
        strncpy(outRequest->Data, data, MAX_DATA_LENGTH - 1);
    }

    return true;
}

int ProtocolSplitData(const char *data, char delimiter, char **outParts, int maxParts) {
    if (data == NULL || outParts == NULL || maxParts <= 0) {
        return 0;
    }

    char *buffer = strdup(data);
    if (buffer == NULL) {
        return 0;
    }

    int count = 0;
    char *start = buffer;
    char *current = buffer;

    while (*current != '\0' && count < maxParts) {
        if (*current == delimiter) {
            *current = '\0';
            outParts[count] = strdup(start);
            count++;
            start = current + 1;
        }
        current++;
    }

    if (count < maxParts && *start != '\0') {
        outParts[count] = strdup(start);
        count++;
    }

    free(buffer);
    return count;
}

void ProtocolFreeDataParts(char **parts, int count) {
    if (parts == NULL) return;
    for (int i = 0; i < count; i++) {
        if (parts[i] != NULL) {
            free(parts[i]);
            parts[i] = nullptr;
        }
    }
}