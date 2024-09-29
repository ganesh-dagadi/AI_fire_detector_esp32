#ifndef COMMUNICATION
#define COMMUNICATION

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>

#define MAX_QUEUE_SIZE 100000
#define UDP_PACKET_PIXEL_CAP 1000
typedef enum CommunicationResult{
    COMM_OK,
    COMM_FAIL
}CommunicationResult;

class Communication{
    private:
    int socketFd;
    std::mutex* streamLock;
    std::condition_variable* streamCond;
    std::atomic<bool>* isReadyToStream;
    std::queue<int>* stream;

    void captureStreamAndFillQueue();
    public:
    Communication(std::mutex* streamLock , std::condition_variable* streamCond , std::atomic<bool>* isReadyToStream , std::queue<int>* buffer);
    ~Communication();

    //Setup connection and be ready to listen to packets
    CommunicationResult setupConnection(int port);

    // called when caller is ready to read packets
    // starts a thread internally that fills ipc queue
    CommunicationResult beginFillingFramePackets();

    CommunicationResult stopFillingFramePackets();

};
#endif