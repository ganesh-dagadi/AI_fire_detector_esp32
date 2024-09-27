#ifndef NETIF
    #define NETIF
    #include "esp_event.h"

    typedef enum NetifInitState{
        NETIF_UNINITIALIZED,
        NETIF_INITIALZING,
        NETIF_INITIALIZED,
        NETIF_INITIALZATION_FAIL
    }NetifInitState;

    typedef enum NetifConnectionState{
        NETIF_HASIP,
        NETIF_HASNOIP,
        NETIF_LOSTIP
    }NetifConnectionState;

    typedef enum NetifResult{
        NETIF_OK,
        NETIF_FAIL,
    }NetifResult;

    extern enum NetifInitState netifInitState;
    extern NetifConnectionState netifConnectionState;
    
    void netifTaskMain(void* params);
    
#endif