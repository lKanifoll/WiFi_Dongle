#ifndef _WIFI_TCP_H_
#define _WIFI_TCP_H_

uint8_t wifi_active_flag;

void smartconfig_example_task(void * parm);
void tcp_server_task(void *pvParameters);
void initialise_wifi();
void set_wifi_sta();
void set_wifi_ap();
void wait_for_ip();
void tcp_client_task(void *pvParameters);

#endif