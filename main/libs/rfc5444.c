#include "rfc5444.h"

static const char *TAG = "espnow_rfc5444";

/* Helper functions */
uint8_t get_tlv_len (tlv_t* tlv_ptr) {
    if (tlv_ptr == NULL) return 0;
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
    if (tlv_block == NULL) return 0;

    uint16_t ret_len = sizeof(tlv_block_t);
    for(int i=0; i < tlv_block->tlv_ptr_len; i++) {
        ret_len += tlv_block->tlv_ptr_list[i]->tlv_value_len + sizeof(tlv_t);
    }
    // double check
    assert(ret_len == sizeof(tlv_block_t) + tlv_block->tlv_block_size);
    return ret_len;
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

// copy the content of buf to tlv_block and return the num of bytes copied.
// this function requires that the mem is allocated for tlv_block.
uint16_t copy_to_tlv_block (uint8_t* src_buf, tlv_block_t* dst_block) {
    uint16_t offset = 0;
    uint8_t* dst_ptr = (uint8_t*) dst_block;
    if(src_buf == NULL || dst_ptr == NULL) {
        return 0;
    }
    // copy the tlv block
    // 1. copy to tlv block header.
    memcpy(dst_ptr, src_buf, sizeof(tlv_block_t));
    offset += sizeof(tlv_block_t);
    // 2. copy to tlv entries (alloc mem first)
    for(int i=0; i < dst_block->tlv_ptr_len; i++) {
        tlv_t* tmp_tlv_ptr = (tlv_t*) (src_buf + offset);
        uint8_t tmp_len = get_tlv_len(tmp_tlv_ptr);
        dst_block->tlv_ptr_list[i] = malloc(tmp_len);
        if (dst_block->tlv_ptr_list[i] == NULL) {
            ESP_LOGE(TAG, "No mem for new tlv entry!");
            return 0;
        }
        // copy one tlv entry
        memcpy(dst_block->tlv_ptr_list[i], src_buf + offset, tmp_len);
        offset += tmp_len;
    }
    // check offset is correct
    assert(offset == get_tlv_block_len(dst_block));
    return offset;
}


uint16_t get_addr_block_len (addr_block_t* addr_block_ptr) {
    if (addr_block_ptr == NULL) return 0;
    return sizeof(addr_block_t) + addr_block_ptr->addr_num * RFC5444_ADDR_LEN;
}

uint16_t copy_from_addr_block (uint8_t* dst_buf, addr_block_t* addr_block_ptr) {
    if (dst_buf == NULL || addr_block_ptr == NULL) return 0;
    uint16_t ret_len = get_addr_block_len(addr_block_ptr);
    memcpy(dst_buf, (uint8_t*)addr_block_ptr, ret_len);
    return ret_len;
}

// copy the content of msg_data to hello_msg and return the num of bytes copied.
// this function requires the mem is allocated for hello_msg alraedy.
uint16_t copy_to_hello_msg(uint8_t* msg_data, hello_msg_t* hello_msg_ptr) {
    uint16_t offset = 0;
    uint16_t tmp_len = 0;
    uint8_t* dst_ptr = (uint8_t*) hello_msg_ptr;
    if (msg_data == NULL || dst_ptr == NULL) {
        ESP_LOGW(TAG, "NULL data input!");
        return 0;
    }
    // copy hello msg
    // 1. copy hello msg header
    memcpy(dst_ptr, msg_data, sizeof(msg_header_t));
    offset += sizeof(msg_header_t);
    // TODO: what if we do not have all blocks in a msg.
    // 2. copy all the blocks (alloc mem first)
    // (1) copy msg_tlv block
    tlv_block_t* tmp_tlv_block_ptr = (tlv_block_t*)(msg_data + offset);
    // this length is special since we store a list of pointers instead of tlv data.
    tmp_len = sizeof(tlv_block_t) + tmp_tlv_block_ptr->tlv_ptr_len * sizeof(tlv_t*);
    hello_msg_ptr->msg_tlv_block_ptr = malloc(tmp_len);
    if (hello_msg_ptr->msg_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for tlv_block!");
        return 0;
    }
    // copy the msg_tlv block
    offset += copy_to_tlv_block(msg_data + offset, hello_msg_ptr->msg_tlv_block_ptr);
    // (2) copy the addr block
    addr_block_t* tmp_addr_block_ptr = (addr_block_t*)(msg_data + offset);
    tmp_len = get_addr_block_len(tmp_addr_block_ptr);
    hello_msg_ptr->addr_block_ptr = malloc(tmp_len);
    if (hello_msg_ptr->addr_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr_block!");
        free(hello_msg_ptr->msg_tlv_block_ptr);
        return 0;
    }
    memcpy(hello_msg_ptr->addr_block_ptr, msg_data + offset, tmp_len);
    offset += tmp_len;
    // (3) copy addr tlv block
    tmp_tlv_block_ptr = (tlv_block_t*)(msg_data + offset);
    // this length is special since we store a list of pointers instead of tlv data.
    tmp_len = sizeof(tlv_block_t) + tmp_tlv_block_ptr->tlv_ptr_len * sizeof(tlv_t*);
    hello_msg_ptr->addr_tlv_block_ptr = malloc(tmp_len);
    if (hello_msg_ptr->addr_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv_block!");
        free(hello_msg_ptr->msg_tlv_block_ptr);
        free(hello_msg_ptr->addr_block_ptr);
        return 0;
    }
    // copy the addr_tlv block
    offset += copy_to_tlv_block(msg_data + offset, hello_msg_ptr->addr_tlv_block_ptr);

    // 3. check offset
    assert(offset == sizeof(msg_header_t) + hello_msg_ptr->header.msg_size);
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
    // only copy mem from raw_packet, do not free it. Event loop will reuse it.
    assert(raw_packet.pkt_len >= 2);
    rfc5444_pkt_t ret_pkt;
    uint8_t* raw_pkt_ptr = raw_packet.pkt_data;
    uint16_t pkt_offset = 0;
    ret_pkt.version = raw_pkt_ptr[0];
    ret_pkt.version = raw_pkt_ptr[1];
    pkt_offset += 2;

    // get msg_type
    // TODO: we only consider one msg in a packet for now.
    switch ((msg_type_t)raw_pkt_ptr[pkt_offset]) {
        case MSG_TYPE_HELLO: {
            // parse HELLO msg
            ret_pkt.hello_msg_ptr = malloc(sizeof(hello_msg_t));
            if(ret_pkt.hello_msg_ptr == NULL) {
                ESP_LOGE(TAG, "No mem for hello msg!");
                return ret_pkt;
            }
            // copy to hello msg and move offset.
            pkt_offset += copy_to_hello_msg(raw_pkt_ptr + pkt_offset, ret_pkt.hello_msg_ptr);
            break;
        }
        case MSG_TYPE_TC: {
            // TODO: parse TC msg
            break;
        }
        default: {
            ESP_LOGW(TAG, "Unknown mse type!");
            break;
        }
    }

    return ret_pkt; // user of this pkt must free it.
}

// rfc5444_pkt should come form info base.
// this function will consume rfc5444 pkt. 
// the caller must check the raw_pkt.pkt_data field is not NULL!
raw_pkt_t gen_raw_packet (rfc5444_pkt_t rfc5444_pkt) {
    raw_pkt_t ret_pkt;
    uint16_t pkt_offset = 0;

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
        pkt_offset += \
            copy_from_addr_block(ret_pkt.pkt_data + pkt_offset, rfc5444_pkt.hello_msg_ptr->addr_block_ptr);
        // 3. copy addr_tlv_block
        pkt_offset += \
            copy_from_tlv_block(ret_pkt.pkt_data + pkt_offset, rfc5444_pkt.hello_msg_ptr->addr_tlv_block_ptr);        
    }
    // assign possible tc msg
    if (rfc5444_pkt.tc_msg_ptr != NULL) {
        // TODO:
        ESP_LOGW(TAG, "TC msg not supported yet!");
    }

    // check pkt offset to make sure pkt len is correct.
    assert(pkt_offset == ret_pkt.pkt_len);
    // MUST free rfc5444_pkt
    free_rfc5444_pkt(rfc5444_pkt);

    return ret_pkt;
}

