
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

#ifndef OLSRV2_LAN_H_
#define OLSRV2_LAN_H_

#include <oonf/libcommon/avl.h>
#include <oonf/oonf.h>
#include <oonf/libcommon/netaddr.h>

#include <oonf/nhdp/nhdp/nhdp.h>
#include <oonf/olsrv2/olsrv2/olsrv2.h>
#include <oonf/base/os_routing.h>

/**
 * per-domain data for locally attached networks
 */
struct olsrv2_lan_domaindata {
  /*! outgoing metric value for LAN */
  uint32_t outgoing_metric;

  /*! routing metric (distance) for LAN */
  uint8_t distance;

  /*! true if LAN is active for this domain */
  bool active;
};

/**
 *  one locally attached network
 */
struct olsrv2_lan_entry {
  /*! unique key for LAN entry */
  struct os_route_key prefix;

  /*! hook into global tree of LANs */
  struct avl_node _node;

  /*! true if LAN has same distance on all active domains */
  bool same_distance;

  /*! domain specific LAN data */
  struct olsrv2_lan_domaindata _domaindata[NHDP_MAXIMUM_DOMAINS];
};

void olsrv2_lan_init(void);
void olsrv2_lan_cleanup(void);

EXPORT struct olsrv2_lan_entry *olsrv2_lan_add(
  struct nhdp_domain *domain, const struct os_route_key *prefix, uint32_t metric, uint8_t distance);
EXPORT void olsrv2_lan_remove(struct nhdp_domain *, const struct os_route_key *prefix);

EXPORT struct avl_tree *olsrv2_lan_get_tree(void);

/**
 * @param prefix source specific prefix
 * @return pointer to LAN set entry, NULL if not found
 */
static INLINE struct olsrv2_lan_entry *
olsrv2_lan_get(const struct os_route_key *prefix) {
  struct olsrv2_lan_entry *entry;

  return avl_find_element(olsrv2_lan_get_tree(), prefix, entry, _node);
}

/**
 * @param domain NHDP domain
 * @param lan OLSRv2 locally attached network
 * @return domain data of specified lan
 */
static INLINE struct olsrv2_lan_domaindata *
olsrv2_lan_get_domaindata(struct nhdp_domain *domain, struct olsrv2_lan_entry *lan) {
  return &lan->_domaindata[domain->index];
}

#endif /* OLSRV2_LAN_H_ */
