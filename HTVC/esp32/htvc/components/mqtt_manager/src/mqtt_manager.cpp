#include "../include/mqtt_manager.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

namespace {

WiFiClient s_net_client;
PubSubClient s_client(s_net_client);

mqtt_data_callback_t s_data_cb = nullptr;
mqtt_connected_callback_t s_connected_cb = nullptr;
bool s_connected = false;

char s_host[128] = {0};
uint16_t s_port = 1883;

bool parse_broker_uri(const char* uri, char* host_out, size_t host_out_size, uint16_t* port_out) {
    if (uri == nullptr || host_out == nullptr || host_out_size == 0 || port_out == nullptr) {
        return false;
    }

    const char* p = uri;
    if (strncmp(uri, "mqtt://", 7) == 0) {
        p = uri + 7;
        *port_out = 1883;
    } else if (strncmp(uri, "tcp://", 6) == 0) {
        p = uri + 6;
        *port_out = 1883;
    } else {
        *port_out = 1883;
    }

    const char* slash = strchr(p, '/');
    const char* colon = strchr(p, ':');

    size_t host_len = 0;
    if (colon != nullptr && (slash == nullptr || colon < slash)) {
        host_len = static_cast<size_t>(colon - p);
        *port_out = static_cast<uint16_t>(atoi(colon + 1));
    } else if (slash != nullptr) {
        host_len = static_cast<size_t>(slash - p);
    } else {
        host_len = strlen(p);
    }

    if (host_len == 0 || host_len >= host_out_size) {
        return false;
    }

    memcpy(host_out, p, host_len);
    host_out[host_len] = '\0';
    return true;
}

void on_message(char* topic, uint8_t* payload, unsigned int length) {
    if (s_data_cb == nullptr || topic == nullptr || payload == nullptr) {
        return;
    }

    s_data_cb(topic, reinterpret_cast<const char*>(payload), static_cast<int>(length));
}

void ensure_connected() {
    if (s_client.connected()) {
        s_connected = true;
        return;
    }

    if (s_client.connect("htvc-mqtt-manager")) {
        s_connected = true;
        if (s_connected_cb != nullptr) {
            s_connected_cb();
        }
    } else {
        s_connected = false;
    }
}

}  // namespace

void mqtt_app_start(const char* broker_uri,
                    mqtt_data_callback_t data_cb,
                    mqtt_connected_callback_t connected_cb) {
    s_data_cb = data_cb;
    s_connected_cb = connected_cb;

    if (!parse_broker_uri(broker_uri, s_host, sizeof(s_host), &s_port)) {
        s_connected = false;
        return;
    }

    s_client.setServer(s_host, s_port);
    s_client.setCallback(on_message);
    ensure_connected();
}

bool mqtt_is_connected(void) {
    return s_connected && s_client.connected();
}

int mqtt_publish(const char* topic, const char* data, int qos, int retain) {
    (void)qos;

    if (topic == nullptr || data == nullptr) {
        return -1;
    }

    ensure_connected();
    if (!mqtt_is_connected()) {
        return -1;
    }

    return s_client.publish(topic, data, retain != 0) ? 1 : -1;
}

int mqtt_subscribe(const char* topic, int qos) {
    (void)qos;

    if (topic == nullptr) {
        return -1;
    }

    ensure_connected();
    if (!mqtt_is_connected()) {
        return -1;
    }

    return s_client.subscribe(topic) ? 1 : -1;
}

void mqtt_loop(void) {
    ensure_connected();
    s_client.loop();
}
