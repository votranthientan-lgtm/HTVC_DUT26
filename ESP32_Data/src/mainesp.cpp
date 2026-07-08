#include <Arduino.h>
#include "esp_data.h"

esp_data myData;

void setup() {
    Serial.begin(115200);
    myData.begin(115200);
    Serial.println(F("ESP32: SAN SANG NHAN DU LIEU TU MEGA..."));
}

void loop() {
    myData.update();
}