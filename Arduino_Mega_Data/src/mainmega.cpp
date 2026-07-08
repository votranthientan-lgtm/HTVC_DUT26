#include <Arduino.h>
#include "time_manager.h"
#include "pump_control.h"
#include "sensor.h"
#include "led_control.h"
#include "ph_temp_sensor.h"
#include "peristaltic_pump.h"
#include "display.h" 
#include "fan.h"
#include "menu_control.h"
#include <ArduinoJson.h>

// --- KHỞI TẠO ĐỐI TƯỢNG ---
TimeManager myClock;
FanControl fanT1(42); FanControl fanT2(44); FanControl fanT3(46);
PumpControl pumpT1(31, 31, 31); PumpControl pumpT2(33, 33, 33); PumpControl pumpT3(35, 35, 35);
LedControl ledT1(8); LedControl ledT2(50); LedControl ledT3(52);

// Mảng con trỏ phục vụ điều khiển Manual từ Menu
PumpControl* pumps[] = { &pumpT1, &pumpT2, &pumpT3 };
LedControl* leds[] = { &ledT1, &ledT2, &ledT3 };
FanControl* fans[] = { &fanT1, &fanT2, &fanT3 };

SensorManager sensorT1(A1, 7); 
SensorManager sensorT2(0, 6);  
SensorManager sensorT3(0, 5);  

PhTempSensor myPhSensor(A0, 13, 0.70); 
PeristalticPump myPeriPumps(9, 10, 11, 12); 
PeristalticPump myThirdPump(22, 24, 0, 0);

DisplayManager displayT1(0x27, 20, 4); 
DisplayManager displayT2(0x26, 20, 4); 
DisplayManager displayT3(0x25, 20, 4); 
MenuControl myMenu(0x24, 20, 4, 41, 43, 45, 47);

unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 200; 
int lastHour = -1; 
const int REMOTE_FAN_ON_MINUTES = 32767;
const int REMOTE_LED_ON_SECONDS = 32767;
const unsigned long MANUAL_PUMP_LOCK_MS = 10UL * 60UL * 1000UL;
const unsigned long MANUAL_LED_LOCK_MS = 10UL * 60UL * 1000UL;
const unsigned long MANUAL_FAN_LOCK_MS = 10UL * 60UL * 1000UL;

static unsigned long pumpManualLockUntil[3] = {0, 0, 0};
static unsigned long ledManualLockUntil[3] = {0, 0, 0};
static unsigned long fanManualLockUntil[3] = {0, 0, 0};

static uint32_t currentDateKey() {
    return static_cast<uint32_t>(myClock.getYear()) * 10000UL +
           static_cast<uint32_t>(myClock.getMonth()) * 100UL +
           static_cast<uint32_t>(myClock.getDay());
}

static void lockManualDevice(int floor, bool isPump) {
    if (floor < 1 || floor > 3) {
        return;
    }

    const unsigned long lockUntil = millis() + (isPump ? MANUAL_PUMP_LOCK_MS : MANUAL_LED_LOCK_MS);
    if (isPump) {
        pumpManualLockUntil[floor - 1] = lockUntil;
    } else {
        ledManualLockUntil[floor - 1] = lockUntil;
    }
}

static void lockFanManualDevice(int floor) {
    if (floor < 1 || floor > 3) {
        return;
    }

    fanManualLockUntil[floor - 1] = millis() + MANUAL_FAN_LOCK_MS;
}

static bool isManualDeviceLocked(int floor, bool isPump) {
    if (floor < 1 || floor > 3) {
        return false;
    }

    return isPump ? (millis() < pumpManualLockUntil[floor - 1])
                  : (millis() < ledManualLockUntil[floor - 1]);
}

static bool isFanManualLocked(int floor) {
    if (floor < 1 || floor > 3) {
        return false;
    }

    return millis() < fanManualLockUntil[floor - 1];
}

void handleEsp32Command() {
    while (Serial1.available() > 0) {
        const char cmd = static_cast<char>(Serial1.read());
        Serial.print("[Mega] RX CMD: ");
        Serial.println(cmd);

        switch (cmd) {
            // Fan tầng 1/2/3
            case 'F': fanT1.startFan(REMOTE_FAN_ON_MINUTES); lockFanManualDevice(1); break;
            case 'f': fanT1.turnOff(); lockFanManualDevice(1); break;
            case 'G': fanT2.startFan(REMOTE_FAN_ON_MINUTES); lockFanManualDevice(2); break;
            case 'g': fanT2.turnOff(); lockFanManualDevice(2); break;
            case 'H': fanT3.startFan(REMOTE_FAN_ON_MINUTES); lockFanManualDevice(3); break;
            case 'h': fanT3.turnOff(); lockFanManualDevice(3); break;

            // Pump tưới tầng 1/2/3
            case 'P': pumpT1.turnOnManual(); lockManualDevice(1, true); break;
            case 'p': pumpT1.stopAll(); lockManualDevice(1, true); break;
            case 'Q': pumpT2.turnOnManual(); lockManualDevice(2, true); break;
            case 'q': pumpT2.stopAll(); lockManualDevice(2, true); break;
            case 'R': pumpT3.turnOnManual(); lockManualDevice(3, true); break;
            case 'r': pumpT3.stopAll(); lockManualDevice(3, true); break;

            // LED tầng 1/2/3
            case 'L': ledT1.turnOn(REMOTE_LED_ON_SECONDS); lockManualDevice(1, false); break;
            case 'l': ledT1.turnOff(); lockManualDevice(1, false); break;
            case 'M': ledT2.turnOn(REMOTE_LED_ON_SECONDS); lockManualDevice(2, false); break;
            case 'm': ledT2.turnOff(); lockManualDevice(2, false); break;
            case 'N': ledT3.turnOn(REMOTE_LED_ON_SECONDS); lockManualDevice(3, false); break;
            case 'n': ledT3.turnOff(); lockManualDevice(3, false); break;

            // Pump dinh duong AB/CD (he thong chung)
            case 'A': myPeriPumps.turnOnAB(); break;
            case 'a': myPeriPumps.stopAll(); break;
            case 'B': myPeriPumps.turnOnCD(); break;
            case 'b': myPeriPumps.stopAll(); break;

            default:
                break;
        }
    }
}

void setup() {
    Serial.begin(115200);   
    Serial1.begin(115200);    
    pinMode(28, OUTPUT); 

    fanT1.begin(); fanT2.begin(); fanT3.begin();
    myClock.begin(); 
    sensorT1.begin(); sensorT2.begin(); sensorT3.begin();
    pumpT1.begin(); pumpT2.begin(); pumpT3.begin();
    ledT1.begin(); ledT2.begin(); ledT3.begin();
    myPhSensor.begin();
    myPeriPumps.begin();
    myThirdPump.begin();
    myMenu.begin();

    displayT1.begin(); displayT2.begin(); displayT3.begin();
    delay(1500);
}
void displaySystemData() {
    myPhSensor.update(); 
    float ph = myPhSensor.getPH();
    float tempWater = myPhSensor.getTemperature();
    
    sensorT1.update(); 
    sensorT2.update(); 
    sensorT3.update();
    
    float tdsChung = sensorT1.getTDS(); 

    // --- GIỮ NGUYÊN HIỂN THỊ LCD CỦA BẠN ---
    displayT1.update(1, tdsChung, sensorT1.getHumidity(), sensorT1.getTemperature(), pumpT1.getState(), ledT1.getState(), fanT1.getState());
    displayT2.update(2, tdsChung, sensorT2.getHumidity(), sensorT2.getTemperature(), pumpT2.getState(), ledT2.getState(), fanT2.getState());
    displayT3.update(3, tdsChung, sensorT3.getHumidity(), sensorT3.getTemperature(), pumpT3.getState(), ledT3.getState(), fanT3.getState());

    static unsigned long lastSerialTime = 0;
    if (millis() - lastSerialTime >= 2000) {
        lastSerialTime = millis();

        StaticJsonDocument<512> doc;

        // Thời gian
        char timeStr[9];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
                myClock.getHour(), myClock.getMinute(), myClock.getSecond());
        doc["time"] = timeStr;

        // Thông số chung
        doc["ph"]   = serialized(String(ph, 2));
        doc["tw"]   = serialized(String(tempWater, 1));
        doc["tds"]  = (int)tdsChung;
        doc["peri"] = (myPeriPumps.getStateAB() || myPeriPumps.getStateCD()) ? 1 : 0;

        // Mảng 3 tầng
        JsonArray floors = doc.createNestedArray("floors");

        float temps[]      = { sensorT1.getTemperature(), sensorT2.getTemperature(), sensorT3.getTemperature() };
        bool  pumpStates[] = { pumpT1.getState(), pumpT2.getState(), pumpT3.getState() };
        bool  ledStates[]  = { ledT1.getState(),  ledT2.getState(),  ledT3.getState()  };
        bool  fanStates[]  = { fanT1.getState(),  fanT2.getState(),  fanT3.getState()  };

        for (int i = 0; i < 3; i++) {
            JsonObject f = floors.createNestedObject();
            f["t"]    = i + 1;
            f["temp"] = serialized(String(temps[i], 1));
            f["pump"] = pumpStates[i] ? 1 : 0;
            f["led"]  = ledStates[i]  ? 1 : 0;
            f["fan"]  = fanStates[i]  ? 1 : 0;
    }

    serializeJson(doc, Serial1);
    Serial1.println(); // newline = kết thúc gói
    }
}
void loop() {
    handleEsp32Command();
    myMenu.update();
    
    fanT1.update(); fanT2.update(); fanT3.update();
    pumpT1.update(); pumpT2.update(); pumpT3.update();
    ledT1.update(); ledT2.update(); ledT3.update();

    int currentHour = myClock.getHour();
    int currentMinute = myClock.getMinute();
    int currentSecond = myClock.getSecond();
    const uint32_t todayKey = currentDateKey();

    static uint32_t lastLedAutoDay[3] = {0, 0, 0};
    static uint32_t lastFanAutoHour[3] = {0, 0, 0};
    static uint32_t lastPumpAutoHour[3][8] = {{0}};

    // LOGIC ĐẾM NGÀY & CHUYỂN GIAI ĐOẠN
    if (currentHour == 0 && lastHour == 23) {
        for (int i = 0; i < 3; i++) {
            if (floorPlant[i] != -1) { 
                floorDay[i]++; 
                int pType = floorPlant[i];
                int cStage = floorStage[i];
                if (cStage < 2 && floorDay[i] >= _plantLib[pType].stages[cStage].durationDays) {
                    floorStage[i]++; 
                    floorDay[i] = 0; 
                }
            }
        }
    }
    lastHour = currentHour;

    // LOGIC ĐIỀU KHIỂN TỰ ĐỘNG
    for(int i = 0; i < 3; i++) {
        if (floorPlant[i] == -1) continue; 

        int pType = floorPlant[i];
        int cStage = floorStage[i];
        StageSchedule currentMode = _plantLib[pType].stages[cStage];

        // Hẹn giờ Bơm
        if (!isManualDeviceLocked(i + 1, true)) {
            for (int j = 0; j < currentMode.pumpTimes; j++) {
                const uint32_t pumpKey = todayKey * 100UL + static_cast<uint32_t>(currentMode.pumpHours[j]);
                if (myClock.isTimeToPump(currentMode.pumpHours[j], 0, 0) && lastPumpAutoHour[i][j] != pumpKey) {
                    pumps[i]->startIrrigation(15000); 
                    lastPumpAutoHour[i][j] = pumpKey;
                }
            }
        }

        // Hẹn giờ Đèn (Bật lúc 6h sáng)
        if (!isManualDeviceLocked(i + 1, false)) {
            if (myClock.isTimeToPump(6, 0, 0) && lastLedAutoDay[i] != todayKey) {
                leds[i]->turnOn((unsigned long)currentMode.lightHours * 3600);
                lastLedAutoDay[i] = todayKey;
            }
        }

        // Hẹn giờ Quạt (X phút mỗi đầu giờ)
        const uint32_t fanKey = todayKey * 100UL + static_cast<uint32_t>(currentHour);
        if (!isFanManualLocked(i + 1) && currentMinute == 0 && currentSecond == 0 && lastFanAutoHour[i] != fanKey) {
            fans[i]->startFan(currentMode.fanMinsPerHour);
            lastFanAutoHour[i] = fanKey;
        }
    }

    // Cập nhật LCD liên tục
    if (millis() - lastDisplayTime >= displayInterval) {
        lastDisplayTime = millis();
        displaySystemData();
    }
}