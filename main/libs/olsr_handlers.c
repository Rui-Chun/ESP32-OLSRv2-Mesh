#include "esp_log.h"

#include "olsr_handlers.h"

static const char *TAG = "espnow_olsr_handler";

espnow_olsr_event_t olsr_recv_pkt_handler(raw_pkt_t recv_pkt) {
    espnow_olsr_event_t ret_evt;
    ret_evt.id = ESPNOW_OLSR_NO_OP;
    // TODO: logic here
    ESP_LOGI(TAG, "Got a packet! len = %d", recv_pkt.pkt_len);
    return ret_evt;
}