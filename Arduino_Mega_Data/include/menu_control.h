#ifndef MENU_CONTROL_H
#define MENU_CONTROL_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

struct StageSchedule {
    int durationDays;
    int pumpTimes;
    int pumpHours[8];
    int fanMinsPerHour;
    int lightHours;
};

struct PlantProfile {
    StageSchedule stages[3];
};

extern PlantProfile _plantLib[3]; 
extern int floorPlant[3]; 
extern int floorDay[3];   
extern int floorStage[3]; 

enum MenuState {
    TRANG_THAI_CHUNG,      // Màn hình mặc định hiện pH và Temp
    MAIN_MENU,
    NORMAL_SELECT_FLOOR,   
    NORMAL_CONTROL_DEVICE, 
    AUTO_SELECT_FLOOR,     
    AUTO_SELECT_PLANT      
};

class MenuControl {
private:
    LiquidCrystal_I2C* _lcd;
    uint8_t _pinUp, _pinDown, _pinBack, _pinOk;
    int _currentIndex;     
    int _selectedFloor;    
    int _selectedPlant;    
    MenuState _currentState;

    void displayGeneralStatus(float ph, float tempWater);
    void displayMainMenu();
    void displayNormalFloor();
    void displayNormalControl();
    void displayAutoFloor();
    void displayAutoPlant();
    void handleButtons();

public:
    MenuControl(uint8_t addr, uint8_t cols, uint8_t rows, uint8_t up, uint8_t down, uint8_t back, uint8_t ok);
    void begin();
    void update();
};

#endif