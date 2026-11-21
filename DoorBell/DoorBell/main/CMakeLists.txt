idf_component_register(SRCS "wifi.c" "led.c" "main.c" "doorbell_button.c" "codec.c" 
                    INCLUDE_DIRS "."
                    EMBED_FILES menling.pcm
                    PRIV_REQUIRES network_provisioning nvs_flash bt
                    )
