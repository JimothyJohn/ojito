// https://raw.githubusercontent.com/limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera/main/take_photos/take_photos.ino
#include <Arduino.h>
#include "WiFi.h"
#include "Ojito.h"
#include "secrets.h"

// DO NOT add a / to the end of the URL path!
#define HOST "http://10.0.0.31/predictions"
// #define HOST "https://api.replicate.com/v1/predictions"
#define MODEL_VERSION "0239647164ce56b643213501d63f6116a76a3c620e4194814968e8ba1aa64cb7"

TaskHandle_t InferenceHandle;
unsigned long lastCaptureTime = 0; // Last shooting time
bool camera_sign = false;          // Check camera status
bool sd_sign = false;              // Check sd status

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

// Ojito ojito(REPLICATE_TOKEN, HOST, MODEL_VERSION);
Ojito ojito(REPLICATE_TOKEN);

void InferenceTask(void *pvParameters)
{
    for (;;)
    {
        // Camera & SD available, start taking pictures
        if (WiFi.status() == WL_CONNECTED)
        {
            // Analyze environment
            DynamicJsonDocument prediction(512);
            prediction = ojito.predict();
            Serial.print("Prediction: ");
            serializeJsonPretty(prediction, Serial);
            Serial.println();
            /*
            // Iterate through the array
            JsonArray predictions = prediction.as<JsonArray>();
            for (JsonVariant v : predictions)
            {
                JsonArray innerArr = v.as<JsonArray>();
                const char *item = innerArr[1]; // The second element - string
                float confidence = innerArr[2]; // The third element - confidence

                Serial.print("Item: ");
                Serial.println(item);
                Serial.print("Confidence: ");
                Serial.println(confidence, 6); // 6 decimals for precision
            }
            */
        }
        else
        {
            Serial.println("WiFi or camera disconnected");
        }
        delay(500);
    }
}

void setup()
{
    Serial.begin(115200);
    // Startup delay
    delay(2000);
    // while (!Serial); // When the serial monitor is turned on, the program starts to execute

    WiFi.begin(SSID, PASSWORD);
    WiFi.setSleep(false);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    camera_sign = setupCamera(config);

    // sd_sign = setupSD();

    xTaskCreatePinnedToCore(
        InferenceTask,    /* Task function. */
        "Inference",      /* name of task. */
        10000,            /* Stack size of task */
        NULL,             /* parameter of the task */
        1,                /* priority of the task */
        &InferenceHandle, /* Task handle to keep track of created task */
        0);               /* pin task to core 0 */

    // Set your endpoint, local or cloud
    ojito.host = HOST;
    ojito.version = MODEL_VERSION;
}

void loop() {}
