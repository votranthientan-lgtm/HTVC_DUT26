#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool totp_init(void);
uint32_t totp_generate(const char* key, uint64_t timestamp);
bool totp_verify(uint32_t input_code, const char* key, uint64_t timestamp);

#ifdef __cplusplus
}
#endif
