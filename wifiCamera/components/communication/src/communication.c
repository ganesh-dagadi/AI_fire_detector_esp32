#include "communication.h"
#include "esp_log.h"
#include "lwip/sockets.h"

#define DOMAIN AF_INET
#define CONN_TYPE SOCK_DGRAM
#define IP_OC_1 192 
#define IP_OC_2 168
#define IP_OC_3 29
#define IP_OC_4 235
#define PORT 5000
void executeStateMachineCommunication();
CommunicationResult initializeCommunicationTask();
CommunicationResult connectToTarget();
void handleConnectionFailure();
CommunicationResult disconnect();
int socketFD;
int packetNO = 0;
char* COMM_TAG = "COMMUNICATION_TASK";
static short communicationReconnectionTimes = 0;
void communicationMainTask(void* params){
    while(1){
        executeStateMachineCommunication();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void executeStateMachineCommunication(){
<<<<<<< Updated upstream
=======

    if(!(communicationFlags & COMM_READY_TO_CONNECT) &&
    (communicationConnectionState == COMM_CONNECTED
    || communicationConnectionState == COMM_CONNECTING
    || communicationConnectionState == COMM_RECONNECTING
    )
    ){
        ESP_LOGW(COMM_TAG , "Disconnecting socket \n");
        disconnect();
    }
    
    if(communicationConnectionState == COMM_CONNECTED){
        int* newPacket = (int*) malloc(4000 * sizeof(int));
        int currPacketNo = 0;
        while(currPacketNo < 4000){
            newPacket[currPacketNo] = packetNO;
            currPacketNo++;
            packetNO++;
            if(packetNO > 19200){
                if(currPacketNo < 4000) newPacket[currPacketNo] = -1;
                packetNO = 0;
                currPacketNo = 0;
                ESP_LOGI(COMM_TAG , "completed a frame");
                break;
            }
        }
        currPacketNo = 0;
        send(socketFD , newPacket , 4000 * sizeof(int) , MSG_DONTWAIT);
        free(newPacket);
        //sleep for 50 ms to let reciever read packet
        vTaskDelay(20 / portTICK_PERIOD_MS);
        return;
    }

>>>>>>> Stashed changes
    if(communicationState == COMM_UNINITIALIZED){
        ESP_LOGI(COMM_TAG , "Initializing Communication machine");
        initializeCommunicationTask();
    }

    if(communicationState == COMM_INITIALIZED && 
    (communicationConnectionState == COMM_DISCONNECTED
    || communicationConnectionState == COMM_RECONNECTING)
    && communicationFlags & COMM_READY_TO_CONNECT){
        ESP_LOGI(COMM_TAG , "Connecting to target");
        connectToTarget();
    }

    if(!(communicationFlags & COMM_READY_TO_CONNECT) &&
    (communicationConnectionState == COMM_CONNECTED
    || communicationConnectionState == COMM_CONNECTING
    || communicationConnectionState == COMM_RECONNECTING
    )
    ){
        ESP_LOGW(COMM_TAG , "Disconnecting socket \n");
        disconnect();
    }

    if(communicationConnectionState == COMM_CONNECTED){
        ESP_LOGI(COMM_TAG ,"SENDING \n");
        int* newPacket = (int*)malloc(4);
        ESP_LOGI(COMM_TAG , "Sending %d" , packetNO);
        *newPacket = packetNO;
        send(socketFD , newPacket , 4 , MSG_DONTWAIT);
        free(newPacket);
        packetNO++;
        ESP_LOGI(COMM_TAG ,"Done sending \n");
    }
}

CommunicationResult initializeCommunicationTask(){
    communicationState = COMM_INITIALIZING;
    
    socketFD = socket(DOMAIN , CONN_TYPE , 0);
    if(socketFD < 0){
        ESP_LOGE(COMM_TAG , "Socket creation failed \n");
        communicationState = COMM_INITIALIZATION_FAIL;
        return COMM_FAIL;
    }
    ESP_LOGI(COMM_TAG , "Socket created \n");
    communicationState = COMM_INITIALIZED;
    return COMM_OK;
}

CommunicationResult connectToTarget(){
    communicationConnectionState = COMM_CONNECTING;
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = htonl((((((IP_OC_1 << 8) | IP_OC_2) << 8) | IP_OC_3) << 8) | IP_OC_4);
    if(connect(socketFD , (struct sockaddr *)&sa , sizeof sa) < 0){
        ESP_LOGW(COMM_TAG , "Socket connection failed \n");
        handleConnectionFailure();
        return COMM_FAIL;
    }
    ESP_LOGI(COMM_TAG ,"Socket connected \n");
    communicationConnectionState = COMM_CONNECTED;
    return COMM_OK;
}

CommunicationResult disconnect(){
    int res = close(socketFD);
    if(socketFD < 0){
        ESP_LOGE(COMM_TAG , "Socket disconnect failed \n");
        return COMM_FAIL;
    }
    communicationConnectionState = COMM_DISCONNECTED;
    // uninitialized to recreate a socket
    communicationState = COMM_UNINITIALIZED;
    return COMM_OK;
}
void handleConnectionFailure(){
    ESP_LOGW(COMM_TAG , "Attempting reconnect in 5s");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    if(communicationReconnectionTimes < 3 && communicationFlags & COMM_READY_TO_CONNECT){
        communicationConnectionState = COMM_RECONNECTING;
        communicationReconnectionTimes++;
        return;
    }
    ESP_LOGE(COMM_TAG , "Failed to Connect. Escalating to Connection fail");
    communicationConnectionState = COMM_CONNECTION_FAIL;
}