#include "rfc5444.h"

static const char *TAG = "espnow_rfc5444";

/* Helper functions */
uint8_t get_tlv_len (tlv_t* tlv_ptr) {
    return sizeof(tlv_t) + tlv_ptr->tlv_value_len;
}

void free_tlv_block (tlv_block_t* tlv_block_ptr) {
    if (tlv_block_ptr == NULL) {
        // empty tlv block
        return;
    }
    // free tlv_ptr entries
    for(int i=0; i < tlv_block_ptr->tlv_ptr_len; i++) {
        free(tlv_block_ptr->tlv_ptr_list[i]);
    }
    // must free tlv_block itself.
    free(tlv_block_ptr);
}

uint16_t get_tlv_block_len (tlv_block_t* tlv_block) {
    uint16_t ret_len = sizeof(tlv_block_t);
    for(int i=0; i < tlv_block->tlv_ptr_len; i++) {
        ret_len += tlv_block->tlv_ptr_list[i]->tlv_value_len + sizeof(tlv_t);
    }
    return 0;
}

// copy the content of a tlv_block and return the num of bytes copied.
uint16_t copy_from_tlv_block (uint8_t* dst_buf, tlv_block_t* src_block) {
    uint16_t offset = 0;
    if(dst_buf == NULL || src_block==NULL) {
        return 0;
    }
    memcpy(dst_buf + offset, (uint8_t*)src_block, sizeof(tlv_block_t));
    offset += sizeof(tlv_block_t);
    for(int i=0; i < src_block->tlv_ptr_len; i++) {
        uint8_t tmp_len = get_tlv_len(src_block->tlv_ptr_list[i]);
        memcpy(dst_buf + offset, (uint8_t*)(src_block->tlv_ptr_list[i]), tmp_len);
        offset += tmp_len;
    }
    assert(offset == get_tlv_block_len(src_block));
    return offset;
}

void free_rfc5444_pkt (rfc5444_pkt_t pkt) {
    // free possible hello msg
    if (pkt.hello_msg_ptr != NULL) {
        free_tlv_block(pkt.hello_msg_ptr->msg_tlv_block_ptr);
        free(pkt.hello_msg_ptr->addr_block_ptr);
        free_tlv_block(pkt.hello_msg_ptr->addr_tlv_block_ptr);
        // free msg struct after free all blocks
        free(pkt.hello_msg_ptr);
    }
    // free possible tc msg
    if (pkt.tc_msg_ptr != NULL) {
        free_tlv_block(pkt.tc_msg_ptr->msg_tlv_block_ptr);
        free(pkt.tc_msg_ptr->addr_block_ptr);
        free_tlv_block(pkt.tc_msg_ptr->addr_tlv_block_ptr);
        // free msg struct after free all blocks
        free(pkt.tc_msg_ptr);
    }
    return;
}
/* Helper functions End */


/* Worker Functions */
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
    switch ((msg_type_t)raw_pkt_ptr[0]) {
        case MSG_TYPE_HELLO: {
            // TODO: parse HELLO msg
            break;
        }
        case MSG_TYPE_TC: {
            break;
        }
        default: {
            ESP_LOGW(TAG, "Unknown mse type!");
            break;
        }
    }

    return ret_pkt; // user of this pkt must free it.
}

// hello_msg_t* parse_hello_msg(uint8_t* msg_data) {

// }

// rfc5444_pkt should come form info base.
// this function will consume rfc5444 pkt. 
// the caller must check the raw_pkt.pkt_data field is not NULL!
raw_pkt_t gen_raw_packet (rfc5444_pkt_t rfc5444_pkt) {
    raw_pkt_t ret_pkt;
    uint16_t pkt_offset = 0;
    tlv_block_t* local_tlv_block_ptr = NULL;

    ret_pkt.pkt_len = rfc5444_pkt.pkt_len;
    // Send_to event handling in main event loop will free this mem. 
    ret_pkt.pkt_data = malloc(rfc5444_pkt.pkt_len);
    if (ret_pkt.pkt_data == NULL) {
        ESP_LOGE(TAG, "No mem for new paket!");
        free_rfc5444_pkt(rfc5444_pkt);
        return ret_pkt; // return a NULL packet.
    }
    // assign the pkt header
    memcpy(ret_pkt.pkt_data, (uint8_t*)(&rfc5444_pkt), 4);
    pkt_offset += 4;
    // assign possible hello msg
    if (rfc5444_pkt.hello_msg_ptr != NULL) {
        // copy header
        memcpy(ret_pkt.pkt_data + pkt_offset, (uint8_t*)(rfc5444_pkt.hello_msg_ptr), sizeof(msg_header_t));
        pkt_offset += sizeof(msg_header_t);
        // double check msg type
        assert(rfc5444_pkt.hello_msg_ptr->header.msg_type == MSG_TYPE_HELLO);
        // copy blocks
        // 1. copy msg_tlv_block
        pkt_offset += \
            copy_from_tlv_block(ret_pkt.pkt_data + pkt_offset, rfc5444_pkt.hello_msg_ptr->msg_tlv_block_ptr);
        // 2. copy addr_block
        // TODO: 
        
    }
    // assign possible tc msg
    // TODO

    // check pkt offset to make sure pkt len is correct.
    assert(pkt_offset == ret_pkt.pkt_len);
    // MUST free rfc5444_pkt
    free_rfc5444_pkt(rfc5444_pkt);

    return ret_pkt;
}

