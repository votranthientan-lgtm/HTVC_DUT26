#include "time_manager.h"

bool TimeManager::begin() {
    Wire.begin();
    if (!_rtc.begin()) return false;

    // ÉP BUỘC CẬP NHẬT GIỜ MÁY TÍNH (Bỏ comment dòng này)
    //_rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 

    if (!_rtc.isrunning()) {
        Serial.println(F("RTC bi mat nguon..."));
    }
    
    return true;
}

// Hàm này cực kỳ quan trọng để gửi dữ liệu sang ESP32
String TimeManager::getTimeString() {
    DateTime now = _rtc.now();
    char buffer[20];
    // Tạo chuỗi định dạng: DD/MM/YYYY HH:MM:SS
    sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d", 
            now.day(), now.month(), now.year(), 
            now.hour(), now.minute(), now.second());
    return String(buffer);
}

void TimeManager::displayCurrentTime(bool pumpState) {
    DateTime now = _rtc.now();
    char fullTime[64];
    sprintf(fullTime, "%02d/%02d/%04d %02d:%02d:%02d | TRANG THAI: %s", 
            now.day(), now.month(), now.year(), 
            now.hour(), now.minute(), now.second(),
            pumpState ? "ON" : "OFF");
            
    Serial.println(fullTime);
}

bool TimeManager::isTimeToPump(int h, int m, int s) {
    DateTime now = _rtc.now();
    return (now.hour() == h && now.minute() == m && now.second() == s);
}