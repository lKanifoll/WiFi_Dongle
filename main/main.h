#ifndef _MAIN_H_
#define _MAIN_H_

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "driver/uart.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "esp_ota_ops.h"
#include "smartconfig_ack.h"
#include "defines.h"
#include "wifi_tcp.h"
#include "esp_ping.h"
#include "OTA.h"
//#include "ping/ping.h"
//#include "esp_ping.h"


#define SC 1
#define AP 2
nvs_handle storage_handle;
char SSID[32];
char PASS[32];
char HOST_ADDR[32];
char HOST_PORT[6];
uint8_t first_link;
uint8_t first_start;
uint8_t wifi_status;
uint8_t connect_type;
EventGroupHandle_t wifi_event_group;
TimerHandle_t xTimerUpdateWifi;
void TimerUpdate_Callback(TimerHandle_t xTimer);
QueueHandle_t uart0_queue;
bool check_crc8(char *pcBlock, int len);
bool crc8_add(char *sBlock, int len);
int sock;

int err_socket_access;

xTaskHandle uart_handle;

#endif