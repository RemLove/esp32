idf_component_register(SRCS "wifi.c" "led.c" "main.c" "doorbell_button.c" "codec.c" 
                    INCLUDE_DIRS "."
                    EMBED_FILES menling.pcm
                    PRIV_REQUIRES espressif__network_provisioning espressif__qrcode espressif__button espressif__led_strip espressif__esp_codec_dev nvs_flash bt esp_wifi esp_event esp_netif esp_driver_i2c esp_driver_i2s
                    )
