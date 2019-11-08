#ifndef _WIFI_TCP_H_
#define _WIFI_TCP_H_
extern const int IPV4_GOTIP_BIT;

char MAC_esp[16];
uint8_t temp_mac[6];
uint8_t wifi_active_flag;
uint8_t disconnect;

uint8_t delete_tcp_task;

wifi_ap_record_t ap_info;

xTaskHandle tcp_client_handle;
xTaskHandle tcp_server_handle;

xTaskHandle smartconfig_handle;

void smartconfig_example_task(void * parm);
void tcp_server_task(void *pvParameters);
void initialise_wifi();
void set_wifi_sta();
void set_wifi_ap();
void wait_for_ip();
void tcp_client_task(void *pvParameters);
void get_mac_buf();
void wifi_rssi();

#endif
