#include "esp-now-gw.h"

char* get_msg_type_name(esp_now_message_type_t type) {
    switch(type) {
    case REGISTER:
        return "REGISTER";
        break;
    case CONFIRM:
        return "CONFIRM";
        break;
    case BUTTON_PRESSED:
        return "BUTTON_PRESSED";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

uint8_t *get_dest_mac_from_tag(uint8_t *tag) {

    xSemaphoreTake(terminals_mutex, portMAX_DELAY);
    for (int i=0 ; i<npeers ; i++) {
        if (strcmp((char *) tag, (char *) terminals[i].peer_tag) == 0) {
            xSemaphoreGive(terminals_mutex);
            return terminals[i].dest_mac;
        }
    }
    xSemaphoreGive(terminals_mutex);
    return NULL;
}
static void dump_esp_msg(esp_now_message_t *msg) {
    ESP_LOGI(PROG, "##################");
    ESP_LOGI(PROG, "# ESP-NOW message ");
    ESP_LOGI(PROG, "##################");
    ESP_LOGI(PROG, "Length: %u", msg->len);
    ESP_LOGI(PROG, "Type: %s", get_msg_type_name(msg->type));
    ESP_LOGI(PROG, "Dest tag: %s", (char *) msg->dest_tag);
    switch(msg->type) {
    case REGISTER:
        ESP_LOGI(PROG, "Dumping register message");
        register_message_t *rmsg;
        rmsg = (register_message_t *) msg->message;
        ESP_LOGI(PROG, "Peer type: %s",get_type_name(rmsg->type));
        ESP_LOGI(PROG, "Peer tag: %s", rmsg->tag);
        break;
    case CONFIRM:
        ESP_LOGI(PROG, "TBD...");
        break;
    case BUTTON_PRESSED:
        ESP_LOGI(PROG, "Dumping button pressed message");
        button_press_message_t *bmsg;
        bmsg = (button_press_message_t *) msg->message;
        ESP_LOGI(PROG, "Peer tag: %s", bmsg->sender_tag);
        for (int i = 0 ; i<MAX_BUTTONS ; i++) {
            ESP_LOGI(PROG, "Button %i: %i", i, bmsg->buttons_pressed[i]);
        }
        break;
    default:
        ESP_LOGI(PROG, "TBD ... UNKNOWN");
        break;
    }
    ESP_LOGI(PROG, "");
}

static void dump_esp_queue_msg(esp_now_queue_message_t *msg) {
    ESP_LOGI(PROG, "##################");
    ESP_LOGI(PROG, "# ESP-NOW Queue message ");
    ESP_LOGI(PROG, "##################");
    ESP_LOGI(PROG, "Mac address: %x:%x:%x %x:%x:%x", msg->mac[0], msg->mac[1], msg->mac[2], msg->mac[3], msg->mac[4], msg->mac[5]);
    ESP_LOGI(PROG, "Dumping esp_now message ...");
    ESP_LOGI(PROG, "");
    dump_esp_msg(& (msg->esp_msg));
}

static void add_or_update_terminal(uint8_t *peer_mac, uint8_t *dest_mac, esp_now_peer_type_t peer_type, uint8_t *peer_tag) {
    int i;
    bool found = false;
    xSemaphoreTake(terminals_mutex, portMAX_DELAY);
    esp_now_peer_info_t peer;
    memset(&peer,0,sizeof(peer));
    peer.channel = 10;
    peer.ifidx = ESPNOW_WIFI_IF;
    peer.encrypt = false;
    memcpy(peer.peer_addr, dest_mac, ESP_NOW_ETH_ALEN);
    esp_now_peer_info_t pi;
    if (esp_now_get_peer(dest_mac,&pi) == ESP_ERR_ESPNOW_NOT_FOUND) {
        ESP_LOGI(PROG, "Added esp peer");
        esp_now_add_peer(&peer);
    } else {
        ESP_LOGI(PROG, "Modified esp peer");
        esp_now_mod_peer(&peer);
    }
    for (i=0;i<=npeers;i++) {
        if (strcmp((char *)terminals[i].peer_tag, (char *) peer_tag) == 0) {
            ESP_LOGI(PROG, "Updated existing peer");
            found = true;
            memcpy(terminals[i].peer_mac, peer_mac, ESP_NOW_ETH_ALEN);
            memcpy(terminals[i].dest_mac, dest_mac, ESP_NOW_ETH_ALEN);
            terminals[i].peer_type = peer_type;
        }
    }
    if (!found) {
        ESP_LOGI(PROG, "Added new peer");
        memcpy(terminals[npeers].peer_mac, peer_mac, ESP_NOW_ETH_ALEN);
        memcpy(terminals[npeers].dest_mac, dest_mac, ESP_NOW_ETH_ALEN);
        terminals[npeers].peer_type = peer_type;
        strcpy((char *) terminals[npeers].peer_tag, (char *) peer_tag);
        npeers++;
    }
    for (i=0;i<npeers;i++) {
        ESP_LOGI(PROG, "Peer no %i",i);
        ESP_LOGI(PROG, "Peer mac %x:%x:%x:%x:%x:%x", terminals[i].peer_mac[0], terminals[i].peer_mac[1], terminals[i].peer_mac[2], terminals[i].peer_mac[3], terminals[i].peer_mac[4], terminals[i].peer_mac[5]);
        ESP_LOGI(PROG, "Dest mac %x:%x:%x:%x:%x:%x", terminals[i].dest_mac[0], terminals[i].dest_mac[1], terminals[i].dest_mac[2], terminals[i].dest_mac[3], terminals[i].dest_mac[4], terminals[i].dest_mac[5]);
        ESP_LOGI(PROG, "Peer type: %s", get_type_name(terminals[i].peer_type));
        ESP_LOGI(PROG, "Peer tag: %s",  terminals[i].peer_tag);
    }
    xSemaphoreGive(terminals_mutex);
}

char* get_type_name(esp_now_peer_type_t type) {
    switch(type) {
    case GW:
        return "GW";
        break;
    case SENSOR:
        return "SENSOR";
        break;
    case BUTTON:
        return "BUTTON";
        break;
    case RELAY:
        return "RELAY";
        break;
    case REPEATER:
        return "REPEATER";
        break;
    default:
        return "UNKNOWN";
    }
}
