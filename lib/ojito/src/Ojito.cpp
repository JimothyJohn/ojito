#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Base64.h>

#include "SD.h"
#include "Ojito.h"

char hostPath[128];

DynamicJsonDocument body(20000);
/*
// https://arduinojson.org/v6/how-to/use-external-ram-on-esp32/
void *SpiRamAllocator::allocate(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void SpiRamAllocator::deallocate(void *pointer)
{
    heap_caps_free(pointer);
}

void *SpiRamAllocator::reallocate(void *ptr, size_t new_size)
{
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
}
*/
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

StaticJsonDocument<JSON_SIZE> replicateRequest(DynamicJsonDocument &body, const char *token, const char *action)
{
    HTTPClient http;
    char authorization[128];
    strcpy(authorization, "Token ");
    strcat(authorization, token);
    StaticJsonDocument<JSON_SIZE> httpResponse;
    httpResponse["error"] = "HTTP Client error!";
    httpResponse["status"] = "Error";
    String request;
    serializeJson(body, request);

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

    http.end();
    return httpResponse;
}

char *encode(const uint8_t *buffer, size_t length)
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
    }

    // Copy the prefix into the buffer
    strcpy(base64Buffer, prefix);

    // Convert the image to base64 and append it after the prefix
    base64_encode(base64Buffer + prefixLength, (char *)buffer, length);

    return base64Buffer;
}

StaticJsonDocument<JSON_SIZE> see_world(const char *token)
{
    DynamicJsonDocument request(10000);
    StaticJsonDocument<JSON_SIZE> response;
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        Serial.println("Failed to get camera frame buffer");
        return response;
    }
    char *encodedImage = encode(fb->buf, fb->len);

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

    return replicateRequest(request, token, "create");
}

bool setupCamera(camera_config_t &config)
{
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
        return false;
    }

    return true; // Camera initialization check passes
}

bool setupSD()
{
    // Initialize SD card
    if (!SD.begin(21))
    {
        Serial.println("Card Mount Failed");
        return false;
    }

    // Determine if the type of SD card is available
    if (SD.cardType() == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return false;
    }

    return true; // SD initialization check passes
}

Ojito::Ojito(const char *t) : _request(10000)
{
    _token = t;
}

void Ojito::setHost(const char *h)
{
    host = h;
}

void Ojito::setModel(const char *v)
{
    version = v;
}

StaticJsonDocument<JSON_SIZE> Ojito::take_photo()
{
    // You'll fill this with logic to take a photo using whatever method/hardware you have.
    // For instance, if you're using the ESP32 camera module, you'll call appropriate functions here.
    _fb = esp_camera_fb_get();
    if (!_fb)
    {
        Serial.println("Failed to get camera frame buffer");
        return response;
    }
    // Release image buffer
    _encodedImage = encode(_fb->buf, _fb->len);
    esp_camera_fb_return(_fb);

    if (host == "")
    {
        Serial.println("Host not set");
        return response;
    }
    if (version == "")
    {
        Serial.println("Version not set");
        return response;
    }

    _request.clear();
    _request["version"] = version;
    _request["input"]["image"] = _encodedImage;
    free(_encodedImage);

    HTTPClient http;
    // TODO Prevent rewriting of this constant
    char authorization[128];
    strcpy(authorization, "Token ");
    strcat(authorization, _token);
    response.clear();
    response["error"] = "HTTP Client error!";
    response["status"] = "Error";
    String request;
    serializeJson(_request, request);

    http.begin(HOST);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", authorization);

    uint8_t responseCode = 404;
    responseCode = http.POST(request);

    deserializeJson(response, http.getString());
    http.end();
    return response["output"];
}