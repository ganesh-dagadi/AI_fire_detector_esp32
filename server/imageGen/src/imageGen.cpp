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
    for(int i = 0 ; i < frameRows ; i++){
        delete[] frame[i];
    }
    delete[] frame;
}
ImageGeneratorResult ImageGenerator::initImageGenerator(ImageResolutions resolution){

    switch (resolution)
    {
    case VGA:
        frameRows = VGA_ROWS;
        frameCols = VGA_COLS;
        break;
    case QVGA:
        frameRows = QVGA_ROWS;
        frameCols = QVGA_COLS;
        break;
    case TEST_1000:
        frameRows = TEST_ROWS;
        frameCols = TEST_COLS;
    default:
        break;
    }
    frame = new int*[frameRows];
    for(int i = 0 ; i < frameRows ; i++){
        frame[i] = new int[frameCols];
    }
    currPixel = 0;
    maxPixelPos = frameRows * frameCols;
    return IMG_GEN_OK;
}

void ImageGenerator::readImageFromBuffer(){
    ThreadManager::getInstance().postRunnable([this](){
        this->generateFrameFromBuffer();
    });
}

void ImageGenerator::generateFrameFromBuffer(){
    int readPixel;
    while(1){
        std::unique_lock<std::mutex> lock(*streamLock);
        streamCond->wait(lock , [this](){return (isReadyToStream->load() && !stream->empty());});
        readPixel = stream->front();
        stream->pop();
        //build image frame from recieved pixel.
        //Done within critical section as currPixel should be in sync with recieved frame
        currPixel++;
        if(currPixel >= maxPixelPos){
            //handle frame complete
            currPixel = 0;
            for(int i = 0 ; i < frameRows ; i++){
                for(int j = 0 ; j < frameCols ; j++){
                    //std::cout << frame[i][j] << " ";
                }
                //std::cout << std::endl;
            }
        }
        else{
            //store the recieved pixel into the frame
            *((*(frame + currPixel / frameRows)) + (currPixel % frameCols)) = readPixel;
        }
        streamCond->notify_all();

        
        
    }
}