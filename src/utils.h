#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "secrets.h"
#include "config.h"
#include "camera_pins.h"

#if CONFIG_IDF_TARGET_ESP32S3
const int FB_COUNT = 2;
#else
const int FB_COUNT = 1;
#endif

unsigned long lastCaptureTime = 0; // Last shooting time
bool camera_sign = false;          // Check camera status
bool sd_sign = false;              // Check sd status

char hostPath[128];

// https://arduinojson.org/v6/how-to/use-external-ram-on-esp32/
struct SpiRamAllocator
{
    void *allocate(size_t size)
    {
        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    }

    void deallocate(void *pointer)
    {
        heap_caps_free(pointer);
    }

    void *reallocate(void *ptr, size_t new_size)
    {
        return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
    }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;

StaticJsonDocument<JSON_SIZE> response;
DynamicJsonDocument body(10000);

bool checkResponse(int responseCode);
StaticJsonDocument<JSON_SIZE> replicateRequest(DynamicJsonDocument body, String action);
char *base64Image(const uint8_t *buffer, size_t length);
void take_photo();
StaticJsonDocument<JSON_SIZE> see_world();
camera_config_t config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_UXGA,
    .jpeg_quality = 12,
    .fb_count = FB_COUNT,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    // .fb_location = CAMERA_FB_IN_PSRAM // This was not in the example you gave, but I've included it based on previous information.
};
