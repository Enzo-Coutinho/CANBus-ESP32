#ifndef CAN_FRC_H
#define CAN_FRC_H

#include <inttypes.h>
#include <driver/gpio.h>

void start_can_bus(const gpio_num_t tx, const gpio_num_t rx, const uint32_t bitrate);
void send_message(const uint32_t id, const uint8_t data[8]);
int read_message();

#endif