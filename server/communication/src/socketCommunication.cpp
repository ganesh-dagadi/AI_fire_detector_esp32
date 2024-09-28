#include "communication.h"
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <threadManager.h>
#include <cstring> // For memset
#include <queue>
#include <stdio.h>

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
<<<<<<< Updated upstream
    int received;
=======
    int* received = new int[4000];
>>>>>>> Stashed changes
    int recvSize;
    std::cout << "started filling stream" << std::endl;
    FILE* file = fopen("socetComm" , "w");
    while (true) {
        std::unique_lock<std::mutex> lock(*streamLock);

        std::cout << "Beginning wait in producer" << std::endl;
        streamCond->wait(lock, [this]() {
            return (isReadyToStream->load() && stream->size() < MAX_QUEUE_SIZE);
        });
<<<<<<< Updated upstream
        std::cout << "entered critical section in producer" << std::endl;
        recvSize = recvfrom(socketFd, &received, sizeof(received), 0, nullptr, nullptr);
=======
        recvSize = recvfrom(socketFd, received, 4000 * sizeof(int), 0, nullptr, nullptr);
>>>>>>> Stashed changes
        if (recvSize < 0) {
            perror("receive");
            streamCond->notify_all();
            return;  // Exit on receive error
        }else if(recvSize > 0){
<<<<<<< Updated upstream
            std::cout << stream->size() << std::endl;
            stream->push(received);
=======
            for(int i = 0 ; i < 4000 ; i++){

                if(received[i] == -1) break;
                stream->push(received[i]);
                fprintf(file , "%d \n" , received[i]);
            }
>>>>>>> Stashed changes
        }
        std::cout << "exiting critical section in producer" << std::endl;
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
