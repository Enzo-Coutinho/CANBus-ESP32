#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include <string.h>
#include "can-frc.h"

#define CAN_SPEED 1000000
#define TX_QUEUE_LENGHT 5
#define CAN_STRUCT_QUEUE 10
#define MODE_LISTEN_ONLY 1

twai_node_handle_t node_hdl = NULL;
twai_onchip_node_config_t node_config;

QueueHandle_t queue_handler;

static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx);

esp_err_t start_can_bus(const gpio_num_t tx, const gpio_num_t rx) {  

    memset(&node_config, 0, sizeof(node_config));

    const uint8_t queue_len = CAN_STRUCT_QUEUE;

    queue_handler = xQueueCreate(queue_len, sizeof(can_message_t));

    if(queue_handler == NULL)
        return ESP_FAIL;

    twai_event_callbacks_t user_cbs = {
        .on_rx_done = twai_rx_cb,
    };

    node_config.io_cfg.tx = tx;
    node_config.io_cfg.rx = rx;
    node_config.bit_timing.bitrate = CAN_SPEED;
    node_config.tx_queue_depth = TX_QUEUE_LENGHT;
    node_config.flags.enable_listen_only = MODE_LISTEN_ONLY;


    if(twai_new_node_onchip(&node_config, &node_hdl) != ESP_OK)
        return ESP_FAIL;
    if(twai_node_register_event_callbacks(node_hdl, &user_cbs, NULL) != ESP_OK)
        return ESP_FAIL;
    if(twai_node_enable(node_hdl) != ESP_OK)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t read_message(can_message_t *can_message) {
    if (queue_handler == NULL)
        return ESP_FAIL;
    if(xQueueReceive(queue_handler, can_message, pdMS_TO_TICKS(100)))
        return ESP_OK;
    return ESP_FAIL;
}


static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    uint8_t recv_buff[8];
    twai_frame_t rx_frame = {
        .buffer = recv_buff,
        .buffer_len = sizeof(recv_buff),
    };
    if (ESP_OK == twai_node_receive_from_isr(handle, &rx_frame)) {
        
        can_message_t can_message = {0};

        can_message.ide = rx_frame.header.id;

        can_message.flags_with_DLC |= (rx_frame.header.rtr & 0x01);
        can_message.flags_with_DLC |= ((rx_frame.header.fdf << 1) & 0x02);
        can_message.flags_with_DLC |= ((rx_frame.header.esi << 2) & 0x04);
        can_message.flags_with_DLC |= ((rx_frame.header.brs << 3) & 0x08);
        can_message.flags_with_DLC |= ((rx_frame.header.dlc << 4) & 0x10);

        can_message.data = 0;

        for(int i=0; i<rx_frame.header.dlc; i++)
            can_message.data |= recv_buff[i];

        xQueueSendFromISR(queue_handler, &can_message, NULL);
    }
    return false;
}