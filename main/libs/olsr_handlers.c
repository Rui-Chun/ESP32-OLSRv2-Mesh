#include "esp_log.h"

#include "olsr_handlers.h"

static const char *TAG = "espnow_olsr_handler";

espnow_olsr_event_t olsr_recv_pkt_handler(raw_pkt_t recv_pkt) {
    espnow_olsr_event_t ret_evt;
    ret_evt.id = ESPNOW_OLSR_NO_OP;
    rfc5444_pkt_t recv_rfc_pkt;
    // set values to 0x0
    memset((void*)(&recv_rfc_pkt), 0, sizeof(rfc5444_pkt_t));
    ESP_LOGI(TAG, "Got a packet! len = %d", recv_pkt.pkt_len);

    // TODO: logic here
    // parse raw packet
    recv_rfc_pkt = parse_raw_packet(recv_pkt);
    // handle possible HELLO msg
    if (recv_rfc_pkt.hello_msg_ptr != NULL) {
        // update info base given hello msg
        parse_hello_msg(recv_rfc_pkt.hello_msg_ptr);
    }


    // MUST free all mem
    free_rfc5444_pkt(recv_rfc_pkt);

    return ret_evt;
}

espnow_olsr_event_t olsr_timer_handler(uint32_t tick_num) {
    espnow_olsr_event_t ret_evt;
    ret_evt.id = ESPNOW_OLSR_NO_OP;
    rfc5444_pkt_t new_rfc_pkt;
    // set values to 0x0
    memset((void*)(&new_rfc_pkt), 0, sizeof(rfc5444_pkt_t));
    ESP_LOGI(TAG, "Time tick #%d is up!", tick_num);
    // TODO: logic here
    // 1. send out possible hello msg
    new_rfc_pkt.hello_msg_ptr = malloc(sizeof(hello_msg_t));
    if (new_rfc_pkt.hello_msg_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for new hello msg!");
        free_rfc5444_pkt(new_rfc_pkt);
        return ret_evt;
    }
    memset(new_rfc_pkt.hello_msg_ptr, 0, sizeof(hello_msg_t));
    gen_hello_msg(new_rfc_pkt.hello_msg_ptr);
    
    // TODO: gen raw pkt and send to event
    // TODO: update info base if time out.
    // MUST free mem
    free_rfc5444_pkt(new_rfc_pkt);
    return ret_evt;
}