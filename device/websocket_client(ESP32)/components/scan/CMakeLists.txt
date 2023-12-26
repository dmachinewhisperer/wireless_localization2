idf_component_register(
    SRCS "scan.c"
    INCLUDE_DIRS "."
    REQUIRES "esp_event" "nvs_flash" "log" "esp_wifi"
)
