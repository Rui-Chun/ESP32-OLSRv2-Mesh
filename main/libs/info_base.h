/*
 * local info base structures and functions 
 */

#ifndef INFO_BASE_H
#define INFO_BASE_H
#include <stdint.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "rfc5444.h"

/* Protocol Parameters and Constants */
#define MAX_PEER_NUM 128
#define MAX_NEIGHBOUR_NUM 64

#define HELLO_VALIDITY_TICKS 15
#define HELLO_INTERVAL_TICKS 3
#define TC_VALIDITY_TICKS 20
#define TC_INTERVAL_TICKS 5
#define IS_MPR_WILLING       1   // Is current node willing to work as MPR node?

#define TC_MSG_TLV_NUM    3   // number of TLV entries in msg_tlv_block
#define TC_ADDR_TLV_NUM    1  // number of TLV entries in addr_tlv_block
#define HELLO_MSG_TLV_NUM    3   // number of TLV entries in msg_tlv_block
#define HELLO_ADDR_TLV_NUM    3  // number of TLV entries in addr_tlv_block
/* Protocol Parameters and Constants End */

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

/* extern variables to share with routing_set.c */
extern uint32_t global_msg_seq_num;
extern uint32_t global_tick_num; // time counter based on tick
extern uint8_t peer_num; // 255 should be enough.
extern uint8_t peer_addr_list[MAX_PEER_NUM][RFC5444_ADDR_LEN]; // use peer_id to get mac address.
extern void* entry_ptr_list[MAX_PEER_NUM];
extern uint8_t neighbor_id_num;
extern uint8_t neighbor_id_list[MAX_NEIGHBOUR_NUM];
extern uint8_t two_hop_id_num;
extern uint8_t two_hop_id_list[MAX_PEER_NUM];
extern uint8_t remote_id_num;
extern uint8_t remote_id_list[MAX_PEER_NUM];
extern uint8_t originator_addr[RFC5444_ADDR_LEN];


typedef enum link_status_t {
    LINK_HEARD = 0,
    LINK_SYMMETRIC,
    LINK_LOST,          // do not use it. Just delete entry if timeout.
} link_status_t;

typedef enum flooding_mpr_status_t {
    NOT_FLOODING = 0,                   // this node is not a flooding MPR
    FLOODING_TO,                    // this node is local node's flooding MPR
    FLOODING_FROM,                  // is flooding selector
    FLOODING_TO_FROM,               // is flooding MPR and selector
} flooding_mpr_status_t;

typedef enum routing_mpr_status_t {
    NOT_ROUTING = 0,
    ROUTING_TO,
    ROUTING_FROM,
    ROUTING_TO_FROM,
} routing_mpr_status_t;

typedef enum entry_type_t {
    NEIGHBOR_ENTRY,
    TWO_HOP_ENTRY,
    REMOTE_NODE_ENTRY,
} entry_type_t;

typedef struct link_info_t {
    uint8_t link_num;
    uint8_t* id_list_ptr; // peer id of the other side.
    uint8_t* metric_list_ptr; // this is out going metric.
    uint8_t* in_metric_list_ptr; // in comming link metric ptr list. TODO: this seems useless, may delete it.
} link_info_t;

typedef struct routing_info_t {
    uint8_t next_hop;
    uint8_t hop_num;
    uint8_t path_metric;
} routing_info_t;

// TODO: add support for defining gateways.

typedef struct remote_node_entry_t {
    uint8_t entry_type;
    uint8_t peer_id;
    uint32_t msg_seq_num;   // most recent msg seq num , to avoid old packet.
    uint32_t valid_until;
    routing_mpr_status_t routing_status;
    link_info_t link_info;
    routing_info_t routing_info;
} remote_node_entry_t;

// Note: two-hop entry and remote entry are inter-changeable.
typedef struct two_hop_entry_t {
    uint8_t entry_type;
    uint8_t peer_id;
    uint32_t msg_seq_num;   // most recent msg seq num , to avoid old packet.
    uint32_t valid_until;
    routing_mpr_status_t routing_status;
    link_info_t link_info;
    routing_info_t routing_info;
}two_hop_entry_t;

/* we only consider one interface, so Interface Information Base merges with Neighbor Information Base. */
typedef struct neighbor_entry_t {
    uint8_t entry_type;
    uint8_t peer_id; // used to index peer mac addr list.
    uint32_t msg_seq_num;   // most recent msg seq num , to avoid old packet.
    uint32_t valid_until;
    link_status_t link_status;
    uint8_t link_metric;  // out going link metric
    uint8_t in_link_metric; // in comming metric
    uint8_t is_mpr_willing;
    flooding_mpr_status_t flooding_status;
    routing_mpr_status_t routing_status;
    link_info_t link_info;
    routing_info_t routing_info; // this is needed because there may be asymmetric neighbors.
} neighbor_entry_t;

// TODO: info_base.c should only store and provide helper functions to operate on info bases.
void info_base_init (uint8_t mac[RFC5444_ADDR_LEN]);
void set_info_base_time (uint32_t tick);
void parse_hello_msg (hello_msg_t* hello_msg_ptr);
void gen_hello_msg (hello_msg_t* hello_msg_ptr);
uint8_t parse_tc_msg (tc_msg_t* tc_msg_ptr, uint8_t recv_mac[RFC5444_ADDR_LEN]);
uint8_t gen_tc_msg (tc_msg_t* tc_msg_ptr);
uint8_t get_or_create_id (uint8_t mac_addr[RFC5444_ADDR_LEN], uint8_t* peer_id);
void update_mpr_status ();
void check_entry_validity();
void update_id_lists();

#endif