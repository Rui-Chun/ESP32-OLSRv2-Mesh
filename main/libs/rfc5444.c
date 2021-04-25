#include "rfc5444.h"

static const char *TAG = "espnow_rfc5444";

// after use this rfc5444_pkt_t, remember to free all mem ...
rfc5444_pkt_t parse_raw_packet (raw_pkt_t raw_packet) {
    // only copy mem from raw_packet, do not free it. Event loop will do this.
    assert(raw_packet.pkt_len >= 2);
    rfc5444_pkt_t ret_pkt;
    uint8_t* raw_pkt_ptr = raw_packet.pkt_data;
    ret_pkt.version = raw_pkt_ptr[0];
    ret_pkt.version = raw_pkt_ptr[1];
    raw_pkt_ptr += 2;

    // get msg_type
    switch *raw_pkt_ptr {
        MSG_TYPE_HELLO: {
            // TODO: parse HELLO msg
            break;
        }
        MSG_TYPE_TC: {
            break;
        }
        default: {
            ESP_LOGW(TAG, "Unknown mse type!");
            break;
        }
    }

}

hello_msg_t* parse_hello_msg(uint8_t* msg_data) {

}

raw_pkt_t gen_raw_packet (rfc5444_pkt_t rfc5444_pkt) {
    
}