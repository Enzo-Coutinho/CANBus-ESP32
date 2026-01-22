#include <stdio.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "can-frc/can-frc.h"

#define TX GPIO_NUM_14
#define RX GPIO_NUM_13

#define SIMULATING_CAN_PACKET 0

static inline void format_can_message_to_send_over_serial(can_message_t * can_message, uint8_t * buffer) {
    buffer[4] = can_message->flags_with_DLC;
    for(int i=0; i<CAN_STRUCT_LENGHT_BYTES; i++) {
        if(i < 4)
            buffer[i] = (can_message->ide >> (8 * i)) & 0xFF;
        else if(i > 4)
            buffer[i] = (can_message->data >> (8 * i)) & 0xFF;
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(start_can_bus(TX, RX));

    can_message_t can_receive = {0};

    for(;;) {
        #ifdef SIMULATING_CAN_PACKET
            can_receive.ide = 0xFFFFFFFF;
            can_receive.flags_with_DLC = 0xFF;
            can_receive.data = 0xFFFFFFFFFFFFFFFF;
        #else
            read_message(&can_receive);
        #endif

        uint8_t serial_message[CAN_STRUCT_LENGHT_BYTES] = {0};
        format_can_message_to_send_over_serial(&can_receive, serial_message);

        for(int i=0; i<CAN_STRUCT_LENGHT_BYTES; i++)
            printf("%d", serial_message[i]);
        printf("\n");
    }
}
