#include "esp-now-gw.h"

#include "esp-now-gw-init.c"
#include "esp-now-gw-utils.c"

static void send_confirm_message(uint8_t *tag) {

    esp_now_queue_message_t qmsg;
    confirm_message_t *cmsg;
    BaseType_t retval_enqueue;
    uint8_t *dest_mac;

    // Setup the confirm message
    cmsg = (confirm_message_t *) qmsg.esp_msg.message;
    cmsg->confirmed = 1;

    // Setup the esp now message
    qmsg.esp_msg.len = sizeof(confirm_message_t);
    qmsg.esp_msg.type = CONFIRM;

    // Setup the queue message
    dest_mac = get_dest_mac_from_tag(tag);
    memcpy(qmsg.mac, dest_mac, ESP_NOW_ETH_ALEN);

    ESP_LOGI(PROG, "Enqueueing confirm msg");
    dump_esp_queue_msg(&qmsg);
    retval_enqueue = xQueueSend(send_queue, (void *) &qmsg, portMAX_DELAY);
    if (retval_enqueue == pdTRUE) {
        ESP_LOGI(PROG, "Enqueue successful");
    } else {
        ESP_LOGI(PROG, "Enqueue failed");
    }

}


static void esp_now_recv_task(void *foo) {
    esp_now_queue_message_t qmsg;
    for(;;) {
        ESP_LOGI(PROG, "Starting to pull queue");
        if( xQueueReceive( recv_queue, &( qmsg ),  ( TickType_t ) 10000 / portTICK_PERIOD_MS) == pdPASS ) {
            ESP_LOGI(PROG, "Passed pulling message");
            dump_esp_queue_msg(&qmsg);
            switch(qmsg.esp_msg.type) {
            case REGISTER:
                ESP_LOGI(PROG, "Dequeued register msg");
                register_message_t *reg_msg;
                reg_msg = (register_message_t *) qmsg.esp_msg.message;
                add_or_update_terminal(reg_msg->mac, qmsg.mac, reg_msg->type, reg_msg->tag);
                send_confirm_message(reg_msg->tag);
                break;
            case BUTTON_PRESSED:
                ESP_LOGI(PROG, "Dequeued button press msg");
                break;
            default:
                ESP_LOGI(PROG, "Recieved unsupported message type");
                break;
            }

        } else {
            ESP_LOGI(PROG, "Failedpulling message");
        }
    }
}
static void my_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {

    esp_now_queue_message_t queue_msg;
    BaseType_t enqueue_retval;

    // Setup the queue message
    if ( len > sizeof(esp_now_message_t) ) {
        ESP_LOGI(PROG, "Recieved to big message, discarding");
        return;
    }
    memcpy(&(queue_msg.esp_msg), data, len);
    memcpy(queue_msg.mac, mac_addr, ESP_NOW_ETH_ALEN);
    dump_esp_queue_msg(&queue_msg);
    ESP_LOGI(PROG, "Enqueuing message with esp-msg length %i",queue_msg.esp_msg.len );
    enqueue_retval = xQueueSend(recv_queue, (void *) &queue_msg, ( TickType_t ) 0);
    ESP_LOGI(PROG, "Enqueued message");
    if ( enqueue_retval == pdTRUE ) {
        ESP_LOGI(PROG, "ESP_NOW_RECV callback, enqueued message sucessfully");
    } else {
        ESP_LOGI(PROG, "ESP_NOW_RECV callback, failed to enqueue message");
    }
    ESP_LOGI(PROG, "RECV_CALLBACK finished");
}

static void my_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS ) {
        ESP_LOGI(PROG, "ESP_NOW_SEND callback, status SUCCESS ");
    } else {
        ESP_LOGI(PROG, "ESP_NOW_SEND callback, status FAILURE");
    }
}

static void send_task() {
    BaseType_t retval_dequeue;
    esp_now_queue_message_t esp_now_queue_msg;

    ESP_LOGI(PROG, "Send task: Started to pull queue");
    for (;;) {
        ESP_LOGI(PROG,"Send task: Waiting for message");
        retval_dequeue = xQueueReceive( send_queue, &(esp_now_queue_msg), portMAX_DELAY);
        if ( retval_dequeue == pdTRUE ) {
            ESP_LOGI(PROG, "Send message, length=%i", esp_now_queue_msg.esp_msg.len);
            ESP_LOGI(PROG, "Sending message to %x:%x:%x:%x:%x:%x", esp_now_queue_msg.mac[0], esp_now_queue_msg.mac[1], esp_now_queue_msg.mac[2], esp_now_queue_msg.mac[3], esp_now_queue_msg.mac[4], esp_now_queue_msg.mac[5]);
            esp_now_send(esp_now_queue_msg.mac, (const uint8_t *) &(esp_now_queue_msg.esp_msg), sizeof(esp_now_message_t));
            vTaskDelay(500 / portTICK_PERIOD_MS);
            ESP_LOGI(PROG, "Send message, message sent");
        } else {
            ESP_LOGI(PROG, "Send task: failed to dequeue message");
        }
        // TODO: free memory allocated in queue messages
    }
}

void app_main(void) {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_base_mac_addr_set(my_mac));

    recv_queue = xQueueCreate(20, sizeof(esp_now_queue_message_t));
    send_queue = xQueueCreate(20, sizeof(esp_now_queue_message_t));

    init_semaphores();
    init_wifi();
    my_esp_now_init();
    asdf();

    xTaskCreate(esp_now_recv_task, "esp_now_recv_task", 64*1024, NULL, 1, NULL);
    xTaskCreate(send_task, "esp_now_send_task", 64*1024, NULL, 1, NULL);
}
