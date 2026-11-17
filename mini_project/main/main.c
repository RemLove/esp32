
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hello_util.h"


void app_main(void) {
  while (1) {
    hello();
    hi();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
