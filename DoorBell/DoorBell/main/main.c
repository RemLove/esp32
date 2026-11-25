#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
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

    // 摄像头初始化失败就别进预览循环了:
    // 进去也是 fb==NULL -> fb->buf 直接 panic, 不如停在健康的任务上(按键/codec 不受影响)
    if (Camera_Init() != ESP_OK)
    {
        ESP_LOGE("app_main", "camera init failed, preview loop not started");
        return;
    }

    rgb565_buffer = (uint8_t *)calloc(240 * 320, sizeof(uint16_t));
    if (rgb565_buffer == NULL)
    {
        // 150KB 大块分配, WiFi/BT 协议栈起来后内部堆很紧张, NULL 不是不可能
        ESP_LOGE("app_main", "rgb565_buffer calloc failed, preview loop not started");
        return;
    }

    while (1)
    {
        camera_fb_t *fb = Camera_Capture();
        if (fb == NULL)
        {
            // 这一帧没拿到(驱动忙/相机异常), 停一拍再试, 别解引用
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }
        // 现在是jpeg格式，需要转成rgb565格式
        if (Camera_jpeg2rgb565(fb->buf, fb->len, rgb565_buffer) == ESP_OK)
        {
            Lcd_Show_Image(rgb565_buffer);
        }
        Camera_Return_Fb(fb);
        // CONFIG_FREERTOS_HZ=100, 裸 500 是 500 tick = 5 秒!
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    /*  while (1)
     {
         // 1.读取
         Codec_Record(data, 1024);
         // 2.播放
         Codec_Play(data, 1024);
     } */
}
