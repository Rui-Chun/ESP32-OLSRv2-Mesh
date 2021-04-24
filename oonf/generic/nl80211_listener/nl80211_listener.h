
/*
 * The olsr.org Optimized Link-State Routing daemon version 2 (olsrd2)
 * Copyright (c) 2004-2015, the olsr.org team - see HISTORY file
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/**
 * @file
 */

#ifndef NL80211_LISTENER_H_
#define NL80211_LISTENER_H_

#include <oonf/oonf.h>
#include <oonf/libcore/oonf_subsystem.h>
#include <oonf/base/oonf_layer2.h>
#include <oonf/base/os_interface.h>

/*! subsystem identifier */
#define OONF_NL80211_LISTENER_SUBSYSTEM "nl80211_listener"

/**
 * Session data of an interface where the listener is probing
 */
struct nl80211_if {
  /*! name of interface */
  char name[IF_NAMESIZE];

  /*! interface listener */
  struct os_interface_listener if_listener;

  /*! layer2 network object of interface */
  struct oonf_layer2_net *l2net;

  /*! physical interface index */
  int wifi_phy_if;

  /*! maximum tx rate */
  uint64_t max_tx;

  /*! maximum rx rate */
  uint64_t max_rx;

  /*! true if data of interface were changed */
  bool ifdata_changed;

  /*! true if interface should be removed */
  bool _remove;

  /*! true if interface section config was already committed for interface */
  bool _if_section;

  /*! true if nl80211 section config was already committed for interface */
  bool _nl80211_section;

  /*! hook into tree of nl80211 interfaces */
  struct avl_node _node;
};

struct oonf_layer2_destination *nl80211_add_dst(struct oonf_layer2_neigh *, const struct netaddr *dst);
bool nl80211_change_l2net_data(struct oonf_layer2_net *l2net, enum oonf_layer2_network_index idx,
                               int64_t value, int64_t scaling);
bool nl80211_change_l2net_neighbor_default(
  struct oonf_layer2_net *l2net, enum oonf_layer2_neighbor_index idx, int64_t value, int64_t scaling);
void nl80211_cleanup_l2neigh_data(struct oonf_layer2_neigh *l2neigh);
bool nl80211_change_l2neigh_data(
  struct oonf_layer2_neigh *l2neigh, enum oonf_layer2_neighbor_index idx, int64_t value, int64_t scaling);
bool nl80211_create_broadcast_neighbor(void);

/**
 * @param interf nl80211 interface
 * @returns nl80211 interface index
 */
static INLINE unsigned
nl80211_get_if_baseindex(struct nl80211_if *interf) {
  return interf->if_listener.data->base_index;
}

#endif /* NL80211_LISTENER_H_ */
