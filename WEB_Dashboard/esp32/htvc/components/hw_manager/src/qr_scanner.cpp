#include "../include/qr_scanner.h"

#include <string.h>

const machine_gpio_entry_t MACHINE_GPIO_MAP[] = {
    {"machine_01", 18},
    {"machine_02", 19},
    {"machine_03", 21},
    {"machine_04", 22},
    {"machine_05", 23},
};

const int MACHINE_COUNT = sizeof(MACHINE_GPIO_MAP) / sizeof(MACHINE_GPIO_MAP[0]);

void gpio_init(uint8_t relay_door_gpio) {
    pinMode(relay_door_gpio, OUTPUT);
    digitalWrite(relay_door_gpio, LOW);

    for (int i = 0; i < MACHINE_COUNT; ++i) {
        pinMode(MACHINE_GPIO_MAP[i].gpio, OUTPUT);
        digitalWrite(MACHINE_GPIO_MAP[i].gpio, LOW);
    }
}

void uart_init(HardwareSerial& door_serial,
               HardwareSerial& machine_serial,
               uint32_t baud_rate) {
    door_serial.begin(baud_rate);
    machine_serial.begin(baud_rate);
}

int gpio_for_machine(const char* machine_id) {
    if (machine_id == nullptr) {
        return -1;
    }

    for (int i = 0; i < MACHINE_COUNT; ++i) {
        if (strcmp(MACHINE_GPIO_MAP[i].machine_id, machine_id) == 0) {
            return MACHINE_GPIO_MAP[i].gpio;
        }
    }

    return -1;
}
