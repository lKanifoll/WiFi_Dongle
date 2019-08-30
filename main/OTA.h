#ifndef _OTA_H_
#define _OTA_H_


#define EXAMPLE_SERVER_IP   "kochnev.asuscomm.com"
#define EXAMPLE_SERVER_PORT "80"
#define EXAMPLE_FILENAME "/WiFi_Dongle.ota.bin"
#define BUFFSIZE 1500
#define TEXT_BUFFSIZE 1024

typedef enum esp_ota_firm_state {
	ESP_OTA_INIT    = 0,
	ESP_OTA_PREPARE,
	ESP_OTA_START,
	ESP_OTA_RECVED,
	ESP_OTA_FINISH,
} esp_ota_firm_state_t;

typedef struct esp_ota_firm {
	uint8_t             ota_num;
	uint8_t             update_ota_num;

	esp_ota_firm_state_t    state;

	size_t              content_len;

	size_t              read_bytes;
	size_t              write_bytes;

	size_t              ota_size;
	size_t              ota_offset;

	const char          *buf;
	size_t              bytes;
} esp_ota_firm_t;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */

void ota_example_task(void *pvParameter);

#endif