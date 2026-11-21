#include "codec.h"

i2c_master_bus_handle_t i2c0_bus_handle = NULL;
static const char *TAG;
// 接收通道和发送通道
i2s_chan_handle_t i2s_rx_handle = NULL;
i2s_chan_handle_t i2s_tx_handle = NULL;

esp_codec_dev_handle_t codec_dev_handle = NULL;

extern const uint8_t menling_start[] asm("_binary_menling_pcm_start");
extern const uint8_t menling_end[] asm("_binary_menling_pcm_end");

esp_err_t Codec_I2c_Init(void)
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
    // 创建通道
    i2s_new_channel(&chan_cfg, &i2s_tx_handle, &i2s_rx_handle);
    // 初始化
    i2s_channel_init_std_mode(i2s_tx_handle, &std_cfg);
    i2s_channel_init_std_mode(i2s_rx_handle, &std_cfg);
    // 启动通道
    i2s_channel_enable(i2s_tx_handle);
    i2s_channel_enable(i2s_rx_handle);
}

void Codec_Init(void)
{
    Codec_I2c_Init();
    Codec_I2s_Init();
    // 为编解码器设备实现控制接口，数据接口和 GPIO 接口 (使用默认提供的接口实现)
    audio_codec_i2s_cfg_t i2s_cfg = {

        .rx_handle = i2s_rx_handle,
        .tx_handle = i2s_tx_handle,

    };
    const audio_codec_data_if_t *data_if = audio_codec_new_i2s_data(&i2s_cfg);

    audio_codec_i2c_cfg_t i2c_cfg = {.addr = ES8311_CODEC_DEFAULT_ADDR,
                                     .bus_handle = i2c0_bus_handle,
                                     .clock_speed_hz = 400000};
    const audio_codec_ctrl_if_t *out_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);

    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();

    // 基于控制接口和 ES8311 特有的配置实现 audio_codec_if_t 接口
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
        .ctrl_if = out_ctrl_if,
        .gpio_if = gpio_if,
        .pa_pin = GPIO_NUM_7,
        .use_mclk = true,
    };
    const audio_codec_if_t *out_codec_if = es8311_codec_new(&es8311_cfg);

    // 通过 API esp_codec_dev_new 获取 esp_codec_dev_handle_t 句柄 参考下面代码用获取到的句柄来进行播放和录制操作:
    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = out_codec_if,             // es8311_codec_new 获取到的接口实现
        .data_if = data_if,                   // audio_codec_new_i2s_data 获取到的数据接口实现
        .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT // 设备同时支持录制和播放
    };
    codec_dev_handle = esp_codec_dev_new(&dev_cfg);
    // 以下代码展示如何播放音频
    esp_codec_dev_set_out_vol(codec_dev_handle, 60.0);
    esp_codec_dev_sample_info_t fs = {
        .sample_rate = 16000,
        .channel = 2,
        .bits_per_sample = 16,
    };
    esp_codec_dev_open(codec_dev_handle, &fs);
    // 增益
    esp_codec_dev_set_in_gain(codec_dev_handle, 30.0);
}

void Codec_Play(const uint8_t *data, int size)
{
    esp_codec_dev_write(codec_dev_handle, data, size);
}

void Codec_MenLing_play(void)
{
    Codec_Play(menling_start, menling_end - menling_start);
}

void Codec_Record(uint8_t *data, int size)
{
    esp_codec_dev_read(codec_dev_handle, data, size);
}
