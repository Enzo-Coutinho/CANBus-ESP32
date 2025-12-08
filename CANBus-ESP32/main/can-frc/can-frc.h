#ifndef CAN_FRC_H
#define CAN_FRC_H

#include <inttypes.h>
#include <driver/gpio.h>

enum CAN_MODES {
    LOOPBACK,
    SELF_TEST,
    LISTEN_ONLY,
    NO_RECEITE_RTR,
    DEFAULT
};

struct can_message_t {

};

void set_can_mode(enum CAN_MODES mode);

void start_can_bus(const gpio_num_t tx, const gpio_num_t rx);

void send_message(void);
int read_message();

#endif