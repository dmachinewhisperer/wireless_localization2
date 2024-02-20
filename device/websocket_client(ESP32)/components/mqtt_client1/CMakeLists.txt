idf_component_register(SRCS "mqtt_client1.c"
                INCLUDE_DIRS "."
                REQUIRES lwip mqtt  protocol_examples_common esp_wifi ssd1306 init
                PRIV_REQUIRES esp_timer esp_event)