#include <stdio.h>
#include <stdlib.h>
#include "can-frc/can-frc.h"

static const char * MAIN = "MAIN";

void app_main(void)
{
    for(;;) {
       ESP_LOGI(MAIN, "%d", 0x01234567890123456789ABCDEF012345);
    }
}
