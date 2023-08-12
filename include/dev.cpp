// https://raw.githubusercontent.com/limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera/main/CameraWebServer/CameraWebServer.ino
#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "SPI.h"

#include "secrets.h"
#include "config.h"
#include "camera_pins.h"
// #include "utils.h"

unsigned long lastCaptureTime = 0; // Last shooting time
int imageCount = 1;                // File Counter
bool camera_sign = false;          // Check camera status
bool sd_sign = false;              // Check sd status

char hostPath[128];

StaticJsonDocument<JSON_SIZE> response;
StaticJsonDocument<JSON_SIZE> body;

const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(char *output, const uint8_t *input, size_t inputLen)
{
    int i = 0;
    int j = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    while (inputLen--)
    {
        char_array_3[i++] = *(input++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                output[j++] = base64_alphabet[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (int k = i; k < 3; k++)
            char_array_3[k] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int k = 0; k < i + 1; k++)
            output[j++] = base64_alphabet[char_array_4[k]];

        while (i++ < 3)
            output[j++] = '=';
    }
    output[j] = '\0'; // null-terminate the string
}

// Save pictures to SD card
void take_photo()
{
    // Take a photo
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Failed to get camera frame buffer");
        return;
    }
    // Save buffer to variable for return statement
    uint8_t *imageBuffer = fb->buf;
    size_t len = sizeof(fb->buf);
    Serial.print("Length: ");
    Serial.println(len);

    // Size for base64 is roughly 4/3 of the original
    size_t base64_len = 4 * ((len + 2) / 3);
    char encodedData[base64_len + 1]; // +1 for the null terminator

    base64_encode(encodedData, imageBuffer, len);
    Serial.println(encodedData);

    // Release image buffer
    esp_camera_fb_return(fb);

    Serial.println("Photo taken!");
}

bool checkResponse(int responseCode)
{
    if (responseCode == HTTP_CODE_NO_CONTENT)
    {
        Serial.println("No Content");
    }
    else if (responseCode == HTTP_CODE_NOT_FOUND)
    {
        Serial.println("Not Found");
    }
    else if (responseCode == HTTP_CODE_FORBIDDEN)
    {
        Serial.println("Forbidden");
    }
    else if (responseCode == HTTP_CODE_INTERNAL_SERVER_ERROR)
    {
        Serial.println("Internal Server Error");
    }
    else if (responseCode == HTTP_CODE_SERVICE_UNAVAILABLE)
    {
        Serial.println("Service Unavailable");
    }
    else if (responseCode == HTTP_CODE_GATEWAY_TIMEOUT)
    {
        Serial.println("Gateway Timeout");
    }
    else if (responseCode == HTTP_CODE_CREATED || responseCode == HTTP_CODE_OK)
    {
        return true;
    }
    else
    {
        Serial.print("Error on sending POST: ");
        Serial.println(responseCode);
    }
    return false;
}

StaticJsonDocument<JSON_SIZE>
replicateRequest(String host, StaticJsonDocument<JSON_SIZE> body, String action)
{
    HTTPClient http;
    char authorization[128];
    strcpy(authorization, "Token ");
    strcat(authorization, REPLICATE_TOKEN);
    StaticJsonDocument<JSON_SIZE> httpResponse;
    httpResponse["error"] = "HTTP Client error!";
    httpResponse["status"] = "Error";
    String request;
    serializeJson(body, request);

    if (WiFi.status() == WL_CONNECTED)
    {
        http.begin(host);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", authorization);

        int httpResponseCode = 404;
        if (action == "create")
        {
            httpResponseCode = http.POST(request);
        }
        else if (action == "get")
        {
            httpResponseCode = http.GET();
        }
        else
        {
            Serial.println("Invalid action");
        }

        if (!checkResponse(httpResponseCode))
        {
            httpResponse["code"] = httpResponseCode;
            // http.end();
            return httpResponse;
        }

        deserializeJson(httpResponse, http.getString());
    }
    else
    {
        Serial.println("WiFi Disconnected");
    }
    http.end();
    return httpResponse;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    WiFi.begin(SSID, PASSWORD);
    WiFi.setSleep(false);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG; // for streaming
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if (config.pixel_format == PIXFORMAT_JPEG)
    {
        if (psramFound())
        {
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        }
        else
        {
            // Limit the frame size when PSRAM is not available
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }
    }
    else
    {
        // Best option for face detection/recognition
        config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
        config.fb_count = 2;
#endif
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    camera_sign = true; // Camera initialization check passes

    Serial.println("Photos will begin in one minute, please be ready.");
}

void loop()
{
    // Camera & SD available, start taking pictures
    if (camera_sign)
    {
        take_photo();
    }
    response.clear();
    body.clear();
    body["version"] = MODEL_VERSION;
    body["input"]["prompt"] = "answer me";
    strcpy(hostPath, HOST);
    strcat(hostPath, "/v1/predictions");
    response = replicateRequest(hostPath, body, "create");
    Serial.print("Prediction: ");
    serializeJson(response, Serial);
    Serial.println();
    if (response["status"] == "starting")
    {
        strcat(hostPath, "/");
        strcat(hostPath, response["id"]);
    }
    while (response["status"] == "starting" || response["status"] == "processing")
    {
        delay(5000);
        Serial.print(".");
        response = replicateRequest(hostPath, body, "get");
    }
    Serial.println();
    if (response["status"] == "failed")
    {
        Serial.println("Failed!");
    }
    else if (response["status"] == "canceled")
    {
        Serial.println("Canceled!");
    }
    else if (response["status"] == "succeded")
    {
        response = replicateRequest(hostPath, body, "get");
    }
    if (response["ouput"] != "")
    {
        Serial.print("Result: ");
        serializeJson(response, Serial);
        Serial.println();
    }
    delay(15000);
}
