#include "imageGen.h"
#include <iostream>
#include "threadManager.h"
ImageGenerator::ImageGenerator(std::mutex* streamLock , std::condition_variable* streamCond , std::atomic<bool>* isReadyToStream , std::queue<int>* buffer){
    this->streamLock = streamLock;
    this->streamCond = streamCond;
    this->isReadyToStream = isReadyToStream;
    this->stream = buffer;
}

ImageGenerator::~ImageGenerator(){
    delete frame;
}
ImageGeneratorResult ImageGenerator::initImageGenerator(ImageResolutions resolution){
    switch (resolution)
    {
    case VGA:
        frame = new std::vector(VGA_ROWS , std::vector<int>(VGA_COLS , 0));
        break;
    case QVGA:
        frame = new std::vector(QVGA_ROWS , std::vector<int>(QVGA_COLS , 0));
    default:
        break;
    }
    return IMG_GEN_OK;
}

void ImageGenerator::readImageFromBuffer(){
    ThreadManager::getInstance().postRunnable([this](){
        this->generateFrameFromBuffer();
    });
}

void ImageGenerator::generateFrameFromBuffer(){
    int item;
    while(1){
        std::unique_lock<std::mutex> lock(*streamLock);
        std::cout << "Waiting in consumer" << std::endl;
        streamCond->wait(lock , [this](){return (isReadyToStream->load() && !stream->empty());});
        std::cout << "Entering critical section in consumer" << std::endl;
        item = stream->front();
        std::cout << "recieved " << item << " from buffer \n";
        stream->pop();
        std::cout << "exiting critical section in consumer" << std::endl;
        streamCond->notify_all();
    }
}