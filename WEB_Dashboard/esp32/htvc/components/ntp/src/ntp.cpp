#include "../include/ntp.h"

#include <Arduino.h>

void ntp_init(void) {
    configTzTime("UTC-7", "pool.ntp.org", "time.google.com", "time.windows.com");
}

bool ntp_wait_for_sync(unsigned long timeout_ms) {
    const unsigned long start = millis();
    struct tm timeinfo;

    while ((millis() - start) < timeout_ms) {
        if (getLocalTime(&timeinfo, 1000)) {
            return true;
        }
        delay(200);
    }

    return false;
}

time_t ntp_now(void) {
    time_t now = 0;
    time(&now);
    return now;
}
