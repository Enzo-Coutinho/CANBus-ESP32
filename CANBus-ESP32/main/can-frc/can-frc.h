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

typedef struct {
    uint8_t deviceNumber;
    uint8_t apiIndex;
    uint8_t apiClass;
    uint8_t manufacturer;
    uint8_t deviceType;
} can_ide_t;

typedef struct {
    can_ide_t canIde;
    uint8_t data[8];
} can_message_t;

void set_can_mode(enum CAN_MODES mode);

void start_can_bus(const gpio_num_t tx, const gpio_num_t rx);

void send_message(can_message_t * can_message);
void read_message(can_message_t * can_message);

#endif