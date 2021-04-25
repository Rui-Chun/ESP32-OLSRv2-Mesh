#ifndef OLSR_HANDLERS_H
#define OLSR_HANDLERS_H

#include "espnow_olsr.h"

/* TODO: do not need to free the pkt. the main event loop will do that */
// the return event must has a separate buf from the recv_pkt.
espnow_olsr_event_t olsr_recv_pkt_handler(raw_packet recv_pkt);




#endif