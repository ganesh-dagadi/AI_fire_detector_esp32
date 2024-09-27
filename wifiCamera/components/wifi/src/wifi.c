#include "wifi.h"
#include "freertos/task.h"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

const char* WIFI_TAG = "WIFI Task";
int32_t wifiFlag = 0;
static short wifiReconnectionAttempts = 0;

enum WifiResult initializeWifiWithSTAMode();
void executeStateMachineWifi();
void handleConnectionFail();
enum WifiResult connectToWifiAP();
enum WifiResult disconnectWifi();

void wifiTaskMain(void* params){
    while(1){
        executeStateMachineWifi();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void executeStateMachineWifi(){
    if(wifiInitState == WIFI_UNINITIALIZED){
        initializeWifiWithSTAMode();
    }

    if(wifiFlags & WIFI_CONNECT_READY && (wifiConnectionState == WIFI_DISCONNETED || wifiConnectionState == WIFI_RECONNECTING)){
        if(wifiConnectionState == WIFI_DISCONNETED){
            ESP_LOGI(WIFI_TAG , "Connecting to Wifi");
        }else{
            ESP_LOGW(WIFI_TAG , "Reconnecting to Wifi");
        }
        connectToWifiAP();
    }

    if(wifiConnectionState == WIFI_CONNECTED || wifiConnectionState == WIFI_RECONNECTING){
        if(!(wifiFlags & WIFI_CONNECT_READY)){
            ESP_LOGI(WIFI_TAG , "Disconnecting is being called");
            disconnectWifi();
        }
    }

}

void event_handler_wifi_t(void* args , esp_event_base_t eventBase, int32_t event_id, void* eventData){
    ESP_LOGI(WIFI_TAG , "Recieved an event => base : %s ; id : %ld" , eventBase , event_id);
    if(event_id == (int32_t)WIFI_EVENT_STA_START){
        wifiInitState = WIFI_INITIALIZED;
        ESP_LOGI(WIFI_TAG , "Wifi started from handler");
    }

    if(event_id == (int32_t) WIFI_EVENT_STA_CONNECTED){
        wifiConnectionState = WIFI_CONNECTED;
        wifiReconnectionAttempts = 0;
        ESP_LOGI(WIFI_TAG ,"Wifi Connected \n");
    }

    if(event_id == (int32_t) WIFI_EVENT_STA_DISCONNECTED){
        wifiConnectionState = WIFI_DISCONNETED;
        ESP_LOGW(WIFI_TAG ,"Wifi Disconnected \n");
        handleConnectionFail();
    }
}

enum WifiResult initializeWifiWithSTAMode(){
    esp_err_t err;
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_LOGI(WIFI_TAG , "Wifi Initialzing in STA Mode");
    wifiInitState = WIFI_INITIALZING;

    ESP_LOGI(WIFI_TAG , "Registering wifi to default looper");
    err = esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler_wifi_t,
        NULL,
        NULL
    );
    if(err != ESP_OK){
        ESP_LOGE(WIFI_TAG , "Unable to register Wifi to default looper : %d" , err);
        wifiInitState = WIFI_INITIALZATION_FAIL;
        return WIFI_FAIL;
    }

    err = esp_wifi_init(&wifi_init_config);
    if(err != ESP_OK){
        ESP_LOGE(WIFI_TAG , "Unable to init ESP Wifi . code : %d" , err);
        wifiInitState = WIFI_INITIALZATION_FAIL;
        return WIFI_FAIL;
    }

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if(err != ESP_OK){
        ESP_LOGE(WIFI_TAG , "Unable to set Wifi to station mode. code : %d" , err);
        wifiInitState = WIFI_INITIALZATION_FAIL;
        return WIFI_FAIL;
    }
    ESP_LOGI(WIFI_TAG , "Wifi Initialzing done in STA mode");
    ESP_LOGI(WIFI_TAG , "Starting Wifi");
    err = esp_wifi_start();
    if(err != ESP_OK){
        ESP_LOGE(WIFI_TAG , "Unable to start WIFI. code : %d" , err);
        wifiInitState = WIFI_INITIALZATION_FAIL;
        return WIFI_FAIL;
    }
    return WIFI_SUCCESS;
    
}

enum WifiResult connectToWifiAP(){
    if(wifiConnectionState == WIFI_CONNECTING){
        return WIFI_FAIL;
    }
    wifiConnectionState = WIFI_CONNECTING;
    esp_err_t err;
    wifi_config_t config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD
        }
    };
    esp_wifi_set_config(WIFI_IF_STA , &config);
    ESP_LOGI(WIFI_TAG , "Calling wifi connect \n");
    err = esp_wifi_connect();
    if(err != ESP_OK){
        ESP_LOGE(WIFI_TAG , "WIFI failed to connect \n");
        handleConnectionFail();
        return WIFI_FAIL;
    }  
    return WIFI_SUCCESS;  
}

enum WifiResult disconnectWifi(){
    esp_err_t err;
    ESP_LOGI(WIFI_TAG , "Disconnecting from Wifi");
    err = esp_wifi_disconnect();
    if(err != ESP_OK){
        ESP_LOGE(WIFI_TAG , "WIFI failed to disconnect \n");
        return WIFI_FAIL;
    }  
    return WIFI_SUCCESS;  
}

void handleConnectionFail(){
    if(wifiReconnectionAttempts < 3 && (wifiFlags & WIFI_WILLING_TO_CONNECT)){
        ESP_LOGW(WIFI_TAG , "Connection failed. Attempting reconnect in 5s");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        wifiConnectionState = WIFI_RECONNECTING;
        wifiReconnectionAttempts++;
        return;
    }
    if(wifiFlags & WIFI_WILLING_TO_CONNECT){
        ESP_LOGE(WIFI_TAG , "Reconnect attemp failed 3 times. Escalating to WIFI CONNECTION FAIL");
        wifiConnectionState = WIFI_CONNECTION_FAIL;
    }
}