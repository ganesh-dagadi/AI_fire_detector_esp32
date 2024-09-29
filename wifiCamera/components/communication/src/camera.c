#include "camera.h"
#include "esp_log.h"
const char* CAMERA_TAG = "CAMERA";
int maxPixelPerFrame;
ImageResolution selectedResolution;
int frameCols;
int frameRows;

void selectDimensions();
void initializeCamera(){
    ESP_LOGI(CAMERA_TAG , "Initializing Camera");
    selectedResolution = RES_QVGA;
    selectDimensions();
    maxPixelPerFrame = frameRows * frameCols;
}

void selectDimensions(){
    switch (selectedResolution)
    {
    case RES_QVGA:
        frameRows = QVGA_ROWS;
        frameCols = QVGA_COLS;
        break;
    
    default:
        break;
    }
}
int getNextPixel(){
    //logic to get pixel from camera
    static int packetNo = -1;
    packetNo++;
    if(packetNo >= maxPixelPerFrame - 1){
        packetNo = -1;
    }
    return packetNo;
}

void fillBufferWithPixels(int* buffer , int size){
    int currPacketNo = 0;
    int currentPixel;
    while(currPacketNo < size){
        currentPixel = getNextPixel();
        buffer[currPacketNo] = currentPixel;
        currPacketNo++;
        if(currentPixel == -1){
            if(currPacketNo < 4000) buffer[currPacketNo] = -2;
            ESP_LOGI(CAMERA_TAG , "completed a frame");
            break;
        }
    }
    return;
}