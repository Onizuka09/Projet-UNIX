#ifndef COMMON_H
#define COMMON_H
#include "service.h"

#define PORT 8080
typedef enum
{
    LISTS_DIR = 0,
    CAT_DIR,
    ELASPSED_TIME,
    DATE_TIME,
    END,
    AUTH,
    ACK,
} Services_e;

typedef struct
{
    char directory_path[256]; // Path for LISTS_DIR or CAT_DIR
    char output[1024];
} ListCatPayload;

typedef struct
{
    char msgACK[1024]; // ACKPayload
} ACKPayload;


typedef struct
{
    char date[50]; // ACKPayload
    char time[50]; // ACKPayload
} TimeDatePayload;
typedef struct
{
    double elapsed_time_s; // Time in milliseconds
} ElapsedTimePayload;

typedef struct
{
    char username[50];
    char password[50];
} AuthPayload;

// Union to encapsulate different message payloads
typedef union
{
    ListCatPayload listCatPayload;
    ElapsedTimePayload elapsedTimePayload;
    AuthPayload authPayload;
    ACKPayload ackPayload;
    TimeDatePayload timeDatePayload ; 
} MessagePayload;

// Main message struct
typedef struct
{
    Services_e serviceType; // Message type
    MessagePayload payload; // Message payload
} Message;

void handle_Auth(AuthPayload *paylod);
void handle_ACK(ACKPayload *paylod);
void handle_listcat(ListCatPayload *payload);
void handle_elspasedTime(ElapsedTimePayload *payload);

#endif