#ifndef ESP_MSG_TYPES_H
#define ESP_MSG_TYPES_H

#define MESSAGE_SIZE   50
#define ESP_NOW_TAGLEN 20
#define MAX_BUTTONS    8

enum esp_now_peer_type_t {
    GW,
    SENSOR,
    BUTTON,
    RELAY,
    REPEATER,
    DISPLAY,
    UNKNOWN
};

typedef enum esp_now_peer_type_t esp_now_peer_type_t;

enum esp_now_message_type_t {
    REGISTER,
    CONFIRM,
    BUTTON_PRESSED,
    SENSOR_FLOAT_VAL,
    SENSOR_READ,
    RELAY_ON,
    RELAY_OFF,
    RELAY_TOGGLE,
    RELAY_GET,
    DISPLAY_OFF,
    DISPLAY_ON,
    DISPLAY_SET,
    LED_ON,
    LED_OFF,
    LED_TOGGLE,
    LED_GET
};
typedef enum esp_now_message_type_t esp_now_message_type_t;

struct esp_now_message_t {
    uint8_t len;
    esp_now_message_type_t type;
    uint8_t message[MESSAGE_SIZE];
};
typedef struct esp_now_message_t esp_now_message_t;

struct button_press_message_t {
    uint8_t buttons_pressed[MAX_BUTTONS];
    uint8_t sender_tag[ESP_NOW_TAGLEN];
};

typedef struct button_press_message_t button_press_message_t;
struct register_message_t {
    esp_now_peer_type_t type;
    uint8_t tag[ESP_NOW_TAGLEN];
    uint8_t mac[ESP_NOW_ETH_ALEN];
};
typedef struct register_message_t register_message_t;

struct confirm_message_t {
    uint8_t confirmed;
};
typedef struct confirm_message_t confirm_message_t;

struct esp_now_queue_message_t {
    esp_now_message_t esp_msg;
    uint8_t mac[ESP_NOW_ETH_ALEN];
};
typedef struct esp_now_queue_message_t esp_now_queue_message_t;

#endif
