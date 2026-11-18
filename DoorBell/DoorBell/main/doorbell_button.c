#include "doorbell_button.h"

/* 日志 tag：同模块内所有 ESP_LOGx 共用，方便在监视器里过滤 */
static const char *TAG = "doorbell_button";

button_handle_t sw2_adc_btn;
button_handle_t sw3_adc_btn;

void doorbell_button_init(void)
{
    button_config_t sw2_btn_cfg = {0};     // 这个结构体 里面成员长按时间和短按时间都为0，表示使用默认值
    button_adc_config_t sw2_btn_adc_cfg = {// sw2这个按键是adc1的通道7
                                           .unit_id = ADC_UNIT_1,
                                           .adc_channel = 7,
                                           .button_index = 0,
                                           .min = 0,
                                           .max = 500};
    sw2_adc_btn = NULL;
    iot_button_new_adc_device(&sw2_btn_cfg, &sw2_btn_adc_cfg, &sw2_adc_btn);
    if (NULL == sw2_adc_btn)
    {
        ESP_LOGE(TAG, "Button sw2 create failed");
    }
    // 创建按键sw3
    button_config_t sw3_btn_cfg = {0};     // 这个结构体 里面成员长按时间和短按时间都为0，表示使用默认值
    button_adc_config_t sw3_btn_adc_cfg = {// sw3这个按键是adc1的通道7
                                           .unit_id = ADC_UNIT_1,
                                           .adc_channel = 7,
                                           .button_index = 1,
                                           .min = 1300,
                                           .max = 1800};
    sw3_adc_btn = NULL;
    iot_button_new_adc_device(&sw3_btn_cfg, &sw3_btn_adc_cfg, &sw3_adc_btn);
    if (NULL == sw3_adc_btn)
    {
        ESP_LOGE(TAG, "Button sw3 create failed");
    }
}

// 给sw2按键注册回调函数
void doorbell_button_sw2_register_callback(button_event_t button_event, button_cb_t callback, void *usr_data)
{
   
    iot_button_register_cb(sw2_adc_btn, button_event,NULL, callback, usr_data);
}

// 给sw3按键注册回调函数
void doorbell_button_sw3_register_callback(button_event_t button_event, button_cb_t callback, void *usr_data)
{
    
    iot_button_register_cb(sw3_adc_btn, button_event,NULL, callback, usr_data);
}


