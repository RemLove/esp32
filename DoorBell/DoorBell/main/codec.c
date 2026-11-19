#include "codec.h"

i2c_master_bus_handle_t i2c0_bus_handle = NULL;

void Codec_I2c_Init(void)
{
    // Implementation for codec I2C initialization
    i2c_master_bus_config_t i2c_bus_config = {0};
    i2c_bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_bus_config.i2c_port = 0;            // Assuming I2C port 0 for codec
    i2c_bus_config.scl_io_num = GPIO_NUM_1; // Example SCL pin
    i2c_bus_config.sda_io_num = GPIO_NUM_0; // Example SDA
    i2c_bus_config.glitch_ignore_cnt = 7;
    i2c_bus_config.flags.enable_internal_pullup = true;
    return i2c_new_master_bus(&i2c_bus_config, &i2c0_bus_handle);
}

void Codec_I2s_Init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(16, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_3,
            .bclk = GPIO_NUM_2,
            .ws = GPIO_NUM_5,
            .dout = GPIO_NUM_6,
            .din = GPIO_NUM_4,
        },
    };
}
