#include <stdio.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "can-frc/can-frc.h"

#define TX GPIO_NUM_14
#define RX GPIO_NUM_13

#define SIMULATING_CAN_PACKET 0

static inline void format_can_message_to_send_over_serial(can_message_t * can_message, uint8_t * buffer) {
    buffer[0] = 0x0A;
    buffer[4] = can_message->flags_with_DLC;
    buffer[CAN_STRUCT_LENGHT_BYTES + 1] = 0x0A;
    for(int i=1; i<=CAN_STRUCT_LENGHT_BYTES; i++) {
        if(i < 4)
            buffer[i] = (can_message->ide >> (8 * i)) & 0xFF;
        else if(i > 4)
            buffer[i] = (can_message->data >> (8 * i)) & 0xFF;
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(start_can_bus(TX, RX));

    QueueHandle_t uart_queue;

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 0, 0, 10, &uart_queue, 0));

    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));


    can_message_t can_receive = {0};


    for(;;) {
        #ifdef SIMULATING_CAN_PACKET
            can_receive.ide = 0xFFFFFFFF;
            can_receive.flags_with_DLC = 0xFF;
            can_receive.data = 0xFFFFFFFFFFFFFFFF;
        #else
            read_message(&can_receive);
        #endif

        uint8_t serial_message[CAN_STRUCT_LENGHT_BYTES + 2] = {0};
        format_can_message_to_send_over_serial(&can_receive, serial_message);

        uart_write_bytes(UART_NUM_0, (const int *)serial_message, CAN_STRUCT_LENGHT_BYTES);
    }
}
