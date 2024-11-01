#include "imageGen.h"
#include <iostream>
#include "threadManager.h"
#include "jpeglib.h"
#include <string.h>

struct jpeg_compress_struct cinfo;
struct jpeg_error_mgr jerr;
FILE *outfile;
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
    jpeg_destroy_compress(&cinfo);
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
        if(readPixel == -1){
            if(isFillingFrame.load()){
                // we were already filling a frame but got a new frame start flag,
                if(CORRUPTED_FRAME_HANDLING == FRAME_STORE){
                    std::cout << "Got corrupted frame. Storing anyways";
                    this->produceTargetImage();
                }
            }
            currPixel = 0;
            isFillingFrame.store(true);
        }else if(readPixel == -2){
            //handle frame complete
            if(!isFillingFrame.load()){
                //not filling a frame but got -2, junk pixel
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
                    //reset the entire frame and wait for -1
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
    //init jpeg lib
    std::cout << "hello" << std::endl;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    outfile = fopen("output.jpg", "wb");
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = frameCols;
    cinfo.image_height = frameRows;
    cinfo.input_components = 3; // For RGB
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    std::cout << "wrote" << " ";
    std::vector<unsigned char> rgbArr;
    for(int i = 0 ; i < frameRows ; i++){
        for(int j = 0 ; j < frameCols ; j++){
           int curr = frame[i][j];
           rgbArr.push_back((unsigned char)(curr >> 16));
           rgbArr.push_back((unsigned char)(curr >> 8));
           rgbArr.push_back((unsigned char)curr);
        }
    }
    
    JSAMPROW row_pointer[1];
    int currRow = 0;
    while (currRow < frameRows) {
        row_pointer[0] = &rgbArr[currRow * frameCols * 3]; // 3 bytes per pixel
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
        currRow++;
    }
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
}