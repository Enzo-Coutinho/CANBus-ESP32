#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include <string.h>
#include "can-frc.h"

twai_node_handle_t node_hdl = NULL;
twai_onchip_node_config_t node_config;

QueueHandle_t queue_handler;

static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx);

static inline uint32_t frc_can_encode_id(const can_ide_t *can_id)
{
    return (((uint32_t)(can_id->device_type   & 0x1F) << 24) |
            ((uint32_t)(can_id->manufacturer & 0xFF) << 16) |
            ((uint32_t)(can_id->api_class     & 0x3F) << 10) |
            ((uint32_t)(can_id->api_index     & 0x0F) <<  6) |
            ((uint32_t)(can_id->device_number & 0x3F)));
}


static inline void frc_can_decode_id(uint32_t id, can_ide_t * can_id)
{
    can_id->device_number = (uint8_t)(id & 0x3F);
    can_id->api_index = (uint8_t)((id >> 6) & 0xF);
    can_id->api_class = (uint8_t)((id >> 10) & 0x3F);
    can_id->manufacturer = (uint8_t)((id >> 16) & 0xFF);
    can_id->device_type = (uint8_t)((id >> 24) & 0x1F);
}

esp_err_t start_can_bus(const gpio_num_t tx, const gpio_num_t rx) {  

    memset(&node_config, 0, sizeof(node_config));

    const uint8_t queue_len = 10;

    queue_handler = xQueueCreate(queue_len, sizeof(can_message_t));

    if(queue_handler == NULL)
        return ESP_FAIL;

    twai_event_callbacks_t user_cbs = {
        .on_rx_done = twai_rx_cb,
    };

    node_config.io_cfg.tx = tx;
    node_config.io_cfg.rx = rx;
    node_config.bit_timing.bitrate = BITRATE;
    node_config.tx_queue_depth = 5;
    node_config.flags.enable_listen_only = 1;


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
}


static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    uint8_t recv_buff[LENGHT];
    twai_frame_t rx_frame = {
        .buffer = recv_buff,
        .buffer_len = sizeof(recv_buff),
    };
    if (ESP_OK == twai_node_receive_from_isr(handle, &rx_frame)) {
        
        can_message_t can_message = {0};

        frc_can_decode_id(rx_frame.header.id, &can_message.can_ide);
        can_message.dlc = rx_frame.header.dlc;
        can_message.data = recv_buff;
        can_message.is_remote_frame = rx_frame.header.rtr;
        can_message.is_fd_format = rx_frame.header.fdf;
        can_message.is_error = rx_frame.header.esi;
        can_message.is_bit_rate_shift = rx_frame.header.brs;

        xQueueSendFromISR(queue_handler, &can_message, NULL);
    }
    return false;
}