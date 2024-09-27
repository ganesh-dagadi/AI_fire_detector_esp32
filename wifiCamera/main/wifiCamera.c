#include <stdio.h>
#include "StateMachineWifiCam.h"
#include "freertos/task.h"
#include "esp_log.h"

#define STATE_MACHINE_MEMORY 2048

void app_main(void)
{
    // ESP_LOGI("MAIN" , "HELLO WORLD");
    xTaskCreate(stateMachineMain,
                "STATE_MACHINE_TASK",
                STATE_MACHINE_MEMORY,
                NULL,
                19,
                NULL
               );
//     int k = 0;
//     for(int i = 0 ; i < 192000000; i++){
//         k = i+1;
//     }
//     ESP_LOGI("something" , "%d " , k);

}
