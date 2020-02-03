
#include "main.h"

#define INFOPAGE "/kochnev/YOGA_Firmware/versions.info"

const char *TAG_UPD = "example";
void http_get_task()
{
	const struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;
	struct in_addr *addr;
	int s, r, q = 0;
	char recv_buf[256];
	char versions_info[2048];
	while (1) 
	{
		/* Wait for the callback to set the CONNECTED_BIT in the
		   event group.
		*/

		int err = getaddrinfo("dongle.rusklimat.ru", "80", &hints, &res);

		if (err != 0 || res == NULL) {
			ESP_LOGE(TAG_UPD, "DNS lookup failed err=%d res=%p", err, res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}

		/* Code to print the resolved IP.

		Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
		
		addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
		ESP_LOGI(TAG_UPD, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

		s = socket(res->ai_family, res->ai_socktype, 0);
		if (s < 0) {
			ESP_LOGE(TAG_UPD, "... Failed to allocate socket.");
			freeaddrinfo(res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG_UPD, "... allocated socket");

		if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
			ESP_LOGE(TAG_UPD, "... socket connect failed errno=%d", errno);
			close(s);
			freeaddrinfo(res);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}

		ESP_LOGI(TAG_UPD, "... connected");
		freeaddrinfo(res);
		
		
		
		const char *REQUEST =
	    "GET %s HTTP/1.0\r\n"
	    "Host: %s:%s\r\n"
	    "User-Agent: esp-idf/3.3 esp8266\r\n\r\n";

		char *http_request = NULL;
		int get_len = asprintf(&http_request, REQUEST, INFOPAGE, "dongle.rusklimat.ru", "80");	
		

		if (write(s, http_request, get_len) < 0) {
			ESP_LOGE(TAG_UPD, "... socket send failed");
			close(s);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG_UPD, "... socket send success");

		struct timeval receiving_timeout;
		receiving_timeout.tv_sec = 1;
		receiving_timeout.tv_usec = 0;
		if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0) 
		{
			ESP_LOGE(TAG_UPD, "... failed to set socket receiving timeout");
			close(s);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG_UPD, "... set socket receiving timeout success");

		/* Read HTTP response */
		do {
			bzero(recv_buf, sizeof(recv_buf));
			r = read(s, recv_buf, sizeof(recv_buf) - 1);
			
			memcpy(versions_info + q, recv_buf, r);
			q += r;
		} while (r > 0);
		
		for(int i = 0 ; i < q ; i++) 
		{
			printf("%c", versions_info[i]);
			//putchar(recv_buf[i]);
		}        

		printf("\n=========== %d ==========\n", q);
		//q = 0;
		//bzero(versions_info, sizeof(versions_info));

		char *soft_v_ptr = NULL;
		char *_v_ptr = NULL;
		char soft_version[3];
		uint8_t SoftwareV = 0;
		soft_v_ptr = strstr(versions_info, "SoftwareV");
		printf("%p\n", soft_v_ptr);
		_v_ptr = strchr(soft_v_ptr+10, ' ');
		printf("%p\n", _v_ptr);
		printf("%d\n", (int)(_v_ptr - soft_v_ptr));
		//strchr(strstr(versions_info, "SoftwareV") + 1, ' ') - strstr(versions_info, "SoftwareV") + 1
		SoftwareV = atoi(memcpy(soft_version, soft_v_ptr + 10, _v_ptr - (soft_v_ptr + 10)));
		printf("SoftwareV: %d\n", SoftwareV);
		//memcpy(soft_version, soft_v_ptr, 5);
		//for (int i = 0; i < 20; i++) 
		{
			//printf("%s", soft_v_ptr);
			//putchar(recv_buf[i]);
		}
		printf("\n");

		ESP_LOGI(TAG_UPD, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
		close(s);
		
		free(http_request);
/*		
		for (int countdown = 10; countdown >= 0; countdown--) {
			ESP_LOGI(TAG_UPD, "%d... ", countdown);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
		ESP_LOGI(TAG_UPD, "Starting again!");
*/		
		printf("Free HEAP: %d\n", esp_get_free_heap_size());
		vTaskDelay(100000 / portTICK_PERIOD_MS);
	}
}