#include <stdio.h>
#include "StateMachineWifiCam.h"
#include "freertos/task.h"
// #include "esp_log.h"

#define STATE_MACHINE_MEMORY 2048

void app_main(void)
{
    // ESP_LOGI("MAIN" , "HELLO WORLD");
    xTaskCreate(stateMachineMain,
                "STATE_MACHINE_TASK",
                STATE_MACHINE_MEMORY,
                NULL,
                6,
                NULL
               );
}
