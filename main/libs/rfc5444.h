/*
 * packet and messgae structures 
 * This file only uses some parts of rfc5444. It is not a lib that complies with the protocol.
 * The Message Types used are the HELLO message and the TC message.
 */

#ifndef RFC_5444_H
#define RFC_5444_H
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "esp_log.h"

// the packet bytes to be sent to or received from.
typedef struct raw_pkt_t {
    uint16_t pkt_len;
    uint8_t *pkt_data;
} raw_pkt_t;


#define RFC5444_MAX_PKT_SIZE 1500
#define RFC5444_MAX_MSG_NUM     3 // max num of msg in one packet
#define RFC5444_ADDR_LEN        6 // we only consider mac address

// hard code some fields since we do need them and will not parse them.
#define PKT_VERSION             0
#define PKT_FLAGS               0x0 // no pkt_seq_num, no pkt_tlv
#define MSG_FLAGS_HELLO         15   // indicating that the message header contains originator address, hop limit, hop count, and message sequence number fields.
#define MSG_FLAGS_TC            15   
#define MSG_ADDR_LEN            (RFC5444_ADDR_LEN - 1)

typedef enum {
    MSG_TYPE_HELLO = 1,
    MSG_TYPE_TC,
} msg_type_t;

typedef struct msg_header_t {
    uint8_t msg_type;   // an 8-bit unsigned integer field, specifying the type of the message.
    uint8_t msg_flags;  // a 4-bit field, specifying the interpretation of the remainder of the Message Header
    uint8_t msg_addr_len; // a 4-bit unsigned integer field, encoding the length of all addresses included in this message
                          //  the length of an address in octets - 1, very strange ...
    uint16_t msg_size;    // a 16-bit unsigned integer field, specifying the number of octets that make up the <message> excluding the header.
    uint8_t msg_orig_addr[RFC5444_ADDR_LEN]; // msg originator address
    uint8_t msg_hop_limit;
    uint8_t msg_hop_count;
    uint32_t msg_seq_num; // is a 16-bit unsigned integer field that can contain a sequence number, 
                          // generated by the message’s originator MANET router.

} msg_header_t;

// Summarize all kinds of tlv here.
typedef enum tlv_type_t {
    VALIDITY_TIME,
    INTERVAL_TIME,
    CONT_SEQ_NUM,
    MPR_WILLING,
    LINK_STATUS, // TODO: combine multiple statistic
    LINK_METRIC,
} tlv_type_t;

// TODO
enum addr_flag {
    NORMAL_ADDR,
};

typedef struct tlv_t {
    uint8_t tlv_type;
    uint8_t tlv_value_len;
    uint8_t tlv_value[0]; // not a pointer finally. just free this tlv!
} __attribute__((packed)) tlv_t;

typedef struct tlv_block_t
{
    uint8_t tlv_block_type;
    uint8_t tlv_ptr_len; // length of the tlv_ptr_list
    uint16_t tlv_block_size; // size of rest of the block.
    tlv_t*   tlv_ptr_list[0]; // Note: this is a list of pointer.
} __attribute__((packed)) tlv_block_t;

typedef struct addr_block_t
{
    uint16_t addr_num;
    uint8_t addr_list[0];       // Notice: each addr has RFC5444_ADDR_LEN bytes.
} __attribute__((packed)) addr_block_t;


// msg must be created by alloc.
typedef struct hello_msg_t {
    msg_header_t header;
    // this struct will only hold pointers to blocks
    tlv_block_t* msg_tlv_block_ptr;
    addr_block_t* addr_block_ptr;
    tlv_block_t* addr_tlv_block_ptr;
} hello_msg_t;

typedef struct tc_msg_t {
    msg_header_t header;
    tlv_block_t* msg_tlv_block_ptr;
    addr_block_t* addr_block_ptr;
    tlv_block_t* addr_tlv_block_ptr;  
} tc_msg_t;

// union of all kinds of msg ptr
// typedef union rfc5444_msg_t {
//     hello_msg_t* hello_msg_ptr;
//     tc_msg_t* tc_msg_ptr;
// } rfc5444_msg_ptr;

#define RFC5444_PKT_HEADER_LEN 4
typedef struct rfc5444_pkt_t {
    // packet header
    // just use a byte to hold 4-bit field to keep things simple ...
    uint8_t version;    // 4-bit unsigned integer field, version 0
    uint8_t pkt_flags;  // 4-bit field, bit 0 (pkt has seq_num), bit 1 (pkt has tlv).
    uint16_t pkt_len;   // length of the whole packet, not in rfc5444, added for convenience.
    // rfc5444_msg_ptr msg_list[RFC5444_MAX_MSG_NUM]; // list of ptr to the msg
    hello_msg_t* hello_msg_ptr;
    tc_msg_t* tc_msg_ptr;
} rfc5444_pkt_t;


/* exported functions */
uint8_t cal_tlv_len(tlv_type_t);
uint16_t get_tlv_block_len (tlv_block_t* tlv_block);
uint16_t get_addr_block_len (addr_block_t* addr_block_ptr);
void free_rfc5444_pkt(rfc5444_pkt_t);
rfc5444_pkt_t parse_raw_packet (raw_pkt_t raw_packet);
raw_pkt_t gen_raw_packet (rfc5444_pkt_t rfc5444_pkt);

#endif