#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class DisplayManager {
public:
    DisplayManager(uint8_t lcd_Addr = 0x27, uint8_t lcd_cols = 20, uint8_t lcd_rows = 4);
    
    void begin();
    
    // Đã bỏ ph và tW, thêm trạng thái fan
    void update(int floor, float tds, float hum, float tAir, bool pT, bool led, bool fan);

    void printf(uint8_t row, uint8_t col, const char* format, ...);

private:
    LiquidCrystal_I2C _lcd;
};

#endif