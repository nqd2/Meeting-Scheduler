#ifndef C_SERVER_PROTOCOL_H
#define C_SERVER_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    StatusOk = 2000,
    StatusBadRequest = 4000,
    StatusConflict = 4001,
    StatusTokenMissing = 4010,
    StatusTokenInvalid = 4011,
    StatusForbidden = 4030,
    StatusNotFound = 4040,
    StatusWrongPassword = 4041,
    StatusServerError = 5000
} StatusCode;

#define MAX_TOKEN_LENGTH 64
#define MAX_DATA_LENGTH 1024
#define MAX_COMMAND_LENGTH 32

typedef struct {
    CommandType Type;
    char Token[MAX_TOKEN_LENGTH];
    char Data[MAX_DATA_LENGTH];
    bool HasToken;
} Request;

#define MAX_PAYLOAD_LENGTH 4096

typedef struct {
    StatusCode Code;
    char Payload[MAX_PAYLOAD_LENGTH];
} Response;

bool ProtocolParseRequest(const char *rawMessage, Request *outRequest);

CommandType ProtocolStringToCommand(const char *cmdStr);

const char* ProtocolCommandToString(CommandType cmd);

void ProtocolBuildResponse(const Response *response, char *outBuffer, size_t bufferSize);

int ProtocolSplitData(const char *data, char delimiter, char **outParts, int maxParts);

void ProtocolFreeDataParts(char **parts, int count);

#endif