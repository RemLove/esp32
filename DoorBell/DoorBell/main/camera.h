#pragma once

#include "esp_camera.h"

#define BOARD_WROVER_KIT 1

#define CAM_PIN_PWDN -1  // power down is not used
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK 37

#define CAM_PIN_D7 38
#define CAM_PIN_D6 36
#define CAM_PIN_D5 35
#define CAM_PIN_D4 33
#define CAM_PIN_D3 10
#define CAM_PIN_D2 12
#define CAM_PIN_D1 11
#define CAM_PIN_D0 9
#define CAM_PIN_VSYNC 41
#define CAM_PIN_HREF 39
#define CAM_PIN_PCLK 34

void Camera_Init(void);

camera_fb_t *Camera_Capture(void);

void Camera_jpeg2rgb565(uint8_t *jpeg_data, uint32_t jpeg_len, uint8_t *jpeg_buffer);

void Camera_Return_Fb(camera_fb_t *pic);
