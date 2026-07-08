#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*mqtt_data_callback_t)(const char* topic, const char* data, int data_len);
typedef void (*mqtt_connected_callback_t)(void);

void mqtt_app_start(const char* broker_uri,
                    mqtt_data_callback_t data_cb,
                    mqtt_connected_callback_t connected_cb);

bool mqtt_is_connected(void);
int mqtt_publish(const char* topic, const char* data, int qos, int retain);
int mqtt_subscribe(const char* topic, int qos);
void mqtt_loop(void);

#ifdef __cplusplus
}
#endif
