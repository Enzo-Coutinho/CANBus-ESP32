#ifndef CAN_FRC_H
#define CAN_FRC_H

#include <inttypes.h>
#include <stdbool.h>
#include <driver/gpio.h>
#include "esp_log.h"

typedef enum {
    DEFAULT,
    LISTEN_ONLY,
    LOOPBACK,
    SELF_TEST
} can_modes_t;

typedef struct {
    uint32_t id;
    uint8_t flags_with_DLC;
    uint64_t data;
} can_message_t;

esp_err_t start_can_bus(const gpio_num_t tx, const gpio_num_t rx, can_modes_t mode);

esp_err_t read_message(can_message_t * can_message);

#endif