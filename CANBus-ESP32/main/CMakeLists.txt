idf_component_register(SRCS "main.c" "can-frc/can-frc.c"
                    REQUIRES esp_driver_twai driver
                    INCLUDE_DIRS ".")
