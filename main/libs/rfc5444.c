#include "rfc5444.h"

static const char *TAG = "espnow_rfc5444";

/* Helper functions */
uint8_t get_tlv_len (tlv_t* tlv_ptr) {
    if (tlv_ptr == NULL) return 0;
    return sizeof(tlv_t) + tlv_ptr->tlv_value_len;
}

// get the specific type of value in the tlv block, return value len.
// use a pointer of pointer to pass the pointer to the value.
uint8_t get_tlv_value (tlv_block_t* tlv_block_ptr, tlv_type_t tt, uint8_t** buf_pp) {
    if(tlv_block_ptr == NULL) {
        return 0;
    }
    tlv_t* tlv_ptr = NULL;
    for (int t=0; t < tlv_block_ptr->tlv_ptr_len; t++) {
        tlv_ptr = tlv_block_ptr->tlv_ptr_list[t];
        if(tlv_ptr->tlv_type == tt) {
            *buf_pp = tlv_ptr->tlv_value;
            return tlv_ptr->tlv_value_len;
        }
    }
    return 0;
}

uint8_t cal_tlv_len (tlv_type_t type) {
    switch (type) {
        case VALIDITY_TIME: {
            return sizeof(tlv_t) + 1;
        }
        case INTERVAL_TIME: {
            return sizeof(tlv_t) + 1;
        }
        case MPR_WILLING: {
            return sizeof(tlv_t) + 1;
        }
        default: {
            ESP_LOGW(TAG, "Unknown tlv type!");
            return 0;
        }
    }
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
    // ESP_LOGI(TAG, "ret_len = %d, tlv_block_len = %d", ret_len, sizeof(tlv_block_t) + tlv_block->tlv_block_size);
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
    memset(hello_msg_ptr->msg_tlv_block_ptr, 0, tmp_len);
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
    memset(hello_msg_ptr->addr_tlv_block_ptr, 0, tmp_len);
    // copy the addr_tlv block
    offset += copy_to_tlv_block(msg_data + offset, hello_msg_ptr->addr_tlv_block_ptr);

    // 3. check offset
    assert(offset == sizeof(msg_header_t) + hello_msg_ptr->header.msg_size);
    return offset;
}

// copy the content of msg_data to tc_msg and return the num of bytes copied.
// this function requires the mem is allocated for tc_msg alraedy.
uint16_t copy_to_tc_msg(uint8_t* msg_data, tc_msg_t* tc_msg_ptr) {
    uint16_t offset = 0;
    uint16_t tmp_len = 0;
    uint8_t* dst_ptr = (uint8_t*) tc_msg_ptr;
    if (msg_data == NULL || dst_ptr == NULL) {
        ESP_LOGW(TAG, "NULL data input!");
        return 0;
    }
    // copy TC msg
    // 1. copy tc msg header
    memcpy(dst_ptr, msg_data, sizeof(msg_header_t));
    offset += sizeof(msg_header_t);
    // we assume that all blocks exist in this msg
    // 2. copy all the blocks (alloc mem first)
    // (1) copy msg_tlv block
    tlv_block_t* tmp_tlv_block_ptr = (tlv_block_t*)(msg_data + offset);
    // this length is special since we store a list of pointers instead of tlv data.
    tmp_len = sizeof(tlv_block_t) + tmp_tlv_block_ptr->tlv_ptr_len * sizeof(tlv_t*);
    tc_msg_ptr->msg_tlv_block_ptr = malloc(tmp_len);
    if (tc_msg_ptr->msg_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for tlv_block!");
        return 0;
    }
    memset(tc_msg_ptr->msg_tlv_block_ptr, 0, tmp_len);
    // copy the msg_tlv block
    offset += copy_to_tlv_block(msg_data + offset, tc_msg_ptr->msg_tlv_block_ptr);
    // (2) copy the addr block
    addr_block_t* tmp_addr_block_ptr = (addr_block_t*)(msg_data + offset);
    tmp_len = get_addr_block_len(tmp_addr_block_ptr);
    tc_msg_ptr->addr_block_ptr = malloc(tmp_len);
    if (tc_msg_ptr->addr_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr_block!");
        free(tc_msg_ptr->msg_tlv_block_ptr);
        return 0;
    }
    memcpy(tc_msg_ptr->addr_block_ptr, msg_data + offset, tmp_len);
    offset += tmp_len;
    // (3) copy addr tlv block
    tmp_tlv_block_ptr = (tlv_block_t*)(msg_data + offset);
    // this length is special since we store a list of pointers instead of tlv data.
    tmp_len = sizeof(tlv_block_t) + tmp_tlv_block_ptr->tlv_ptr_len * sizeof(tlv_t*);
    tc_msg_ptr->addr_tlv_block_ptr = malloc(tmp_len);
    if (tc_msg_ptr->addr_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv_block!");
        free(tc_msg_ptr->msg_tlv_block_ptr);
        free(tc_msg_ptr->addr_block_ptr);
        return 0;
    }
    memset(tc_msg_ptr->addr_tlv_block_ptr, 0, tmp_len);
    // copy the addr_tlv block
    offset += copy_to_tlv_block(msg_data + offset, tc_msg_ptr->addr_tlv_block_ptr);

    // 3. check offset
    assert(offset == sizeof(msg_header_t) + tc_msg_ptr->header.msg_size);
    return offset;
}


void free_rfc5444_pkt (rfc5444_pkt_t pkt) {
    // free possible hello msg
    if (pkt.hello_msg_ptr != NULL) {
        if (pkt.hello_msg_ptr->msg_tlv_block_ptr != NULL)
            free_tlv_block(pkt.hello_msg_ptr->msg_tlv_block_ptr);
        if (pkt.hello_msg_ptr->addr_block_ptr != NULL)
            free(pkt.hello_msg_ptr->addr_block_ptr);
        if (pkt.hello_msg_ptr->addr_tlv_block_ptr != NULL)
            free_tlv_block(pkt.hello_msg_ptr->addr_tlv_block_ptr);
        // free msg struct after free all blocks
        free(pkt.hello_msg_ptr);
    }
    // free possible tc msg
    if (pkt.tc_msg_ptr != NULL) {
        if (pkt.tc_msg_ptr->msg_tlv_block_ptr != NULL)
            free_tlv_block(pkt.tc_msg_ptr->msg_tlv_block_ptr);
        if (pkt.tc_msg_ptr->addr_block_ptr != NULL)
            free(pkt.tc_msg_ptr->addr_block_ptr);
        if (pkt.tc_msg_ptr->addr_tlv_block_ptr != NULL)
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
    // assign values 0x0
    memset((void*)(&ret_pkt), 0, sizeof(rfc5444_pkt_t));
    uint8_t* raw_pkt_ptr = raw_packet.pkt_data;
    uint16_t pkt_offset = 0;
    ret_pkt.version = raw_pkt_ptr[0];
    ret_pkt.pkt_flags = raw_pkt_ptr[1];
    ret_pkt.pkt_len = *((uint16_t*)(raw_pkt_ptr + 2));
    ESP_LOGI(TAG, "Got pkt_len = %d", ret_pkt.pkt_len);
    pkt_offset += RFC5444_PKT_HEADER_LEN;

    // if it is unknown packet. return.
    if (ret_pkt.version != 0) {
        ESP_LOGW(TAG, "Unknown raw packet type!");
        ret_pkt.version = 0;
        ret_pkt.pkt_flags = 0;
        return ret_pkt;
    }

    // loop to parse all msg in one packet.
    while(pkt_offset < ret_pkt.pkt_len) {
        // get msg_type
        switch ((msg_type_t)raw_pkt_ptr[pkt_offset]) {
            case MSG_TYPE_HELLO: {
                // parse HELLO msg
                ret_pkt.hello_msg_ptr = malloc(sizeof(hello_msg_t));
                if(ret_pkt.hello_msg_ptr == NULL) {
                    ESP_LOGE(TAG, "No mem for hello msg!");
                    return ret_pkt;
                }
                memset(ret_pkt.hello_msg_ptr, 0, sizeof(hello_msg_t));
                // copy to hello msg and move offset.
                pkt_offset += copy_to_hello_msg(raw_pkt_ptr + pkt_offset, ret_pkt.hello_msg_ptr);
                break;
            }
            case MSG_TYPE_TC: {
                // parse TC msg
                ret_pkt.tc_msg_ptr = malloc(sizeof(tc_msg_t));
                if(ret_pkt.tc_msg_ptr == NULL) {
                    ESP_LOGE(TAG, "No mem for TC msg!");
                    return ret_pkt;
                }
                memset(ret_pkt.tc_msg_ptr, 0, sizeof(tc_msg_t));
                // copy to hello msg and move offset.
                pkt_offset += copy_to_tc_msg(raw_pkt_ptr + pkt_offset, ret_pkt.tc_msg_ptr);
                break;
            }
            default: {
                ESP_LOGW(TAG, "Unknown msg type = %d !", raw_pkt_ptr[pkt_offset]);
                break;
            }
        }
    }
    assert(pkt_offset == ret_pkt.pkt_len);
    return ret_pkt; // user of this pkt must free it.
}

// rfc5444_pkt should come form info base.
// this function will not consume rfc5444 pkt. remember free it!
// the caller must check the raw_pkt.pkt_data field is not NULL!
raw_pkt_t gen_raw_packet (rfc5444_pkt_t rfc5444_pkt) {
    raw_pkt_t ret_pkt;
    uint16_t pkt_offset = 0;

    ret_pkt.pkt_len = rfc5444_pkt.pkt_len;
    // Send_to event handling in main event loop will free this mem. 
    ret_pkt.pkt_data = malloc(rfc5444_pkt.pkt_len);
    if (ret_pkt.pkt_data == NULL) {
        ESP_LOGE(TAG, "No mem for new paket!");
        return ret_pkt; // return a NULL packet.
    }
    memset(ret_pkt.pkt_data, 0, rfc5444_pkt.pkt_len);
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
        // copy header
        memcpy(ret_pkt.pkt_data + pkt_offset, (uint8_t*)(rfc5444_pkt.tc_msg_ptr), sizeof(msg_header_t));
        pkt_offset += sizeof(msg_header_t);
        // double check msg type
        assert(rfc5444_pkt.tc_msg_ptr->header.msg_type == MSG_TYPE_TC);
        // copy blocks
        // 1. copy msg_tlv_block
        pkt_offset += \
            copy_from_tlv_block(ret_pkt.pkt_data + pkt_offset, rfc5444_pkt.tc_msg_ptr->msg_tlv_block_ptr);
        // 2. copy addr_block
        pkt_offset += \
            copy_from_addr_block(ret_pkt.pkt_data + pkt_offset, rfc5444_pkt.tc_msg_ptr->addr_block_ptr);
        // 3. copy addr_tlv_block
        pkt_offset += \
            copy_from_tlv_block(ret_pkt.pkt_data + pkt_offset, rfc5444_pkt.tc_msg_ptr->addr_tlv_block_ptr);        
    }

    // check pkt offset to make sure pkt len is correct.
    ESP_LOGI(TAG, "offset = %d, len = %d", pkt_offset, ret_pkt.pkt_len);
    assert(pkt_offset == ret_pkt.pkt_len);

    return ret_pkt;
}

