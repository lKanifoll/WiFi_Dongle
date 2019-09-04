#include "main.h"


const char *TAG_UART = "UART";


bool check_crc8(char *pcBlock, int len)
{
	int i;
	char pec;
	pec = 0;
	for (i = 0; i < len; i++)
	{
		pec ^= pcBlock[i];
		pec = pec ^ (pec << 1) ^ (pec << 2) ^ ((pec & 128) ? 9 : 0) ^ ((pec & 64) ? 7 : 0);
	}
	if (pcBlock[len] == pec)
	{
		return true;
	}
	return false;
}


void uart_event_task(void *pvParameters)
{
	uart_event_t event;
	uint8_t *dtmp = (uint8_t *) malloc(RD_BUF_SIZE);

	for (;;) {
		// Waiting for UART event.
		if(xQueueReceive(uart0_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
			bzero(dtmp, RD_BUF_SIZE);

			uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
			ESP_LOGI(TAG_UART, "[DATA EVT]: %s", dtmp);

			if ((dtmp[0] == 0xAB) && (dtmp[1] == 0xCD))
			{
				if (check_crc8((char*)dtmp, 5))
				{
					if (dtmp[4] == 0xA1)
					{
						set_wifi_sta();
					}
					else if (dtmp[4] == 0xB1)
					{
						set_wifi_ap();
					}
					dtmp[2] = 0x01;
					dtmp[3] = 0x01;
					uart_write_bytes(EX_UART_NUM, (const char *) dtmp, 4);
				}
				else
				{
					dtmp[2] = 0x01;
					dtmp[3] = 0x00;
					uart_write_bytes(EX_UART_NUM, (const char *) dtmp, 4);
				}
			}
			else
			{
				printf("Response: ");
				for (uint8_t i = 0; i < event.size; i++)
				{
					printf("%02X ", dtmp[i]);
				}
				printf("\n");
				int err = send(sock, dtmp, event.size, 0);
				if (err < 0) {
					ESP_LOGE(TAG_UART, "Error occured during sending: errno %d", errno);
					//break;
				}
			}

			//uart_write_bytes(EX_UART_NUM, (const char *) dtmp, event.size);
//			int err = send(sock, dtmp, event.size, 0);
//			if (err < 0) {
//				ESP_LOGE(TAG_UART, "Error occured during sending: errno %d", errno);
//				//break;
//			}
		}
	}
	ESP_LOGE(TAG_UART, "UART DELETE TASK");
	free(dtmp);
	dtmp = NULL;
	vTaskDelete(NULL);
}

void app_main()
{
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	
	nvs_open("storage", NVS_READWRITE, &storage_handle);
	
	nvs_get_u8(storage_handle, "first_start", &first_start);
	if (!first_start)
	{
		/*strcpy((char*)&HOST_ADDR[0], "dongle.rusklimat.ru");
		nvs_set_str(storage_handle, "HOST_ADDR", HOST_ADDR);
		
		strcpy((char*)&HOST_PORT[0], "10001");
		nvs_set_str(storage_handle, "HOST_PORT", HOST_PORT);*/
		
		strcpy((char*)&HOST_ADDR[0], "192.168.0.8");
		nvs_set_str(storage_handle, "HOST_ADDR", HOST_ADDR);
		
		strcpy((char*)&HOST_PORT[0], "3333");
		nvs_set_str(storage_handle, "HOST_PORT", HOST_PORT);
		
		first_start = true;
		nvs_set_u8(storage_handle, "first_start", first_start);
	}
	
	size_t host_len = 0;
	nvs_get_str(storage_handle, "HOST_ADDR", NULL, &host_len);
	nvs_get_str(storage_handle, "HOST_ADDR", (char *)&HOST_ADDR[0], &host_len);
	
	size_t port_len = 0;
	nvs_get_str(storage_handle, "HOST_PORT", NULL, &port_len);
	nvs_get_str(storage_handle, "HOST_PORT", (char *)&HOST_PORT[0], &port_len);
	
	size_t ssid_len = 0;
	nvs_get_str(storage_handle, "SSID", NULL, &ssid_len);
	nvs_get_str(storage_handle, "SSID", (char *)&SSID[0], &ssid_len);
	
	size_t pass_len = 0;
	nvs_get_str(storage_handle, "PASS", NULL, &pass_len);
	nvs_get_str(storage_handle, "PASS", (char *)&PASS[0], &pass_len);
	
	nvs_commit(storage_handle);
	nvs_close(storage_handle);
//	
//	printf("ssid_len: %d\n", ssid_len);
//	printf("pass_len: %d\n", pass_len);
//	
//	
//	
	uart_config_t uart_config = {
		.baud_rate = 9600,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(EX_UART_NUM, &uart_config);
	uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 3, &uart0_queue);
	
	//xTaskCreate(tcp_client_task, "tcp_client",		4096, NULL, 5, NULL);
	xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 5, &uart_handle);
	
	get_mac_buf();
	initialise_wifi();
	//wait_for_ip();

}
