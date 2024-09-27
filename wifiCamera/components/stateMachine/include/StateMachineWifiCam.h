#ifndef STATE_MACHINE_WIFI_CAMERA
    #define STATE_MACHINE_WIFI_CAMERA
    #include "esp_event.h"

    enum InitStateGlobal{
        STATE_UNINITIALIZED,
        STATE_INITIALZING,
        STATE_INITIALIZED,
        STATE_INITIALZATION_FAIL
    };
    
    typedef enum StateConnectionState{
        STATE_DISCONNECTED,
        STATE_CONNECTED
    }StateConnectionState;

    enum StateMachineResult{
        STATE_OK,
        STATE_FAIL
    };

    extern enum InitStateGlobal stateMachineState;
    extern StateConnectionState stateConnectionState;

    void stateMachineMain(void* params);
    
#endif
