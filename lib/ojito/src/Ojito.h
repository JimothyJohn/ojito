/*
  Ojito.h - Class wrappers for Ojito library
  that allow for easily configureable data collection
  Created by Nick Armenta, August 12, 2023.
  Released into the public domain.
*/

#include <ArduinoJson.h>

// Must go above camera_pins.h
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#define JSON_SIZE 512

#include "esp_camera.h"
#include "camera_pins.h"

#if CONFIG_IDF_TARGET_ESP32S3
const int FB_COUNT = 2;
#else
const int FB_COUNT = 1;
#endif

// https://arduinojson.org/v6/how-to/use-external-ram-on-esp32/
struct Detection
{
    const char *item;
    float confidence;
};

bool checkResponse(int responseCode);
StaticJsonDocument<JSON_SIZE> replicateRequest(DynamicJsonDocument &body, const char *token, const char *action);
char *base64Image(const uint8_t *buffer, size_t length);
void take_photo();
StaticJsonDocument<JSON_SIZE> see_world(const char *token);
bool setupCamera(camera_config_t &config);
bool setupSD();

class Ojito
{
private:
    const char *_token;
    camera_fb_t *_fb;
    char *_encodedImage;
    DynamicJsonDocument _request;
    char _authorization[46];

public:
    const char *host;
    const char *version;
    StaticJsonDocument<JSON_SIZE> response;
    Ojito(const char *t);
    StaticJsonDocument<JSON_SIZE> predict();
};
