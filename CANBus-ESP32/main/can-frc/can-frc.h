#ifndef CAN_FRC_H
#define CAN_FRC_H

#include <inttypes.h>
#include <driver/gpio.h>

void start_can_bus(const gpio_num_t tx, const gpio_num_t rx);
void send_message(void);
int read_message();

#endif