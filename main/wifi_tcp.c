#include "main.h"
const int ESPTOUCH_DONE_BIT = BIT1;
const int IPV4_GOTIP_BIT = BIT0;
const char *TAG_WIFI = "WIFI";
const char *TAG_SC = "SC";
const char *TAG_TCP = "TCP";

esp_err_t event_handler(void *ctx, system_event_t *event)
{
	/* For accessing reason codes in case of disconnection */
	//system_event_info_t *info = &event->event_info;

	switch(event->event_id) {
	case SYSTEM_EVENT_STA_CONNECTED:
		wifi_active_flag = true;

		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
		wifi_active_flag = true;
		wifi_status = 1;
		if (disconnect)
		{
			vTaskResume(tcp_client_handle);	
			disconnect = false;
		}
		else
		{
			//delete_tcp_task = false;
			xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, &tcp_client_handle);
		}
		break;
		
	case SYSTEM_EVENT_AP_STACONNECTED:
		ESP_LOGI(TAG_WIFI, "station:"MACSTR" join, AID=%d", MAC2STR(event->event_info.sta_connected.mac), event->event_info.sta_connected.aid);
		wifi_status = 4;
		wifi_active_flag = true;		
		break;
		
	case SYSTEM_EVENT_AP_START:
		ESP_LOGI(TAG_WIFI, "Access point is active: RK_Dongle");
		wifi_active_flag = true;
		
		if (disconnect)
		{
			vTaskResume(tcp_server_handle);	
			disconnect = false;
		}
		else
		{
			//delete_tcp_task = false;
			xTaskCreate(tcp_server_task, "tcp_client", 4096, NULL, 5, &tcp_server_handle);
		}
		break;
		
    case SYSTEM_EVENT_AP_STADISCONNECTED:
		esp_restart();
		break;

		
	case SYSTEM_EVENT_STA_DISCONNECTED:
		//ESP_LOGE(TAG_WIFI, "Disconnect reason : %d", info->disconnected.reason);
		ESP_LOGW(TAG_WIFI, "RECONNECT...%d", wifi_active_flag);
		if (wifi_active_flag)
		{
			vTaskSuspend(tcp_client_handle);
			ESP_LOGW(TAG_WIFI, "SUSPEND TCP TASK...");
			//delete_tcp_task = true;
			wifi_active_flag = false;
			disconnect = true;
			wifi_status = false;
		}
		ESP_LOGW(TAG_WIFI, "wifi_status: %d", wifi_status);
		if (wifi_status == 3)
		{
			xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
			wifi_status = false;
			//ESP_ERROR_CHECK(esp_wifi_disconnect());
			ESP_ERROR_CHECK(esp_wifi_stop());
		}	
			
		esp_wifi_connect();
		xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
		break;

	default:
		break;
	}
	return ESP_OK;
}


void TimerUpdate_Callback(TimerHandle_t xTimer)
{
	esp_restart();
}

void initialise_wifi()
{
	tcpip_adapter_init();

	wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	nvs_open("storage", NVS_READWRITE, &storage_handle);
	nvs_get_u8(storage_handle, "first_link", &first_link);
	nvs_commit(storage_handle);
	nvs_close(storage_handle);
	//printf("first_link %d\n", first_link);
	
	if(connect_type)
	{
		if (connect_type == SC)
		{
			set_wifi_sta();
			
		}
		else if (connect_type == AP)
		{
			set_wifi_ap();
		}
		
		//xTimerReset(xTimerUpdateWifi, 10);
		nvs_open("storage", NVS_READWRITE, &storage_handle);
		connect_type = pdFALSE;
		nvs_set_u8(storage_handle, "connect_type", connect_type);
		nvs_commit(storage_handle);
		nvs_close(storage_handle);
	}
	else
	{
		//if (first_link)
		{
			wifi_config_t wifi_config =
			{
				.sta = { .ssid = "IZTT_2.4", .password = "iztt110907", },
			};
			printf("SSID: %s\n", SSID);
			printf("PASS: %s\n", PASS);
		
			//strcpy((char*) &wifi_config.sta.ssid[0], (const char*) &SSID[0]);
			//strcpy((char*) &wifi_config.sta.password[0], (const char*) &PASS[0]);

			ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
			ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

			ESP_ERROR_CHECK(esp_wifi_start());
			ESP_ERROR_CHECK(esp_wifi_connect());
		}
	}
}




void set_wifi_sta()
{
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());
	xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 5, &smartconfig_handle);
	
}


void set_wifi_ap()
{
	 wifi_config_t wifi_config1 = {
		.ap = {
		.ssid = "Yin-Yang",
		.ssid_len = 9,
		.password = "",
		.channel = 6,
		.authmode = WIFI_AUTH_OPEN,
		.ssid_hidden = 0,
		.max_connection = 1,
		.beacon_interval = 100
		},
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config1));
	ESP_ERROR_CHECK(esp_wifi_start());
	
}

void get_mac_buf()
{
	while (ESP_OK != esp_wifi_get_mac(ESP_IF_WIFI_STA, temp_mac)) ;
	sprintf(&MAC_esp[0], "+ok=%02X%02X%02X%02X%02X%02X", temp_mac[0], temp_mac[1], temp_mac[2], temp_mac[3], temp_mac[4], temp_mac[5]);
	printf("MAC: %s\n", MAC_esp);
}


void wait_for_ip()
{
	uint32_t bits = IPV4_GOTIP_BIT;
	ESP_LOGI(TAG_WIFI, "Waiting for AP connection...");
	xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
	ESP_LOGI(TAG_WIFI, "Connected to AP");
	xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, &tcp_client_handle);
}

void wifi_rssi()
{
	esp_wifi_sta_get_ap_info(&ap_info);
//	if (ap_info.rssi > (-60))  							  {WIFI_Level = 0x04; }
//	if ((ap_info.rssi <= (-60))&&(ap_info.rssi >= (-70)))  {WIFI_Level = 0x03; }
//	if ((ap_info.rssi <= (-70))&&(ap_info.rssi >= (-80)))  {WIFI_Level = 0x02; }
//	if (ap_info.rssi < (-80))  							  {WIFI_Level = 0x01; }
	printf("RSSI %d\n",ap_info.rssi);
}


void tcp_client_task(void *pvParameters)
{
	char rx_buffer[128];
	//char rx_buffer[128];
    //char addr_str[128];
	//int addr_family;
	//int ip_protocol;	
	
	while(1) {
	    
		struct addrinfo *res;
		const struct addrinfo hints =
		{ .ai_family = AF_INET, .ai_socktype = SOCK_STREAM, };

		//printf("MQTT_Port %s\n", MQTT_Port);
		// resolve the IP of the target website
		int result = getaddrinfo(HOST_ADDR, HOST_PORT, &hints, &res);
		if ((result != 0) || (res == NULL))
		{
			printf("Unable to resolve IP for target website %s\n", HOST_ADDR);
			freeaddrinfo(res);
			break;
		}
		//printf("%s", (char *) hints.ai_addr);

		sock =  socket(res->ai_family, res->ai_socktype, 0);
		if (sock < 0) {
			ESP_LOGE(TAG_TCP, "Unable to create socket: errno %d", errno);
			break;
		}
		
		ESP_LOGI(TAG_TCP, "Socket created");

		err_socket_access = connect(sock, res->ai_addr, res->ai_addrlen);
		
		if (err_socket_access != 0) 
		{
			ESP_LOGE(TAG_TCP, "Socket unable to connect: errno %d", errno);
		}
		else
		{
			ESP_LOGI(TAG_TCP, "Successfully connected");
			wifi_status = 2;
		}

		while (1) {
			
			if (delete_tcp_task)
			{
				ESP_LOGI(TAG_TCP, "DELETE...");
				disconnect = 0;
				vTaskDelete(NULL);
			}
			wifi_rssi();
			int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
			// Error occured during receiving
			ESP_LOGI(TAG_TCP, "len: %d", len);
			if (len <= 0) 
			{
				ESP_LOGE(TAG_TCP, "recv failed: errno %d", errno);
				wifi_status = 1;
				break;
			}
			// Data received
			else //if(len > 0)
			{
				bool tcp_cmd = 0;
				rx_buffer[len] = 0;    // Null-terminate whatever we received and treat like a string

				ESP_LOGW(TAG_TCP, "%s", rx_buffer);
				for (uint8_t i = 0; i < len; i++)
				{
					printf("%02X ", rx_buffer[i]);
				}
				printf("\n");
				
				
				if (strstr(rx_buffer, "AT+RSSI") && (strspn(strstr(rx_buffer, "AT+RSSI"), "AT+RSSI") == 7))				
				{
					esp_wifi_sta_get_ap_info(&ap_info);
					
					char rssi[12] = "+ok=RSSI:";  //{ ap_info.rssi };
					//memcpy(rssi + 9, (const void *)ap_info.rssi, 1);
					sprintf(rssi + 9, "%d", ap_info.rssi);
					ESP_LOGI(TAG_TCP, "%s", rssi);
					int err = send(sock, rssi, 12, 0);
					tcp_cmd = true;
					if (err < 0) 
					{
						ESP_LOGE(TAG_TCP, "Error occured during sending: errno %d", errno);
						break;
					}
					
				}				

				if(strstr(rx_buffer, "AT+APPVER") && (strspn(strstr(rx_buffer, "AT+APPVER"), "AT+APPVER") == 9))
				{
					
					char appver[17] = "+ok=ESP8266-";
					memcpy(appver + 12, VERSION, sizeof(VERSION));
					ESP_LOGI(TAG_TCP, "-----------------------------------------------------");
					//ESP_LOGI(TAG_TCP, "%s", rx_buffer);
					ESP_LOGI(TAG_TCP, "%s", appver);
					ESP_LOGI(TAG_TCP, "-----------------------------------------------------");
					int err = send(sock, appver, 17, 0);
					tcp_cmd = true;
					if (err < 0) {
						ESP_LOGE(TAG_TCP, "Error occured during sending: errno %d", errno);
						break;
					}

				}
				//else if (!(strncmp(rx_buffer, "AT+WSMAC", 8)))
				if(strstr(rx_buffer, "AT+WSMAC") && (strspn(strstr(rx_buffer, "AT+WSMAC"), "AT+WSMAC") == 8))
				{
					ESP_LOGI(TAG_TCP, "-----------------------------------------------------");
					//ESP_LOGI(TAG_TCP, "%s", rx_buffer);
					ESP_LOGI(TAG_TCP, "%s", MAC_esp);
					ESP_LOGI(TAG_TCP, "-----------------------------------------------------");
					int err = send(sock, MAC_esp, 17, 0);
					tcp_cmd = true;
					if (err < 0) 
					{
						ESP_LOGE(TAG_TCP, "Error occured during sending: errno %d", errno);
						break;
					}
				}
							
				if (!(strncmp(rx_buffer, "AT+UPDATE", 9)))
				{
					xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
					tcp_cmd = true;
				}
				else if (!(strncmp(rx_buffer, "AT+HOST+", 8)))
				{
					ESP_LOGE(TAG_TCP, "HOST FOUND");
					bzero(HOST_ADDR, sizeof(HOST_ADDR));
					bzero(HOST_PORT, sizeof(HOST_PORT));
					uint8_t y = 0;
					uint8_t i = 0;
					//uint8_t j = 0;
					for(i = 8 ; i < len ; i++)
					{
						if (rx_buffer[i] == 0x3A)
						{
							i++;
							break;
						}
						HOST_ADDR[y] = rx_buffer[i];
						y++;
					}
					y = 0;
					
					for (; i < len; i++)
					{
						HOST_PORT[y] = rx_buffer[i];
						y++;
					}
					
					ESP_LOGW(TAG_TCP, "HOST: %s:%s", HOST_ADDR, HOST_PORT);
					nvs_open("storage", NVS_READWRITE, &storage_handle);
					nvs_set_str(storage_handle, "HOST_ADDR", HOST_ADDR);
					nvs_set_str(storage_handle, "HOST_PORT", HOST_PORT);
					nvs_commit(storage_handle);
					nvs_close(storage_handle);
					//esp_restart();
					shutdown(sock, 0);
					close(sock);
					bzero(rx_buffer, sizeof(rx_buffer));
					break;
				}
				else if (!(strncmp(rx_buffer, "AT+FOTA", 7)))
				{
					uint8_t y = 0;
					uint8_t i = 0;
					//uint8_t j = 0;
					for(i = 8 ; i < len ; i++)
					{
						if (rx_buffer[i] == 0x3A)
						{
							i++;
							break;
						}
						FOTA_ADDR[y] = rx_buffer[i];
						y++;
					}
					y = 0;
					
					for (; i < len; i++)
					{
						FOTA_PORT[y] = rx_buffer[i];
						y++;
					}
					bzero(FOTA_ADDR, sizeof(FOTA_ADDR));
					bzero(FOTA_PORT, sizeof(FOTA_PORT));
					nvs_open("storage", NVS_READWRITE, &storage_handle);
					nvs_set_str(storage_handle, "HOST_ADDR", FOTA_ADDR);
					nvs_set_str(storage_handle, "HOST_PORT", FOTA_PORT);
					nvs_commit(storage_handle);
					nvs_close(storage_handle);					
				}				
				else if (!tcp_cmd)
				{
					printf("-->TCP TO UART: ");
					for (uint8_t i = 0; i < len; i++)
					{
						//ESP_LOGI(TAG_TCP, "[DATA EVT]: %02X ", dtmp[i]);
						printf("%02X ", rx_buffer[i]);
					}
					printf("\n");
					uart_write_bytes(EX_UART_NUM, (const char *) rx_buffer, len);
				}
				tcp_cmd = false;
			}
			//ESP_LOGE(TAG_TCP, "Send");
			//vTaskDelay(1000 / portTICK_PERIOD_MS);
		}

		if (sock != -1) {
			ESP_LOGE(TAG_TCP, "Shutting down socket and restarting...");
			shutdown(sock, 0);
			close(sock);
		}
		
		// reconnect treshhold
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
	ESP_LOGE(TAG_TCP, "TCP DELETE TASK");
	vTaskDelete(NULL);
}

void tcp_server_task(void *pvParameters)
{
	char rx_buffer[128];
	char addr_str[128];
	int addr_family;
	int ip_protocol;

	struct sockaddr_in destAddr;
	destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(2228);
	addr_family = AF_INET;
	ip_protocol = IPPROTO_IP;
	inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);


	int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
	if (listen_sock < 0) {
		ESP_LOGE(TAG_TCP, "Unable to create socket: errno %d", errno);
	}
	else
	{
		ESP_LOGI(TAG_TCP, "Socket created");
	}
	

	int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
	if (err != 0) {
		ESP_LOGE(TAG_TCP, "Socket unable to bind: errno %d", errno);
		//break;
	}
	else
	{
		ESP_LOGI(TAG_TCP, "Socket binded");
	}
	
	while (1) {
		err = listen(listen_sock, 1);
		if (err != 0) {
			ESP_LOGE(TAG_TCP, "Error occured during listen: errno %d", errno);
			break;
		}
		else
		{
			ESP_LOGI(TAG_TCP, "Socket listening");
		}
		

		struct sockaddr_in sourceAddr;

		uint addrLen = sizeof(sourceAddr);
		int sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
		if (sock < 0) {
			ESP_LOGE(TAG_TCP, "Unable to accept connection: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG_TCP, "Socket accepted");

		while (1) {
			int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
			// Error occured during receiving
			if(len < 0) {
				ESP_LOGE(TAG_TCP, "recv failed: errno %d", errno);
				break;
			}
			// Connection closed
			else if(len == 0) {
				ESP_LOGI(TAG_TCP, "Connection closed");
				break;
			}
			// Data received
			else {

				inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);

				rx_buffer[len] = 0;  // Null-terminate whatever we received and treat like a string
				ESP_LOGI(TAG_TCP, "Received %d bytes from %s:", len, addr_str);
				ESP_LOGI(TAG_TCP, "%s", rx_buffer);

				
				if(!(strncmp(rx_buffer, "AT+HOST+", 8)))
				{
					ESP_LOGE(TAG_TCP, "HOST FOUND");
					bzero(HOST_ADDR, sizeof(HOST_ADDR));
					bzero(HOST_PORT, sizeof(HOST_PORT));
					uint8_t y = 0;
					uint8_t i = 0;
					//uint8_t j = 0;
					for(i = 8 ; i < len ; i++)
					{
						if (rx_buffer[i] == 0x3A)
						{
							i++;
							break;
						}
						HOST_ADDR[y] = rx_buffer[i];
						y++;
					}
					y = 0;
					
					for (; i < len; i++)
					{
						HOST_PORT[y] = rx_buffer[i];
						y++;
					}
					
					nvs_open("storage", NVS_READWRITE, &storage_handle);
					nvs_set_str(storage_handle, "HOST_ADDR", HOST_ADDR);
					nvs_set_str(storage_handle, "HOST_PORT", HOST_PORT);
					nvs_commit(storage_handle);
					nvs_close(storage_handle);
					esp_restart();
				}
				if (!(strncmp(rx_buffer, "AT+WIFI+", 8)))
				{
					uint8_t y = 0;
					uint8_t i = 0;
					//uint8_t j = 0;
					bzero(SSID, 32);
					bzero(PASS, 32);
					for(i = 8 ; i < len ; i++)
					{
						if (rx_buffer[i] == 0x3A)
						{
							i++;
							break;
						}
						SSID[y] = rx_buffer[i];
						y++;
					}
					y = 0;
					
					for (; i < len; i++)
					{
						PASS[y] = rx_buffer[i];
						y++;
					}
					//response
					int err = send(sock, MAC_esp, 17, 0);
					if (err < 0) 
					{
						ESP_LOGE(TAG_TCP, "Error occured during sending: errno %d", errno);
						break;
					}
					
					first_link = true;
					nvs_open("storage", NVS_READWRITE, &storage_handle);
					nvs_set_str(storage_handle, "SSID", (const char *)SSID);
					nvs_set_str(storage_handle, "PASS", (const char *)PASS);
					nvs_set_u8(storage_handle, "first_link", first_link);
					nvs_commit(storage_handle);
					nvs_close(storage_handle);
					esp_restart();
				}				
				/*
				int err = send(sock, rx_buffer, len, 0);
				if (err < 0) 
				{
					ESP_LOGE(TAG_TCP, "Error occured during sending: errno %d", errno);
					break;
				}*/
			}
		}

		if (sock != -1) {
			ESP_LOGE(TAG_TCP, "Shutting down socket and restarting...");
			shutdown(sock, 0);
			close(sock);
		}
	}
	ESP_LOGE(TAG_TCP, "DELETE TCP TASK");
	vTaskDelete(NULL);
}

void sc_callback(smartconfig_status_t status, void *pdata)
{
	switch (status) {
	case SC_STATUS_WAIT:
		ESP_LOGI(TAG_SC, "SC_STATUS_WAIT");
		break;
	case SC_STATUS_FIND_CHANNEL:
		ESP_LOGI(TAG_SC, "SC_STATUS_FINDING_CHANNEL");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		ESP_LOGI(TAG_SC, "SC_STATUS_GETTING_SSID_PSWD");
		break;
	case SC_STATUS_LINK:
		ESP_LOGI(TAG_SC, "SC_STATUS_LINK");
		wifi_config_t *wifi_config = pdata;
		//ESP_LOGI(TAG_SC, "SSID:%s", wifi_config->sta.ssid);
		//ESP_LOGI(TAG_SC, "PASSWORD:%s", wifi_config->sta.password);
		first_link = true;
		nvs_open("storage", NVS_READWRITE, &storage_handle);
		nvs_set_str(storage_handle, "SSID", (const char *)wifi_config->sta.ssid);
		nvs_set_str(storage_handle, "PASS", (const char *)wifi_config->sta.password);
		nvs_set_u8(storage_handle, "first_link", first_link);
		nvs_commit(storage_handle);
		nvs_close(storage_handle);

		ESP_ERROR_CHECK(esp_wifi_disconnect());
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config));
		ESP_ERROR_CHECK(esp_wifi_connect()) ;
	break;
	case SC_STATUS_LINK_OVER:
		ESP_LOGI(TAG_SC, "SC_STATUS_LINK_OVER");
		if (pdata != NULL) {
			sc_callback_data_t *sc_callback_data = (sc_callback_data_t *)pdata;
			switch (sc_callback_data->type) {
			case SC_ACK_TYPE_ESPTOUCH:
				ESP_LOGI(TAG_SC, "Phone ip: %d.%d.%d.%d", sc_callback_data->ip[0], sc_callback_data->ip[1], sc_callback_data->ip[2], sc_callback_data->ip[3]);
				ESP_LOGI(TAG_SC, "TYPE: ESPTOUCH");
				break;
			case SC_ACK_TYPE_AIRKISS:
				ESP_LOGI(TAG_SC, "TYPE: AIRKISS");
				break;
			default:
				ESP_LOGE(TAG_SC, "TYPE: ERROR");
				break;
			}
		}
		xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
		break;
	default:
		break;
	}
}

void smartconfig_example_task(void * parm)
{
	
	EventBits_t uxBits;
	wifi_status = 3;
	ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS));
	ESP_ERROR_CHECK(esp_smartconfig_start(sc_callback));
	while (1) {
		uxBits = xEventGroupWaitBits(wifi_event_group, IPV4_GOTIP_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
		if (uxBits & IPV4_GOTIP_BIT) {
			ESP_LOGI(TAG_SC, "WiFi Connected to ap");
		}
		if (uxBits & ESPTOUCH_DONE_BIT) {
			ESP_LOGI(TAG_SC, "smartconfig over");
			esp_smartconfig_stop();			
			vTaskDelete(NULL);
		}
	}
}