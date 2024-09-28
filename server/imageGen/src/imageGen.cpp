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
<<<<<<< Updated upstream
        frame = new std::vector(QVGA_ROWS , std::vector<int>(QVGA_COLS , 0));
=======
        frameRows = QVGA_ROWS;
        frameCols = QVGA_COLS;
        break;
    case TEST_1000:
        frameRows = TEST_ROWS;
        frameCols = TEST_COLS;
        break;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    int item;
=======
    int readPixel;
    FILE* reader = fopen("reader.txt" , "w");
    FILE* frameReader = fopen("frameReader.txt" , "w");
>>>>>>> Stashed changes
    while(1){
        std::unique_lock<std::mutex> lock(*streamLock);
        std::cout << "Waiting in consumer" << std::endl;
        streamCond->wait(lock , [this](){return (isReadyToStream->load() && !stream->empty());});
        std::cout << "Entering critical section in consumer" << std::endl;
        item = stream->front();
        std::cout << "recieved " << item << " from buffer \n";
        stream->pop();
<<<<<<< Updated upstream
        std::cout << "exiting critical section in consumer" << std::endl;
=======

        fprintf(reader , "%d \n" , readPixel);
        //build image frame from recieved pixel.
        //Done within critical section as currPixel should be in sync with recieved frame
        currPixel++;
        if(currPixel >= maxPixelPos){
            //std::cout << "frame complete " << currPixel << std::endl;
            //handle frame complete
             currPixel = 0;
            for(int i = 0 ; i < frameRows ; i++){
                for(int j = 0 ; j < frameCols ; j++){
                    fprintf(frameReader , "%d " , frame[i][j]);
                }
                fprintf(frameReader , "\n");
            }
            fprintf(frameReader , "\n");
            fprintf(frameReader , "\n");
            fprintf(frameReader , "\n");
            fprintf(frameReader , "\n");
            fprintf(frameReader , "\n");
        }else{
            //store the recieved pixel into the frame
             *((*(frame + currPixel / frameCols)) + (currPixel % frameCols)) = readPixel;
            // frame.push_back(readPixel);
        }
>>>>>>> Stashed changes
        streamCond->notify_all();
    }
}