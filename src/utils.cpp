#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Base64.h>

#include "esp_camera.h"
#include "secrets.h"
#include "config.h"

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

StaticJsonDocument<JSON_SIZE> replicateRequest(DynamicJsonDocument body, String action)
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
        http.begin(HOST);
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

char *base64Image(const uint8_t *buffer, size_t length)
{
    const char *prefix = "data:image/jpeg;base64,";
    size_t prefixLength = strlen(prefix);

    // Calculate size needed for the base64 output
    size_t base64Length = base64_enc_len(length); // Approximate size of base64 encoded data

    // Create buffer in PSRAM to avoid overloading the memory
    // char *base64Buffer = (char *)ps_malloc(base64Length + prefixLength + 1);
    char *base64Buffer = (char *)ps_malloc(base64Length + prefixLength + 1);
    if (!base64Buffer)
    {
        Serial.println("Failed to allocate base64 buffer in PSRAM");
        return "";
    }

    // Copy the prefix into the buffer
    strcpy(base64Buffer, prefix);

    // Convert the image to base64 and append it after the prefix
    base64_encode(base64Buffer + prefixLength, (char *)buffer, length);

    return base64Buffer;
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

    char *encodedImage = base64Image(fb->buf, fb->len);
    if (encodedImage)
    {
        Serial.print("Encoded Image: ");
        Serial.println(encodedImage);

        // Remember to free the encoded image buffer
        free(encodedImage);
    }

    // Release image buffer
    esp_camera_fb_return(fb);
}

StaticJsonDocument<JSON_SIZE> see_world()
{
    DynamicJsonDocument request(10000);
    StaticJsonDocument<JSON_SIZE> response;
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        Serial.println("Failed to get camera frame buffer");
        return response;
    }
    char *encodedImage = base64Image(fb->buf, fb->len);

    request["version"] = MODEL_VERSION;
    request["input"]["image"] = encodedImage;
    // body["input"]["image"] = "https://pbxt.replicate.delivery/IWHv3cYJ7CAVPFJN5M9JohLfLr2XaGxXgh5ykca1kvourUZV/taylor1.jpg";
    // serializeJson(body["input"]["image"], Serial);
    size_t base64Length = base64_enc_len(fb->len); // Approximate size of base64 encoded data

    // body["input"]["image"] = "";
    // TODO Stop rewriting memory every time

    // Release buffers
    free(encodedImage);
    esp_camera_fb_return(fb);

    return replicateRequest(request, "create");
}