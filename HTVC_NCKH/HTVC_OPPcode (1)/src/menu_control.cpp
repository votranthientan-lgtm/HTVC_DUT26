#include "menu_control.h"
#include "pump_control.h" 
#include "led_control.h"
#include "fan.h"
#include "ph_temp_sensor.h" 

// Gọi mảng thiết bị và cảm biến từ mainmega sang
extern PumpControl* pumps[];
extern LedControl* leds[];
extern FanControl* fans[];
extern PhTempSensor myPhSensor; 

int floorPlant[3] = {-1, -1, -1};
int floorDay[3] = {0, 0, 0};      
int floorStage[3] = {0, 0, 0};    
PlantProfile _plantLib[3];

MenuControl::MenuControl(uint8_t addr, uint8_t cols, uint8_t rows, uint8_t up, uint8_t down, uint8_t back, uint8_t ok) {
    _lcd = new LiquidCrystal_I2C(addr, cols, rows);
    _pinUp = up; _pinDown = down; _pinBack = back; _pinOk = ok;
    _currentIndex = 0;
    _currentState = TRANG_THAI_CHUNG; // Sét trạng thái mặc định ban đầu là Trạng Thái Chung
}

void MenuControl::begin() {
    _lcd->init();
    _lcd->backlight();
    
    pinMode(_pinUp, INPUT_PULLUP);
    pinMode(_pinDown, INPUT_PULLUP);
    pinMode(_pinBack, INPUT_PULLUP);
    pinMode(_pinOk, INPUT_PULLUP);

    // Dữ liệu mảng cây trồng
    _plantLib[0].stages[0] = {7, 2, {8, 17}, 10, 12};
    _plantLib[0].stages[1] = {25, 5, {6, 9, 12, 15, 18}, 20, 14};
    _plantLib[0].stages[2] = {5, 3, {7, 12, 17}, 15, 12};

    _plantLib[1].stages[0] = {10, 2, {8, 17}, 10, 10};
    _plantLib[1].stages[1] = {20, 4, {7, 11, 15, 19}, 15, 12};
    _plantLib[1].stages[2] = {5, 3, {7, 12, 17}, 15, 10};

    _plantLib[2].stages[0] = {7, 3, {7, 12, 17}, 5, 12};
    _plantLib[2].stages[1] = {20, 7, {6, 9, 11, 13, 15, 17, 19}, 10, 15};
    _plantLib[2].stages[2] = {3, 5, {6, 9, 12, 15, 18}, 10, 12};

    _lcd->clear();
}

// --- CÁC HÀM HIỂN THỊ ---
void MenuControl::displayGeneralStatus(float ph, float tempWater) {
    _lcd->setCursor(0, 0); _lcd->print("-- TRANG THAI CHUNG --");
    
    _lcd->setCursor(0, 1); _lcd->print("pH Water : "); 
    _lcd->print(ph, 1); _lcd->print("  "); 
    
    _lcd->setCursor(0, 2); _lcd->print("TempWater: "); 
    _lcd->print(tempWater, 1); _lcd->print(" C  ");
    
    _lcd->setCursor(0, 3); _lcd->print(">> Nhan OK de vao <<");
}

void MenuControl::displayMainMenu() {
    _lcd->clear();
    _lcd->setCursor(0, 0); _lcd->print(_currentIndex == 0 ? "> 1.Manual Setting" : "  1.Manual Setting");
    _lcd->setCursor(0, 1); _lcd->print(_currentIndex == 1 ? "> 2.Auto Setting"   : "  2.Auto Setting");
}

void MenuControl::displayNormalFloor() {
    _lcd->clear();
    _lcd->setCursor(0, 0); _lcd->print("--- MANUAL MODE ---");
    _lcd->setCursor(0, 1); _lcd->print("Chon Tang: "); _lcd->print(_currentIndex + 1);
}

void MenuControl::displayNormalControl() {
    _lcd->clear();
    _lcd->setCursor(0, 0); _lcd->print("Tang "); _lcd->print(_selectedFloor + 1); _lcd->print(" (OK:Toggle)");

    _lcd->setCursor(0, 1); 
    _lcd->print(_currentIndex == 0 ? "> PUMP: " : "  PUMP: ");
    _lcd->print(pumps[_selectedFloor]->getState() ? "ON " : "OFF");

    _lcd->setCursor(0, 2); 
    _lcd->print(_currentIndex == 1 ? "> LED : " : "  LED : ");
    _lcd->print(leds[_selectedFloor]->getState() ? "ON " : "OFF");

    _lcd->setCursor(0, 3); 
    _lcd->print(_currentIndex == 2 ? "> FAN : " : "  FAN : ");
    _lcd->print(fans[_selectedFloor]->getState() ? "ON " : "OFF");
}

void MenuControl::displayAutoFloor() {
    _lcd->clear();
    _lcd->setCursor(0, 0); _lcd->print("--- AUTO MODE ---");
    _lcd->setCursor(0, 1); _lcd->print("Chon Tang: "); _lcd->print(_currentIndex + 1);
}

void MenuControl::displayAutoPlant() {
    _lcd->clear();
    _lcd->setCursor(0, 0); _lcd->print("Tang "); _lcd->print(_selectedFloor + 1); _lcd->print(" Trong Cay Gi?");
    _lcd->setCursor(0, 1); _lcd->print(_currentIndex == 0 ? "> XA LACH" : "  XA LACH");
    _lcd->setCursor(0, 2); _lcd->print(_currentIndex == 1 ? "> HO CAI"  : "  HO CAI");
    _lcd->setCursor(0, 3); _lcd->print(_currentIndex == 2 ? "> NHIET DOI" : "  NHIET DOI");
}

// --- LOGIC XỬ LÝ NÚT BẤM ---
void MenuControl::handleButtons() {
    int btnUp = digitalRead(_pinUp);
    int btnDown = digitalRead(_pinDown);
    int btnOk = digitalRead(_pinOk);
    int btnBack = digitalRead(_pinBack);

    // 0. TRẠNG THÁI CHUNG
    if (_currentState == TRANG_THAI_CHUNG) {
        if (btnOk == LOW) {
            _currentState = MAIN_MENU;
            _currentIndex = 0;
            displayMainMenu();
            delay(300);
        }
    }
    // 1. MENU CHÍNH
    else if (_currentState == MAIN_MENU) {
        if (btnDown == LOW) { _currentIndex = 1; displayMainMenu(); delay(200); }
        if (btnUp == LOW)   { _currentIndex = 0; displayMainMenu(); delay(200); }
        if (btnOk == LOW) {
            _currentState = (_currentIndex == 0) ? NORMAL_SELECT_FLOOR : AUTO_SELECT_FLOOR;
            _currentIndex = 0;
            (_currentState == NORMAL_SELECT_FLOOR) ? displayNormalFloor() : displayAutoFloor();
            delay(300);
        }
        if (btnBack == LOW) {
            _currentState = TRANG_THAI_CHUNG;
            _lcd->clear();
            delay(300);
        }
    } 
    // 2. MANUAL SELECT FLOOR
    else if (_currentState == NORMAL_SELECT_FLOOR) {
        if (btnDown == LOW) { _currentIndex = (_currentIndex + 1) % 3; displayNormalFloor(); delay(200); }
        if (btnUp == LOW)   { _currentIndex = (_currentIndex - 1 + 3) % 3; displayNormalFloor(); delay(200); }
        if (btnOk == LOW) {
            _selectedFloor = _currentIndex;
            _currentState = NORMAL_CONTROL_DEVICE;
            _currentIndex = 0;
            displayNormalControl(); delay(300);
        }
        if (btnBack == LOW) { _currentState = MAIN_MENU; _currentIndex = 0; displayMainMenu(); delay(200); }
    }
    // 3. MANUAL CONTROL DEVICE (Sử dụng OK để Toggle On/Off)
    else if (_currentState == NORMAL_CONTROL_DEVICE) {
        // Dùng Up/Down để chọn dòng
        if (btnDown == LOW) { 
            _currentIndex = (_currentIndex + 1) % 3; 
            displayNormalControl(); 
            delay(200); 
        }
        if (btnUp == LOW)   { 
            _currentIndex = (_currentIndex - 1 + 3) % 3; 
            displayNormalControl(); 
            delay(200); 
        }

        // Nhấn OK để Toggle
        if (btnOk == LOW) {
            if (_currentIndex == 0) { // PUMP
                if (pumps[_selectedFloor]->getState()) {
                    pumps[_selectedFloor]->stopAll();
                } else {
                    pumps[_selectedFloor]->startIrrigation(3600000UL); 
                }
            }
            else if (_currentIndex == 1) { // LED
                if (leds[_selectedFloor]->getState()) {
                    leds[_selectedFloor]->turnOff();
                } else {
                    leds[_selectedFloor]->turnOn(3600000UL);
                }
            }
            else if (_currentIndex == 2) { // FAN
                if (fans[_selectedFloor]->getState()) {
                    fans[_selectedFloor]->turnOff();
                } else {
                    fans[_selectedFloor]->startFan(60); 
                }
            }
            
            displayNormalControl(); 
            delay(300);
        }

        if (btnBack == LOW) { 
            _currentState = NORMAL_SELECT_FLOOR; 
            _currentIndex = _selectedFloor; 
            displayNormalFloor(); 
            delay(200); 
        }
    }
    // 4. AUTO SELECT FLOOR
    else if (_currentState == AUTO_SELECT_FLOOR) {
        if (btnDown == LOW) { _currentIndex = (_currentIndex + 1) % 3; displayAutoFloor(); delay(200); }
        if (btnUp == LOW)   { _currentIndex = (_currentIndex - 1 + 3) % 3; displayAutoFloor(); delay(200); }
        if (btnOk == LOW) {
            _selectedFloor = _currentIndex;
            _currentState = AUTO_SELECT_PLANT;
            _currentIndex = 0;
            displayAutoPlant(); delay(300);
        }
        if (btnBack == LOW) { _currentState = MAIN_MENU; _currentIndex = 1; displayMainMenu(); delay(200); }
    }
    // 5. AUTO SELECT PLANT
    else if (_currentState == AUTO_SELECT_PLANT) {
        if (btnDown == LOW) { _currentIndex = (_currentIndex + 1) % 3; displayAutoPlant(); delay(200); }
        if (btnUp == LOW)   { _currentIndex = (_currentIndex - 1 + 3) % 3; displayAutoPlant(); delay(200); }
        if (btnOk == LOW) {
            _selectedPlant = _currentIndex;
            floorPlant[_selectedFloor] = _selectedPlant;
            floorDay[_selectedFloor] = 0;
            floorStage[_selectedFloor] = 0;

            _lcd->clear(); _lcd->print("DA LUU AUTO!"); delay(1000);
            _currentState = MAIN_MENU; _currentIndex = 1; displayMainMenu(); delay(200);
        }
        if (btnBack == LOW) { _currentState = AUTO_SELECT_FLOOR; _currentIndex = _selectedFloor; displayAutoFloor(); delay(200); }
    }
}

void MenuControl::update() {
    if (_currentState == TRANG_THAI_CHUNG) {
        static unsigned long lastHomeUpdate = 0;
        if (millis() - lastHomeUpdate >= 1000) {
            lastHomeUpdate = millis();
            displayGeneralStatus(myPhSensor.getPH(), myPhSensor.getTemperature());
        }
    }
    handleButtons(); 
}