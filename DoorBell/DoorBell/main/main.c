#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "doorbell_button.h"
#include "led.h"
#include "codec.h"
#include "wifi.h"
#include "lcd.h"
#include "camera.h"

uint8_t *rgb565_buffer = NULL;

void button_callback(void *btn_handle, void *usr_data)
{
    // 三个事件 sw2单击 sw3点击 长按
    int event = (int)(intptr_t)usr_data;

    if (event == 1)
    {
        ESP_LOGI("button_callback", "sw2 single click");
        led_toggle();
        // Codec_MenLing_play();
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

uint8_t data[1024];

void app_main(void)
{
    Codec_Init();
    Wifi_Init();
    Lcd_init();
    Lcd_Test();

    doorbell_button_init();
    // 注册sw2按键的回调函数
    doorbell_button_sw2_register_callback(BUTTON_SINGLE_CLICK, button_callback, (void *)1);
    // 注册sw3按键的回调函数
    doorbell_button_sw3_register_callback(BUTTON_SINGLE_CLICK, button_callback, (void *)2);
    doorbell_button_sw3_register_callback(BUTTON_LONG_PRESS_START, button_callback, (void *)3);

    Camera_Init();
    rgb565_buffer = (uint8_t *)calloc(240 * 320, sizeof(uint16_t));

    while (1)
    {
        camera_fb_t *fb = Camera_Capture();
        // 现在是jpeg格式，需要转成rgb565格式
        Camera_jpeg2rgb565(fb->buf, fb->len, rgb565_buffer);
        Lcd_Show_Image(rgb565_buffer);
        Camera_Return_Fb(fb);
        vTaskDelay(500);
    }

    /*  while (1)
     {
         // 1.读取
         Codec_Record(data, 1024);
         // 2.播放
         Codec_Play(data, 1024);
     } */
}
