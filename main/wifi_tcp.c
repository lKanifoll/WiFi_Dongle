#include "main.h"
const int ESPTOUCH_DONE_BIT = BIT1;
const int IPV4_GOTIP_BIT = BIT0;
const char *TAG_WIFI = "WIFI";
const char *TAG = "SC";

esp_err_t event_handler(void *ctx, system_event_t *event)
{
	/* For accessing reason codes in case of disconnection */
	//system_event_info_t *info = &event->event_info;
	
	
    
	switch (event->event_id) {
	case SYSTEM_EVENT_STA_START:
		wifi_active_flag = true;
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
		break;
	case SYSTEM_EVENT_AP_STACONNECTED:
		ESP_LOGI(TAG_WIFI,
			"station:"MACSTR" join, AID=%d",
			MAC2STR(event->event_info.sta_connected.mac),
			event->event_info.sta_connected.aid);
		break;
	case SYSTEM_EVENT_AP_START:
		wifi_active_flag = true;
		ESP_LOGI(TAG_WIFI, "Access point is active: Dongle_Rusklimat");
		xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		//ESP_LOGE(TAG_WIFI, "Disconnect reason : %d", info->disconnected.reason);
		ESP_LOGW(TAG_WIFI, "RECONNECT...");
		wifi_active_flag = false;
		esp_wifi_connect();
		xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
		break;
		
//	case SYSTEM_EVENT_STA_CONNECTED:
//		wifi_active_flag = true;
//		wait_for_ip();
		
//		break;
	default:
		break;
	}
	return ESP_OK;
}




void initialise_wifi()
{
	tcpip_adapter_init();

	wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	//ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	nvs_open("storage", NVS_READWRITE, &storage_handle);
	nvs_get_u8(storage_handle, "first_link", &first_link);
	nvs_commit(storage_handle);
	nvs_close(storage_handle);
	printf("first_link %d\n", first_link);
	
	if(first_link)
	{
		wifi_config_t wifi_config =
		{
			.sta = { .ssid = "", .password = "", },
		};
		printf("SSID %s\n", SSID);
		printf("PASS %s\n", PASS);
		strcpy((char*) &wifi_config.sta.ssid[0], (const char*) &SSID[0]);
		strcpy((char*) &wifi_config.sta.password[0], (const char*) &PASS[0]);

		//curr_interface = ESP_IF_WIFI_STA;

		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
		//curr_interface = ESP_IF_WIFI_STA;

		ESP_ERROR_CHECK(esp_wifi_start());
		ESP_ERROR_CHECK(esp_wifi_connect());
	}
	else
	{	
		wifi_config_t wifi_config =
		{
			.sta = { 
				.ssid = "Dongle_Rusklimat", 
				.password = "12345678", 
			},
		};
		//printf("first_link %s\n", SSID);
		//printf("first_link %s\n", PASS);
		//curr_interface = ESP_IF_WIFI_STA;

		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
		//curr_interface = ESP_IF_WIFI_STA;

		ESP_ERROR_CHECK(esp_wifi_start());
		ESP_ERROR_CHECK(esp_wifi_connect());
	}
}




void set_wifi_sta()
{
	if (wifi_active_flag)
	{
		//ESP_ERROR_CHECK(esp_wifi_deauth_sta(0));
		ESP_ERROR_CHECK(esp_wifi_disconnect());
		ESP_ERROR_CHECK(esp_wifi_stop());
	}
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	/*
    wifi_country_t info_country;
    strcpy((char*)&info_country.cc[0],"JP");

    info_country.schan = 1;
    info_country.nchan = 14;
    info_country.policy = WIFI_COUNTRY_POLICY_MANUAL;
*/
	//conn_flag = 1;

	//curr_interface = ESP_IF_WIFI_STA;
	ESP_ERROR_CHECK(esp_wifi_start());
	//esp_wifi_set_country(&info_country);
	xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 5, NULL);
	
}


void set_wifi_ap()
{
//	sta_ap_flag = 1;
//	curr_interface = ESP_IF_WIFI_STA;
//	get_mac_buf();
	if(wifi_active_flag)
	{
		ESP_ERROR_CHECK(esp_wifi_disconnect());
		ESP_ERROR_CHECK(esp_wifi_stop());
	}
	 wifi_config_t wifi_config1 = {
		.ap = {
		.ssid="Dongle_Rusklimat",
		.ssid_len = 16,
		.password = "12345678",
		.channel = 6,
		.authmode = WIFI_AUTH_WPA_WPA2_PSK,
		.ssid_hidden = 0,
		.max_connection = 1,
		.beacon_interval = 100
	},
	};
	//strncpy((char*)wifi_config1.ap.ssid, (char*)MAC_esp, 17);

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config1));
	ESP_ERROR_CHECK(esp_wifi_start());
	
}

void get_mac_buf()
{
	while (ESP_OK != esp_wifi_get_mac(ESP_IF_WIFI_STA, temp_mac)) ;
	sprintf(&MAC_esp[0], "+ok=%02X%02X%02X%02X%02X%02X", temp_mac[0], temp_mac[1], temp_mac[2], temp_mac[3], temp_mac[4], temp_mac[5]);
	printf("%s\n", MAC_esp);
}


void wait_for_ip()
{
	uint32_t bits = IPV4_GOTIP_BIT;
	ESP_LOGI(TAG_WIFI, "Waiting for AP connection...");
	xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
	ESP_LOGI(TAG_WIFI, "Connected to AP");
	xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
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
		int result = getaddrinfo(HOST_IP_ADDR, PORT, &hints, &res);
		if ((result != 0) || (res == NULL))
		{
			printf("Unable to resolve IP for target website %s\n", HOST_IP_ADDR);
			freeaddrinfo(res);
			//return pdFALSE;
		}
		//printf("%s", (char *) hints.ai_addr);

		int sock =  socket(res->ai_family, res->ai_socktype, 0);
		if (sock < 0) {
			ESP_LOGE(TAG_WIFI, "Unable to create socket: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG_WIFI, "Socket created");

		int err = connect(sock, res->ai_addr, res->ai_addrlen);
		if (err != 0) {
			ESP_LOGE(TAG_WIFI, "Socket unable to connect: errno %d", errno);
		}
		ESP_LOGI(TAG_WIFI, "Successfully connected");

		while (1) {
						ESP_LOGI(TAG_WIFI, "Receiving TCP...");
						int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
						// Error occured during receiving
						ESP_LOGI(TAG_WIFI, "len: %d", len);
						if(len <= 0) {
							ESP_LOGE(TAG_WIFI, "recv failed: errno %d", errno);
							break;
						}
						// Data received
						else if(len > 0)
						{
							rx_buffer[len] = 0;   // Null-terminate whatever we received and treat like a string
			
							//uart_write_bytes(UART_NUM_0, (const char *) rx_buffer, len);
				            
							if (!(strncmp(rx_buffer, "AT+APPVER", 9)))
							{
								ESP_LOGI(TAG_WIFI, "-----------------------------------------------------");
								ESP_LOGI(TAG_WIFI, "%s", rx_buffer);
								ESP_LOGI(TAG_WIFI, "%s", "+ok=3.0.1-V.1.0.0");
								ESP_LOGI(TAG_WIFI, "-----------------------------------------------------");
								int err = send(sock, "+ok=3.0.1-V.1.0.0", 17, 0);
								if (err < 0) {
								ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
								break;
								}
							}
							else if (!(strncmp(rx_buffer, "AT+WSMAC", 8)))
							{
								ESP_LOGI(TAG_WIFI, "-----------------------------------------------------");
								ESP_LOGI(TAG_WIFI, "%s", rx_buffer);
								ESP_LOGI(TAG_WIFI, "%s", MAC_esp);
								ESP_LOGI(TAG_WIFI, "-----------------------------------------------------");
								int err = send(sock, MAC_esp, 17, 0);
								if (err < 0) {
									ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
									break;
								}
							}
							else
							{
								printf("Request: ");
								for (uint8_t i = 0; i < len; i++)
								{
									//ESP_LOGI(TAG, "[DATA EVT]: %02X ", dtmp[i]);
									printf("%02X ", rx_buffer[i]);
								}
								printf("\n");
								uart_write_bytes(EX_UART_NUM, (const char *) rx_buffer, len);
							}
						}
			//			rx_buffer[4] = {0xAA, 0x01, 0x07, 0xB2};
			//uart_write_bytes(EX_UART_NUM, (const char *) rx_buffer, 4);
			//ESP_LOGE(TAG_WIFI, "Send");
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}

		if (sock != -1) {
			ESP_LOGE(TAG_WIFI, "Shutting down socket and restarting...");
			shutdown(sock, 0);
			close(sock);
		}
	}
	ESP_LOGE(TAG_WIFI, "TCP DELETE TASK");
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
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		//break;
	}
	ESP_LOGI(TAG, "Socket created");

	int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
		//break;
	}
	ESP_LOGI(TAG, "Socket binded");
	while (1) {
		err = listen(listen_sock, 1);
		if (err != 0) {
			ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG, "Socket listening");

		struct sockaddr_in sourceAddr;

		uint addrLen = sizeof(sourceAddr);
		int sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
		if (sock < 0) {
			ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG, "Socket accepted");

		while (1) {
			int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
			// Error occured during receiving
			if(len < 0) {
				ESP_LOGE(TAG, "recv failed: errno %d", errno);
				break;
			}
			// Connection closed
			else if(len == 0) {
				ESP_LOGI(TAG, "Connection closed");
				break;
			}
			// Data received
			else {

				inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);

				rx_buffer[len] = 0;  // Null-terminate whatever we received and treat like a string
				ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
				ESP_LOGI(TAG, "%s", rx_buffer);

				int err = send(sock, rx_buffer, len, 0);
				if (err < 0) {
					ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
					break;
				}
			}
		}

		if (sock != -1) {
			ESP_LOGE(TAG, "Shutting down socket and restarting...");
			shutdown(sock, 0);
			close(sock);
		}
	}
	vTaskDelete(NULL);
}

void sc_callback(smartconfig_status_t status, void *pdata)
{
	switch (status) {
	case SC_STATUS_WAIT:
		ESP_LOGI(TAG, "SC_STATUS_WAIT");
		break;
	case SC_STATUS_FIND_CHANNEL:
		ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
		break;
	case SC_STATUS_LINK:
		ESP_LOGI(TAG, "SC_STATUS_LINK");
		wifi_config_t *wifi_config = pdata;
		ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
		ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);
		first_link = true;
		nvs_open("storage", NVS_READWRITE, &storage_handle);
		nvs_set_str(storage_handle, "SSID", (const char *)wifi_config->sta.ssid);
		nvs_set_str(storage_handle, "PASS", (const char *)wifi_config->sta.password);
		nvs_set_u8(storage_handle, "first_link", first_link);
		nvs_commit(storage_handle);
		nvs_close(storage_handle);
		
		ESP_ERROR_CHECK(esp_wifi_disconnect());
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config));
		ESP_ERROR_CHECK(esp_wifi_connect());
		break;
	case SC_STATUS_LINK_OVER:
		ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
		if (pdata != NULL) {
			sc_callback_data_t *sc_callback_data = (sc_callback_data_t *)pdata;
			switch (sc_callback_data->type) {
			case SC_ACK_TYPE_ESPTOUCH:
				ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d", sc_callback_data->ip[0], sc_callback_data->ip[1], sc_callback_data->ip[2], sc_callback_data->ip[3]);
				ESP_LOGI(TAG, "TYPE: ESPTOUCH");
				break;
			case SC_ACK_TYPE_AIRKISS:
				ESP_LOGI(TAG, "TYPE: AIRKISS");
				break;
			default:
				ESP_LOGE(TAG, "TYPE: ERROR");
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
	ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS));
	ESP_ERROR_CHECK(esp_smartconfig_start(sc_callback));
	while (1) {
		uxBits = xEventGroupWaitBits(wifi_event_group, IPV4_GOTIP_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
		if (uxBits & IPV4_GOTIP_BIT) {
			ESP_LOGI(TAG, "WiFi Connected to ap");
		}
		if (uxBits & ESPTOUCH_DONE_BIT) {
			ESP_LOGI(TAG, "smartconfig over");
			esp_smartconfig_stop();
			vTaskDelete(NULL);
		}
	}
}