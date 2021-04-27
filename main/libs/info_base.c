#include "info_base.h"

static const char *TAG = "espnow_info_base";

// static variables should be init as zeros by the compiler.

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
    
}

void gen_hello_msg (hello_msg_t* hello_msg_ptr) {

}