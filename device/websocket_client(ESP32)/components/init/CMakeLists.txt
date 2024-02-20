idf_component_register(SRCS "init.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_hw_support nvs_flash ssd1306
                    )
