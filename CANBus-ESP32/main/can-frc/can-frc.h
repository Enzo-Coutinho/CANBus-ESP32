#ifndef CAN_FRC_H
#define CAN_FRC_H

#include <inttypes.h>
#include <stdbool.h>
#include <driver/gpio.h>
#include "esp_log.h"

#define CAN_STRUCT_LENGHT_BYTES 12

typedef struct {
    uint32_t ide;
    uint8_t flags_with_DLC;
    uint64_t data;
} can_message_t;

esp_err_t start_can_bus(const gpio_num_t tx, const gpio_num_t rx);

esp_err_t read_message(can_message_t * can_message);

#endif