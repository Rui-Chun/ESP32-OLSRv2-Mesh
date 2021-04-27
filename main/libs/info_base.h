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

typedef struct remote_node_entry_t {
    uint8_t peer_id;
    uint8_t routing_next_hop; // what is the next hop to get to the remote node
    uint8_t is_routing_mpr;
    uint8_t remote_neighbors[MAX_NEIGHBOUR_NUM]; // this is only useful when is_routing_mpr>0
    uint8_t validity_time_left;
} remote_node_entry_t;

typedef struct two_hop_entry_t {
    uint8_t peer_id;
    link_status_t link_status;
    uint8_t link_metric;
    uint8_t validity_time_left;
}two_hop_entry_t;

/* we only consider one interface, so Interface Information Base merges with Neighbor Information Base. */
typedef struct neighbor_entry_t {
    uint8_t peer_id; // used to index peer mac addr list.
    link_status_t link_status;
    uint8_t link_metric;
    uint8_t is_mpr_willing;
    uint8_t is_flooding_mpr;
    uint8_t is_routing_mpr;
    uint8_t is_flooding_selector;
    uint8_t is_routing_selector;
    uint8_t validity_time_left; // TODO: a time counter to keep entry fresh.
    // record associated two-hop nodes.
    uint8_t associated_peer_ids[MAX_NEIGHBOUR_NUM]; 
} neighbor_entry_t;

// TODO: info_base.c should only store and provide helper functions to operate on info bases.
void info_base_init (uint8_t mac[RFC5444_ADDR_LEN]);
void parse_hello_msg (hello_msg_t* hello_msg_ptr);
void gen_hello_msg (hello_msg_t* hello_msg_ptr);

#endif