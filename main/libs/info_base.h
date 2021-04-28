/*
 * local info base structures and functions 
 */

#ifndef INFO_BASE_H
#define INFO_BASE_H
#include <stdint.h>
#include "rfc5444.h"

/* Protocol Parameters and Constants */
#define MAX_PEER_NUM 128
#define MAX_NEIGHBOUR_NUM 64

#define HELLO_VALIDITY_TICKS 5
#define HELLO_INTERVAL_TICKS 1
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
    HEARD,
    SYMMETRIC,
    LOST,
} link_status_t;

typedef enum flooding_mpr_status_t {
    NOT_FLOODING,                   // this node is not a flooding MPR
    FLOODING_TO,                    // this node is local node's flooding MPR
    FLOODING_FROM,                  // is flooding selector
    FLOODING_TO_FROM,               // is flooding MPR and selector
} flooding_mpr_status_t;

typedef enum routing_mpr_status_t {
    NOT_ROUTING,
    ROUTING_TO,
    ROUTING_FROM,
    ROUTING_TO_FROM,
} routing_mpr_status_t;

typedef struct remote_node_entry_t {
    uint8_t peer_id;
    uint8_t routing_next_hop; // what is the next hop to get to the remote node
    routing_mpr_status_t routing_status;
    uint8_t neighbor_id_list[MAX_NEIGHBOUR_NUM]; // this is only useful when is_routing_mpr
    uint32_t valid_until;
} remote_node_entry_t;

typedef struct two_hop_entry_t {
    uint8_t peer_id;
    uint8_t routing_next_hop; // what is the next hop to get to the remote node
    link_status_t link_status;
    uint8_t link_metric;
    routing_mpr_status_t routing_status;
    uint8_t neighbor_id_list[MAX_NEIGHBOUR_NUM]; // this is only useful when is_routing_mpr
    uint32_t valid_until;
}two_hop_entry_t;

/* we only consider one interface, so Interface Information Base merges with Neighbor Information Base. */
typedef struct neighbor_entry_t {
    uint8_t peer_id; // used to index peer mac addr list.
    link_status_t link_status;
    uint8_t link_metric;
    uint8_t is_mpr_willing;
    flooding_mpr_status_t flooding_status;
    routing_mpr_status_t routing_status;
    uint32_t valid_until; // TODO: a time limit to keep entry fresh.
    // record associated two-hop nodes.
    uint8_t neighbor_id_list[MAX_NEIGHBOUR_NUM]; 
} neighbor_entry_t;

// TODO: info_base.c should only store and provide helper functions to operate on info bases.
void info_base_init (uint8_t mac[RFC5444_ADDR_LEN]);
void parse_hello_msg (hello_msg_t* hello_msg_ptr);
void gen_hello_msg (hello_msg_t* hello_msg_ptr);

#endif