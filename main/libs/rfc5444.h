/*
 * packet and messgae structures 
 */

#ifndef RFC_5444_H
#define RFC_5444_H
#include <stdint.h>

typedef struct raw_packet {
    uint16_t pkt_len;
    uint8_t *pkt_data;
} raw_packet;

#endif