#include "esp-now-gw.h"


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    asdf();
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (wifi_retry_num < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            wifi_retry_num++;
            ESP_LOGI(PROG, "ESP_STA retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(PROG, "ERROR connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(PROG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        wifi_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void init_wifi() {

    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t sta_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&sta_init_config));


    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_got_ip);

    ESP_LOGI(PROG,"Event handlers registered");
    wifi_config_t sta_wifi_config = {
        .sta = {
            .ssid = STA_WIFI_SSID,
            .password = STA_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_OPEN,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    wifi_config_t ap_wifi_config = {
        .ap = {
            .ssid = AP_WIFI_SSID,
            .password = AP_WIFI_PASSWORD,
            .authmode = WIFI_AUTH_OPEN,
            .channel  = 10,
            .max_connection = 15
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(PROG, "wifi_init_apsta finished.");
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(PROG, "connected to ap SSID: %s", STA_WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(PROG, "Failed to connect to SSID: %s",STA_WIFI_SSID);
    } else {
        ESP_LOGE(PROG, "UNEXPECTED EVENT");
    }
}


void my_esp_now_init() {
    esp_now_init();
    esp_now_register_send_cb(my_espnow_send_cb);
    esp_now_register_recv_cb(my_espnow_recv_cb);
    // Add broadcast peer
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = 10;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    esp_now_add_peer(peer);
}

static void init_semaphores() {
    terminals_mutex = xSemaphoreCreateMutex();
    send_mutex = xSemaphoreCreateMutex();
    send_semaphore = xSemaphoreCreateBinary();
    if( send_semaphore == NULL ) {
        ESP_LOGI(PROG, "Initial semaphore allocation failed");
        esp_restart();
    }
    if (xSemaphoreGive(send_semaphore) != pdTRUE) {
        ESP_LOGI(PROG, "Initial semaphore give failed");
    }
}

static void asdf() {
    printf("foo\n");
}
