#ifndef IMAGE_GENERATOR
#define IMAGE_GENERATOR

#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>

#define VGA_ROWS 640
#define VGA_COLS 480
#define QVGA_ROWS 160
#define QVGA_COLS 120
#define TEST_ROWS 100
#define TEST_COLS 10
typedef enum ImageGeneratorResult{
    IMG_GEN_OK,
    IMG_GEN_FAIL
}ImageGeneratorResult;


typedef enum ImageResolutions{
    VGA,
    QVGA,
    TEST_1000
}ImageResolutions;

class ImageGenerator{
    private:
        std::mutex* streamLock;
        std::condition_variable* streamCond;
        std::atomic<bool>* isReadyToStream;
        std::queue<int>* stream;
        // for synchronization between image buffer reader and image producer
        std::mutex frameLock;
        std::condition_variable frameCond;
        std::atomic<bool> isFrameFull;

        int frameRows;
        int frameCols;
        int maxPixelPos;
        // std::vector<std::vector<int>>* frame; // stores a frame. dimensions setup by init
        int** frame;
        int currPixel;

        void generateFrameFromBuffer();
    public:
        ImageGenerator(std::mutex* streamLock , std::condition_variable* streamCond , std::atomic<bool>* isReadyToStream, std::queue<int>* stream);
        ~ImageGenerator();
        
        ImageGeneratorResult initImageGenerator(ImageResolutions imageResolution);

        //starts a thread that reads the IPC buffer filled by communication and stores in Frame buffer
        void readImageFromBuffer();

        void produceTargetImage(); // for now just print...
        //later create virtual function and override in target image type class

    };
#endif