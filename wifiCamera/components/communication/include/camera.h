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

extern ImageResolution selectedResolution;
extern int frameCols;
extern int frameRows;
extern int maxPixelPerFrame;
void initializeCamera();
void fillBufferWithPixels(int* buffer , int size);
#endif