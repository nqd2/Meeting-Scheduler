//
// Created by Unity on 1/2/2026.
//

#ifndef C_SERVER_DB_H
#define C_SERVER_DB_H

#include <mongoc/mongoc.h>
#include <bson/bcon.h>
#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// CONSTANTS
// ============================================================================
#define COLLECTION_USERS "users"
#define COLLECTION_SLOTS "slots"
#define COLLECTION_MEETINGS "meetings"
#define MAX_URI_LENGTH 512
#define MAX_DB_NAME_LENGTH 64

// ============================================================================
// ENUMS (PascalCase)
// ============================================================================
typedef enum {
    RoleTeacher,
    RoleStudent
} UserRole;

typedef enum {
    MeetingTypeIndividual,
    MeetingTypeGroup
} MeetingType;

typedef enum {
    MeetingStatusScheduled,
    MeetingStatusCompleted,
    MeetingStatusCancelled
} MeetingStatus;

// ============================================================================
// STRUCTS (PascalCase)
// ============================================================================
typedef struct {
    mongoc_client_t *Client;
    mongoc_database_t *Database;
    bool IsConnected;
} DatabaseContext;

typedef struct {
    bson_oid_t Id;
    char Username[64];
    char PasswordHash[256];
    char FullName[128];
    UserRole Role;
    int64_t CreatedAt;
} User;

typedef struct {
    bson_oid_t Id;
    bson_oid_t TeacherId;
    int64_t StartTime;
    int64_t EndTime;
    bool IsBooked;
    int64_t CreatedAt;
} Slot;

typedef struct {
    bson_oid_t Id;
    bson_oid_t SlotId;
    bson_oid_t TeacherId;
    bson_oid_t *StudentIds;
    int StudentCount;
    MeetingType Type;
    MeetingStatus Status;
    char *Minutes;
    int64_t CreatedAt;
} Meeting;


// init mongodb driver
void DbInit(void);
// cleanup mongodb driver
void DbCleanup(void);
// connect to database from environment variable
bool DbConnect(DatabaseContext *ctx);
// disconnect from database
void DbDisconnect(DatabaseContext *ctx);
// check if database is connected
bool DbPing(DatabaseContext *ctx);


// load uri from environment variable
bool DbLoadUriFromEnv(char *uriBuffer, size_t bufferSize);
// load db name from environment variable
bool DbLoadDbNameFromEnv(char *dbNameBuffer, size_t bufferSize);
// create user
bool DbCreateUser(DatabaseContext *ctx, const User *user, bson_oid_t *outId);
// find user by username
bool DbFindUserByUsername(DatabaseContext *ctx, const char *username, User *outUser);
// find user by id
bool DbFindUserById(DatabaseContext *ctx, const bson_oid_t *id, User *outUser);
// check if username exists
bool DbUserExists(DatabaseContext *ctx, const char *username);


// create slot
bool DbCreateSlot(DatabaseContext *ctx, const Slot *slot, bson_oid_t *outId);
// find free slots by teacher id
bool DbFindFreeSlots(DatabaseContext *ctx, const bson_oid_t *teacherId,
                     Slot **outSlots, int *outCount);
// update slot booked
bool DbUpdateSlotBooked(DatabaseContext *ctx, const bson_oid_t *slotId, bool isBooked);
// delete slot
bool DbDeleteSlot(DatabaseContext *ctx, const bson_oid_t *slotId);
// check if slot overlaps with another slot
bool DbCheckSlotOverlap(DatabaseContext *ctx, const bson_oid_t *teacherId,
                        int64_t startTime, int64_t endTime);


// create meeting
bool DbCreateMeeting(DatabaseContext *ctx, const Meeting *meeting, bson_oid_t *outId);
// find meetings by teacher id and date
bool DbFindMeetingsByTeacher(DatabaseContext *ctx, const bson_oid_t *teacherId,
                             int64_t date, Meeting **outMeetings, int *outCount);
// find meetings by student id and week
bool DbFindMeetingsByStudentWeek(DatabaseContext *ctx, const bson_oid_t *studentId,
                                 int year, int week, Meeting **outMeetings, int *outCount);
// update meeting status
bool DbUpdateMeetingStatus(DatabaseContext *ctx, const bson_oid_t *meetingId,
                           MeetingStatus status);
// update meeting minutes
bool DbUpdateMeetingMinutes(DatabaseContext *ctx, const bson_oid_t *meetingId,
                            const char *minutesContent);
// get meeting minutes
bool DbGetMeetingMinutes(DatabaseContext *ctx, const bson_oid_t *meetingId,
                         char **outMinutes);


// utility functions
// convert string to oid
bool DbStringToOid(const char *str, bson_oid_t *oid);
// convert oid to string
void DbOidToString(const bson_oid_t *oid, char *str);
// free meeting
void DbFreeMeeting(Meeting *meeting);
// free meetings
void DbFreeMeetings(Meeting *meetings, int count);

#endif //C_SERVER_DB_H
