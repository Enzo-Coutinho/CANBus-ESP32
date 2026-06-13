idf_component_register(SRCS "can-frc.c"
                    PRIV_REQUIRES esp_driver_twai
                    REQUIRES esp_driver_gpio
                    INCLUDE_DIRS "include")
