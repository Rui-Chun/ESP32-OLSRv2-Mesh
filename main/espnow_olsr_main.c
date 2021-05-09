/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
   This example shows how to use ESPNOW.
   Prepare two device, one for sending ESPNOW data and another for receiving
   ESPNOW data.
*/
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "esp_private/wifi.h"

#include "espnow_olsr.h"
#include "libs/olsr_handlers.h"

static const char *TAG = "espnow_event_loop";

static xQueueHandle s_espnow_olsr_queue;

static uint8_t espnow_broadcast_mac[RFC5444_ADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint16_t s_espnow_olsr_seq = 0;

static void espnow_olsr_deinit();

/* WiFi should start before using ESPNOW */
static void example_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());
    // ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11N) );

    // get and set the txpower as the lowest value
    int8_t power = 0;
    ESP_ERROR_CHECK( esp_wifi_get_max_tx_power(&power) );
    ESP_LOGI(TAG, "Default max power =%d * 0.25dbm", power);
    ESP_ERROR_CHECK( esp_wifi_set_max_tx_power(8) );
    ESP_ERROR_CHECK( esp_wifi_get_max_tx_power(&power) );
    ESP_LOGI(TAG, "New max tx power =%d * 0.25dbm", power);

    /*set the rate*/
    ESP_ERROR_CHECK( esp_wifi_internal_set_fix_rate(ESPNOW_WIFI_IF, true, WIFI_PHY_RATE_MCS7_SGI) );
}

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void espnow_olsr_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    espnow_olsr_event_t evt;
    espnow_olsr_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }
    if (status == ESP_NOW_SEND_FAIL) {
        ESP_LOGE(TAG, "Send espnow frame failed!");
    }

    evt.id = ESPNOW_OLSR_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;

    // stop send this event.
    // if (xQueueSend(s_espnow_olsr_queue, &evt, portMAX_DELAY) != pdTRUE) {
    //     ESP_LOGW(TAG, "Send send queue fail");
    // }
}

static void espnow_olsr_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    espnow_olsr_event_t evt;
    espnow_olsr_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    evt.id = ESPNOW_OLSR_RECV_CB;
    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(s_espnow_olsr_queue, &evt, portMAX_DELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }
}

/* Check received ESPNOW data crc and len. */
int espnow_olsr_data_check(uint8_t *data, uint16_t data_len)
{
    espnow_olsr_frame_t *frame = (espnow_olsr_frame_t *)data;
    assert(frame != NULL);
    uint16_t crc, crc_cal = 0;

    if (data_len < sizeof(espnow_olsr_frame_t)) {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
        return -1;
    }

    crc = frame->crc;
    frame->crc = 0;
    crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)frame, data_len);

    if (crc_cal == crc && frame->len == data_len) {
        return 1; // check done
    }

    // crc check fail
    return -1;
}

/* Prepare ESPNOW data to be sent. */
void espnow_olsr_frame_prepare(espnow_olsr_frame_t *espnow_frame, espnow_seg_state_t state, uint8_t* payload, uint8_t payload_len)
{
    assert(espnow_frame != NULL); // please also make sure it has enough space.
    assert(payload_len <= ESPNOW_MAX_PAYLOAD_LEN);

    espnow_frame->seq_num = s_espnow_olsr_seq++;
    espnow_frame->seg_state = state;
    espnow_frame->crc = 0;
    espnow_frame->len = payload_len + sizeof(espnow_olsr_frame_t);
    memcpy(espnow_frame->payload, payload, payload_len); 
    espnow_frame->crc = esp_crc16_le(UINT16_MAX, (uint8_t const *)espnow_frame, espnow_frame->len);
}

static void espnow_olsr_task(void *pvParameter)
{
    espnow_olsr_event_t evt;
    espnow_olsr_frame_t *local_frame = NULL; 
    uint8_t pkt_seg_num = 0; // how many seg in a packet.

    // for recv packet
    uint8_t recv_mac_addr[ESP_NOW_ETH_ALEN];
    memcpy(recv_mac_addr, espnow_broadcast_mac, ESP_NOW_ETH_ALEN);
    uint16_t recv_seq_num = 0;
    uint8_t *recv_pkt_buf = NULL;
    uint16_t recv_pkt_offset = 0;

    // for handler return event
    espnow_olsr_event_t ret_evt;
    
    // why wait? this is from the example code.
    vTaskDelay(100 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "ESPNOW event loop starts");

    /* Initialize an empty frame to hold data for local use  */
    local_frame = malloc(ESPNOW_MAX_DATA_LEN);
    if (local_frame == NULL) {
        ESP_LOGE(TAG, "frame alloc failed");
        espnow_olsr_deinit();
        vTaskDelete(NULL);
    }
    memset(local_frame, 0, ESPNOW_MAX_DATA_LEN);
    /* Initialize an empty packet to hold data for recv buf  */
    recv_pkt_buf = malloc(ESPNOW_MAX_PKT_LEN);
    if (recv_pkt_buf == NULL) {
        ESP_LOGE(TAG, "packet buf alloc failed");
        free(local_frame);
        espnow_olsr_deinit();
        vTaskDelete(NULL);
    }
    memset(recv_pkt_buf, 0, ESPNOW_MAX_PKT_LEN);


    // espnow event loop, should loop forever.
    while (xQueueReceive(s_espnow_olsr_queue, &evt, portMAX_DELAY) == pdTRUE) {
        // ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());
        // ESP_LOGI(TAG, "task stack water mark : %d", uxTaskGetStackHighWaterMark(NULL));
        switch (evt.id) {
            // a packet need to be sent, most likely we need send multiple frames
            case ESPNOW_OLSR_SEND_TO:
            {
                espnow_olsr_event_send_to_t *send_to_info = &evt.info.send_to;

                ESP_LOGI(TAG, "Handling SEND_TO event");
                if(send_to_info->pkt.pkt_len == 0) break;
                // calculate number of frames/segments needed for this packet.
                pkt_seg_num = (send_to_info->pkt.pkt_len + ESPNOW_MAX_PAYLOAD_LEN -1 )/ ESPNOW_MAX_PAYLOAD_LEN; 
                assert(pkt_seg_num >= 1 && pkt_seg_num < 16); // it should not be very large.

                /* prepare data and send out. */
                // loop over segments/frames
                for (int p = 0; p < pkt_seg_num; p++) {
                    // Is this the last segment/frame?
                    if (p == pkt_seg_num - 1) {
                        // first and also the last
                        if (p == 0) {
                            espnow_olsr_frame_prepare(local_frame, ESPNOW_OLSR_DATA_S_END,\
                                                  send_to_info->pkt.pkt_data, send_to_info->pkt.pkt_len);
                        } else {
                            espnow_olsr_frame_prepare(local_frame, ESPNOW_OLSR_DATA_END,\
                                                  send_to_info->pkt.pkt_data + p * ESPNOW_MAX_PAYLOAD_LEN, send_to_info->pkt.pkt_len - p*ESPNOW_MAX_PAYLOAD_LEN);
                        }
                    } else {
                        // if first frame of multiple ones
                        if (p == 0) {
                            espnow_olsr_frame_prepare(local_frame, ESPNOW_OLSR_DATA_START,\
                                                  send_to_info->pkt.pkt_data, ESPNOW_MAX_PAYLOAD_LEN);
                        } else {
                            espnow_olsr_frame_prepare(local_frame, ESPNOW_OLSR_DATA_MORE,\
                                                  send_to_info->pkt.pkt_data + p * ESPNOW_MAX_PAYLOAD_LEN, ESPNOW_MAX_PAYLOAD_LEN);
                        }
                    }
                    // send the frame to broadcast address now 
                    if (esp_now_send(espnow_broadcast_mac, (const uint8_t *)local_frame, local_frame->len) != ESP_OK) {
                        ESP_LOGE(TAG, "ESPNOW Send error!");
                        espnow_olsr_deinit();
                        vTaskDelete(NULL);
                    }
                    // clear up
                    memset(local_frame, 0, ESPNOW_MAX_DATA_LEN);
                }
                // MUST free the data
                free(send_to_info->pkt.pkt_data);
                break;
            }
            case ESPNOW_OLSR_SEND_CB:
            {
                // do nothing, this should not be called.
                ESP_LOGI(TAG, "Handling SEND_CB event");
                break;
            }
            case ESPNOW_OLSR_RECV_CB:
            {
                espnow_olsr_event_recv_cb_t *recv_cb_info = &evt.info.recv_cb;
                assert(recv_cb_info != NULL);
                ESP_LOGI(TAG, "Handling RECV_CB event");

                if (espnow_olsr_data_check(recv_cb_info->data, recv_cb_info->data_len) < 0 ) {
                    ESP_LOGE(TAG, "Recv data check failed. len = %d", recv_cb_info->data_len);
                    break;
                }
                // check done, get frame now
                espnow_olsr_frame_t *recv_frame = (espnow_olsr_frame_t *)recv_cb_info->data;
                // print recv info
                ESP_LOGI(TAG, "Receive %dth broadcast data from: "MACSTR", len: %d",\
                            recv_frame->seq_num, MAC2STR(recv_cb_info->mac_addr), recv_cb_info->data_len);

                // handle segments
                switch (recv_frame->seg_state)
                {
                // handle segmets, what if we lose some segments?
                case ESPNOW_OLSR_DATA_S_END: {
                    // this case does not involve pkt buf
                    // call olsr recv packet handler
                    raw_pkt_t recv_pkt;
                    memcpy(recv_pkt.mac_addr, recv_cb_info->mac_addr, RFC5444_ADDR_LEN);
                    recv_pkt.pkt_len = recv_frame->len - sizeof(espnow_olsr_frame_t);
                    recv_pkt.pkt_data = recv_frame->payload;
                    // TODO: this is jsut a fake return value
                    ret_evt = olsr_recv_pkt_handler(recv_pkt);
                    // push to queue
                    if (xQueueSend(s_espnow_olsr_queue, &ret_evt, portMAX_DELAY) != pdTRUE) {
                        ESP_LOGW(TAG, "Send receive queue fail");
                    }
                    break;
                }
                case ESPNOW_OLSR_DATA_START: {
                    if(recv_pkt_offset != 0) {
                        ESP_LOGW(TAG, "A newpacket starts. Old packet is dropped!");
                        // clean up the buf
                        memset(recv_pkt_buf, 0, ESPNOW_MAX_PKT_LEN);
                        recv_pkt_offset = 0;
                    }
                    // update all recv states
                    uint8_t tmp_len = recv_frame->len - sizeof(espnow_olsr_frame_t);
                    memcpy(recv_pkt_buf, recv_frame->payload, tmp_len);
                    recv_pkt_offset += tmp_len;
                    memcpy(recv_mac_addr, recv_cb_info->mac_addr, ESP_NOW_ETH_ALEN);
                    recv_seq_num = recv_frame->seq_num;
                    break;
                }
                case ESPNOW_OLSR_DATA_MORE: {
                    int tmp_ret = memcmp(recv_mac_addr, recv_cb_info->mac_addr, ESP_NOW_ETH_ALEN);
                    // the frame must be from the same mac, right seq_num and with right offset value
                    if (tmp_ret != 0 || recv_seq_num != recv_frame->seq_num - 1 || recv_pkt_offset == 0) {
                        ESP_LOGW(TAG, "A wrong frame with DATA_MORE flag!");
                        break;
                    }
                    // update all recv states
                    uint8_t tmp_len = recv_frame->len - sizeof(espnow_olsr_frame_t);
                    memcpy(recv_pkt_buf + recv_pkt_offset, recv_frame->payload, tmp_len);
                    recv_pkt_offset += tmp_len;
                    recv_seq_num ++;
                    break;
                }
                case ESPNOW_OLSR_DATA_END: {
                    int tmp_ret = memcmp(recv_mac_addr, recv_cb_info->mac_addr, ESP_NOW_ETH_ALEN);
                    // the frame must be from the same mac, right seq_num and with right offset value
                    if (tmp_ret != 0 || recv_seq_num != recv_frame->seq_num - 1 || recv_pkt_offset == 0) {
                        ESP_LOGW(TAG, "A wrong frame with DATA_END flag!");
                        break;
                    }
                    // update all recv states
                    uint8_t tmp_len = recv_frame->len - sizeof(espnow_olsr_frame_t);
                    memcpy(recv_pkt_buf + recv_pkt_offset, recv_frame->payload, tmp_len);
                    recv_pkt_offset += tmp_len;
                    ESP_LOGI(TAG, "A new packet received, len = %d", recv_pkt_offset);
                    // call recv pkt handler
                    raw_pkt_t recv_pkt;
                    memcpy(recv_pkt.mac_addr, recv_cb_info->mac_addr, RFC5444_ADDR_LEN);
                    recv_pkt.pkt_len = recv_pkt_offset;
                    recv_pkt.pkt_data = recv_pkt_buf;
                    // TODO: this is jsut a fake return value
                    ret_evt = olsr_recv_pkt_handler(recv_pkt);
                    // push to queue
                    if (xQueueSend(s_espnow_olsr_queue, &ret_evt, portMAX_DELAY) != pdTRUE) {
                        ESP_LOGW(TAG, "Send receive queue fail");
                    }
                    // clean up the buf
                    memset(recv_pkt_buf, 0, ESPNOW_MAX_PKT_LEN);
                    recv_pkt_offset = 0;
                    recv_seq_num = 0;
                    break;
                }
                default:
                    ESP_LOGE(TAG, "Recv frame seg_state unknown");
                    break;
                }

                free(recv_frame); // MUST free data! this is allocated in recv_cb
                break;
            }
            case ESPNOW_OLSR_TIMER_CB:
            {
                ESP_LOGI(TAG, "Handling TIMER CB event.");
                // call olsr handler
                ret_evt = olsr_timer_handler(evt.info.timer_cb.timer_tick);
                // push the return event to queue
                if (xQueueSend(s_espnow_olsr_queue, &ret_evt, portMAX_DELAY) != pdTRUE) {
                    ESP_LOGE(TAG, "Timer send evt to queue fail!");
                    return;
                }
                break;
            }
            case ESPNOW_OLSR_NO_OP: {
                ESP_LOGI(TAG, "NO OP event called.");
                break;
            }
            default:
                ESP_LOGE(TAG, "Callback type error: %d", evt.id);
                break;
        }
    }

    // free local buf now
    free(local_frame);
    free(recv_pkt_buf);
}


static void espnow_timer_cb( TimerHandle_t xExpiredTimer )
{
    static uint32_t timer_tick_count = 0;
    // Increment the variable to show the timer callback has executed.
    timer_tick_count ++;
    ESP_LOGI(TAG, "timer tick = %d", timer_tick_count);

    // send TIMER_CB event, let the event loop do the heavy work.
    espnow_olsr_event_t evt;
    evt.id = ESPNOW_OLSR_TIMER_CB;
    evt.info.timer_cb.timer_tick = timer_tick_count;
    // push to queue
    if (xQueueSend(s_espnow_olsr_queue, &evt, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Timer send evt to queue fail!");
        return;
    }

    /* legacy test code */
    // If this callback has executed the required number of times, stop the
    // timer.
    // if( timer_tick_count == 100 )
    // {
    //     // This is called from a timer callback so must not block.
    //     xTimerStop( xExpiredTimer, 0 );
    // }

    // // push a fake packet
    // raw_pkt_t recv_pkt;
    // recv_pkt.pkt_len = 555;
    // // this will be freeed by event loop.
    // recv_pkt.pkt_data = malloc(recv_pkt.pkt_len);
    // if (recv_pkt.pkt_data == NULL) {
    //     ESP_LOGE(TAG, "packet alloc error!");
    //     return;
    // }
    // espnow_olsr_event_t evt;
    // evt.id = ESPNOW_OLSR_SEND_TO;
    // evt.info.send_to.pkt = recv_pkt;
    // // push to queue
    // if (xQueueSend(s_espnow_olsr_queue, &evt, portMAX_DELAY) != pdTRUE) {
    //     ESP_LOGW(TAG, "Send receive queue fail");
    // }
}

static esp_err_t espnow_olsr_init(void)
{

    s_espnow_olsr_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(espnow_olsr_event_t));
    if (s_espnow_olsr_queue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(espnow_olsr_send_cb) ); // Do we need this?
    ESP_ERROR_CHECK( esp_now_register_recv_cb(espnow_olsr_recv_cb) );

    /* Set primary master key. */
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        vSemaphoreDelete(s_espnow_olsr_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, espnow_broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    // ==== init info base ====
    uint8_t my_mac[RFC5444_ADDR_LEN];
    ESP_ERROR_CHECK( esp_wifi_get_mac(ESPNOW_WIFI_IF, my_mac) );
    info_base_init(my_mac); // pass local mac addr

    // ==== start a task for OLSR event loop ====
    xTaskCreate(espnow_olsr_task, "espnow_olsr_task", 4096, NULL, 4, NULL);
    // ==== set up a freeRTOS timer to send out packets. ====
    TimerHandle_t xTimer_h = xTimerCreate( "T1",             // Text name for the task.  Helps debugging only.  Not used by FreeRTOS.
                                 xTIMER_PERIOD,     // The period of the timer in ticks.
                                 pdTRUE,           // This is an auto-reload timer.
                                 NULL,    // An identifier that is assigned to the timer being created.
                                 espnow_timer_cb // The function to execute when the timer expires.
                                 );

    // The scheduler has not started yet so a block time is not used.
    if( xTimerStart( xTimer_h, 0 ) != pdPASS )
    {
        // The timer could not be set into the Active state.
        ESP_LOGE(TAG, "Timer start error!");
    }

    // This FreeRTOS call is not required as the scheduler is already started before app_main() call
    // vTaskStartScheduler();

    return ESP_OK;
}

static void espnow_olsr_deinit()
{
    vSemaphoreDelete(s_espnow_olsr_queue);
    esp_now_deinit();
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    example_wifi_init();
    espnow_olsr_init();
}
