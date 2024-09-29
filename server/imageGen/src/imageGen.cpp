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
    isFillingFrame.store(false);
    return IMG_GEN_OK;
}

void ImageGenerator::readImageFromBuffer(){
    ThreadManager::getInstance().postRunnable([this](){
        this->generateImageFromBuffer();
    });
}

void ImageGenerator::generateImageFromBuffer(){
    int readPixel;
    while(1){
        std::unique_lock<std::mutex> lock(*streamLock);
        streamCond->wait(lock , [this](){return (isReadyToStream->load() && !stream->empty());});
        readPixel = stream->front();
        stream->pop();
        streamCond->notify_all();  

        //build image frame from recieved pixel.
        if(isFillingFrame.load())
            currPixel++;
        if(readPixel == 0){
            if(isFillingFrame.load()){
                // we were already filling a frame but got a new frame start flag,
                if(CORRUPTED_FRAME_HANDLING == FRAME_STORE){
                    std::cout << "Got corrupted frame. Storing anyways";
                    this->produceTargetImage();
                }
            }
            currPixel = 0;
            isFillingFrame.store(true);
        }else if(readPixel == -1){
            //handle frame complete
            if(!isFillingFrame.load()){
                //not filling a frame but got -1, junk pixel
                continue;
            }
            this->produceTargetImage();
            currPixel = 0;
            isFillingFrame.store(false);
        }
        else{
            if(isFillingFrame.load()){  
                if(currPixel >= maxPixelPos){
                    //if currPixel has crossed maxPixel, we should have lost both end frame and start frame flags.
                    //reset the entire frame and wait for 0
                    isFillingFrame.store(false);
                    currPixel = 0;
                    continue;
                } 
                *((*(frame + currPixel / frameCols)) + (currPixel % frameCols)) = readPixel;
            }       
        }
    }
}

void ImageGenerator::produceTargetImage(){
    static int frameNo = 0;
    std::cout << "frame " << frameNo << std::endl;
    frameNo++;
    for(int i = 0 ; i < frameRows ; i++){
        for(int j = 0 ; j < frameCols ; j++){
            std::cout << frame[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}