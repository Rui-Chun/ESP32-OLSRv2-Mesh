/*  routing_set.c
    Algorithm for Calculating the Routing Set and TC msg
    This file try to follow the algorithm described in RFC7181 Appendix C.(a variation of Dijkstra’s algorithm)
    To separate this part of code from info_base.c, so each file does not get too long.
*/

#include "info_base.h"


static const char *TAG = "espnow_routing_set";

// label the usage of routing info (for Dijkstra’s), only valid for routing MPR nodes
// 0 -> NULL, not routing MPR; 1 -> unused info; -1 -> used info
// NOTE: this is int !
static int8_t routing_info_usage[MAX_PEER_NUM];


remote_node_entry_t* register_new_remote(uint8_t node_id) {
    if (node_id == 0 ) {
        ESP_LOGW(TAG, "Do not register peer #0!");
        return NULL;
    }
    // must be unregistered
    assert(entry_ptr_list[node_id] == NULL);
    remote_node_entry_t* ret_entry = calloc(1, sizeof(remote_node_entry_t)); // set to zeros
    if(ret_entry == NULL) {
        ESP_LOGE(TAG, "No mem for a new remote node entry.");
        return NULL;
    }
    // init two-hop entry
    ret_entry->entry_type = REMOTE_NODE_ENTRY;
    ret_entry->peer_id = node_id;
    // MUST set path metric as INF at init stage
    ret_entry->routing_info.next_hop = 0;
    ret_entry->routing_info.hop_num = 255;
    ret_entry->routing_info.path_metric = 255;
    // register the entry to the entry list
    entry_ptr_list[node_id] = ret_entry;

    ESP_LOGI(TAG, "A new remote node entry registered.");
    return ret_entry;
}

// parse the link info given a TC msg
void parse_tc_addr_block(remote_node_entry_t* remote_entry_ptr, tc_msg_t* tc_msg_ptr, uint32_t tc_valid_until) {
    // 1. get addr tlv pointers.
    uint8_t link_num = tc_msg_ptr->addr_block_ptr->addr_num;
    assert( tc_msg_ptr->addr_tlv_block_ptr->tlv_ptr_len == TC_ADDR_TLV_NUM );
    tlv_t* link_metric_tlv_ptr = tc_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[0];
    assert( link_metric_tlv_ptr->tlv_value_len == link_num * 2); // out metric list + in metric list !

    // 2. delete and alloc link info struct
    if (remote_entry_ptr->link_info.link_num != 0) {
        // delete old values
        free(remote_entry_ptr->link_info.id_list_ptr);
        free(remote_entry_ptr->link_info.metric_list_ptr);
        free(remote_entry_ptr->link_info.in_metric_list_ptr);
    }
    remote_entry_ptr->link_info.link_num = link_num;
    remote_entry_ptr->link_info.id_list_ptr = calloc(link_num, sizeof(uint8_t));
    if (remote_entry_ptr->link_info.id_list_ptr == NULL) return;
    remote_entry_ptr->link_info.metric_list_ptr = calloc(link_num, sizeof(uint8_t));
    if (remote_entry_ptr->link_info.metric_list_ptr == NULL) {
        free(remote_entry_ptr->link_info.id_list_ptr);
        return;
    }
    remote_entry_ptr->link_info.in_metric_list_ptr = calloc(link_num, sizeof(uint8_t));
    if (remote_entry_ptr->link_info.in_metric_list_ptr == NULL) {
        free(remote_entry_ptr->link_info.id_list_ptr);
        free(remote_entry_ptr->link_info.metric_list_ptr);
        return;
    }
    // copy in metric data, two lists
    memcpy(remote_entry_ptr->link_info.metric_list_ptr, link_metric_tlv_ptr->tlv_value, link_num);
    memcpy(remote_entry_ptr->link_info.in_metric_list_ptr, link_metric_tlv_ptr->tlv_value + link_num, link_num);


    // 3. loop over addr block values.
    uint8_t* link_addr_ptr = NULL;
    uint8_t sender_selector_id = 0; // here means the selector of the TC msg sender
    for(int l=0; l < link_num; l++) {
        link_addr_ptr = tc_msg_ptr->addr_block_ptr->addr_list + l * RFC5444_ADDR_LEN;
        // it should not point to me
        assert(memcmp(link_addr_ptr, originator_addr, RFC5444_ADDR_LEN) != 0);
        
        // if we have seen this node before.
        if( get_or_create_id(link_addr_ptr, &sender_selector_id) ) {
            // store this id
            remote_entry_ptr->link_info.id_list_ptr[l] = sender_selector_id;
            uint8_t* tmp_type_ptr = (uint8_t*)(entry_ptr_list[sender_selector_id]);
            remote_node_entry_t* tmp_remote_ptr = NULL;
            // if this is a remote node entry.
            if (tmp_type_ptr[0] == REMOTE_NODE_ENTRY) {
                // update validity
                tmp_remote_ptr = (remote_node_entry_t*) tmp_type_ptr;
                tmp_remote_ptr->valid_until = tc_valid_until;
            }
            // if this is a neighbor node or two hop node. do nothing.
        }
        else {
            // a new two hop entry.
            assert(sender_selector_id != 0);
            remote_entry_ptr->link_info.id_list_ptr[l] = sender_selector_id;
            remote_node_entry_t* ret_entry_ptr = register_new_remote(sender_selector_id);
            ret_entry_ptr->valid_until = tc_valid_until;
        }

    }

}


// NOTE:(flooding reduction)
//      TODO!:only forward this msg if it comes from one of your flooding MPR selectors.
//      also check the msg_num to make sure msg is fresh. And add hop_count by 1.
// return 0 to indicate handler not to forward.
uint8_t parse_tc_msg (tc_msg_t* tc_msg_ptr) {
    assert(tc_msg_ptr != NULL);
    ESP_LOGI(TAG, "Start to parse TC msg.");

    // get msg originator address.
    uint8_t remote_id = 0;
    remote_node_entry_t* tc_remote_entry_ptr = NULL; // remote entry and two-hop entry are inter-changeable.
    uint8_t* tc_orig_addr = tc_msg_ptr->header.msg_orig_addr;

    // 1. check and update peer_list and entry_list
    if (get_or_create_id(tc_orig_addr, &remote_id)) {
        // if this node has already been stored
        uint8_t* unknown_entry = entry_ptr_list[remote_id];
        // check the current entry type
        switch (unknown_entry[0]) {
            case NEIGHBOR_ENTRY: {
                // we already know the links of all neighbors, do not process the TC msg, just forward if needed.
                ESP_LOGI(TAG, "TC msg is from a familiar neighbor node! Nothing will be done.");
                neighbor_entry_t* hello_neighbor_entry = (neighbor_entry_t*) unknown_entry;
                // check TC msg seq_num
                // if the node restarts, do not drop the packet.
                if (tc_msg_ptr->header.msg_seq_num > 0 && tc_msg_ptr->header.msg_seq_num <= hello_neighbor_entry->msg_seq_num) {
                    ESP_LOGW(TAG, "Got an out-dated packet, drop it.");
                    // do not forward this msg
                    return 0;
                }
                // otherwise, done parsing, forward this msg
                hello_neighbor_entry->msg_seq_num = tc_msg_ptr->header.msg_seq_num;
                return 1;
                break;
            }
            case TWO_HOP_ENTRY: {
                ESP_LOGI(TAG, "TC msg is from a two-hop node.");
                tc_remote_entry_ptr = (remote_node_entry_t*) unknown_entry;
                break;
            }
            case REMOTE_NODE_ENTRY: {
                ESP_LOGI(TAG, "TC msg is from a remote node.");
                tc_remote_entry_ptr = (remote_node_entry_t*) unknown_entry;
                break;
            }
            default: {
                ESP_LOGE(TAG, "An unknown entry type!");
                break;
            }
        }
    } else {
        // a new remote node.
        ESP_LOGI(TAG, "A new remote MPR node is heard! addr = "MACSTR" .", MAC2STR(tc_orig_addr));
        tc_remote_entry_ptr = register_new_remote(remote_id);
    }
    
    // 2. update entry, neighor and two hop entries
    assert(tc_remote_entry_ptr->peer_id == remote_id);
    // check msg seq_num is fresh
    if (tc_msg_ptr->header.msg_seq_num > 0 && tc_msg_ptr->header.msg_seq_num <= tc_remote_entry_ptr->msg_seq_num) {
        ESP_LOGW(TAG, "Got an out-dated packet, drop it.");
        return 0;
    }
    // update seq_num
    tc_remote_entry_ptr->msg_seq_num = tc_msg_ptr->header.msg_seq_num;
    uint8_t* tmp_value_ptr = NULL;
    // TODO: does remote node need this MPR willing field?
    // assert( get_tlv_value(tc_msg_ptr->msg_tlv_block_ptr, IS_MPR_WILLING, &tmp_value_ptr) == 1 );
    // tc_remote_entry_ptr->is_mpr_willing = *tmp_value_ptr;
    assert( get_tlv_value(tc_msg_ptr->msg_tlv_block_ptr, VALIDITY_TIME, &tmp_value_ptr) == 1 );
    tc_remote_entry_ptr->valid_until =  global_tick_num + *tmp_value_ptr;
    // update MPR status
    tc_remote_entry_ptr->routing_status = ROUTING_TO;
    
    // update link info, TODO: also add remote node entries!
    parse_tc_addr_block(tc_remote_entry_ptr, tc_msg_ptr, tc_remote_entry_ptr->valid_until);

    // update id_lists, to keep them correct
    update_id_lists();
    return;
}

// return the number of routing MPR selectors
uint8_t update_routing_selectors (uint8_t* selector_id_list) {
    uint8_t neighbor_id = 0;
    neighbor_entry_t* neighbor_ptr = NULL;
    uint8_t ret_num = 0;
    for (int n=0; n < neighbor_id_num; n++) {
        neighbor_id = neighbor_id_list[n];
        neighbor_ptr = entry_ptr_list[neighbor_id];
        assert(neighbor_ptr != NULL);
        // only consider symmetric neighbors !
        if (neighbor_ptr->link_status != LINK_SYMMETRIC) continue;
        if (neighbor_ptr->routing_status == ROUTING_FROM || neighbor_ptr->routing_status == ROUTING_TO_FROM) {
            selector_id_list[ret_num++] = neighbor_id;
        }
    }
    return ret_num;
}

void gen_tc_msg_tlv (tlv_block_t* msg_tlv_block_ptr) {
    msg_tlv_block_ptr->tlv_block_type = 0;
    msg_tlv_block_ptr->tlv_ptr_len = TC_MSG_TLV_NUM; // three tlv entries
    msg_tlv_block_ptr->tlv_block_size = 0;

    // assign tlv entries
    // 1. VALIDITY_TIME
    msg_tlv_block_ptr->tlv_ptr_list[0] = malloc(cal_tlv_len(VALIDITY_TIME));
    if(msg_tlv_block_ptr->tlv_ptr_list[0] == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        free(msg_tlv_block_ptr);
        return;
    }
    msg_tlv_block_ptr->tlv_ptr_list[0]->tlv_type = VALIDITY_TIME;
    msg_tlv_block_ptr->tlv_ptr_list[0]->tlv_value_len = 1;
    msg_tlv_block_ptr->tlv_ptr_list[0]->tlv_value[0] = TC_VALIDITY_TICKS;
    msg_tlv_block_ptr->tlv_block_size += cal_tlv_len(VALIDITY_TIME);

    // 2. INTERVAL_TIME
    msg_tlv_block_ptr->tlv_ptr_list[1] = malloc(cal_tlv_len(INTERVAL_TIME));
    if(msg_tlv_block_ptr->tlv_ptr_list[1] == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        free(msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(msg_tlv_block_ptr);
        return;
    }
    msg_tlv_block_ptr->tlv_ptr_list[1]->tlv_type = INTERVAL_TIME;
    msg_tlv_block_ptr->tlv_ptr_list[1]->tlv_value_len = 1;
    msg_tlv_block_ptr->tlv_ptr_list[1]->tlv_value[0] = TC_INTERVAL_TICKS;
    msg_tlv_block_ptr->tlv_block_size += cal_tlv_len(INTERVAL_TIME);

    // 3. MPR_WILLING
    msg_tlv_block_ptr->tlv_ptr_list[2] = malloc(cal_tlv_len(MPR_WILLING));
    if(msg_tlv_block_ptr->tlv_ptr_list[2] == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        free(msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(msg_tlv_block_ptr);
        return;
    }
    msg_tlv_block_ptr->tlv_ptr_list[2]->tlv_type = MPR_WILLING;
    msg_tlv_block_ptr->tlv_ptr_list[2]->tlv_value_len = 1;
    msg_tlv_block_ptr->tlv_ptr_list[2]->tlv_value[0] = IS_MPR_WILLING;
    msg_tlv_block_ptr->tlv_block_size += cal_tlv_len(MPR_WILLING);

    return;
}

// NOTE:(topology reduction)
//      only generate TC msg if you are routing MPR selected by at least one of the neighbors.
//      only contain info of your routing MPR selector.
// this function assumes that tc msg has got mem allocated.
// return 0 to indicate that no TC is generated.
uint8_t gen_tc_msg (tc_msg_t* tc_msg_ptr) {
    assert(tc_msg_ptr != NULL);
    
    uint8_t selector_id_list[MAX_NEIGHBOUR_NUM];
    uint8_t selector_num = 0;

    selector_num = update_routing_selectors(selector_id_list);
    if (selector_num == 0) {
        ESP_LOGI(TAG, "No routing selector, no TX msg.");
        return 0;
    }

    // assign values to the header.
    msg_header_t* header_ptr = &tc_msg_ptr->header;
    header_ptr->msg_type = MSG_TYPE_TC;
    header_ptr->msg_flags = 0; // useless currently
    header_ptr->msg_addr_len = RFC5444_ADDR_LEN - 1; // useless since we only consider MAC addr
    header_ptr->msg_size = 0; // this needs to be calculated later.
    memcpy(header_ptr->msg_orig_addr, originator_addr, RFC5444_ADDR_LEN);
    header_ptr->msg_hop_limit = 255;
    header_ptr->msg_hop_count = 0;
    header_ptr->msg_seq_num = global_msg_seq_num++;

    // alloc and set mem for blocks
    // 1. msg tlv block, validity time and interval time.
    uint16_t tmp_len = sizeof(tlv_block_t) + TC_MSG_TLV_NUM * sizeof(tlv_t*); // three pointers.
    tc_msg_ptr->msg_tlv_block_ptr = malloc(tmp_len);
    if(tc_msg_ptr->msg_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        return 0;
    }
    gen_tc_msg_tlv(tc_msg_ptr->msg_tlv_block_ptr);
    header_ptr->msg_size += get_tlv_block_len(tc_msg_ptr->msg_tlv_block_ptr);
    ESP_LOGI(TAG, "TC Msg with tlv block, len = %d", header_ptr->msg_size);

    // 2. addr block, put in all neighbors.
    ESP_LOGI(TAG, "routing selector_num = %d", selector_num);
    tmp_len = sizeof(addr_block_t) + selector_num * RFC5444_ADDR_LEN;
    tc_msg_ptr->addr_block_ptr = malloc(tmp_len);
    if(tc_msg_ptr->addr_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr block!");
        free(tc_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(tc_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(tc_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[2]);
        free(tc_msg_ptr->msg_tlv_block_ptr);
        return 0;
    }
    tc_msg_ptr->addr_block_ptr->addr_num = selector_num;
    for(int s=0; s < selector_num; s++) {
        memcpy(tc_msg_ptr->addr_block_ptr->addr_list + s * RFC5444_ADDR_LEN,\
                peer_addr_list[selector_id_list[s]], RFC5444_ADDR_LEN);
    }
    header_ptr->msg_size += get_addr_block_len(tc_msg_ptr->addr_block_ptr);
    ESP_LOGI(TAG, "TC Msg with addr block, len = %d", header_ptr->msg_size);

    // 3. addr tlv block.
    tmp_len = sizeof(tlv_block_t) + sizeof(tlv_t*) * TC_ADDR_TLV_NUM ; // three tlv entry pointers!
    tc_msg_ptr->addr_tlv_block_ptr = malloc(tmp_len);
    if(tc_msg_ptr->addr_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv block!");
        free(tc_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(tc_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(tc_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[2]);
        free(tc_msg_ptr->msg_tlv_block_ptr);
        free(tc_msg_ptr->addr_block_ptr);
        return 0;
    }
    // generate addr tlv block and all entries
    tc_msg_ptr->addr_tlv_block_ptr->tlv_block_type = 0;
    tc_msg_ptr->addr_tlv_block_ptr->tlv_ptr_len = TC_ADDR_TLV_NUM; // three tlv entry
    tc_msg_ptr->addr_tlv_block_ptr->tlv_block_size = 0; // to be updated.
    // (1) LINK_STATUS TLV not needed

    // (2) LINK_METRIC TLV
    tmp_len = sizeof(tlv_t) + selector_num * 2; // out and in metric lists
    tlv_t* tmp_tlv_ptr = malloc(tmp_len);
    tc_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[0] = tmp_tlv_ptr;
    if(tmp_tlv_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv 1 entry!");
        free(tc_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(tc_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(tc_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[2]);
        free(tc_msg_ptr->msg_tlv_block_ptr);
        free(tc_msg_ptr->addr_block_ptr);
        free(tc_msg_ptr->addr_tlv_block_ptr);
        return 0;
    }
    tmp_tlv_ptr->tlv_type = LINK_METRIC;
    tmp_tlv_ptr->tlv_value_len = selector_num * 2;
    for(int s=0; s < selector_num; s++) {
        neighbor_entry_t* neighbor_entry_ptr = entry_ptr_list[selector_id_list[s]];
        assert(neighbor_entry_ptr->entry_type == NEIGHBOR_ENTRY && neighbor_entry_ptr->peer_id == selector_id_list[s]);
        ESP_LOGI(TAG, "routing selector #%d with out metric %d, in metric %d", selector_id_list[s], neighbor_entry_ptr->link_metric, neighbor_entry_ptr->in_link_metric);
        tmp_tlv_ptr->tlv_value[s] = neighbor_entry_ptr->link_metric; // assign out link metric value
        tmp_tlv_ptr->tlv_value[s + selector_num] = neighbor_entry_ptr->in_link_metric; // assign in link metric value
    }
    // udpate block size
    tc_msg_ptr->addr_tlv_block_ptr->tlv_block_size += tmp_len;

    // (3) MPR_STATUS not needed
   
    // update msg size given the addr tlv block
    header_ptr->msg_size += get_tlv_block_len(tc_msg_ptr->addr_tlv_block_ptr);

    // calculate msg len!
    ESP_LOGI(TAG, "A new TC msg with len = %d", header_ptr->msg_size);
    // done.
    // ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());
    // ESP_LOGI(TAG, "task stack water mark : %d", uxTaskGetStackHighWaterMark(NULL));
    return 1;
}

