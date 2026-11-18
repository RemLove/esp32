#pragma once

#include "button_adc.h"
#include "button_gpio.h"
#include "iot_button.h"
#include "esp_log.h"

void doorbell_button_init(void);

void doorbell_button_sw2_register_callback(button_event_t button_event, button_cb_t callback, void *usr_data);

void doorbell_button_sw3_register_callback(button_event_t button_event, button_cb_t callback, void *usr_data);