#ifndef CAMERA
#define CAMERA

typedef enum ImageResolution{
    RES_VGA,
    RES_QVGA,
}ImageResolution;

#define VGA_ROWS 640
#define VGA_COLS 480
#define QVGA_ROWS 160
#define QVGA_COLS 120

#define TEST_CAMERA 0
#define OV7670 1

extern ImageResolution selectedResolution;
extern int frameCols;
extern int frameRows;
extern int maxPixelPerFrame;
extern int** frame;
void initializeCamera();
void fillBufferWithPixels(int* buffer , int size);

void captureFrame();
#endif