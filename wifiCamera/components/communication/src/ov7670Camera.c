#include "camera.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "hal/gpio_types.h"
#include "freertos/task.h"

const char* CAMERA_TAG = "CAMERA";
const char* I2C_TAG = "I2C";
int maxPixelPerFrame;
ImageResolution selectedResolution;
int frameCols;
int frameRows;
esp_err_t err;
int** frame;

//REGISTER ADDRESSES AND OTHER CONFIG
#define I2C_SCL_PIN GPIO_NUM_22
#define I2C_SDA_PIN GPIO_NUM_21

#define XCLK_PIN GPIO_NUM_18
#define GPIO_PIN_DEBUG GPIO_NUM_2
#define VSYNC_PIN GPIO_NUM_13
#define HSYNC_PIN GPIO_NUM_4
#define PCLK_PIN GPIO_NUM_19
#define DATA_7 GPIO_NUM_14
#define DATA_6 GPIO_NUM_27
#define DATA_5 GPIO_NUM_26
#define DATA_4 GPIO_NUM_25
#define DATA_3 GPIO_NUM_33
#define DATA_2 GPIO_NUM_32
#define DATA_1 GPIO_NUM_35
#define DATA_0 GPIO_NUM_34

#define OV7670_SLAVE_WRITE_ADDR 0x42
#define OV7670_SLAVE_READ_ADDR 0x43

//registers
#define REG_COM7 0x12
#define REG_COM11 0x3B
#define REG_CLKRC 0x11
#define REG_TSLB 0x3A
#define REG_COM15 0x40
#define REG_COM10 0x15
#define REG_COM3 0x0C
#define REG_MVFP 0X1E
#define REG_COM14 0x3E
#define SCALING_DCWCTR 0x72
#define SCALING_PCLK_DIV 0x73

void selectDimensions();
int getNextPixelTestFrame();
void initializeOV7670();
void initI2CInterface();
esp_err_t i2cWriteToSlave(uint8_t slaveAddress , uint8_t regsisterAddr , uint8_t data);
void configureXCLK();
void configureGPIO();


void initializeCamera(){
    ESP_LOGI(CAMERA_TAG , "Initializing Camera");
    selectedResolution = RES_QVGA;
    selectDimensions();
    maxPixelPerFrame = frameRows * frameCols;
    frame = (int** )malloc(sizeof(int*) * frameRows);
    for(int i = 0 ; i < frameRows ; i++){
        frame[i] = (int*)malloc(sizeof(int) * frameCols);
    }
    for(int i = 0 ; i < frameRows ; i++){
        for(int j = 0 ; j< frameCols ; j++){
            frame[i][j] = 0;
        }
    }
    if(OV7670){
        initializeOV7670();
    }
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
    if(TEST_CAMERA){
        return getNextPixelTestFrame();
    }else{
        if(OV7670){
            captureFrame();
        }
        return 0;
    }
    // static int packetNo = -1;
    // packetNo++;
    // if(packetNo >= maxPixelPerFrame - 1){
    //     packetNo = -1;
    // }
    // return packetNo;
}

void fillBufferWithPixels(int* buffer , int size){
    int currPacketNo = 0;
    int currentPixel;
    while(currPacketNo < size){
        currentPixel = getNextPixel();
        buffer[currPacketNo] = currentPixel;
        currPacketNo++;
        if(currentPixel == -1){
            if(currPacketNo < 4000) buffer[currPacketNo] = -3;
            ESP_LOGI(CAMERA_TAG , "completed a frame");
            break;
        }
    }
    return;
}

int getNextPixelTestFrame(){
    static int pixelNo = -1;
    static int pixelValue = 0;
    pixelNo++;
    //frame sync flags
    if(pixelNo == 0) return -1;
    if(pixelNo >= maxPixelPerFrame - 1){
        pixelNo = -1;
        return -2;
    }

    int row = pixelNo / frameCols;
    if(row < frameRows/3){
        //red
        pixelValue = 255 << 16;
    }else if (row > frameRows/3 && row < 2 * (frameRows/3)){
        //green
        pixelValue = 255 << 8;
    }else{
        //blue
        pixelValue = 255;
    }
    return pixelValue;
}

void initializeOV7670(){
    //initialize i2c driver
    initI2CInterface();
    configureXCLK();
    configureGPIO();
    //configure the camera as required
    ESP_LOGI(CAMERA_TAG , "Starting I2C camera config");
    esp_err_t res = i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1) , REG_COM7 ,  0b10000000);
    if(res == ESP_OK){
        gpio_set_level(GPIO_PIN_DEBUG , 1);
    }    
    //delay 1ms as suggested in data sheet
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1) , REG_COM11 , 0b1000 | 0b10);
    // i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), REG_CLKRC , 0b10000000);
    i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), REG_TSLB, 0b100); //sequence UYVY

    // //RGB565
    // i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1) , REG_COM7 , 0b100);
    // i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), REG_COM15, 0b11000000 | 0b010000);

    // i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), REG_COM10, 0x02); //VSYNC negative
    // i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), REG_COM3, 0x04);  //DCW enable
    // i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), REG_MVFP, 0x2b);  //mirror flip

    // if(selectedResolution == RES_QVGA){
    //     i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), REG_COM14, 0x1a); //pixel clock divided by 4, manual scaling enable, DCW and PCLK controlled by register
    //     i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), SCALING_DCWCTR, 0x22); //downsample by 4
    //     i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), SCALING_PCLK_DIV, 0xf2); //pixel clock divided by 4
    // }
    i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0xb0, 0x84);// no clue what this is but it's most important for colors
    i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x13, 0xe7); //AWB on
    i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x6f, 0x9f); // Simple AWB


// Set RGB mode (default format)
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x12, 0b00000000);  // COM7: RGB mode

// Clock settings
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x11, 0b00000001);  // CLKRC: Internal clock prescaler

// Set output format to RGB565
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x40, 0b11010000);  // COM15: RGB565, full range

// Enable scaling
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x0C, 0b00000100);  // COM3: Enable scaling

// Set scaling factor for QQVGA
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x3E, 0b00011001);  // COM14: Divide input clock for scaling

// Set the scale for QQVGA resolution
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x72, 0b00100010);  // DSP Scale: QQVGA scale
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x73, 0b11110010);  // DSP Scale: QQVGA scale

// Horizontal window settings
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x17, 0b00010110);  // HSTART
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x18, 0b00000100);  // HSTOP
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x32, 0b10100100);  // HREF

// Vertical window settings
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x19, 0b00000010);  // VSTART
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x1A, 0b01111010);  // VSTOP
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x03, 0b00001010);  // VREF

// Default gamma and color settings
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x67, 0x80);  // Gamma curve
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x6B, 0x0A);  // PLL control
i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x3D, 0xC0);  // Enable gamma, UV adjustment

// i2cWriteToSlave((OV7670_SLAVE_WRITE_ADDR >> 1), 0x71, 0xB5);  // TEST PATTERN

    ESP_LOGI(CAMERA_TAG , "Completed I2C camera config");
}

void initI2CInterface(){
    ESP_LOGI(I2C_TAG , "Initializing I2C as Master");
    i2c_config_t config;
    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = I2C_SDA_PIN;
    config.scl_io_num = I2C_SCL_PIN;
    config.sda_pullup_en = GPIO_PULLUP_DISABLE;
    config.scl_pullup_en = GPIO_PULLUP_DISABLE;
    config.master.clk_speed = 100000;
    config.clk_flags = 0;

    err = i2c_param_config(I2C_NUM_0 , &config);
    if(err != ESP_OK){
        ESP_LOGE(I2C_TAG , "Failed to set param config");
        return;
    }
    err = i2c_driver_install(I2C_NUM_0 , config.mode , 0 , 0 , 0);

    if(err != ESP_OK){
        ESP_LOGE(I2C_TAG , "Failed to set param config");
        return;
    }
    ESP_LOGI(I2C_TAG , "Initialized I2C as Master");
}

esp_err_t i2cWriteToSlave(uint8_t slaveAddress , uint8_t registerAddr , uint8_t data){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    
    err = i2c_master_write_byte(cmd , (slaveAddress << 1) | I2C_MASTER_WRITE , 1);
    i2c_master_write_byte(cmd , registerAddr , 1);
    i2c_master_write_byte(cmd , data , 1);
    i2c_master_stop(cmd);

    err = i2c_master_cmd_begin(I2C_NUM_0 , cmd , 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (err == ESP_OK) {
        ESP_LOGI("TAG", "I2C write successful.");
    } else if (err == ESP_ERR_INVALID_ARG) {
        ESP_LOGE("TAG", "Parameter error (ESP_ERR_INVALID_ARG).");
    } else if (err == ESP_FAIL) {
        ESP_LOGE("TAG", "Sending command error, slave doesn’t ACK the transfer (ESP_FAIL).");
    } else if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGE("TAG", "I2C driver not installed or not in master mode (ESP_ERR_INVALID_STATE).");
    } else if (err == ESP_ERR_TIMEOUT) {
        ESP_LOGE("TAG", "Operation timeout because the bus is busy (ESP_ERR_TIMEOUT).");
    } else {
        ESP_LOGE("TAG", "Unknown error code %d.", err);
    }
    return err;
}

void configureXCLK() {
    ESP_LOGI(CAMERA_TAG , "configuring XCLK");
    // Configure LEDC for generating XCLK signal
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_1_BIT, // 1-bit resolution
        .freq_hz = 10000000, // Frequency for XCLK (10 MHz)
        .clk_cfg = LEDC_USE_APB_CLK,
    };
    
    // Configure the LEDC timer
    ledc_timer_config(&ledc_timer);
    
    // Configure LEDC channel for XCLK
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .gpio_num = XCLK_PIN,
        .duty = 1,
        .hpoint = 0,
    };

    // Configure the LEDC channel
    ledc_channel_config(&ledc_channel);
    
    // Start the LEDC to generate XCLK
    ESP_LOGI(CAMERA_TAG , "started XCLK");
    ledc_fade_func_install(0);
}

void configureGPIO(){

    gpio_set_direction(GPIO_PIN_DEBUG , GPIO_MODE_OUTPUT);
    // SYNC pins
    gpio_set_direction(VSYNC_PIN , GPIO_MODE_INPUT);
    gpio_pullup_en(VSYNC_PIN);
    gpio_set_direction(HSYNC_PIN , GPIO_MODE_INPUT);
    gpio_pullup_en(HSYNC_PIN);
    gpio_set_direction(PCLK_PIN , GPIO_MODE_INPUT);
    gpio_pullup_en(PCLK_PIN);
    //DATA PINS
    gpio_set_direction(DATA_7 , GPIO_MODE_INPUT);
    gpio_set_direction(DATA_6 , GPIO_MODE_INPUT);
    gpio_set_direction(DATA_5 , GPIO_MODE_INPUT);
    gpio_set_direction(DATA_4 , GPIO_MODE_INPUT);
    gpio_set_direction(DATA_3 , GPIO_MODE_INPUT);
    gpio_set_direction(DATA_2 , GPIO_MODE_INPUT);
    gpio_set_direction(DATA_1 , GPIO_MODE_INPUT);
    gpio_set_direction(DATA_0 , GPIO_MODE_INPUT);
}

void captureFrame(){
    // while(1){
    //     ESP_LOGI("DEBUG" , "%d %d %d" , gpio_get_level(VSYNC_PIN) , gpio_get_level(HSYNC_PIN) , gpio_get_level(PCLK_PIN));
    // }
    // while(gpio_get_level(VSYNC_PIN) == 0);
    // while(gpio_get_level(VSYNC_PIN) == 1); // GO high AND low
    // ESP_LOGI("DEBUG" , "VSYNC");
    // for(int i = 0 ; i < frameRows ; i++){
    //     // while(gpio_get_level(HSYNC_PIN) == 1); //wait for low
    //     while(gpio_get_level(HSYNC_PIN) == 0); //wait for high
    //     //ESP_LOGI("DEBUG" , "HSYNC");
    //     for(int j = 0 ; j < frameCols ; j++){
    //         uint8_t pxl1 = 0;
    //         uint8_t pxl2 =0;
    //         while(gpio_get_level(PCLK_PIN) == 0); // wait go high
    //         while(gpio_get_level(PCLK_PIN) == 1); // wait for fall            
    //         pxl1 |= (gpio_get_level(DATA_7) << 7);
    //         pxl1 |= (gpio_get_level(DATA_6) << 6);
    //         pxl1 |= (gpio_get_level(DATA_5) << 5);
    //         pxl1 |= (gpio_get_level(DATA_4) << 4);
    //         pxl1 |= (gpio_get_level(DATA_3) << 3);
    //         pxl1 |= (gpio_get_level(DATA_2) << 2);
    //         pxl1 |= (gpio_get_level(DATA_1) << 1);
    //         pxl1 |= (gpio_get_level(DATA_0) << 0);

    //         while(gpio_get_level(PCLK_PIN) == 0); // wait go high
    //         while(gpio_get_level(PCLK_PIN) == 1); // wait for fall            
    //         pxl2 |= (gpio_get_level(DATA_7) << 7);
    //         pxl2 |= (gpio_get_level(DATA_6) << 6);
    //         pxl2 |= (gpio_get_level(DATA_5) << 5);
    //         pxl2 |= (gpio_get_level(DATA_4) << 4);
    //         pxl2 |= (gpio_get_level(DATA_3) << 3);
    //         pxl2 |= (gpio_get_level(DATA_2) << 2);
    //         pxl2 |= (gpio_get_level(DATA_1) << 1);
    //         pxl2 |= (gpio_get_level(DATA_0) << 0);
            
    //         uint8_t R5 = pxl1 & 0b11111000;
    //         uint8_t G6 = pxl1 & 0b00000111;
    //         G6 <<= 3;
    //         G6 = G6 | (pxl2 >> 5);
    //         uint8_t B5 = pxl2 & 0b00011111;

    //         uint8_t R8 = ( R5 * 527 + 23 ) >> 6;
    //         uint8_t G8 = ( G6 * 259 + 33 ) >> 6;
    //         uint8_t B8 = ( B5 * 527 + 23 ) >> 6;

    //         frame[i][j] |= R8 << 16;
    //         frame[i][j] |= G8 << 8;
    //         frame[i][j] |= B8;
    //     }

    //     //while(gpio_get_level(HSYNC_PIN)); // wait for low

    //  }

    while (gpio_get_level(VSYNC_PIN) == 0);
while (gpio_get_level(VSYNC_PIN) == 1);
ESP_LOGI("DEBUG", "VSYNC");

for (int i = 0; i < frameRows; i++) {
    while (gpio_get_level(HSYNC_PIN) == 0); // Wait for HSYNC to go high

    for (int j = 0; j < frameCols; j++) {
        uint8_t pxl1 = 0;
        uint8_t pxl2 = 0;

        while (gpio_get_level(PCLK_PIN) == 0); // Wait for PCLK high
        while (gpio_get_level(PCLK_PIN) == 1); // Wait for PCLK low

        // Read first byte
        pxl1 |= (gpio_get_level(DATA_7) << 7);
        pxl1 |= (gpio_get_level(DATA_6) << 6);
        pxl1 |= (gpio_get_level(DATA_5) << 5);
        pxl1 |= (gpio_get_level(DATA_4) << 4);
        pxl1 |= (gpio_get_level(DATA_3) << 3);
        pxl1 |= (gpio_get_level(DATA_2) << 2);
        pxl1 |= (gpio_get_level(DATA_1) << 1);
        pxl1 |= (gpio_get_level(DATA_0) << 0);

        while (gpio_get_level(PCLK_PIN) == 0); // Wait for PCLK high
        while (gpio_get_level(PCLK_PIN) == 1); // Wait for PCLK low

        // Read second byte
        pxl2 |= (gpio_get_level(DATA_7) << 7);
        pxl2 |= (gpio_get_level(DATA_6) << 6);
        pxl2 |= (gpio_get_level(DATA_5) << 5);
        pxl2 |= (gpio_get_level(DATA_4) << 4);
        pxl2 |= (gpio_get_level(DATA_3) << 3);
        pxl2 |= (gpio_get_level(DATA_2) << 2);
        pxl2 |= (gpio_get_level(DATA_1) << 1);
        pxl2 |= (gpio_get_level(DATA_0) << 0);

        // Convert RGB565 to RGB888
        uint8_t R5 = pxl1 & 0b11111000;
        uint8_t G6 = ((pxl1 & 0b00000111) << 3) | (pxl2 >> 5);
        uint8_t B5 = pxl2 & 0b00011111;

        uint8_t R8 = (R5 * 527 + 23) >> 6;
        uint8_t G8 = (G6 * 259 + 33) >> 6;
        uint8_t B8 = (B5 * 527 + 23) >> 6;

        // Store in frame array
        frame[i][j] = (R8 << 16) | (G8 << 8) | B8;
    }
}

}
