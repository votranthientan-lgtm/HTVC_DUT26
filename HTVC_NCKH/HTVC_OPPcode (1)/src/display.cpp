#include "display.h"
#include <stdarg.h>

DisplayManager::DisplayManager(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows) 
    : _lcd(lcd_Addr, lcd_cols, lcd_rows) {}

void DisplayManager::begin() {
    _lcd.init();
    _lcd.backlight();
    _lcd.clear();
}

void DisplayManager::printf(uint8_t row, uint8_t col, const char* format, ...) {
    char buffer[21]; 
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    _lcd.setCursor(col, row);
    _lcd.print(buffer);
}

void DisplayManager::update(int floor, float tds, float hum, float tAir, bool pT, bool led, bool fan) {
    char lineBuf[21];

    // Hàng 0: Chỉ hiển thị Tên tầng ra giữa màn hình
    snprintf(lineBuf, sizeof(lineBuf), "       TANG %d       ", floor);
    _lcd.setCursor(0, 0);
    _lcd.print(lineBuf);

    // Hàng 1: Nhiệt độ không khí & Độ ẩm
    snprintf(lineBuf, sizeof(lineBuf), "Temp:%2dC | Hum:%2d%% ", (int)tAir, (int)hum);
    _lcd.setCursor(0, 1);
    _lcd.print(lineBuf);

    // Hàng 2: TDS và Trạng thái Bơm
    snprintf(lineBuf, sizeof(lineBuf), "TDS:%3d | Pump:%-3s", (int)tds, pT ? "ON" : "OFF");
    _lcd.setCursor(0, 2);
    _lcd.print(lineBuf);

    // Hàng 3: Trạng thái Đèn và Quạt
    snprintf(lineBuf, sizeof(lineBuf), "LED:%-3s  | Fan :%-3s", led ? "ON" : "OFF", fan ? "ON" : "OFF");
    _lcd.setCursor(0, 3);
    _lcd.print(lineBuf);
}