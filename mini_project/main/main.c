
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

void app_main(void) {
    while (1) {
        printf("hello world\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
