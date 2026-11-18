#include "doorbell_button.h"

void doorbell_button_init(void)
{
    button_config_t sw2_btn_cfg = {0};     // 这个结构体 里面成员长按时间和短按时间都为0，表示使用默认值
    button_adc_config_t sw2_btn_adc_cfg = {// sw2这个按键是adc1的通道7
                                           .unit_id = ADC_UNIT_1,
                                           .adc_channel = 7,
                                           .button_index = 0,
                                           .min = 0,
                                           .max = 500};
    button_handle_t sw2_adc_btn = NULL;
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
    button_handle_t sw3_adc_btn = NULL;
    iot_button_new_adc_device(&sw3_btn_cfg, &sw3_btn_adc_cfg, &sw3_adc_btn);
    if (NULL == sw3_adc_btn)
    {
        ESP_LOGE(TAG, "Button sw3 create failed");
    }
}
