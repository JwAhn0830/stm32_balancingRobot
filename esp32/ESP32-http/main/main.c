#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h" //for delay,mutexs,semphrs rtos operations
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_wifi.h"
#include "nvs_flash.h" //non volatile storage
#include "esp_event.h" //for wifi event
#include "esp_http_client.h"
//#include "lwip/sys.h" //system applications for light weight ip apps
//#include "lwip/err.h" //light weight ip packets error handling
//#include "esp_system.h" //esp_init funtions esp_err_t 

#define UART_NUM UART_NUM_2
#define TX_PIN 16 // rx from stm32
#define RX_PIN 17 // tx from stm32
#define LED 25
#define ON 0
#define OFF 1
#define SSID "" 
#define PASSWORD "" //
#define URL "http://localhost:3000"

 /* test data 
      {"Gyro" : "100", "Bat" : "7.4"} 
      {"Gyro" : "50", "Bat" : "3.7"} 
      {"Gyro" : "10", "Bat" : "1"} 
      */
static bool wifi_connected = false;
char data[128] = "{\"Gyro\":\"0\",\"Bat\":\"0\"}"; //default
void send_uart(char* message) {
    uart_write_bytes(UART_NUM, message, strlen(message));
}

esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt) {
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        //printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void send_post() {
    esp_http_client_config_t http_config = {
        .url = URL,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler
    };

    esp_http_client_handle_t client = esp_http_client_init(&http_config);
    esp_http_client_set_post_field(client, data, strlen(data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}
void init_uart() {
    uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(UART_NUM, 1024, 1024, 0, NULL, 0);
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_param_config(UART_NUM, &uart_config);
    

    printf("UART CONFIG DONE\r\n");
}


void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data) {
  if (event_id == WIFI_EVENT_STA_START) {
    printf("Connecting WiFi...\r\n");
  }
  else if (event_id == WIFI_EVENT_STA_CONNECTED) {
    printf("WiFi CONNECTED\r\n");
    gpio_set_level(LED, ON);
  }
  else if (event_id == WIFI_EVENT_STA_DISCONNECTED){
    gpio_set_level(LED, OFF);
    printf("WiFi DISCONNECTED Retrying to Connect...\r\n");
    wifi_connected = false;
    esp_wifi_connect(); 
  }
  else if(event_id == IP_EVENT_STA_GOT_IP) {
    printf("GET IP address.\r\n");
    wifi_connected = true;

  }
}
void init_wifi(){
  esp_netif_init(); //init wifi interface
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&wifi_init_config);

  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

  wifi_config_t wifi_config = {
    .sta = {
      .ssid = SSID,
      .password = PASSWORD,
    },
  };

  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
  esp_wifi_start(); // start WiFi driver 
  esp_wifi_set_mode(WIFI_MODE_STA); // set wifi mode as STA

  /*
  once this called, the wifi driver start to scan and coneect wifi
  when connection is successful, WIFI_EVENT_STA_CONNECTED is generated
  */ 
  esp_wifi_connect(); 
}

void http_task(void *pvParameters) {
    while(1) {
        if(wifi_connected) {
            send_post();
        } 
    }
}
void uart_task() {
  uint8_t buffer[128];
    while(1) {
        int length = uart_read_bytes(UART_NUM, buffer, 1024, 20 / portTICK_PERIOD_MS);
        if (length > 0) {
            buffer[length] = '\0';
            strncpy(data, (char*)buffer, sizeof(data) - 1);
            printf("Received data: %s\n", data);
         }

    }
}
void app_main() {
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, OFF);
    nvs_flash_init();
    init_uart();
    vTaskDelay(pdMS_TO_TICKS(2000));
    init_wifi();
    vTaskDelay(pdMS_TO_TICKS(2000));
    xTaskCreate(http_task, "http_task", 4096, NULL, 5, NULL);  
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 10, NULL); 
}