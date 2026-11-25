#include "camera.h"
#include "jpeg_decoder.h"
#include "esp_log.h"

static const char *TAG = "camera";

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,   // QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 12, // 0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 2,      // When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    .jpeg_buffer_size = 0, // Set to a custom size in bytes for JPEG mode, or 0 to use the default size
};

esp_err_t Camera_Init(void)
{
    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        // 初始化失败必须把错误码交回调用方:
        // 带着"半死"的摄像头继续跑, 后面 fb_get 全是 NULL, 错误会层层放大
        ESP_LOGE(TAG, "Camera Init Failed 0x%x", err);
        return err;
    }
    ESP_LOGI(TAG, "Camera Init OK");
    return ESP_OK;
}

camera_fb_t *Camera_Capture(void)
{
    camera_fb_t *pic = esp_camera_fb_get();
    if (pic == NULL)
    {
        // 拿不到帧就返回 NULL, 判空是调用方的责任;
        // 这里若直接用, 调用方解引用 fb->buf 就是 LoadProhibited panic
        ESP_LOGE(TAG, "Camera Capture Failed (fb == NULL)");
    }
    return pic;
}

esp_err_t Camera_jpeg2rgb565(uint8_t *jpeg_data, uint32_t jpeg_len, uint8_t *jpeg_buffer)
{
    // JPEG decode config
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = jpeg_data,
        .indata_size = jpeg_len,
        .outbuf = jpeg_buffer,
        .outbuf_size = 320 * 240 * sizeof(uint16_t),
        .out_format = JPEG_IMAGE_FORMAT_RGB565,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .flags = {
            .swap_color_bytes = 1,
        }};

    // JPEG decode
    // 返回值必须接: 解码失败时 outbuf 内容不可信, 直接刷屏会花屏
    esp_jpeg_image_output_t outimg = {0};
    esp_err_t err = esp_jpeg_decode(&jpeg_cfg, &outimg);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "JPEG decode Failed 0x%x", err);
        return err;
    }
    return ESP_OK;
}

void Camera_Return_Fb(camera_fb_t *pic)
{
    esp_camera_fb_return(pic);
}
