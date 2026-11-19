#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "doorbell_button.h"
#include "led.h"

void button_callback(void *btn_handle, void *usr_data)
{
    // 三个事件 sw2单击 sw3点击 长按
    int event = (int)(intptr_t)usr_data;

    if (event == 1)
    {
        ESP_LOGI("button_callback", "sw2 single click");
        led_toggle();
    }
    if (event == 2)
    {
        ESP_LOGI("button_callback", "sw3 single click");
    }
    if (event == 3)
    {
        ESP_LOGI("button_callback", "sw3 long press");
    }
}

void app_main(void)
{
    doorbell_button_init();
    // 注册sw2按键的回调函数
    doorbell_button_sw2_register_callback(BUTTON_SINGLE_CLICK, button_callback, (void *)1);
    // 注册sw3按键的回调函数
    doorbell_button_sw3_register_callback(BUTTON_SINGLE_CLICK, button_callback, (void *)2);
    doorbell_button_sw3_register_callback(BUTTON_LONG_PRESS_START, button_callback, (void *)3);
}
