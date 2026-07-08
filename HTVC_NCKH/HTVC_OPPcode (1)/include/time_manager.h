#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

class TimeManager {
private:
    RTC_DS1307 _rtc;
public:
    int getHour() { return _rtc.now().hour(); }
    int getMinute() { return _rtc.now().minute(); }
    int getSecond() { return _rtc.now().second(); }
    int getDay() { return _rtc.now().day(); }
    int getMonth() { return _rtc.now().month(); }
    int getYear() { return _rtc.now().year(); }
    bool begin();
    String getTimeString(); // Hàm lấy chuỗi thời gian cho JSON
    void displayCurrentTime(bool pumpState);
    bool isTimeToPump(int h, int m, int s);
};

#endif