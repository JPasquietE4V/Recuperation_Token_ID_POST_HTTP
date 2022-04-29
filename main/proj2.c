#include <stdio.h>
#include <esp_event_base.h>
#include <esp_wifi.h>
#include <esp_http_server.h>
#include <esp_http_client.h>

#include <freertos/FreeRTOS.h> 
#include <freertos/task.h> 
#include <freertos/timers.h>
#include <freertos/event_groups.h>
#include <esp_log.h> 
#include <esp_netif.h> 

#include <nvs_flash.h> 


static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) 
{ 
    switch (event_id) 
    { 
    case WIFI_EVENT_STA_START: 
        printf("WiFi connecting ... \n"); 
        break; 

    case WIFI_EVENT_STA_CONNECTED: 
        printf("WiFi connected ... \n");
        break;

    case WIFI_EVENT_STA_DISCONNECTED: 
        printf("WiFi lost connection ... \n"); 
        break; 

    case IP_EVENT_STA_GOT_IP: 
        printf("WiFi got IP ... \n\n"); 
        break; 

    default: 
        break; 
    } 
}


esp_err_t client_event_handler(esp_http_client_event_handle_t evt) 
{  
    switch (evt->event_id) 
    { 
    case HTTP_EVENT_ON_DATA: 
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data); 
        break; 

    default: 
        break; 
    }
    return ESP_OK; 
}


void get_rest_function() 
{ 
    esp_http_client_config_t client_configuration = { 
        .url = "http://worldclockapi.com/api/json/utc/now", 
        .event_handler = client_event_handler }; 
    esp_http_client_handle_t client = esp_http_client_init(&client_configuration); 
    esp_http_client_perform(client); 
    esp_http_client_cleanup(client); 
} 



static void post_rest_function() 
{ 
    esp_http_client_config_t config_post = { 
        .url = "https://datacollectandtransformapi.azurewebsites.net//Login/authenticate",//"https://firestore.googleapis.com/v1/projects/sims-iot-devices/databases/(default)/documents/my_collection", 
        .method = HTTP_METHOD_POST, 
        //.cert_pem = (const char *)certificate_pem_start, 
        .event_handler = client_event_handler}; //client_event_get_handler

    esp_http_client_handle_t client = esp_http_client_init(&config_post); 
    
    char *post_data = "{\"userName\":\"api\",\"password\":\"api\"}";//"{\"fields\":{\"Memory\":{\"doubleValue\":\"2\"},\"Name\":{\"stringValue\":\"Old ESP32\"}}";
    esp_http_client_set_post_field(client, post_data, strlen(post_data)); 
    esp_http_client_set_header(client, "Content-Type", "application/json"); 
    printf("POST REQUEST \n\n\n");
    esp_http_client_perform(client); 
    esp_http_client_cleanup(client); 
} 



void app_main(void)
{ 
    nvs_flash_init(); // Non-volatile storage 
    //1 - Wi-Fi/LwIP Init Phase
    esp_netif_init(); // TCP/IP initiation s1.1
    esp_event_loop_create_default(); // event loop s1.2 
    esp_netif_create_default_wifi_sta(); // WiFi station s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT(); 
    esp_wifi_init(&wifi_initiation); // 

    // 2 - Wi-Fi Configuration Phase 
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL); 
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL); 
    wifi_config_t wifi_configuration = { 
        .sta = { 
            .ssid = "E4V-Bordeaux", 
            .password = "E3FE63E566D56F3E3CC7943A9A"}}; 
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration); 

    // 3 - Wi-Fi Start Phase 
    esp_wifi_start(); 

    // 4- Wi-Fi Connect Phase 
    esp_wifi_connect(); 

    // REST GET data 
    vTaskDelay(5000 / portTICK_PERIOD_MS); 
    //get_rest_function(); 
    post_rest_function();
} 
