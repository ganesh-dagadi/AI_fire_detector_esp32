#include "communication.h"
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <threadManager.h>
#include <cstring> // For memset
#include <queue>

Communication::Communication(std::mutex* streamLock , std::condition_variable* streamCond , std::atomic<bool>* isReadyToStream , std::queue<int>* buffer){
    this->streamLock = streamLock;
    this->streamCond = streamCond;
    this->isReadyToStream = isReadyToStream;
    this->stream = buffer;
}

Communication::~Communication(){
    std::cout << "Destructing Communication" << std::endl;
}

CommunicationResult Communication::setupConnection(int port){
    socketFd = socket(AF_INET , SOCK_DGRAM , 0);
    if(socketFd < 0){
        std::cerr << "Failed to create a socket" << std::endl;
        perror("Communication : socket create");
        return COMM_FAIL;
    }
    struct sockaddr_in sadr;
    memset(&sadr, 0, sizeof(sadr));
    sadr.sin_addr.s_addr = INADDR_ANY;
    sadr.sin_family = AF_INET;
    sadr.sin_port = htons(port);
    int bindErr = bind(socketFd , (struct sockaddr *)&sadr , sizeof sadr);
    if(bindErr < 0){
        std::cerr << "Failed to bind" << std::endl;
        perror("bind");
        return COMM_FAIL;
    }
    listen(socketFd , 2);
    std::cout << "Waiting for UDP packets on port 5000..." << std::endl;
    return COMM_OK;
}

// void Communication::captureStreamAndFillQueue(){
//     int recieved;
//     int recvSize;
//     while(1){
//         std::unique_lock<std::mutex> lock(*streamLock);
//         streamCond->wait(*streamLock , [this]() {
//             return (isReadyToStream->load() && stream->size() < MAX_QUEUE_SIZE);
//         });
            
//         recvSize = recvfrom(socketFd , &recieved , 4 , 0 , NULL , NULL);
//         if(recvSize < 0){
//             perror("recieve");
//             return;
//         }
//         stream->push(recieved);
//         streamLock->unlock();
//     }
// }
// CommunicationResult Communication::beginFillingFramePackets(){
//     ThreadManager::getInstance().postRunnable([this]() {
//         this->captureStreamAndFillQueue();
//     });    
// }

void Communication::captureStreamAndFillQueue() {
    int* received = new int[100];
    int recvSize;
    std::cout << "started filling stream" << std::endl;
    while (true) {
        std::unique_lock<std::mutex> lock(*streamLock);
        streamCond->wait(lock, [this]() {
            return (isReadyToStream->load() && stream->size() < MAX_QUEUE_SIZE);
        });
        recvSize = recvfrom(socketFd, received, 100 * sizeof(int), 0, nullptr, nullptr);
        if (recvSize < 0) {
            perror("receive");
            streamCond->notify_all();
            return;  // Exit on receive error
        }else if(recvSize > 0){
            for(int i = 0 ; i < 100 ; i++){
                stream->push(received[i]);
                std::cout << received[i] << " ";
            }
        }
        streamCond->notify_all();
    }
}

CommunicationResult Communication::beginFillingFramePackets() {
    // Correctly pass the member function to a thread
    ThreadManager::getInstance().postRunnable([this]() {
        this->captureStreamAndFillQueue();
    });

    return COMM_OK;  // Assuming COMM_OK is a valid return value for success
}
