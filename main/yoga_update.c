
#include "main.h"


static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

void http_get_task()
{
	const struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;
	struct in_addr *addr;
	int s, r;
	char recv_buf[314];

	while (1) {
		/* Wait for the callback to set the CONNECTED_BIT in the
		   event group.
		*/

		int err = getaddrinfo(FOTA_ADDR, FOTA_PORT, &hints, &res);

		if (err != 0 || res == NULL) {
			ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}

		/* Code to print the resolved IP.

		Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
		
		addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
		ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

		s = socket(res->ai_family, res->ai_socktype, 0);
		if (s < 0) {
			ESP_LOGE(TAG, "... Failed to allocate socket.");
			freeaddrinfo(res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG, "... allocated socket");

		if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
			ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
			close(s);
			freeaddrinfo(res);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}

		ESP_LOGI(TAG, "... connected");
		freeaddrinfo(res);

		if (write(s, REQUEST, strlen(REQUEST)) < 0) {
			ESP_LOGE(TAG, "... socket send failed");
			close(s);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG, "... socket send success");

		struct timeval receiving_timeout;
		receiving_timeout.tv_sec = 1;
		receiving_timeout.tv_usec = 0;
		if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0) 
		{
			ESP_LOGE(TAG, "... failed to set socket receiving timeout");
			close(s);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG, "... set socket receiving timeout success");

		/* Read HTTP response */
		do {
			bzero(recv_buf, sizeof(recv_buf));
			r = read(s, recv_buf, sizeof(recv_buf) - 1);
			//vTaskDelay(1000 / portTICK_PERIOD_MS);
			for(int i = 0 ; i < r ; i++) 
			{
				printf("%02X", recv_buf[i]);
				//putchar(recv_buf[i]);
			}
			printf("\n");
		} while (r > 0);
         
		printf("\n");
		ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
		close(s);
		for (int countdown = 10; countdown >= 0; countdown--) {
			ESP_LOGI(TAG, "%d... ", countdown);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
		ESP_LOGI(TAG, "Starting again!");
	}
}