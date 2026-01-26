#include <stdio.h>
#include <stdlib.h>
#include "can-frc/can-frc.h"

#define TX GPIO_NUM_14
#define RX GPIO_NUM_13

#define SIMULATING_CAN_PACKET 0

#define ESP_LOG_COLOR_DISABLED     (1)  
#define ESP_LOG_TIMESTAMP_DISABLED (1) 
#define ESP_LOG_FORMATTING_DISABLED (1)

static inline void format_can_message_to_send_over_serial(can_message_t * can_message, uint8_t * buffer) {
    buffer[0] = 0x0A;
    buffer[1] = (uint8_t)((can_message->ide >> 24) & 0xFF);
    buffer[2] = (uint8_t)((can_message->ide >> 16) & 0xFF);
    buffer[3] = (uint8_t)((can_message->ide >> 8) & 0xFF);
    buffer[4] = (uint8_t)((can_message->ide & 0xFF));
    buffer[5] = can_message->flags_with_DLC;
    buffer[6] = (uint8_t)((can_message->data >> 56) & 0xFF);
    buffer[7] = (uint8_t)((can_message->data >> 48) & 0xFF);
    buffer[8] = (uint8_t)((can_message->data >> 40) & 0xFF);
    buffer[9] = (uint8_t)((can_message->data >> 32) & 0xFF);
    buffer[10] = (uint8_t)((can_message->data >> 24) & 0xFF);
    buffer[11] = (uint8_t)((can_message->data >> 16) & 0xFF);
    buffer[12] = (uint8_t)((can_message->data >> 8) & 0xFF);
    buffer[13] = (uint8_t)(can_message->data & 0xFF);
    buffer[14] = 0x0A;
}

void app_main(void)
{
    ESP_ERROR_CHECK(start_can_bus(TX, RX));

    can_message_t can_receive = {0};

    for(;;) {
        #ifdef SIMULATING_CAN_PACKET
            can_receive.ide = 0x01011840;
            can_receive.flags_with_DLC = 0xFF;
            can_receive.data = 0xFFFFFFFFFFFFFFFF;
        #else
            read_message(&can_receive);
        #endif

        uint8_t serial_message[CAN_STRUCT_LENGHT_BYTES] = {0};
        format_can_message_to_send_over_serial(&can_receive, serial_message);

        for(int i=0; i<CAN_STRUCT_LENGHT_BYTES; i++)
            printf("%02X", serial_message[i]);
        printf("\n");
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
