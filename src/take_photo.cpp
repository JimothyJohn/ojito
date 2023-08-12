// https://raw.githubusercontent.com/limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera/main/take_photos/take_photos.ino
#include "esp_camera.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Base64.h>

#include "secrets.h"
#include "config.h"
#include "utils.h"

// char *base64Buffer = (char *)ps_malloc(100000);

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ; // When the serial monitor is turned on, the program starts to execute

    WiFi.begin(SSID, PASSWORD);
    WiFi.setSleep(false);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
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
        // TODO Best option for face detection/recognition
        config.frame_size = FRAMESIZE_240X240;
    }
    config.fb_count = 1;
    config.frame_size = FRAMESIZE_240X240;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    camera_sign = true; // Camera initialization check passes

    // Initialize SD card
    if (!SD.begin(21))
    {
        Serial.println("Card Mount Failed");
        return;
    }

    // Determine if the type of SD card is available
    if (SD.cardType() == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    sd_sign = true; // sd initialization check passes
}

void loop()
{
    // Camera & SD available, start taking pictures
    if (camera_sign && sd_sign)
    {
        // Get the current time
        unsigned long now = millis();

        // If it has been more than 1 minute since the last shot, take a picture and save it to the SD card
        if ((now - lastCaptureTime) >= 5000)
        {
            response = see_world();
            Serial.print("Prediction: ");
            serializeJson(response["output"], Serial);
            response.clear();
            Serial.println();
            lastCaptureTime = now;
        }
    }
}