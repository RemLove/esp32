#pragma once

#include "driver/i2c_master.h"
#include "driver/i2s_types.h"
#include "hal/i2s_types.h"

#include "esp_types.h"
#include "esp_err.h"

// #include "i2s_common.h"

extern i2c_master_bus_handle_t i2c0_bus_handle;

void Codec_I2c_Init(void);
void Codec_I2s_Init(void);