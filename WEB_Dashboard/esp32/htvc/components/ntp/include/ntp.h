#pragma once

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void ntp_init(void);
bool ntp_wait_for_sync(unsigned long timeout_ms);
time_t ntp_now(void);

#ifdef __cplusplus
}
#endif
