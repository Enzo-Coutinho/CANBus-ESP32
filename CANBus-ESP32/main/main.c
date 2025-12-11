#include <stdio.h>
#include <stdlib.h>
#include "can-frc/can-frc.h"

#define TX GPIO_NUM_14
#define RX GPIO_NUM_13

static const char * MAIN = "MAIN";

void app_main(void)
{
    start_can_bus(TX, RX);

    can_message_t can_receive = {0};

    for(;;) {
        read_message(&can_receive);
        const uint8_t manufacturer = can_receive.can_ide.manufacturer;
        const uint8_t deviceType = can_receive.can_ide.device_type;
        const uint8_t apiClass = can_receive.can_ide.api_class;
        const uint8_t apiIndex = can_receive.can_ide.api_index;
        ESP_LOGI(MAIN, "Manufacturer: %s", can_receive.can_ide.manufacturer);
        ESP_LOGI(MAIN, "Device Type: %s", can_receive.can_ide.device_type); 
        ESP_LOGI(MAIN, "API Class: %d", can_receive.can_ide.api_class);
        ESP_LOGI(MAIN, "API Index: %d", can_receive.can_ide.api_index);
        ESP_LOGI(MAIN, "Device Number: %d", can_receive.can_ide.device_number);
    }
}
