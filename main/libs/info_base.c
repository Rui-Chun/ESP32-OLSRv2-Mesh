#include "info_base.h"

static const char *TAG = "espnow_info_base";

// static variables should be init as zeros by the compiler.

// message seq num, to indicate a new msg
static uint32_t global_msg_seq_num = 0;

static uint32_t global_tick_num = 0; // time counter based on tick

// do not use #0, use [1, peer_num]
// for example, 'for(int n=1; n <= peer_num; n++)'
static uint8_t peer_num = 0; // 255 should be enough.

// a static list for all peer nodes' addresses.
// Note: We use peer_id #0 to mark empty or originator(self)!
static uint8_t peer_addr_list[MAX_PEER_NUM][RFC5444_ADDR_LEN]; // use peer_id to get mac address.
static void* entry_ptr_list[MAX_PEER_NUM];

// Neighbor Information Base
// info is stored in entries, get it from entry ptr list.
static uint8_t neighbor_id_num = 0;
static uint8_t neighbor_id_list[MAX_NEIGHBOUR_NUM];
static uint8_t two_hop_id_num = 0;
static uint8_t two_hop_id_list[MAX_PEER_NUM];

// Topology Information Base
// get remote_node_entry for the info !
// 1. A Routable Address Topology Set
// 2. A Router Topology Set, recording links between routers in the MANET
// 3. A Routing Set, recording routes from this router to all available destinations.
static uint8_t remote_id_num = 0;
static uint8_t remote_id_list[MAX_PEER_NUM];
// TODO: 
// 3. An Attached Network Set, recording a gateway

// received message info base, to prevent msg processed/forwarded twice.
// this has been moved to node entry, msg_seq_num field.

// Local Information Base: Originator address / my own address
static uint8_t originator_addr[RFC5444_ADDR_LEN];

/* Helper functions */

// search for the addr in the peer list, (if not existing, append one) and assign the peer_id.
// return 1 if already in list, else 0.
uint8_t get_or_create_id (uint8_t mac_addr[RFC5444_ADDR_LEN], uint8_t* peer_id) {
    for(int p = 1; p <= peer_num; p++) { // do not use #0, use [1, peer_num]
        if (memcmp(peer_addr_list[p], mac_addr, RFC5444_ADDR_LEN) == 0) {
            // a match in the list.
            *peer_id = p;
            if (entry_ptr_list[p] == NULL) {
                // if this node was deleted before.
                return 0; // register it agagin.
            }
            // do not need alloc new entry
            return 1;
        }
    }
    // if no match, append the list
    memcpy(peer_addr_list[++peer_num], mac_addr, RFC5444_ADDR_LEN);
    *peer_id = peer_num;
    return 0;

}

// register a new neighbor struct into the entry_ptr_list, id_list needs to be updated later.
neighbor_entry_t* register_new_neighbor(uint8_t new_neighbor_id) {
    if (new_neighbor_id == 0 ) {
        ESP_LOGW(TAG, "Do not register peer #0!");
        return NULL;
    }
    // must be unregistered
    assert(entry_ptr_list[new_neighbor_id] == NULL);
    neighbor_entry_t* ret_entry = calloc(1, sizeof(neighbor_entry_t)); // set to zeros
    if(ret_entry == NULL) {
        ESP_LOGE(TAG, "No mem for a new neighbor entry.");
        return NULL;
    }
    // init neighbor entry
    ret_entry->entry_type = NEIGHBOR_ENTRY;
    ret_entry->peer_id = new_neighbor_id;
    ret_entry->link_status = LINK_HEARD;
    // MUST set link metric as INF at init stage
    ret_entry->link_metric = 255;
    ret_entry->in_link_metric = 255;
    // register the entry to the entry list
    entry_ptr_list[new_neighbor_id] = ret_entry;

    ESP_LOGI(TAG, "A new neighbor node entry registered.");
    return ret_entry;
}

// register a new two-hop struct into the entry_ptr_list, id_list needs to be updated later,
two_hop_entry_t* register_new_two_hop(uint8_t new_two_hop_id) {
    if (new_two_hop_id == 0 ) {
        ESP_LOGW(TAG, "Do not register peer #0!");
        return NULL;
    }
    // must be unregistered
    assert(entry_ptr_list[new_two_hop_id] == NULL);
    two_hop_entry_t* ret_entry = calloc(1, sizeof(two_hop_entry_t)); // set to zeros
    if(ret_entry == NULL) {
        ESP_LOGE(TAG, "No mem for a new two-hop entry.");
        return NULL;
    }
    // init two-hop entry
    ret_entry->entry_type = TWO_HOP_ENTRY;
    ret_entry->peer_id = new_two_hop_id;
    // TODO: do we need to assign link status?
    // register the entry to the entry list
    entry_ptr_list[new_two_hop_id] = ret_entry;

    ESP_LOGI(TAG, "A new two-hop node entry registered.");
    return ret_entry;
}

// delete a entry and free the mem. 
// msut call update_id_lists() after calling this function.
void delete_entry_by_id (uint8_t node_id) {
    uint8_t* tmp_entry_ptr = entry_ptr_list[node_id];
    if (tmp_entry_ptr == NULL ) return;

    switch (tmp_entry_ptr[0]) {
        case NEIGHBOR_ENTRY: {
            neighbor_entry_t* neighbor_entry_ptr = (neighbor_entry_t*) tmp_entry_ptr;
            // it should be fine to free NULL
            free(neighbor_entry_ptr->link_info.id_list_ptr);
            free(neighbor_entry_ptr->link_info.metric_list_ptr);
            free(neighbor_entry_ptr->link_info.in_metric_list_ptr);
            free(neighbor_entry_ptr);
            entry_ptr_list[node_id] = NULL;
            break;
        }
        case TWO_HOP_ENTRY: {
            // leak through to next case
        }
        case REMOTE_NODE_ENTRY: {
            remote_node_entry_t* remote_entry_ptr = (remote_node_entry_t*) tmp_entry_ptr;
            // it should be fine to free NULL
            free(remote_entry_ptr->link_info.id_list_ptr);
            free(remote_entry_ptr->link_info.metric_list_ptr);
            free(remote_entry_ptr->link_info.in_metric_list_ptr);
            free(remote_entry_ptr);
            entry_ptr_list[node_id] = NULL;
            break;
        }
        default: {
            ESP_LOGW(TAG, "Wrong entry type!");
        }
    }
}

// loop over the entry list to count the number of neighbor entries.
void update_id_lists() {
    ESP_LOGI(TAG, "Updating id lists ...");
    neighbor_id_num = 0;
    two_hop_id_num = 0;
    remote_id_num = 0;

    for(int p=1; p <= peer_num; p++) { // do not use #0
        if (entry_ptr_list[p] == NULL) {
            // the entry has been deleted later.
            continue;
        }
        switch ( ((uint8_t*)entry_ptr_list[p])[0] ) {
            case NEIGHBOR_ENTRY: {
                neighbor_id_list[neighbor_id_num++] = p;
                break;
            }
            case TWO_HOP_ENTRY: {
                two_hop_id_list[two_hop_id_num++] = p;
                break;
            }
            case REMOTE_NODE_ENTRY: {
                remote_id_list[remote_id_num++] = p;
                break;
            }
            default: {
                ESP_LOGE(TAG, "Unknown entry type!");
            }
        }
    }
    ESP_LOGI(TAG, "neighbor num = %d, two-hop num = %d, remote num = %d", neighbor_id_num, two_hop_id_num, remote_id_num);
    return;
}

// loop over all entries and delete invalid entries
// by comparing global_tick_num and entry->valid_until. 
// valid_until field should be at the same location for all entries.
void check_entry_validity() {
    uint8_t* tmp_entry_ptr = NULL;
    uint8_t delete_flag = 0;
    for(int n=1; n <= peer_num; n++) { // do not use #0
        if(entry_ptr_list[n] == NULL) continue;
        tmp_entry_ptr = (uint8_t*)entry_ptr_list[n];
        // check entry type
        if (tmp_entry_ptr[0] == NEIGHBOR_ENTRY) {
            neighbor_entry_t* neighbor_entry_ptr = (neighbor_entry_t*)tmp_entry_ptr;
            // check if valid
            if(neighbor_entry_ptr->valid_until < global_tick_num) {
                //delete that entry, also need to free link info.
                delete_flag = 1;
                delete_entry_by_id(n);
                ESP_LOGW(TAG, "A neighbor node entry is deleted due to timeout!");
            }
        } 
        else {
            // two-hop and remote are inter-changeable.
            two_hop_entry_t* two_hop_entry_ptr = (two_hop_entry_t*)tmp_entry_ptr;
            // check if valid
            if (two_hop_entry_ptr->valid_until < global_tick_num) {
                delete_flag = 1;
                delete_entry_by_id(n);
                ESP_LOGW(TAG, "A two-hop/remote node entry is deleted due to timeout!");
            }
        }
    }
    // if delete some entry, update the id_lists.
    if(delete_flag) {
        update_id_lists();
    }
}


/* Worker functions */

void set_info_base_time (uint32_t tick) {
    global_tick_num = tick;
}

void info_base_init (uint8_t mac[RFC5444_ADDR_LEN]) {
    memcpy(originator_addr, mac, RFC5444_ADDR_LEN);
    ESP_LOGI(TAG, "init done, mac addr =  "MACSTR".", MAC2STR(originator_addr));
}

// print info baesd on id_lists and entry_ptr_list.
void print_topology_set () {
    ESP_LOGI(TAG, "");
    printf("Start printing topology info.\n");
    uint8_t node_id = 0; // peer equals with node.
    neighbor_entry_t* neighbor_ptr = NULL;
    two_hop_entry_t* two_hop_ptr = NULL;
    for(int n=0; n < neighbor_id_num; n++) {
        node_id = neighbor_id_list[n];
        assert( ((uint8_t*)entry_ptr_list[node_id])[0] == NEIGHBOR_ENTRY);
        neighbor_ptr = entry_ptr_list[node_id];
        printf("Neighbor:\tnode id = #%d: "MACSTR" \t link_status = %d, routing next_hop = #%d\n", node_id, MAC2STR(peer_addr_list[node_id]), neighbor_ptr->link_status, neighbor_ptr->routing_info.next_hop);
        printf("\tMPR status = (%d, %d), \tout_metric = %d, in_metric = %d \n", neighbor_ptr->flooding_status, neighbor_ptr->routing_status, neighbor_ptr->link_metric, neighbor_ptr->in_link_metric);
    }
    for(int n=0; n < two_hop_id_num; n++) {
        node_id = two_hop_id_list[n];
        assert( ((uint8_t*)entry_ptr_list[node_id])[0] == TWO_HOP_ENTRY);
        printf("TWO_HOP: \tnode id = #%d: "MACSTR" \n", node_id, MAC2STR(peer_addr_list[node_id]));
        two_hop_ptr = entry_ptr_list[node_id];
        printf(" \trouting next hop = #%d \n", two_hop_ptr->routing_info.next_hop);
    }
    for(int n=0; n < remote_id_num; n++) {
        node_id = remote_id_list[n];
        assert( ((uint8_t*)entry_ptr_list[node_id])[0] == REMOTE_NODE_ENTRY);
        printf("REMOTE: \tnode id = #%d: "MACSTR" \n", node_id, MAC2STR(peer_addr_list[node_id]));
    }
    printf("Done printing topology info.\n");
    ESP_LOGI(TAG, "");
}

// parse the link info given a HELLO msg
void parse_hello_addr_block(neighbor_entry_t* neighbor_entry_ptr, hello_msg_t* hello_msg_ptr, uint32_t hello_valid_until) {
    // 1. get addr tlv pointers.
    uint8_t link_num = hello_msg_ptr->addr_block_ptr->addr_num;
    assert( hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_len == HELLO_ADDR_TLV_NUM );
    tlv_t* link_status_tlv_ptr = hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[0];
    assert( link_status_tlv_ptr->tlv_value_len == link_num);
    tlv_t* link_metric_tlv_ptr = hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[1];
    assert( link_metric_tlv_ptr->tlv_value_len == link_num * 2); // out metric list + in metric list !
    tlv_t* mpr_status_tlv_ptr = hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[2];
    assert( mpr_status_tlv_ptr->tlv_value_len == link_num * 2); // 2 bytes each value, for flooding and routing

    // 2. delete and alloc link info struct
    if (neighbor_entry_ptr->link_info.link_num != 0) {
        // delete old values
        free(neighbor_entry_ptr->link_info.id_list_ptr);
        free(neighbor_entry_ptr->link_info.metric_list_ptr);
        free(neighbor_entry_ptr->link_info.in_metric_list_ptr);
    }
    neighbor_entry_ptr->link_info.link_num = link_num;
    neighbor_entry_ptr->link_info.id_list_ptr = calloc(link_num, sizeof(uint8_t));
    if (neighbor_entry_ptr->link_info.id_list_ptr == NULL) return;
    neighbor_entry_ptr->link_info.metric_list_ptr = calloc(link_num, sizeof(uint8_t));
    if (neighbor_entry_ptr->link_info.metric_list_ptr == NULL) {
        free(neighbor_entry_ptr->link_info.id_list_ptr);
        return;
    }
    neighbor_entry_ptr->link_info.in_metric_list_ptr = calloc(link_num, sizeof(uint8_t));
    if (neighbor_entry_ptr->link_info.in_metric_list_ptr == NULL) {
        free(neighbor_entry_ptr->link_info.id_list_ptr);
        free(neighbor_entry_ptr->link_info.metric_list_ptr);
        return;
    }
    // copy in metric data, two lists
    memcpy(neighbor_entry_ptr->link_info.metric_list_ptr, link_metric_tlv_ptr->tlv_value, link_num);
    memcpy(neighbor_entry_ptr->link_info.in_metric_list_ptr, link_metric_tlv_ptr->tlv_value + link_num, link_num);


    // 3. loop over addr block values.
    uint8_t* link_addr_ptr = NULL;
    uint8_t sender_neighbor_id = 0; // here means the neighbor of the HELLO sender
    uint8_t is_link_summetric = 0;
    for(int l=0; l < link_num; l++) {
        link_addr_ptr = hello_msg_ptr->addr_block_ptr->addr_list + l * RFC5444_ADDR_LEN;
        assert(link_status_tlv_ptr->tlv_value[l] != LINK_LOST);
        // (1) if this link point to me/self_addr
        if (memcmp(link_addr_ptr, originator_addr, RFC5444_ADDR_LEN) == 0) {
            // we have a symmetric link
            neighbor_entry_ptr->link_info.id_list_ptr[l] = 0; // empty or originator.
            is_link_summetric = 1;
            neighbor_entry_ptr->link_status = LINK_SYMMETRIC;
            // update neighbor out metric using the neighbor's in metric
            neighbor_entry_ptr->link_metric = link_metric_tlv_ptr->tlv_value[link_num + l];
            // TODO: define a reasonable in link metric
            neighbor_entry_ptr->in_link_metric = 1;
            // update MPR info
            // if this node chooses me as the routing MPR.
            if (mpr_status_tlv_ptr->tlv_value[l*2] == FLOODING_TO || mpr_status_tlv_ptr->tlv_value[l*2] == FLOODING_TO_FROM) {
                // it is my MPR selector
                if (neighbor_entry_ptr->flooding_status == NOT_FLOODING) 
                    neighbor_entry_ptr->flooding_status = FLOODING_FROM;
                if (neighbor_entry_ptr->flooding_status == FLOODING_TO) 
                    neighbor_entry_ptr->flooding_status = FLOODING_TO_FROM;
            }
            // if this node chooses me as the routing MPR.
            if (mpr_status_tlv_ptr->tlv_value[l*2+1] == ROUTING_TO || mpr_status_tlv_ptr->tlv_value[l*2+1] == ROUTING_TO_FROM) {
                // it is my MPR selector
                if (neighbor_entry_ptr->routing_status == NOT_ROUTING) 
                    neighbor_entry_ptr->routing_status = ROUTING_FROM;
                if (neighbor_entry_ptr->routing_status == ROUTING_TO) 
                    neighbor_entry_ptr->routing_status = ROUTING_TO_FROM;
            }
        } 
        else if (link_status_tlv_ptr->tlv_value[l] == LINK_SYMMETRIC) { 
            // (2) if is other nodes and link is symmetric (we only add symmetric two hop)
            // if we have seen this node before.
            if( get_or_create_id(link_addr_ptr, &sender_neighbor_id) ) {
                // store this id
                neighbor_entry_ptr->link_info.id_list_ptr[l] = sender_neighbor_id;
                uint8_t* tmp_type_ptr = (uint8_t*)(entry_ptr_list[sender_neighbor_id]);
                two_hop_entry_t* tmp_two_hop_ptr = NULL;
                // if this is a remote node entry.
                if (tmp_type_ptr[0] == REMOTE_NODE_ENTRY || tmp_type_ptr[0] == TWO_HOP_ENTRY) {
                    tmp_type_ptr[0] = TWO_HOP_ENTRY; // entry must switch from remote to two-hop.
                                                // id_lists will be updated later to keep consistence.
                    // update validity
                    tmp_two_hop_ptr = (two_hop_entry_t*) tmp_type_ptr;
                    tmp_two_hop_ptr->valid_until = hello_valid_until;
                }
                // if this is a neighbor node id. do nothing.
            }
            else {
                // a new two hop entry.
                assert(sender_neighbor_id != 0);
                neighbor_entry_ptr->link_info.id_list_ptr[l] = sender_neighbor_id;
                two_hop_entry_t* ret_entry_ptr = register_new_two_hop(sender_neighbor_id);
                ret_entry_ptr->valid_until = hello_valid_until;
            }
        }
    }
    // update if not symmetric link
    if (is_link_summetric == 0) {
        neighbor_entry_ptr->link_status = LINK_HEARD;
        // out metric stays INF
        // TODO: define a reasonable in link metric
        neighbor_entry_ptr->in_link_metric = 1;
    }

}

void parse_hello_msg (hello_msg_t* hello_msg_ptr) {
    // TODO: update info bases based on HELLO
    ESP_LOGI(TAG, "Start to parse HELLO msg.");
    // get msg originator address.
    uint8_t neighbor_id = 0;
    neighbor_entry_t* hello_neighbor_entry = NULL;
    uint8_t* hello_orig_addr = hello_msg_ptr->header.msg_orig_addr;

    // 1. check and update peer_list and entry_list
    if (get_or_create_id(hello_orig_addr, &neighbor_id)) {
        // if this node has already been stored
        uint8_t* unknown_entry = entry_ptr_list[neighbor_id];
        // check the current entry type
        switch (unknown_entry[0]) {
            case NEIGHBOR_ENTRY: {
                ESP_LOGI(TAG, "Hello msg is from a familiar neighbor node!");
                hello_neighbor_entry = (neighbor_entry_t*) unknown_entry;
                break;
            }
            case TWO_HOP_ENTRY: {
                ESP_LOGI(TAG, "Hello msg is from a two-hop node?");
                // handle node type switch
                // (1) delete old entry
                delete_entry_by_id(neighbor_id);
                // (2) register new entry
                ESP_LOGI(TAG, "A new neighbor node is heard! addr = "MACSTR" .", MAC2STR(hello_orig_addr));
                hello_neighbor_entry = register_new_neighbor(neighbor_id);
                break;
            }
            case REMOTE_NODE_ENTRY: {
                ESP_LOGI(TAG, "Hello msg is from a remote node?");
                // handle node type switch
                // (1) delete old entry
                delete_entry_by_id(neighbor_id);
                // (2) register new entry
                ESP_LOGI(TAG, "A new neighbor node is heard! addr = "MACSTR" .", MAC2STR(hello_orig_addr));
                hello_neighbor_entry = register_new_neighbor(neighbor_id);
                break;
            }
            default: {
                ESP_LOGW(TAG, "An unknown entry type!");
                break;
            }
        }
    } else {
        // a new peer node.
        ESP_LOGI(TAG, "A new neighbor node is heard! addr = "MACSTR" .", MAC2STR(hello_orig_addr));
        hello_neighbor_entry = register_new_neighbor(neighbor_id);
    }
    
    // 2. update entry, neighor and two hop entries
    assert(hello_neighbor_entry->peer_id == neighbor_id);
    // if the node restarts, do not drop the packet.
    if (hello_msg_ptr->header.msg_seq_num > 0 && hello_msg_ptr->header.msg_seq_num <= hello_neighbor_entry->msg_seq_num) {
        ESP_LOGW(TAG, "Got an out-dated packet, drop it.");
        // update id_lists, to keep them correct
        update_id_lists();
        return;
    }
    // update seq_num
    hello_neighbor_entry->msg_seq_num = hello_msg_ptr->header.msg_seq_num;
    // TODO: how to assign link metric ?? This is reduntant, but not a big issue.
    hello_neighbor_entry->in_link_metric = 1; // default metric -> 1 hop cost.
    uint8_t* tmp_value_ptr = NULL;
    assert( get_tlv_value(hello_msg_ptr->msg_tlv_block_ptr, IS_MPR_WILLING, &tmp_value_ptr) == 1 );
    hello_neighbor_entry->is_mpr_willing = *tmp_value_ptr;
    assert( get_tlv_value(hello_msg_ptr->msg_tlv_block_ptr, VALIDITY_TIME, &tmp_value_ptr) == 1 );
    hello_neighbor_entry->valid_until =  global_tick_num + *tmp_value_ptr;
    
    // update mpr and link info
    parse_hello_addr_block(hello_neighbor_entry, hello_msg_ptr, hello_neighbor_entry->valid_until);

    // update id_lists, to keep them correct
    update_id_lists();
}

void gen_hello_msg_tlv (tlv_block_t* msg_tlv_block_ptr) {
    msg_tlv_block_ptr->tlv_block_type = 0;
    msg_tlv_block_ptr->tlv_ptr_len = HELLO_MSG_TLV_NUM; // three tlv entries
    msg_tlv_block_ptr->tlv_block_size = 0;

    // assign tlv entries
    // 1. VALIDITY_TIME
    msg_tlv_block_ptr->tlv_ptr_list[0] = malloc(cal_tlv_len(VALIDITY_TIME));
    if(msg_tlv_block_ptr->tlv_ptr_list[0] == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        free(msg_tlv_block_ptr);
        return;
    }
    msg_tlv_block_ptr->tlv_ptr_list[0]->tlv_type = VALIDITY_TIME;
    msg_tlv_block_ptr->tlv_ptr_list[0]->tlv_value_len = 1;
    msg_tlv_block_ptr->tlv_ptr_list[0]->tlv_value[0] = HELLO_VALIDITY_TICKS;
    msg_tlv_block_ptr->tlv_block_size += cal_tlv_len(VALIDITY_TIME);

    // 2. INTERVAL_TIME
    msg_tlv_block_ptr->tlv_ptr_list[1] = malloc(cal_tlv_len(INTERVAL_TIME));
    if(msg_tlv_block_ptr->tlv_ptr_list[1] == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        free(msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(msg_tlv_block_ptr);
        return;
    }
    msg_tlv_block_ptr->tlv_ptr_list[1]->tlv_type = INTERVAL_TIME;
    msg_tlv_block_ptr->tlv_ptr_list[1]->tlv_value_len = 1;
    msg_tlv_block_ptr->tlv_ptr_list[1]->tlv_value[0] = HELLO_INTERVAL_TICKS;
    msg_tlv_block_ptr->tlv_block_size += cal_tlv_len(INTERVAL_TIME);

    // 3. MPR_WILLING
    msg_tlv_block_ptr->tlv_ptr_list[2] = malloc(cal_tlv_len(MPR_WILLING));
    if(msg_tlv_block_ptr->tlv_ptr_list[2] == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        free(msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(msg_tlv_block_ptr);
        return;
    }
    msg_tlv_block_ptr->tlv_ptr_list[2]->tlv_type = MPR_WILLING;
    msg_tlv_block_ptr->tlv_ptr_list[2]->tlv_value_len = 1;
    msg_tlv_block_ptr->tlv_ptr_list[2]->tlv_value[0] = IS_MPR_WILLING;
    msg_tlv_block_ptr->tlv_block_size += cal_tlv_len(MPR_WILLING);

    return;
}

// this function assumes that hello msg has got mem allocated.
void gen_hello_msg (hello_msg_t* hello_msg_ptr) {
    if(hello_msg_ptr == NULL) return;

    // assign values to the header.
    msg_header_t* header_ptr = &hello_msg_ptr->header;
    header_ptr->msg_type = MSG_TYPE_HELLO;
    header_ptr->msg_flags = 0; // useless currently
    header_ptr->msg_addr_len = RFC5444_ADDR_LEN - 1; // useless since we only consider MAC addr
    header_ptr->msg_size = 0; // this needs to be calculated later.
    memcpy(header_ptr->msg_orig_addr, originator_addr, RFC5444_ADDR_LEN);
    header_ptr->msg_hop_limit = 1;
    header_ptr->msg_hop_count = 0;
    header_ptr->msg_seq_num = global_msg_seq_num++;

    // alloc and set mem for blocks
    // 1. msg tlv block, validity time and interval time.
    uint16_t tmp_len = sizeof(tlv_block_t) + HELLO_MSG_TLV_NUM * sizeof(tlv_t*); // three pointers.
    hello_msg_ptr->msg_tlv_block_ptr = malloc(tmp_len);
    if(hello_msg_ptr->msg_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for tlv block!");
        return;
    }
    gen_hello_msg_tlv(hello_msg_ptr->msg_tlv_block_ptr);
    header_ptr->msg_size += get_tlv_block_len(hello_msg_ptr->msg_tlv_block_ptr);
    ESP_LOGI(TAG, "Msg with tlv block, len = %d", header_ptr->msg_size);

    // 2. addr block, put in all neighbors.
    uint16_t neighbor_num = neighbor_id_num;
    ESP_LOGI(TAG, "neighbor_num = %d", neighbor_num);
    tmp_len = sizeof(addr_block_t) + neighbor_num * RFC5444_ADDR_LEN;
    hello_msg_ptr->addr_block_ptr = malloc(tmp_len);
    if(hello_msg_ptr->addr_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr block!");
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(hello_msg_ptr->msg_tlv_block_ptr);
        return;
    }
    hello_msg_ptr->addr_block_ptr->addr_num = neighbor_num;
    for(int n=0; n < neighbor_num; n++) {
        memcpy(hello_msg_ptr->addr_block_ptr->addr_list + n * RFC5444_ADDR_LEN,\
                peer_addr_list[neighbor_id_list[n]], RFC5444_ADDR_LEN);
    }
    header_ptr->msg_size += get_addr_block_len(hello_msg_ptr->addr_block_ptr);
    ESP_LOGI(TAG, "Msg with addr block, len = %d", header_ptr->msg_size);

    // 3. addr tlv block.
    tmp_len = sizeof(tlv_block_t) + sizeof(tlv_t*) * HELLO_ADDR_TLV_NUM ; // three tlv entry pointers!
    hello_msg_ptr->addr_tlv_block_ptr = malloc(tmp_len);
    if(hello_msg_ptr->addr_tlv_block_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv block!");
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(hello_msg_ptr->msg_tlv_block_ptr);
        free(hello_msg_ptr->addr_block_ptr);
        return;
    }
    // generate addr tlv block and all entries
    hello_msg_ptr->addr_tlv_block_ptr->tlv_block_type = 0;
    hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_len = HELLO_ADDR_TLV_NUM; // three tlv entry
    hello_msg_ptr->addr_tlv_block_ptr->tlv_block_size = 0; // to be updated.
    // (1) LINK_STATUS TLV
    tmp_len = sizeof(tlv_t) + neighbor_num;
    hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[0] = malloc(tmp_len);
    tlv_t* tmp_tlv_ptr = hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[0];
    if(tmp_tlv_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv 0 entry!");
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(hello_msg_ptr->msg_tlv_block_ptr);
        free(hello_msg_ptr->addr_block_ptr);
        free(hello_msg_ptr->addr_tlv_block_ptr);
        return;
    }
    tmp_tlv_ptr->tlv_type = LINK_STATUS;
    tmp_tlv_ptr->tlv_value_len = neighbor_num;
    for(int n=0; n < neighbor_num; n++) {
        neighbor_entry_t* neighbor_entry_ptr = entry_ptr_list[neighbor_id_list[n]];
        tmp_tlv_ptr->tlv_value[n] = neighbor_entry_ptr->link_status; // assign link status values
    }
    // udpate block size
    hello_msg_ptr->addr_tlv_block_ptr->tlv_block_size += tmp_len;

    // (2) LINK_METRIC TLV
    tmp_len = sizeof(tlv_t) + neighbor_num * 2; // out and in metric lists
    hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[1] = malloc(tmp_len);
    tmp_tlv_ptr = hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[1];
    if(tmp_tlv_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv 1 entry!");
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(hello_msg_ptr->msg_tlv_block_ptr);
        free(hello_msg_ptr->addr_block_ptr);
        free(hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->addr_tlv_block_ptr);
        return;
    }
    tmp_tlv_ptr->tlv_type = LINK_METRIC;
    tmp_tlv_ptr->tlv_value_len = neighbor_num * 2;
    for(int n=0; n < neighbor_num; n++) {
        neighbor_entry_t* neighbor_entry_ptr = entry_ptr_list[neighbor_id_list[n]];
        tmp_tlv_ptr->tlv_value[n] = neighbor_entry_ptr->link_metric; // assign out link metric value
        tmp_tlv_ptr->tlv_value[n + neighbor_num] = neighbor_entry_ptr->in_link_metric; // assign in link metric value
    }
    // udpate block size
    hello_msg_ptr->addr_tlv_block_ptr->tlv_block_size += tmp_len;

    // (3) MPR_STATUS
    tmp_len = sizeof(tlv_t) + neighbor_num * 2; // 2 bytes for each neighbor
    hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[2] = malloc(tmp_len);
    tmp_tlv_ptr = hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[2];
    if(tmp_tlv_ptr == NULL) {
        ESP_LOGE(TAG, "No mem for addr tlv 2 entry!");
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->msg_tlv_block_ptr->tlv_ptr_list[1]);
        free(hello_msg_ptr->msg_tlv_block_ptr);
        free(hello_msg_ptr->addr_block_ptr);
        free(hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[0]);
        free(hello_msg_ptr->addr_tlv_block_ptr->tlv_ptr_list[1]);
        free(hello_msg_ptr->addr_tlv_block_ptr);
        return;
    }
    tmp_tlv_ptr->tlv_type = MPR_STATUS;
    tmp_tlv_ptr->tlv_value_len = neighbor_num * 2;
    for(int n=0; n < neighbor_num; n++) {
        neighbor_entry_t* neighbor_entry_ptr = entry_ptr_list[neighbor_id_list[n]];
        // assign MPR status values, both flooding and routing MPR status
        tmp_tlv_ptr->tlv_value[n*2] = neighbor_entry_ptr->flooding_status;
        tmp_tlv_ptr->tlv_value[n*2 + 1] = neighbor_entry_ptr->routing_status;
    }
    // udpate block size
    hello_msg_ptr->addr_tlv_block_ptr->tlv_block_size += tmp_len;
   
    // update msg size given the addr tlv block
    header_ptr->msg_size += get_tlv_block_len(hello_msg_ptr->addr_tlv_block_ptr);

    // calculate msg len!
    ESP_LOGI(TAG, "A new HELLO with len = %d", header_ptr->msg_size);
    // done.
    // ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());
    // ESP_LOGI(TAG, "task stack water mark : %d", uxTaskGetStackHighWaterMark(NULL));

    // print tpology info
    print_topology_set();
}

// check whether this node id points to a two hop node.
uint8_t is_two_hop_node (uint8_t node_id) {
    // There are a few possibilities.
    // 1. it is zero, which means emtpy node_id. 2. the entry pointer is NULL, it just got deleted!
    // 3. the entry is not a two_hop entry. 4. this line must be symmetric (this is already satisfied since we only register symmetric two hop)
    if( node_id == 0 || entry_ptr_list[node_id] == NULL ) {
        return 0;
    }
    // a special case: an asym neighbor node.
    if ( ((uint8_t*)entry_ptr_list[node_id])[0] == NEIGHBOR_ENTRY && ((neighbor_entry_t*)entry_ptr_list[node_id])->link_status == LINK_HEARD )
        return 1;
    if ( ((uint8_t*)entry_ptr_list[node_id])[0] != TWO_HOP_ENTRY ) 
        return 0;
    return 1;
}

// compute the min metric to get to two-hop nodes
void compute_min_metric (uint8_t* min_metric_list) {
    uint8_t neighbor_id = 0;
    neighbor_entry_t* neighbor_ptr = NULL;
    uint8_t two_hop_id = 0;
    for (int n=0; n < neighbor_id_num; n++) {
        neighbor_id = neighbor_id_list[n];
        neighbor_ptr = entry_ptr_list[neighbor_id];
        // only consider symmetric neighbors
        if (neighbor_ptr->link_status != LINK_SYMMETRIC) continue;
        for(int l=0; l < neighbor_ptr->link_info.link_num; l++) {
            two_hop_id = neighbor_ptr->link_info.id_list_ptr[l];
            // ESP_LOGW(TAG, "computing min metric, neighbor #%d, has two-hop #%d", neighbor_id, two_hop_id);
            // if this is not a two hop node, skip this one.
            if ( !is_two_hop_node(two_hop_id)) {
                continue;
            }
            // update min
            uint16_t tmp_metric = neighbor_ptr->link_metric + neighbor_ptr->link_info.metric_list_ptr[l];
            if (tmp_metric < min_metric_list[two_hop_id]) {
                min_metric_list[two_hop_id] = tmp_metric;
            }
        }
    }
}

// assign the min metric according the new mpr's link info
void update_with_new_mpr (int16_t* mpr_list, uint8_t* metric_list, uint8_t new_mpr_id) {
    ESP_LOGI(TAG, "Updating new MPR #%d .", new_mpr_id);
    neighbor_entry_t* neighbor_ptr = entry_ptr_list[new_mpr_id];
    assert(neighbor_ptr !=  NULL);
    assert(neighbor_ptr->link_status == LINK_SYMMETRIC);
    uint8_t two_hop_id = 0;
    for(int l=0; l < neighbor_ptr->link_info.link_num; l++) {
        two_hop_id = neighbor_ptr->link_info.id_list_ptr[l];
        // if this is not a two hop node, skip this one.
        if ( !is_two_hop_node(two_hop_id)) {
            continue;
        }
        // update metric list
        uint16_t tmp_metric = neighbor_ptr->link_metric + neighbor_ptr->link_info.metric_list_ptr[l];
        if (metric_list[two_hop_id] > tmp_metric) {
            metric_list[two_hop_id] = tmp_metric;
            // udpate mpr list
            mpr_list[two_hop_id] = new_mpr_id;
        }
    }
}

// update MPR selection. We use the same selection for flooding and routing MPR.
void record_mpr_selection (int16_t* potential_mpr_list, uint8_t* mpr_metric_list) {
    uint8_t neighbor_id = 0;
    neighbor_entry_t* neighbor_ptr = NULL;
    two_hop_entry_t* two_hop_ptr = NULL;
    for (int x=0; x < two_hop_id_num; x++) {
        // simple check
        assert(potential_mpr_list[two_hop_id_list[x]] >= 0);
        if (potential_mpr_list[two_hop_id_list[x]] == 0) {
            ESP_LOGW(TAG, "Unlinked two-hop node #%d", two_hop_id_list[x]);
            continue;
        }
        // mark thie neighbor as MPR
        neighbor_id = potential_mpr_list[two_hop_id_list[x]];
        neighbor_ptr = entry_ptr_list[neighbor_id];
        if (neighbor_ptr->flooding_status == NOT_FLOODING)
            neighbor_ptr->flooding_status = FLOODING_TO;
        if (neighbor_ptr->flooding_status == FLOODING_FROM)
            neighbor_ptr->flooding_status = FLOODING_TO_FROM;
        if (neighbor_ptr->routing_status == NOT_ROUTING)
            neighbor_ptr->routing_status = ROUTING_TO;
        if (neighbor_ptr->routing_status == ROUTING_FROM)
            neighbor_ptr->routing_status = ROUTING_TO_FROM;
        // record this two-hop node's routing path
        two_hop_ptr = entry_ptr_list[two_hop_id_list[x]];
        two_hop_ptr->routing_info.next_hop = neighbor_id;
        two_hop_ptr->routing_info.hop_num = 2;
        two_hop_ptr->routing_info.path_metric = mpr_metric_list[two_hop_id_list[x]];
    }
    // there may be asym neighbors
    for (int n=0; n < neighbor_id_num; n++) {
        // if asym link
        if ( ((neighbor_entry_t*)entry_ptr_list[neighbor_id_list[n]])->link_status == LINK_HEARD ) {
            // if there is a path.
            if (potential_mpr_list[neighbor_id_list[n]] > 0) {
                neighbor_entry_t* asym_neighbor_ptr = entry_ptr_list[neighbor_id_list[n]];
                asym_neighbor_ptr->routing_info.next_hop = potential_mpr_list[neighbor_id_list[n]];
                asym_neighbor_ptr->routing_info.hop_num = 2;
                asym_neighbor_ptr->routing_info.path_metric = mpr_metric_list[neighbor_id_list[n]];
            }
        }
    }
}

// select MPR according latest info base and update node entry status
void update_mpr_status () {
    // the list of potential MPR node id
    int16_t *potential_mpr_list = NULL;
    uint8_t *min_metric_list = NULL; // the min metric can be achieved by N1
    uint8_t *mpr_metric_list = NULL; // the min metric can be achieved by M
    uint8_t neighbor_id = 0;
    neighbor_entry_t* neighbor_ptr = NULL;
    uint8_t two_hop_id = 0;

    // alloc mem
    potential_mpr_list = calloc(MAX_PEER_NUM, 4);
    if (potential_mpr_list == NULL) {
        ESP_LOGE(TAG, "Can not alloc mem for lists.");
        return;
    }
    min_metric_list = (uint8_t*)potential_mpr_list + MAX_PEER_NUM * 2;
    mpr_metric_list = (uint8_t*)potential_mpr_list + MAX_PEER_NUM * 3;

    // init min metric list
    for(int i=0; i < MAX_PEER_NUM; i++) {
        mpr_metric_list[i] = 255; // inf
        min_metric_list[i] = 255;
    }
    compute_min_metric(min_metric_list);

    // this is according to the example MPR Selection Algorithm in RFC7181 Appendix B.2
    // Notations:
    //      (TODO: update two hop entry list to match N with two-hop entry list.) 
    //      N -> two hop nodes that can not be accessed in one hop, or neighbor nodes that have a lower metric if routing through two hops.
    //      M -> MPR set
    //      R(x,M): For an element x in N1, the number of elements y in N for which d(x,y) is defined has minimal value.
    //              And no such minimal value can be achieved form M. D(x) = R(x,0)
    
    // 1. Add all elements x in N1 that have W(x) = WILL_ALWAYS to M.
    // TODO: we skip this. Do not support willingness for now.
    
    // 2. For each element y in N for which there is only one element x in N1 such that d2(x,y) is defined, add that element x to M.
    for (int n=0; n < neighbor_id_num; n++) {
        neighbor_id = neighbor_id_list[n];
        neighbor_ptr = entry_ptr_list[neighbor_id];
        // only consider symmetric neighbors
        if (neighbor_ptr->link_status != LINK_SYMMETRIC) continue;
        for(int l=0; l < neighbor_ptr->link_info.link_num; l++) {
            two_hop_id = neighbor_ptr->link_info.id_list_ptr[l];
            // if this is not a two hop node, skip this one.
            ESP_LOGI(TAG, "neighbor #%d, has two-hop #%d", neighbor_id, two_hop_id);
            if ( !is_two_hop_node(two_hop_id)) {
                continue;
            }
            // if no one has claimed MPR for that, take the spot
            if (potential_mpr_list[two_hop_id] == 0) {
                potential_mpr_list[two_hop_id] = neighbor_id;
            }
            // if some node has claimed. make it invalid.
            else if (potential_mpr_list[two_hop_id] > 0 ) {
                potential_mpr_list[two_hop_id] = -1;
            }
        }
    }
    for (int x=0; x < two_hop_id_num; x++) {
        // simple coverage check
        // this must be int!
        int16_t potential_mpr = potential_mpr_list[two_hop_id_list[x]];
        ESP_LOGI(TAG, "x=%d, two_hop id = %d, potential mpr = #%d", x, two_hop_id_list[x], potential_mpr);
        if (potential_mpr == 0) {
            ESP_LOGW(TAG, "Unlinked two-hop node #%d", two_hop_id_list[x]);
            continue;
        }
        // we have a slot with only one competer. it wins.
        if (potential_mpr > 0) {
            // update according to selected MPR
            update_with_new_mpr(potential_mpr_list, mpr_metric_list, potential_mpr);
        }
    }

    // we only need to resolve invalid MPR in the list below.

    // 3. While there exists any element x in N1 with R(x,M) > 0:
    //    Select an element x in N1 with greatest R(x,M) then add to M.
    while(1) {
        uint8_t best_MPR = 0;
        uint8_t max_R = 0;
        uint8_t max_D = 0; // this is used to decide MPR, when two nodes has equal max_R.
        // loop over N1
        for (int n=0; n < neighbor_id_num; n++) {
            neighbor_id = neighbor_id_list[n];
            neighbor_ptr = entry_ptr_list[neighbor_id];
            // only consider symmetric neighbors
            if (neighbor_ptr->link_status != LINK_SYMMETRIC) continue;
            uint8_t tmp_R = 0;
            uint8_t tmp_D = 0;
            // loop over N1 node's coverage
            for(int l=0; l < neighbor_ptr->link_info.link_num; l++) {
                two_hop_id = neighbor_ptr->link_info.id_list_ptr[l];
                // if this is not a two hop node, skip this one.
                if ( !is_two_hop_node(two_hop_id)) {
                    continue;
                }
                tmp_D ++; // add the number of covered two hop nodes
                uint8_t tmp_metric = neighbor_ptr->link_metric + neighbor_ptr->link_info.metric_list_ptr[l];
                // if reaches min metric and smaller than mpr_metric, add R by one. I can cover this one.
                assert(tmp_metric >= min_metric_list[two_hop_id]);
                if (tmp_metric == min_metric_list[two_hop_id] && tmp_metric < mpr_metric_list[two_hop_id]) {
                    tmp_R ++;
                }
            }
            // compare with max_R
            if (tmp_R > max_R) {
                best_MPR = neighbor_id;
                max_R = tmp_R;
                max_D = tmp_D;
            }
            if(tmp_R == max_R && max_R > 0) {
                if(tmp_D > max_D) {
                    best_MPR = neighbor_id;
                    max_R = tmp_R;
                    max_D = tmp_D;
                }
            }
        }
        // check MPR id
        if (best_MPR == 0) {
            break;
        }
        // we have a winner
        update_with_new_mpr(potential_mpr_list, mpr_metric_list, best_MPR);
    }

    // 4. record MPR results
    record_mpr_selection(potential_mpr_list, mpr_metric_list);

    // FREE mem
    free(potential_mpr_list);
    
}