#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "MPU9250.h"

MPU9250 mpu;
const int flexPin1 = A0;
const int flexPin2 = A3;
const int flexPin3 = A6;
const int flexPin4 = A7;
const int flexPin5 = A4;
const int minSensorValue = 100;
const int maxSensorValue = 900;

int flexValue1Global;
int flexValue2Global;
int flexValue3Global;
int flexValue4Global;
int flexValue5Global;

const char* ssid = "iPhone";
const char* password = "qwerty12345";

AsyncWebServer server(80);

void setup() {
    Serial.begin(9600);
    Wire.begin();
    delay(2000);

    if (!mpu.setup(0x68)) {
        while (1) {
            Serial.println("Koneksi MPU gagal. Periksa koneksi dengan contoh `connection_check`.");
            delay(5000);
        }
    }

    Serial.println("Mencoba terhubung ke ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Terhubung ke WiFi berhasil");
    Serial.print("IP didapat: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);

    server.begin();
    Serial.println("Server HTTP dimulai");
    delay(100);
}

void loop() {
    if (mpu.update()) {
        static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 100) {
            updateSensorValues();
            prev_ms = millis();
        }
    }
}

void updateSensorValues() {
    int flexValue1 = analogRead(flexPin1);
    int flexValue2 = analogRead(flexPin2);
    int flexValue3 = analogRead(flexPin3);
    int flexValue4 = analogRead(flexPin4);
    int flexValue5 = analogRead(flexPin5);

    flexValue1 = map(flexValue1, minSensorValue, maxSensorValue, 0, 100);
    flexValue2 = map(flexValue2, minSensorValue, maxSensorValue, 0, 100);
    flexValue3 = map(flexValue3, minSensorValue, maxSensorValue, 0, 100);
    flexValue4 = map(flexValue4, minSensorValue, maxSensorValue, 0, 100);
    flexValue5 = map(flexValue5, minSensorValue, maxSensorValue, 0, 100);

    flexValue1Global = flexValue1;
    flexValue2Global = flexValue2;
    flexValue3Global = flexValue3;
    flexValue4Global = flexValue4;
    flexValue5Global = flexValue5;
}

void handleRoot(AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html>\
                    <html>\
                    <body>\
                    <h1>Server Web Pertama Saya dengan ESP32 - Mode Stasiun &#128522;</h1>\
                    <p>Yaw, Pitch, Roll: <span id='yaw'></span>, <span id='pitch'></span>, <span id='roll'></span></p>\
                    <p>Flex Values: f1: <span id='flex1'></span>, f2: <span id='flex2'></span>, f3: <span id='flex3'></span>, f4: <span id='flex4'></span>, f5: <span id='flex5'></span></p>\
                    <script>\
                    setInterval(function() {\
                      var xhttp = new XMLHttpRequest();\
                      xhttp.onreadystatechange = function() {\
                        if (this.readyState == 4 && this.status == 200) {\
                          var data = JSON.parse(this.responseText);\
                          document.getElementById('yaw').innerHTML = data.yaw;\
                          document.getElementById('pitch').innerHTML = data.pitch;\
                          document.getElementById('roll').innerHTML = data.roll;\
                          document.getElementById('flex1').innerHTML = data.flex1;\
                          document.getElementById('flex2').innerHTML = data.flex2;\
                          document.getElementById('flex3').innerHTML = data.flex3;\
                          document.getElementById('flex4').innerHTML = data.flex4;\
                          document.getElementById('flex5').innerHTML = data.flex5;\
                        }\
                      };\
                      xhttp.open('GET', '/data', true);\
                      xhttp.send();\
                    }, 1000);\
                    </script>\
                    </body>\
                    </html>";
    request->send(200, "text/html", html);
}

void handleData(AsyncWebServerRequest *request) {
    String json_data = "{\"yaw\":" + String(mpu.getYaw(), 2) +
                     ",\"pitch\":" + String(mpu.getPitch(), 2) +
                     ",\"roll\":" + String(mpu.getRoll(), 2) +
                     ",\"flex1\":" + String(flexValue1Global) +
                     ",\"flex2\":" + String(flexValue2Global) +
                     ",\"flex3\":" + String(flexValue3Global) +
                     ",\"flex4\":" + String(flexValue4Global) +
                     ",\"flex5\":" + String(flexValue5Global) + "}";

    // Set header CORS
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json_data);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}
