#include "a301.h"
#include "can.h"

#define BASIC_ID 0x0203
#define DEVICE 0x03
#define TEMPERATURE_ID (BASIC_ID + 0x0B8 + DEVICE)
#define THROTTLE_ID (BASIC_ID + 0x08 + DEVICE)
#define ABSOLUTE_POSITION_ID (BASIC_ID + 0x0B8C + DEVICE)
#define RPM_ID (BASIC_ID + 0x0B88 + DEVICE)

