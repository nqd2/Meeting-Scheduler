#include "protocol.h"
#include <stdio.h>
#include <string.h>

void ProtocolBuildResponse(const Response *response, char *outBuffer, size_t bufferSize) {
    if (response == NULL || outBuffer == NULL || bufferSize == 0) {
        return;
    }

    if (response->Payload[0] != '\0') {
        snprintf(outBuffer, bufferSize, "%d %s\n", response->Code, response->Payload);
    } else {
        snprintf(outBuffer, bufferSize, "%d\n", response->Code);
    }
}

const char* ProtocolCommandToString(CommandType cmd) {
    switch (cmd) {
        case CmdRegister: return "REGISTER";
        case CmdLogin: return "LOGIN";
        case CmdLogout: return "LOGOUT";
        case CmdAddSlot: return "ADD_SLOT";
        case CmdDeleteSlot: return "DELETE_SLOT";
        case CmdListAppointments: return "LIST_APPOINTMENTS";
        case CmdAddMinutes: return "ADD_MINUTES";
        case CmdGetMinutes: return "GET_MINUTES";
        case CmdBookIndividual: return "BOOK_INDIVIDUAL";
        case CmdBookGroup: return "BOOK_GROUP";
        case CmdCancelMeeting: return "CANCEL_MEETING";
        case CmdListWeekMeetings: return "LIST_WEEK_MEETINGS";
        case CmdListFreeSlots: return "LIST_FREE_SLOTS";
        default: return "UNKNOWN";
    }
}