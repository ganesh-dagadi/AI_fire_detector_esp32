idf_component_register(SRCS "src/stateMachineWifiCam.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_event nvs_flash wifi netif communication
                       )
