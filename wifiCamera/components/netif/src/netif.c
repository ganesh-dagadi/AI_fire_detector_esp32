#include "netif.h"
#include "esp_wifi.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_netif.h"
const char* NETIF_TAG = "Netif Task";
NetifResult initializeNetifForWifi();
void executeStateMachineNetif();

void netifTaskMain(void* params){
    while(1){
        executeStateMachineNetif();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void event_handler_netif_t(void* args , esp_event_base_t eventBase, int32_t event_id, void* eventData){
    ESP_LOGI(NETIF_TAG , "Recieved an event => base : %s ; id : %ld" , eventBase , event_id);
    switch(event_id){
        case ((int32_t)IP_EVENT_STA_GOT_IP):
        netifConnectionState = NETIF_HASIP;
        ESP_LOGI(NETIF_TAG , "IP address recieved");
        break;
        case ((int32_t)IP_EVENT_STA_LOST_IP):
        netifConnectionState = NETIF_LOSTIP;
        ESP_LOGW(NETIF_TAG , "IP address lost");
        break;
    }
    
}

void executeStateMachineNetif(){
    if(netifInitState == NETIF_UNINITIALIZED){
        initializeNetifForWifi();
    }
}

NetifResult initializeNetifForWifi(){
    esp_err_t err;
    netifInitState = NETIF_INITIALZING;
    ESP_LOGI(NETIF_TAG , "Initializing Netif");
    err = esp_netif_init();
    if(err != ESP_OK){
        ESP_LOGE(NETIF_TAG ,"Failed to initialize netif  \n");
        netifInitState = NETIF_INITIALZATION_FAIL;
        return NETIF_FAIL;
    }
    esp_netif_create_default_wifi_sta();

    ESP_LOGI(NETIF_TAG , "Registering to default event Looper");
    err = esp_event_handler_instance_register(
        IP_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler_netif_t,
        NULL,
        NULL
    );
    if(err != ESP_OK){
        ESP_LOGE(NETIF_TAG ,"Failed to register handler to default looper \n");
        netifInitState = NETIF_INITIALZATION_FAIL;
        return NETIF_FAIL;
    }
    ESP_LOGI(NETIF_TAG , "NETIF has been initialized");
    netifInitState = NETIF_INITIALIZED;
    return NETIF_OK;
}