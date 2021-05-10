#include "olsr_handlers.h"

static const char *TAG = "espnow_olsr_handler";

espnow_olsr_event_t olsr_recv_pkt_handler(raw_pkt_t recv_pkt) {
    espnow_olsr_event_t ret_evt;
    ret_evt.id = ESPNOW_OLSR_NO_OP;
    rfc5444_pkt_t recv_rfc_pkt;
    // set values to 0x0
    memset((void*)(&recv_rfc_pkt), 0, sizeof(rfc5444_pkt_t));
    ESP_LOGI(TAG, "Got a packet! len = %d", recv_pkt.pkt_len);
    
    // we may need to forward or reply certain msg
    raw_pkt_t new_raw_pkt;
    rfc5444_pkt_t new_rfc_pkt;
    // set values to 0x0
    memset((void*)(&new_rfc_pkt), 0, sizeof(rfc5444_pkt_t));
    new_rfc_pkt.pkt_len = RFC5444_PKT_HEADER_LEN;
    uint8_t tc_forward_flag = 0;

    // parse raw packet
    recv_rfc_pkt = parse_raw_packet(recv_pkt);

    // 1. handle possible HELLO msg
    if (recv_rfc_pkt.hello_msg_ptr != NULL) {
        // update info base given hello msg
        parse_hello_msg(recv_rfc_pkt.hello_msg_ptr);
        // only update, no event scheduled.
    }

    // 2. handle possible TC msg
    if (recv_rfc_pkt.tc_msg_ptr != NULL) {
        // update info base given TC msg
        if(parse_tc_msg(recv_rfc_pkt.tc_msg_ptr, recv_pkt.mac_addr)) {
            // forward this TC msg, prepare the packet
            new_rfc_pkt.tc_msg_ptr = malloc(sizeof(tc_msg_t));
            if (new_rfc_pkt.tc_msg_ptr == NULL) {
                ESP_LOGE(TAG, "No mem for new TC msg!");
                free_rfc5444_pkt(recv_rfc_pkt);
                free_rfc5444_pkt(new_rfc_pkt);
                return ret_evt;
            }
            // borrow the content of recv packet
            memcpy(new_rfc_pkt.tc_msg_ptr, recv_rfc_pkt.tc_msg_ptr, sizeof(tc_msg_t));
            // update pkt len
            new_rfc_pkt.pkt_len += sizeof(msg_header_t) + new_rfc_pkt.tc_msg_ptr->header.msg_size;
            tc_forward_flag = 1;
        }
    }

    // gen raw pkt and send to event, only if there is msg
    if(new_rfc_pkt.pkt_len > RFC5444_PKT_HEADER_LEN) {
        new_raw_pkt = gen_raw_packet(new_rfc_pkt);
        assert(new_raw_pkt.pkt_data != NULL);
        ret_evt.id = ESPNOW_OLSR_SEND_TO;
        ret_evt.info.send_to.pkt = new_raw_pkt;
        ESP_LOGW(TAG, "A Msg is forwarded!");
    }

    // MUST free all mem
    free_rfc5444_pkt(recv_rfc_pkt);
    // we borrowed content form recv_rfc_pkt, but we can not free mem twice.
    if(tc_forward_flag) {
        free(new_rfc_pkt.tc_msg_ptr);
        new_rfc_pkt.tc_msg_ptr = NULL;
    }
    free_rfc5444_pkt(new_rfc_pkt);

    return ret_evt;
}

espnow_olsr_event_t olsr_timer_handler(uint32_t tick_num) {
    espnow_olsr_event_t ret_evt;
    ret_evt.id = ESPNOW_OLSR_NO_OP;
    raw_pkt_t new_raw_pkt;
    rfc5444_pkt_t new_rfc_pkt;
    // set values to 0x0
    memset((void*)(&new_rfc_pkt), 0, sizeof(rfc5444_pkt_t));
    new_rfc_pkt.pkt_len = RFC5444_PKT_HEADER_LEN;

    ESP_LOGI(TAG, "Time tick #%d is up!", tick_num);
    set_info_base_time (tick_num);

    // TODO: logic here
    // 1. send out possible hello msg
    if (tick_num % HELLO_INTERVAL_TICKS == 0) {
        // check validity and delete timeout entries
        check_entry_validity();
        // update flooding and routing MPR
        update_mpr_status(0);
        update_mpr_status(1);
        // generate and prepare hello msg
        new_rfc_pkt.hello_msg_ptr = malloc(sizeof(hello_msg_t));
        if (new_rfc_pkt.hello_msg_ptr == NULL) {
            ESP_LOGE(TAG, "No mem for new hello msg!");
            free_rfc5444_pkt(new_rfc_pkt);
            return ret_evt;
        }
        memset(new_rfc_pkt.hello_msg_ptr, 0, sizeof(hello_msg_t));
        // generate hello msg content
        gen_hello_msg(new_rfc_pkt.hello_msg_ptr);
        new_rfc_pkt.pkt_len += sizeof(msg_header_t) + new_rfc_pkt.hello_msg_ptr->header.msg_size;
    }
    // 2. send out possible TC msg
    if (tick_num % TC_INTERVAL_TICKS == 0) {
        if (tick_num % HELLO_INTERVAL_TICKS != 0) {
            // check validity and delete timeout entries
            check_entry_validity();
            // update flooding and routing MPR
            update_mpr_status(0);
            update_mpr_status(1);
        }
        // generate and prepare TC msg
        new_rfc_pkt.tc_msg_ptr = malloc(sizeof(tc_msg_t));
        if (new_rfc_pkt.tc_msg_ptr == NULL) {
            ESP_LOGE(TAG, "No mem for new TC msg!");
            free_rfc5444_pkt(new_rfc_pkt);
            return ret_evt;
        }
        memset(new_rfc_pkt.tc_msg_ptr, 0, sizeof(tc_msg_t));
        // generate tc msg content
        if( gen_tc_msg(new_rfc_pkt.tc_msg_ptr) ) {
            new_rfc_pkt.pkt_len += sizeof(msg_header_t) + new_rfc_pkt.tc_msg_ptr->header.msg_size;    
        } else {
            free(new_rfc_pkt.tc_msg_ptr);
            new_rfc_pkt.tc_msg_ptr = NULL;
        }
    }
    // 3. compute routing paths
    if (tick_num % RC_INTERVAL_TICKS == 0) {
        compute_routing_set();
    }
    
    // gen raw pkt and send to event, only if there is msg
    if(new_rfc_pkt.pkt_len > RFC5444_PKT_HEADER_LEN) {
        new_raw_pkt = gen_raw_packet(new_rfc_pkt);
        assert(new_raw_pkt.pkt_data != NULL);
        ret_evt.id = ESPNOW_OLSR_SEND_TO;
        ret_evt.info.send_to.pkt = new_raw_pkt;
    }

    // MUST free mem
    free_rfc5444_pkt(new_rfc_pkt);
    return ret_evt;
}

