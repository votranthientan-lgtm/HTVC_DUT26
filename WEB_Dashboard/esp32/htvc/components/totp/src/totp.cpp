#include "../include/totp.h"

#include <mbedtls/md.h>
#include <string.h>

namespace {

int base32_decode_value(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= '2' && c <= '7') {
        return c - '2' + 26;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a';
    }
    return -1;
}

int base32_decode(const char* encoded, uint8_t* result, size_t result_size) {
    int buffer = 0;
    int bits_left = 0;
    int count = 0;

    for (const char* p = encoded; *p != '\0'; ++p) {
        const int val = base32_decode_value(*p);
        if (val < 0) {
            continue;
        }

        buffer = (buffer << 5) | val;
        bits_left += 5;

        if (bits_left >= 8) {
            if (count >= static_cast<int>(result_size)) {
                return -1;
            }
            result[count++] = static_cast<uint8_t>((buffer >> (bits_left - 8)) & 0xFF);
            bits_left -= 8;
        }
    }

    return count;
}

}  // namespace

bool totp_init(void) {
    return true;
}

uint32_t totp_generate(const char* key, uint64_t timestamp) {
    if (key == nullptr || key[0] == '\0') {
        return 0;
    }

    uint8_t binary_key[64] = {0};
    const int key_len = base32_decode(key, binary_key, sizeof(binary_key));
    if (key_len <= 0) {
        return 0;
    }

    uint64_t interval = timestamp / 30;
    uint8_t msg[8] = {0};
    for (int i = 7; i >= 0; --i) {
        msg[i] = static_cast<uint8_t>(interval & 0xFF);
        interval >>= 8;
    }

    uint8_t hash[20] = {0};
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    if (info == nullptr || mbedtls_md_setup(&ctx, info, 1) != 0) {
        mbedtls_md_free(&ctx);
        return 0;
    }

    mbedtls_md_hmac_starts(&ctx, binary_key, static_cast<size_t>(key_len));
    mbedtls_md_hmac_update(&ctx, msg, sizeof(msg));
    mbedtls_md_hmac_finish(&ctx, hash);
    mbedtls_md_free(&ctx);

    const int offset = hash[19] & 0x0F;
    const uint32_t binary = ((hash[offset] & 0x7F) << 24) |
                            ((hash[offset + 1] & 0xFF) << 16) |
                            ((hash[offset + 2] & 0xFF) << 8) |
                            (hash[offset + 3] & 0xFF);

    return binary % 1000000U;
}

bool totp_verify(uint32_t input_code, const char* key, uint64_t timestamp) {
    if (input_code == totp_generate(key, timestamp)) {
        return true;
    }

    if (timestamp >= 30 && input_code == totp_generate(key, timestamp - 30)) {
        return true;
    }

    return false;
}
