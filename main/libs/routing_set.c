/*  routing_set.c
    Algorithm for Calculating the Routing Set and TC msg
    This file try to follow the algorithm described in RFC7181 Appendix C.(a variation of Dijkstra’s algorithm)
    To separate this part of code from info_base.c, so each file does not get too long.
*/

#include "info_base.h"

// label the usage of routing info (for Dijkstra’s), only valid for routing MPR nodes
// 0 -> NULL, no meaning; 1 -> unused info; -1 -> used info
// NOTE: this is int !
static int8_t routing_info_usage[MAX_PEER_NUM];


void parse_tc_msg (tc_msg_t* tc_msg_ptr) {
    return;
}

void gen_tc_msg (tc_msg_t* tc_msg_ptr) {
    return;
}

