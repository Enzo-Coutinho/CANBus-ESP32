#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "can-frc.h"

twai_node_handle_t node_hdl = NULL;
twai_onchip_node_config_t node_config;

QueueHandle_t queue_handler;

static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx);

void start_can_bus(const gpio_num_t tx, const gpio_num_t rx, const uint32_t bitrate) {

    const uint8_t queue_len = 10;

    queue_handler = xQueueGenericCreate(queue_len, sizeof(uint64_t), queueQUEUE_TYPE_SET);

    twai_event_callbacks_t user_cbs = {
        .on_rx_done = twai_rx_cb,
    };

    ESP_ERROR_CHECK(twai_node_register_event_callbacks(node_hdl, &user_cbs, NULL));
    
    
    node_config.io_cfg.tx = tx;
    node_config.io_cfg.rx = rx;
    node_config.bit_timing.bitrate = bitrate;
    node_config.tx_queue_depth = 5;

    // Create a new TWAI controller driver instance
    ESP_ERROR_CHECK(twai_new_node_onchip(&node_config, &node_hdl));

    // Start the TWAI controller
    ESP_ERROR_CHECK(twai_node_enable(node_hdl));
}

void send_message(const uint32_t id, const uint8_t data[8]) {
    twai_frame_t tx_msg = {
        .header.id = 0x1,           // Message ID
        .header.ide = true,         // Use 29-bit extended ID format
        .buffer = data,        // Pointer to data to transmit
        .buffer_len = sizeof(data),  // Length of data to transmit
    };

    ESP_ERROR_CHECK(twai_node_transmit(node_hdl, &tx_msg, 0)); 
}

int read_message(void) {
    uint64_t data = 0;
    xQueueReceive(queue_handler, &data, pdMS_TO_TICKS(1000));
    return data;
}

static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    uint8_t recv_buff[8];
    twai_frame_t rx_frame = {
        .buffer = recv_buff,
        .buffer_len = sizeof(recv_buff),
    };
    if (ESP_OK == twai_node_receive_from_isr(handle, &rx_frame)) {
        uint64_t data_response = 0;
        for(int i=0; i<sizeof(recv_buff)/sizeof(uint8_t); i++) {
            data_response |= recv_buff[i] << (i * 8);
        }
         xQueueSendFromISR(queue_handler, data_response, pdFALSE);
    }
    return false;
}