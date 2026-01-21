idf_component_register(SRCS "main.c" "can-frc/can-frc.c"
                    REQUIRES esp_driver_twai driver
                    REQUIRES esp_driver_uart
                    INCLUDE_DIRS ".")
