deps_config := \
	/rtos-sdk/v3.1/components/app_update/Kconfig \
	/rtos-sdk/v3.1/components/aws_iot/Kconfig \
	/rtos-sdk/v3.1/components/esp8266/Kconfig \
	/rtos-sdk/v3.1/components/freertos/Kconfig \
	/rtos-sdk/v3.1/components/libsodium/Kconfig \
	/rtos-sdk/v3.1/components/log/Kconfig \
	/rtos-sdk/v3.1/components/lwip/Kconfig \
	/rtos-sdk/v3.1/components/mdns/Kconfig \
	/rtos-sdk/v3.1/components/mqtt/Kconfig \
	/rtos-sdk/v3.1/components/newlib/Kconfig \
	/rtos-sdk/v3.1/components/pthread/Kconfig \
	/rtos-sdk/v3.1/components/ssl/Kconfig \
	/rtos-sdk/v3.1/components/tcpip_adapter/Kconfig \
	/rtos-sdk/v3.1/components/wpa_supplicant/Kconfig \
	/rtos-sdk/v3.1/components/bootloader/Kconfig.projbuild \
	/rtos-sdk/v3.1/components/esptool_py/Kconfig.projbuild \
	/rtos-sdk/v3.1/components/partition_table/Kconfig.projbuild \
	/rtos-sdk/v3.1/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
