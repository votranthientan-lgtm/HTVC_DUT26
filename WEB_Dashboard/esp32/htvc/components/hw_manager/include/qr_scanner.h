#pragma once

#include <Arduino.h>

typedef struct {
    const char* machine_id;
    uint8_t gpio;
} machine_gpio_entry_t;

extern const machine_gpio_entry_t MACHINE_GPIO_MAP[];
extern const int MACHINE_COUNT;

void gpio_init(uint8_t relay_door_gpio = 2);
void uart_init(HardwareSerial& door_serial,
               HardwareSerial& machine_serial,
               uint32_t baud_rate = 115200U);
int gpio_for_machine(const char* machine_id);
