idf_component_register(SRCS "src/communication.c" "src/ov7670Camera.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_i2c driver
                       )