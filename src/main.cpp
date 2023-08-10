// https://raw.githubusercontent.com/limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera/main/CameraWebServer/CameraWebServer.ino
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "secrets.h"
#include "config.h"

HTTPClient http;

char authorization[128];
char hostPath[128];

StaticJsonDocument<JSON_SIZE> response;
StaticJsonDocument<JSON_SIZE> body;

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
replicateRequest(String host, StaticJsonDocument<JSON_SIZE> body, const char *action)
{
    StaticJsonDocument<JSON_SIZE> httpResponse;
    httpResponse["error"] = "HTTP Client error!";
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
            http.end();
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

    strcpy(authorization, "Token ");
    strcat(authorization, REPLICATE_TOKEN);

    body["version"] = MODEL_VERSION;
}

void loop()
{
    // Do nothing. Everything is done in another task by the web server
    body["input"]["prompt"] = "answer me";
    strcpy(hostPath, HOST);
    response = replicateRequest(hostPath, body, "test");
    Serial.print("Prediction ID: ");
    serializeJson(response["id"], Serial);
    Serial.println();
    strcat(hostPath, "/");
    strcat(hostPath, response["id"]);
    while (response["status"] == "starting" || response["status"] == "processing")
    {
        delay(5000);
        response = replicateRequest(hostPath, body, "get");
    }
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
        serializeJson(response["output"], Serial);
        Serial.println();
    }
    delay(5000);
}
