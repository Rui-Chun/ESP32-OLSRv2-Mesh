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
#define IS_MPR_WILLING       1   // Is current node willing to work as MPR node?

#define HELLO_MSG_TLV_NUM    3   // number of TLV entries in msg_tlv_block
#define HELLO_ADDR_TLV_NUM    3  // number of TLV entries in addr_tlv_block
/* Protocol Parameters and Constants End */

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

// #define IS_BROADCAST_ADDR(addr) (memcmp(addr, s_example_broadcast_mac, RFC5444_ADDR_LEN) == 0)

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
    uint8_t* metric_list_ptr;
} link_info_t;

typedef struct remote_node_entry_t {
    uint8_t entry_type;
    uint8_t peer_id;
    uint32_t msg_seq_num;   // most recent msg seq num , to avoid old packet.
    uint32_t valid_until;
    uint8_t routing_next_hop; // what is the next hop to get to the remote node
    routing_mpr_status_t routing_status;
    link_info_t link_info;
} remote_node_entry_t;

// Note: two-hop entry and remote entry are inter-changeable.
typedef struct two_hop_entry_t {
    uint8_t entry_type;
    uint8_t peer_id;
    uint32_t msg_seq_num;   // most recent msg seq num , to avoid old packet.
    uint32_t valid_until;
    uint8_t routing_next_hop; // what is the next hop to get to the remote node
    routing_mpr_status_t routing_status;
    link_info_t link_info;
}two_hop_entry_t;

/* we only consider one interface, so Interface Information Base merges with Neighbor Information Base. */
typedef struct neighbor_entry_t {
    uint8_t entry_type;
    uint8_t peer_id; // used to index peer mac addr list.
    uint32_t msg_seq_num;   // most recent msg seq num , to avoid old packet.
    uint32_t valid_until;
    link_status_t link_status;
    uint8_t link_metric;  // TODO: enable link metric.
    uint8_t is_mpr_willing;
    flooding_mpr_status_t flooding_status;
    routing_mpr_status_t routing_status;
    link_info_t link_info;
} neighbor_entry_t;

// TODO: info_base.c should only store and provide helper functions to operate on info bases.
void info_base_init (uint8_t mac[RFC5444_ADDR_LEN]);
void set_info_base_time (uint32_t tick);
void parse_hello_msg (hello_msg_t* hello_msg_ptr);
void gen_hello_msg (hello_msg_t* hello_msg_ptr);
void update_mpr_status ();

#endif