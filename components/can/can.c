#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include <stdbool.h>
#include <string.h>
#include "can.h"

#define CAN_SPEED 1000000
#define TX_QUEUE_LENGHT 5
#define CAN_STRUCT_QUEUE 10
#define EXTENDED_IDE 1

static twai_node_handle_t node_hdl = NULL;
static twai_onchip_node_config_t node_config;

static QueueHandle_t received_messages;

static bool receive_message_finish_callback(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx);
static bool send_message_finish_callback(twai_node_handle_t handle, const twai_tx_done_event_data_t *edata, void *user_ctx);

static can_modes_t can_mode;

esp_err_t start_can_bus(const gpio_num_t tx, const gpio_num_t rx, can_modes_t mode) {  

    memset(&node_config, 0, sizeof(node_config));

    const uint8_t queue_len = CAN_STRUCT_QUEUE;

    received_messages = xQueueCreate(queue_len, sizeof(can_message_t));

    if(received_messages == NULL)
        return ESP_FAIL;

    twai_event_callbacks_t user_cbs = {
        .on_rx_done = receive_message_finish_callback,
        .on_tx_done = send_message_finish_callback
    };

    node_config.io_cfg.tx = tx;
    node_config.io_cfg.rx = rx;
    node_config.bit_timing.bitrate = CAN_SPEED;
    node_config.tx_queue_depth = TX_QUEUE_LENGHT;

    can_mode = mode;

    if(mode == SELF_TEST)
        node_config.flags.enable_self_test = 1;
    else if(mode == LOOPBACK)
        node_config.flags.enable_loopback = 1;
    else if(mode == LISTEN_ONLY)
        node_config.flags.enable_listen_only = 1;


    if(twai_new_node_onchip(&node_config, &node_hdl) != ESP_OK)
        return ESP_FAIL;
    if(twai_node_register_event_callbacks(node_hdl, &user_cbs, NULL) != ESP_OK)
        return ESP_FAIL;
    if(twai_node_enable(node_hdl) != ESP_OK)
        return ESP_FAIL;

    return ESP_OK;
}

static uint8_t get_dlc(can_message_t * can_message) {
    return (can_message->flags_with_DLC >> 4) & 0x0F;
}

static bool is_remote_frame(can_message_t * can_message) {
    return (bool)(can_message->flags_with_DLC & 0x01);
}

static bool is_FD(can_message_t * can_message) {
    return (can_message->flags_with_DLC & 0x02) >> 1;
}

esp_err_t read_message(can_message_t *can_message) {
    if (received_messages == NULL)
        return ESP_FAIL;
    if(xQueueReceive(received_messages, can_message, pdMS_TO_TICKS(100)))
        return ESP_OK;
    return ESP_FAIL;
}

esp_err_t write_message(can_message_t * can_message) {

    if(can_mode == LISTEN_ONLY)
        return ESP_FAIL;

    uint8_t send_buff[8] = {0};

    uint8_t dlc = get_dlc(can_message);

    if(dlc > 8)
        return ESP_FAIL;

    for(uint8_t i=0; i<dlc; i++)
        send_buff[i] = (uint8_t)((can_message->data >> 8 * i) & 0xFF);

    twai_frame_t message = {
        .header.id = can_message->id,           // Message ID
        .header.ide = EXTENDED_IDE,         // Use 29-bit extended ID format
        .buffer = send_buff,        // Pointer to data to transmit
        .buffer_len = dlc,  // Length of data to transmit
    };

    return twai_node_transmit(node_hdl, &message, 500);
}


static bool receive_message_finish_callback(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    uint8_t recv_buff[8] = {0};
    
    twai_frame_t rx_frame = {
        .buffer = recv_buff,
        .buffer_len = sizeof(recv_buff),
    };

    if (ESP_OK == twai_node_receive_from_isr(handle, &rx_frame)) {
        
        can_message_t can_message = {0};

        can_message.id = rx_frame.header.id;

        can_message.flags_with_DLC |= (rx_frame.header.rtr & 0x01);
        can_message.flags_with_DLC |= ((rx_frame.header.fdf << 1) & 0x02);
        can_message.flags_with_DLC |= ((rx_frame.header.esi << 2) & 0x04);
        can_message.flags_with_DLC |= ((rx_frame.header.brs << 3) & 0x08);
        can_message.flags_with_DLC |= ((rx_frame.header.dlc & 0x0F) << 4);

        can_message.data = 0;

        for(uint8_t i=0; i<rx_frame.header.dlc; i++)
            can_message.data |= (uint64_t)recv_buff[i] << (8 * i);

        if (xQueueSendFromISR(received_messages, &can_message, &xHigherPriorityTaskWoken) != pdTRUE) {
            // overflow (opcional: contador de erro)
        }
    }
    return false;
}

static bool send_message_finish_callback(twai_node_handle_t handle, const twai_tx_done_event_data_t *edata, void *user_ctx)
{
    printf("Frame sends.");
    return false;
}