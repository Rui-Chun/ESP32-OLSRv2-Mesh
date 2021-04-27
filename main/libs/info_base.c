#include "info_base.h"

static const char *TAG = "espnow_info_base";

// static variables should be init as zeros by the compiler.

// message seq num, to indicate a new msg
static uint32_t global_msg_seq_num = 0;

// a static list for all peer nodes' addresses.
// Note: We use peer_id #0 to mark empty!
static uint8_t peer_addr_list[MAX_PEER_NUM][RFC5444_ADDR_LEN]; // use peer_id to get mac address.
static void* entry_ptr_list[MAX_PEER_NUM];

// Neighbor Information Base
// info is stored in entries, get it from entry ptr list.
static uint8_t neighbor_id_list[MAX_NEIGHBOUR_NUM];
static uint8_t two_hop_id_list[MAX_PEER_NUM];

// Topology Information Base
// get remote_node_entry for the info !
// 1. A Routable Address Topology Set
// 2. A Router Topology Set, recording links between routers in the MANET
// 3. A Routing Set, recording routes from this router to all available destinations.
static uint8_t remote_id_list[MAX_PEER_NUM];
// TODO: 
// 3. An Attached Network Set, recording a gateway

// received message info base, to prevent msg processed/forwarded twice.
static uint32_t seq_num_seen[MAX_PEER_NUM];

// Local Information Base: Originator address
static uint8_t originator_addr[RFC5444_ADDR_LEN];

void info_base_init (uint8_t mac[RFC5444_ADDR_LEN]) {
    memcpy(originator_addr, mac, RFC5444_ADDR_LEN);
    ESP_LOGI(TAG, "init done, mac addr =  "MACSTR".", MAC2STR(originator_addr));
}

void parse_hello_msg (hello_msg_t* hello_msg_ptr) {
    // TODO:
    ESP_LOGI(TAG, "Start to parse HELLO msg.");

}

void gen_hello_msg_tlv (tlv_block_t* msg_tlv_block_ptr) {
    msg_tlv_block_ptr->tlv_block_type = 0;
    msg_tlv_block_ptr->tlv_ptr_len = 2;
    msg_tlv_block_ptr->tlv_block_size = 0;

    // assign tlv entries
    msg_tlv_block_ptr->tlv_ptr_list[0] = malloc(cal_tlv_len(VALIDITY_TIME));
    if(msg_tlv_block_ptr->tlv_ptr_list[0] == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        free(msg_tlv_block_ptr);
        return;
    }
    msg_tlv_block_ptr->tlv_ptr_list[0]->tlv_type = VALIDITY_TIME;
    msg_tlv_block_ptr->tlv_ptr_list[0]->tlv_value_len = 1;
    msg_tlv_block_ptr->tlv_ptr_list[0]->tlv_value[0] = HELLO_VALIDITY_TICKS;
    msg_tlv_block_ptr->tlv_block_size += cal_tlv_len(VALIDITY_TIME);

    msg_tlv_block_ptr->tlv_ptr_list[1] = malloc(cal_tlv_len(INTERVAL_TIME));
    if(msg_tlv_block_ptr->tlv_ptr_list[1] == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        free(msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(msg_tlv_block_ptr);
        return;
    }
    msg_tlv_block_ptr->tlv_ptr_list[1]->tlv_type = INTERVAL_TIME;
    msg_tlv_block_ptr->tlv_ptr_list[1]->tlv_value_len = 1;
    msg_tlv_block_ptr->tlv_ptr_list[1]->tlv_value[1] = HELLO_INTERVAL_TICKS;
    msg_tlv_block_ptr->tlv_block_size += cal_tlv_len(INTERVAL_TIME);

    return;
}

// this function assumes that hello msg has got mem allocated.
void gen_hello_msg (hello_msg_t* hello_msg_ptr) {
    // TODO:
    if(hello_msg_ptr == NULL) return;

    // assign values to the header.
    msg_header_t* header_ptr = &hello_msg_ptr->header;
    header_ptr->msg_type = MSG_TYPE_HELLO;
    header_ptr->msg_flags = 0; // useless currently
    header_ptr->msg_addr_len = RFC5444_ADDR_LEN - 1; // useless since we only consider MAC addr
    header_ptr->msg_size = 0; // this needs to be calculated later.
    memcpy(header_ptr->msg_orig_addr, originator_addr, RFC5444_ADDR_LEN);
    header_ptr->msg_hop_limit = 1;
    header_ptr->msg_hop_count = 0;
    header_ptr->msg_seq_num = global_msg_seq_num++;

    // alloc and set mem for blocks
    // 1. msg tlv block, validity time and interval time.
    uint16_t tmp_len = sizeof(tlv_block_t) + 2 * sizeof(tlv_t*); // two pointers.
    hello_msg_ptr->msg_tlv_block_ptr = malloc(tmp_len);
    if(hello_msg_ptr->msg_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        return;
    }
    gen_hello_msg_tlv(hello_msg_ptr->msg_tlv_block_ptr);
    header_ptr->msg_size += get_tlv_block_len(hello_msg_ptr->msg_tlv_block_ptr);
    ESP_LOGI(TAG, "Msg with tlv block, len = %d", header_ptr->msg_size);

    // 2. addr block, put in all neighbors.
    uint16_t neighbor_num = strlen((char*)neighbor_id_list); // consider it as str, stop until first non-zero
    ESP_LOGI(TAG, "neighbor_num = %d", neighbor_num);
    tmp_len = sizeof(addr_block_t) + neighbor_num * RFC5444_ADDR_LEN;
    hello_msg_ptr->addr_block_ptr = malloc(tmp_len);
    if(hello_msg_ptr->addr_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr block!");
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(hello_msg_ptr->msg_tlv_block_ptr);
        return;
    }
    hello_msg_ptr->addr_block_ptr->addr_num = neighbor_num;
    for(int n=0; n < neighbor_num; n++) {
        memcpy(hello_msg_ptr->addr_block_ptr->addr_list + n * RFC5444_ADDR_LEN,\
                peer_addr_list[neighbor_id_list[n]], RFC5444_ADDR_LEN);
    }
    header_ptr->msg_size += get_addr_block_len(hello_msg_ptr->addr_block_ptr);
    ESP_LOGI(TAG, "Msg with addr block, len = %d", header_ptr->msg_size);

    // 3. addr tlv block.
    tmp_len = sizeof(tlv_block_t) + sizeof(tlv_t*); // one tlv ptr
    hello_msg_ptr->addr_tlv_block_ptr = malloc(tmp_len);
    if(hello_msg_ptr->addr_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv block!");
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(hello_msg_ptr->msg_tlv_block_ptr);
        free(hello_msg_ptr->addr_block_ptr);
        return;
    }
    // one addr tlv entry -> LINK_STATUS
    hello_msg_ptr->addr_tlv_block_ptr->tlv_block_type = 0;
    hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_len = 1; // one tlv entry
    hello_msg_ptr->addr_tlv_block_ptr->tlv_block_size = 0; // to be updated.

    tmp_len = sizeof(tlv_t) + neighbor_num;
    hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[0] = malloc(tmp_len);
    tlv_t* tmp_tlv_ptr = hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[0];
    if(tmp_tlv_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv entry!");
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(hello_msg_ptr->msg_tlv_block_ptr);
        free(hello_msg_ptr->addr_block_ptr);
        free(hello_msg_ptr->addr_tlv_block_ptr);
        return;
    }
    hello_msg_ptr->addr_tlv_block_ptr->tlv_block_size += tmp_len;
    tmp_tlv_ptr->tlv_type = LINK_STATUS; // TODO: change this type to include more link info!
    tmp_tlv_ptr->tlv_value_len = neighbor_num;
    for(int n=0; n < neighbor_num; n++) {
        neighbor_entry_t* neighbor_entry_ptr = entry_ptr_list[neighbor_id_list[n]];
        tmp_tlv_ptr->tlv_value[n] = neighbor_entry_ptr->link_status;
    }
    header_ptr->msg_size += get_tlv_block_len(hello_msg_ptr->addr_tlv_block_ptr);
    // calculate msg len!
    ESP_LOGI(TAG, "A new HELLO with len = %d", header_ptr->msg_size);
    // done.
}