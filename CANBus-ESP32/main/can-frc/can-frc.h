#ifndef CAN_FRC_H
#define CAN_FRC_H

#include <inttypes.h>
#include <stdbool.h>
#include <driver/gpio.h>
#include "can-patterns.h"

typedef struct {
    uint8_t device_number;
    uint8_t api_index;
    uint8_t api_class;
    uint8_t manufacturer;
    uint8_t device_type;
} can_ide_t;

typedef struct {
    can_ide_t can_ide;
    bool is_remote_frame;
    bool is_fd_format;
    bool is_bit_rate_shift;
    bool is_error;
    uint8_t dlc;
    uint8_t *data;
    uint32_t timestamp;
} can_message_t;

esp_err_t start_can_bus(const gpio_num_t tx, const gpio_num_t rx);

esp_err_t read_message(can_message_t * can_message);

#endif