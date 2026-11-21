#pragma once

#include "driver/i2c_master.h"

#include "driver/i2s_std.h"
// #include "i2s_common.h"

#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"

extern i2c_master_bus_handle_t i2c0_bus_handle;

void Codec_Init(void);
void Codec_MenLing_play(void);

void Codec_Play(uint8_t *data, int size);
void Codec_Record(uint8_t *data, int size);
