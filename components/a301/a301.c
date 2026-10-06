#include "a301.h"
#include "can.h"

#define BASIC_ID 0x0203
#define DEVICE 0x03
#define TEMPERATURE_ID (BASIC_ID + 0x0B8 + DEVICE)
#define THROTTLE_ID (BASIC_ID + 0x08 + DEVICE)
#define ABSOLUTE_POSITION_ID (BASIC_ID + 0x0B8C + DEVICE)
#define RPM_ID (BASIC_ID + 0x0B88 + DEVICE)

void setThrottle(float power) {
    uint32_t power_int = memcpy(&power, &power_int, 4);

    can_message_t can_message = {.id = THROTTLE_ID, .data = power_int};

    write_message(&can_message);
}

int getTemperature() {
    can_message_t can_message = {0}

    write_message(&can_message);
}