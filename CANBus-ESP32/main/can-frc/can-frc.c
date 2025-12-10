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

enum CAN_MODES mode_to_switch = DEFAULT;

static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx);

static inline uint32_t frc_can_encode_id(const can_ide_t *can_id)
{
    return (((uint32_t)(can_id->deviceType   & 0x1F) << 24) |
            ((uint32_t)(can_id->manufacturer & 0xFF) << 16) |
            ((uint32_t)(can_id->apiClass     & 0x3F) << 10) |
            ((uint32_t)(can_id->apiIndex     & 0x0F) <<  6) |
            ((uint32_t)(can_id->deviceNumber & 0x3F)));
}


static inline void frc_can_decode_id(uint32_t id, can_ide_t * can_id)
{
    can_id->deviceNumber = (uint8_t)(id & 0x3F);
    can_id->apiIndex = (uint8_t)((id >> 6) & 0xF);
    can_id->apiClass = (uint8_t)((id >> 10) & 0x3F);
    can_id->manufacturer = (uint8_t)((id >> 16) & 0xFF);
    can_id->deviceType = (uint8_t)((id >> 24) & 0x1F);
}

void set_can_mode(enum CAN_MODES mode) {
    mode_to_switch = mode;
}

void start_can_bus(const gpio_num_t tx, const gpio_num_t rx) {

    memset(&node_config, 0, sizeof(node_config));

    const uint8_t queue_len = 10;

    queue_handler = xQueueCreate(queue_len, sizeof(can_message_t));

    twai_event_callbacks_t user_cbs = {
        .on_rx_done = twai_rx_cb,
    };
    
    
    const uint32_t bitrate = 1e6;

    node_config.io_cfg.tx = tx;
    node_config.io_cfg.rx = rx;
    node_config.bit_timing.bitrate = bitrate;
    node_config.tx_queue_depth = 5;

    switch(mode_to_switch)
    {
        case LISTEN_ONLY:
            node_config.flags.enable_listen_only = 1;
            break;
        case SELF_TEST:
            node_config.flags.enable_self_test = 1;
            break;
        case NO_RECEITE_RTR:
            node_config.flags.no_receive_rtr = 1;
            break;
        case LOOPBACK:
            node_config.flags.enable_loopback = 1;
            break;
        case DEFAULT:
        default:
            break;
    }

    ESP_ERROR_CHECK(twai_new_node_onchip(&node_config, &node_hdl));
    ESP_ERROR_CHECK(twai_node_register_event_callbacks(node_hdl, &user_cbs, NULL));
    ESP_ERROR_CHECK(twai_node_enable(node_hdl));
}

bool read_message(can_message_t *can_message) {
    if (queue_handler == NULL)
        return false;
    return xQueueReceive(queue_handler, can_message, pdMS_TO_TICKS(100)) == pdTRUE;
}


static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    uint8_t recv_buff[8];
    twai_frame_t rx_frame = {
        .buffer = recv_buff,
        .buffer_len = sizeof(recv_buff),
    };
    if (ESP_OK == twai_node_receive_from_isr(handle, &rx_frame)) {
        can_message_t can_message;

        for(int i=0; i<rx_frame.header.dlc; i++)
            can_message.data[i] = recv_buff[i];

        frc_can_decode_id(rx_frame.header.id, &can_message.canIde);

        xQueueSendFromISR(queue_handler, &can_message, NULL);
    }
    return false;
}