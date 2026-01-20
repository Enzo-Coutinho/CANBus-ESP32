#include <stdio.h>
#include <stdlib.h>
#include "can-frc/can-frc.h"

#define TX GPIO_NUM_14
#define RX GPIO_NUM_13

static const char * MAIN = "MAIN";

void app_main(void)
{
    esp_err_t result_of_starting_can_bus = start_can_bus(TX, RX);
    while (result_of_starting_can_bus != ESP_OK)
    {
        ESP_LOGI(MAIN, "Failed to start CAN Bus");
    }
    

    can_message_t can_receive = {0};

    for(;;) {
        read_message(&can_receive);
    }
}
