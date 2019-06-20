deps_config := \
	/rtos-sdk/master/components/app_update/Kconfig \
	/rtos-sdk/master/components/aws_iot/Kconfig \
	/rtos-sdk/master/components/console/Kconfig \
	/rtos-sdk/master/components/esp8266/Kconfig \
	/rtos-sdk/master/components/esp_http_client/Kconfig \
	/rtos-sdk/master/components/esp_http_server/Kconfig \
	/rtos-sdk/master/components/freertos/Kconfig \
	/rtos-sdk/master/components/libsodium/Kconfig \
	/rtos-sdk/master/components/log/Kconfig \
	/rtos-sdk/master/components/lwip/Kconfig \
	/rtos-sdk/master/components/mdns/Kconfig \
	/rtos-sdk/master/components/mqtt/Kconfig \
	/rtos-sdk/master/components/newlib/Kconfig \
	/rtos-sdk/master/components/pthread/Kconfig \
	/rtos-sdk/master/components/spiffs/Kconfig \
	/rtos-sdk/master/components/ssl/Kconfig \
	/rtos-sdk/master/components/tcpip_adapter/Kconfig \
	/rtos-sdk/master/components/util/Kconfig \
	/rtos-sdk/master/components/vfs/Kconfig \
	/rtos-sdk/master/components/wifi_provisioning/Kconfig \
	/rtos-sdk/master/components/wpa_supplicant/Kconfig \
	/rtos-sdk/master/components/bootloader/Kconfig.projbuild \
	/rtos-sdk/master/components/esptool_py/Kconfig.projbuild \
	/rtos-sdk/master/components/partition_table/Kconfig.projbuild \
	/rtos-sdk/master/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
