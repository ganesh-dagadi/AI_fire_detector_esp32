#ifndef WIFI
    #define WIFI
    #include "esp_wifi.h"
    #include "esp_event.h"
    #include "esp_log.h"
    #include "esp_err.h"

    #define WIFI_CONNECT_READY 0x00000001
    #define WIFI_WILLING_TO_CONNECT 0x00000002
    enum WifiInitState{
        WIFI_UNINITIALIZED,
        WIFI_INITIALZING,
        WIFI_INITIALIZED,
        WIFI_INITIALZATION_FAIL
    };

    enum WifiScanState{
        IDLE,
        SCANNING,
        SCAN_COMPLETED
    };

    enum WifiConnectionState{
        WIFI_DISCONNETED,
        WIFI_CONNECTING,
        WIFI_RECONNECTING,
        WIFI_CONNECTED,
        WIFI_CONNECTION_FAIL
    };

    enum WifiResult{
        WIFI_SUCCESS,
        WIFI_FAIL
    };

    typedef enum WifiEventId{
        WIFI_BEGIN_CONNECTION
    } WifiEventId;

    extern enum WifiInitState wifiInitState;
    extern enum WifiScanState wifiScanState;
    extern enum WifiConnectionState wifiConnectionState;
/*
    bit 0 : wifiReadToConnect - every thing is initialized and we are ready to connect - default false
    bit 1 : wifi willing to connect - false if permanent disconnect - default true
*/
    extern int32_t wifiFlags;
    
    void wifiTaskMain(void* params);



#endif