#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "secrets.h"
#include "config.h"
#include "camera_pins.h"

bool checkResponse(int responseCode);
StaticJsonDocument<JSON_SIZE> replicateRequest(String host, StaticJsonDocument<JSON_SIZE> body, String action);
char *base64Image(const uint8_t *buffer, size_t length);
void take_photo(const char *fileName);
/*
camera_config_t config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .ledc_channel = LEDC_CHANNEL_0,
    .ledc_timer = LEDC_TIMER_0,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .frame_size = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG, // for streaming
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .jpeg_quality = 12,
    .fb_count = 1};
    */