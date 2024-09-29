#include <iostream>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include "communication.h"
#include "imageGen.h"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <threadManager.h>
#include <queue>
using namespace std;
int main(){
    atomic<bool> isReadyToConnect;
    mutex streamLock;
    condition_variable streamCond;
    std::queue<int> stream;
    Communication communication(&streamLock , &streamCond , &isReadyToConnect, &stream);
    ImageGenerator imageGenerator(&streamLock , &streamCond , &isReadyToConnect, &stream);
    communication.setupConnection(5000);
    imageGenerator.initImageGenerator(QVGA);
    isReadyToConnect.store(true);
    communication.beginFillingFramePackets();
    imageGenerator.readImageFromBuffer();
    
    ThreadManager::getInstance().waitForAllThreads();

}