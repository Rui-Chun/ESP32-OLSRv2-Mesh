/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef ESPNOW_OLSR_H
#define ESPNOW_OLSR_H

#include "esp_now.h"
#include "libs/rfc5444.h"
#include "libs/info_base.h"

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */
#if CONFIG_ESPNOW_WIFI_MODE_STATION
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_AP
#endif

#define ESPNOW_QUEUE_SIZE           16

#define ESPNOW_MAX_DATA_LEN        (250)
#define ESPNOW_MAX_PAYLOAD_LEN     (ESPNOW_MAX_DATA_LEN - sizeof(espnow_olsr_frame_t)) // the length of payload part in one ESPNOW frame.
#define ESPNOW_MAX_PKT_LEN         (ESPNOW_MAX_PAYLOAD_LEN * 16) // max supported len of a packet.

// the period of timer (ms)
#define xTIMER_PERIOD               (1000 / portTICK_PERIOD_MS) // 1000 ms

typedef enum {
    ESPNOW_OLSR_SEND_CB,
    ESPNOW_OLSR_RECV_CB,
    ESPNOW_OLSR_SEND_TO,   // to send a packet
    ESPNOW_OLSR_NO_OP,     // to indicate no op is needed.
    ESPNOW_OLSR_UNDEFINE,
} espnow_olsr_event_id_t;

typedef struct {
    uint8_t mac_addr[RFC5444_ADDR_LEN];
    esp_now_send_status_t status;
} espnow_olsr_event_send_cb_t;

typedef struct {
    uint8_t mac_addr[RFC5444_ADDR_LEN];
    uint8_t *data;
    int data_len;
} espnow_olsr_event_recv_cb_t;

typedef struct {
    // uint8_t dest_addr[RFC5444_ADDR_LEN]; // we always broadcast
    raw_pkt_t pkt;
} espnow_olsr_event_send_to_t;

typedef union {
    espnow_olsr_event_send_cb_t send_cb;
    espnow_olsr_event_recv_cb_t recv_cb;
    espnow_olsr_event_send_to_t send_to;
} espnow_olsr_event_info_t;

/* When ESPNOW sending or receiving callback function is called, post event to ESPNOW task. */
typedef struct {
    espnow_olsr_event_id_t id;
    espnow_olsr_event_info_t info;
} espnow_olsr_event_t;

typedef enum {
    ESPNOW_OLSR_DATA_START = 0, // start of a packet consisting of multiple frames.
    ESPNOW_OLSR_DATA_MORE, // more frames will follow to form a full packet.
    ESPNOW_OLSR_DATA_END,
    ESPNOW_OLSR_DATA_S_END, // only one frame.
} espnow_seg_state_t; // packet segmentation state

/* User defined field of ESPNOW data in this example. */
typedef struct {
    uint16_t seq_num;                     //Sequence number of ESPNOW data from a given device
    uint8_t seg_state;                    //Indicate packet segmentation state
    uint16_t crc;                         //CRC16 value of ESPNOW data.
    uint8_t len;                          //total len of the frame including header. this must <= 250 (espnow limit)
    uint8_t payload[0];                   //Payload of ESPNOW data. zero size array so payload will point to successive bytes.
} __attribute__((packed)) espnow_olsr_frame_t;


#endif
