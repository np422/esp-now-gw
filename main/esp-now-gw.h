#ifndef ESP_NOW_GW_H
#define ESP_NOW_GW_H

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "esp_system.h"
#include <stdio.h>
#include "esp_msg_types.h"

const int max_peers = 20;

#define ESPNOW_WIFI_MODE WIFI_MODE_APSTA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_AP

#define STA_WIFI_SSID      "FRA_LSTN_adf"
#define STA_WIFI_PASSWORD ""

#define AP_WIFI_SSID       "ESP_NOW_AP"
#define AP_WIFI_PASSWORD  ""

#define WIFI_MAX_RETRY 10
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define QUEUE_SIZE     20
#define MAX_TERMINALS  20

#define TAG "gw"

const uint8_t my_mac[ESP_NOW_ETH_ALEN] = { 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00 };


struct esp_now_terminal_t {
    uint8_t peer_mac[ESP_NOW_ETH_ALEN];
    uint8_t dest_mac[ESP_NOW_ETH_ALEN];
    esp_now_peer_type_t peer_type;
    uint8_t peer_tag[ESP_NOW_TAGLEN];
};
typedef struct esp_now_terminal_t esp_now_terminal_t;

static const char *PROG = "espnow_gateway";
static EventGroupHandle_t s_wifi_event_group;
int wifi_retry_num = 0;
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static int npeers = 0;
static esp_now_terminal_t terminals[MAX_TERMINALS];
//static xQueueHandle send_queue;
static SemaphoreHandle_t send_semaphore;
static xQueueHandle recv_queue;
static xQueueHandle send_queue;
//static xQueueHandle asdf_queue;
static SemaphoreHandle_t terminals_mutex;
static SemaphoreHandle_t send_mutex;
//static SemaphoreHandle_t send_free_mutex;

char* get_type_name(esp_now_peer_type_t type);
static void add_or_update_terminal(uint8_t *peer_mac, uint8_t *dest_mac, esp_now_peer_type_t peer_type, uint8_t *peer_tag);
char* get_type_name(esp_now_peer_type_t type);
static void send_confirm_message(uint8_t *mac);
static void esp_now_recv_task(void *foo);
static void my_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len);
static void my_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
static void send_task();
void app_main(void);
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);
void init_wifi();
void my_esp_now_init();
static void init_semaphores();
static void asdf();
uint8_t *get_dest_mac_from_tag(uint8_t *tag);
static void dump_esp_msg(esp_now_message_t *msg);
static void dump_esp_queue_msg(esp_now_queue_message_t *msg);


#endif
