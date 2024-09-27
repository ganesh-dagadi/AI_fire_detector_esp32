#ifndef COMMUNICATION
#define COMMUNICATION

#define COMM_READY_TO_CONNECT 0x00000001
typedef enum CommunicationState{
    COMM_UNINITIALIZED,
    COMM_INITIALIZING,
    COMM_INITIALIZATION_FAIL,
    COMM_INITIALIZED
} CommunicationState;

typedef enum CommunicationConnectionState{
    COMM_DISCONNECTED,
    COMM_CONNECTING,
    COMM_RECONNECTING,
    COMM_CONNECTED,
    COMM_CONNECTION_FAIL
} CommunicationConnectionState;

typedef enum CommunicationResult{
    COMM_OK,
    COMM_FAIL
}CommunicationResult;

extern CommunicationState communicationState;
extern CommunicationConnectionState communicationConnectionState;

/*
    bit 0 -> communication is ready to connect - default falses
*/
extern int communicationFlags;

void communicationMainTask(void* params);

#endif