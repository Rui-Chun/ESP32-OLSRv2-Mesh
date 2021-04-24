
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

#ifndef NHDP_DB_H_
#define NHDP_DB_H_

struct nhdp_neighbor;
struct nhdp_link;
struct nhdp_l2hop;
struct nhdp_laddr;

#include <oonf/libcommon/avl.h>
#include <oonf/oonf.h>
#include <oonf/libcommon/list.h>
#include <oonf/libcommon/netaddr.h>
#include <oonf/base/oonf_rfc5444.h>
#include <oonf/base/oonf_timer.h>

#include <oonf/nhdp/nhdp/nhdp.h>

/*! memory class for NHDP links */
#define NHDP_CLASS_LINK "nhdp_link"

/*! memory class for NHDP link address */
#define NHDP_CLASS_LINK_ADDRESS "nhdp_laddr"

/*! memory class for NHDP 2hop link */
#define NHDP_CLASS_LINK_2HOP "nhdp_l2hop"

/*! memory class for NHDP neighbor */
#define NHDP_CLASS_NEIGHBOR "nhdp_neighbor"

/*! memory class for NHDP neighbor address */
#define NHDP_CLASS_NEIGHBOR_ADDRESS "nhdp_naddr"

/**
 * link status of NHDP neighbors
 */
enum nhdp_link_status
{
  /*! link is pending */
  NHDP_LINK_PENDING = -1,

  /*! link is lost */
  NHDP_LINK_LOST = RFC6130_LINKSTATUS_LOST,

  /*! link is symmetric */
  NHDP_LINK_SYMMETRIC = RFC6130_LINKSTATUS_SYMMETRIC,

  /*! link has been heard */
  NHDP_LINK_HEARD = RFC6130_LINKSTATUS_HEARD,
};

/**
 * NHDP db constants
 */
enum
{
  /*! maximum text length of link status */
  NHDP_LINK_STATUS_TXTLENGTH = 10
};

/**
 * Represents a NHDP link metric pair
 */
struct nhdp_metric {
  /*! incoming link metric cost */
  uint32_t in;

  /*! outgoing link metric cost */
  uint32_t out;
};

/**
 * Data for one NHDP domain of a link
 */
struct nhdp_link_domaindata {
  /*! incoming and outgoing metric cost */
  struct nhdp_metric metric;

  /*! time when this metric value was changed */
  uint64_t last_metric_change;
};

/**
 * Data for one NHDP domain of a neighbor
 */
struct nhdp_neighbor_domaindata {
  /*! incoming and outgoing metric cost */
  struct nhdp_metric metric;

  /*! pointer to the best link available to the neighbor */
  struct nhdp_link *best_out_link;

  /*! outgoing link metric of the best link */
  uint32_t best_out_link_metric;

  /*! interface index for the best link available to the neighbor */
  unsigned best_link_ifindex;

  /*! true if the local router has been selected as a MPR by the neighbor */
  bool local_is_mpr;

  /*! true if the neighbor has been selected as a MPR by this router */
  bool neigh_is_mpr;

  /*! true if the neighbor was a MPR of this router before the MPR recalculation */
  bool _neigh_was_mpr;

  /*! Routing willingness of neighbor */
  uint8_t willingness;
};

/**
 * Data for one NHDP domain of a 2-hop neighbor
 */
struct nhdp_l2hop_domaindata {
  /*! incoming and outgoing metric */
  struct nhdp_metric metric;

  /*! metric used for last MPR calculation */
  uint32_t _last_used_outgoing_metric;
};

/**
 * nhdl_link represents a link by a specific local interface to one interface
 * of a one-hop neighbor.
 */
struct nhdp_link {
  /*! last received validity time */
  uint64_t vtime_value;

  /*! last received interval time */
  uint64_t itime_value;

  /*! timer that fires if this link is not symmetric anymore */
  struct oonf_timer_instance sym_time;

  /*! timer that fires if the last received neighbor HELLO timed out */
  struct oonf_timer_instance heard_time;

  /*! timer that fires when the link has to be removed from the database */
  struct oonf_timer_instance vtime;

  /*! last status of the linked, used to detect status changes */
  enum nhdp_link_status last_status;

  /*! cached status of the linked */
  enum nhdp_link_status status;

  /*! pointer to local interface for this link */
  struct nhdp_interface *local_if;

  /*! pointer to other (dualstack) representation of this link */
  struct nhdp_link *dualstack_partner;

  /*! pointer to neighbor entry of the other side of the link */
  struct nhdp_neighbor *neigh;

  /*! interface address used for NHDP of remote link end */
  struct netaddr if_addr;

  /*! mac address of remote link end */
  struct netaddr remote_mac;

  /*! timestamp when the current link status was set */
  uint64_t last_status_change;

  /*! true if the local router has been selected as a MPR by the neighbor */
  bool local_is_flooding_mpr;

  /*! true if the neighbor has been selected as a MPR by this router */
  bool neigh_is_flooding_mpr;

  /*! Willingness of neighbor for flooding data */
  uint8_t flooding_willingness;

  /*! true if the neighbor has been selected as a MPR by this router */
  bool _neigh_was_flooding_mpr;

  /*! internal field for NHDP processing */
  int _process_count;

  /*! tree of local addresses of the other side of the link */
  struct avl_tree _addresses;

  /*! tree of two-hop addresses reachable through the other side of the link */
  struct avl_tree _2hop;

  /*! member entry for global list of nhdp links */
  struct list_entity _global_node;

  /*! member entry for nhdp links of local interface */
  struct list_entity _if_node;

  /*! member entry for nhdp links of neighbor node */
  struct list_entity _neigh_node;

  /*! optional member node for interface tree of originators */
  struct avl_node _originator_node;

  /*! Array of link metrics */
  struct nhdp_link_domaindata _domaindata[NHDP_MAXIMUM_DOMAINS];
};

/**
 * nhdp_laddr represents an interface address of a link
 */
struct nhdp_laddr {
  /*! links interface address */
  struct netaddr link_addr;

  /*! link entry for address */
  struct nhdp_link *link;

  /*! internal variable for NHDP processing */
  bool _might_be_removed;

  /*! member entry for addresses of neighbor link */
  struct avl_node _link_node;

  /*! member entry for addresss of neighbor */
  struct avl_node _neigh_node;

  /*! member entry for interface tree of link addresses */
  struct avl_node _if_node;
};

/**
 * nhdp_l2hop represents an address of a two-hop neighbor
 */
struct nhdp_l2hop {
  /*! address of two-hop neighbor */
  struct netaddr twohop_addr;

  /*! true if neighbor is on the same interface as the link */
  bool same_interface;

  /*! link entry for two-hop address */
  struct nhdp_link *link;

  /*! validity time for this address */
  struct oonf_timer_instance _vtime;

  /*! member entry for two-hop addresses of neighbor link */
  struct avl_node _link_node;

  /*! member entry for interface list of two-hop addresses */
  struct avl_node _if_node;

  /*! Array of link metrics */
  struct nhdp_l2hop_domaindata _domaindata[NHDP_MAXIMUM_DOMAINS];
};

/**
 * nhdp_neighbor represents a neighbor node (with one or multiple interfaces
 */
struct nhdp_neighbor {
  /*! originator address of this node, might by type AF_UNSPEC */
  struct netaddr originator;

  /*! number of links to this neighbor which are symmetric */
  int symmetric;

  /*! pointer to other (dualstack) representation of this neighbor */
  struct nhdp_neighbor *dualstack_partner;

  /*! internal field for NHDP processing */
  int _process_count;

  /*! true if the neighbor has been selected as an MPR during selection algorithm */
  bool selection_is_mpr;

  /*!
   * originator address of this node before it was changed.
   * Mostly used in CHANGE events
   */
  struct netaddr _old_originator;

  /*! list of links for this neighbor */
  struct list_entity _links;

  /*! tree of addresses of this neighbor */
  struct avl_tree _neigh_addresses;

  /*! tree of addresses of this neighbors links */
  struct avl_tree _link_addresses;

  /*! member entry for global list of neighbors */
  struct list_entity _global_node;

  /*! optional member node for global tree of originators */
  struct avl_node _originator_node;

  /*! Array of link metrics */
  struct nhdp_neighbor_domaindata _domaindata[NHDP_MAXIMUM_DOMAINS];
};

/**
 * nhdp_naddr represents an address of a known 1-hop neighbor
 * or a former (lost) address which will be removed soon
 */
struct nhdp_naddr {
  /*! neighbor interface address */
  struct netaddr neigh_addr;

  /*! backlink to neighbor */
  struct nhdp_neighbor *neigh;

  /*! link address usage counter */
  int laddr_count;

  /*! validity time for this address when its lost */
  struct oonf_timer_instance _lost_vtime;

  /*! member entry for neighbor address tree */
  struct avl_node _neigh_node;

  /*! member entry for global neighbor address tree */
  struct avl_node _global_node;

  /**
   * temporary variables for NHDP Hello processing
   * true if address is part of the local interface
   */
  bool _this_if;

  /**
   * temporary variables for NHDP Hello processing
   * true if address is considered for removal
   */
  bool _might_be_removed;
};

void nhdp_db_init(void);
void nhdp_db_cleanup(void);

EXPORT struct nhdp_neighbor *nhdp_db_neighbor_add(void);
EXPORT void nhdp_db_neighbor_remove(struct nhdp_neighbor *);
EXPORT void nhdp_db_neighbor_set_unsymmetric(struct nhdp_neighbor *neigh);
EXPORT void nhdp_db_neighbor_join(struct nhdp_neighbor *, struct nhdp_neighbor *);
EXPORT struct nhdp_naddr *nhdp_db_neighbor_addr_add(struct nhdp_neighbor *, const struct netaddr *);
EXPORT void nhdp_db_neighbor_addr_remove(struct nhdp_naddr *);
EXPORT void nhdp_db_neighbor_addr_move(struct nhdp_neighbor *, struct nhdp_naddr *);
EXPORT void nhdp_db_neighbor_set_originator(struct nhdp_neighbor *, const struct netaddr *);
EXPORT void nhdp_db_neighbor_connect_dualstack(struct nhdp_neighbor *, struct nhdp_neighbor *);
EXPORT void nhdp_db_neigbor_disconnect_dualstack(struct nhdp_neighbor *neigh);
EXPORT uint32_t nhdp_db_neighbor_get_set_id(void);

EXPORT struct nhdp_link *nhdp_db_link_add(struct nhdp_neighbor *ipv4, struct nhdp_interface *ipv6);
EXPORT void nhdp_db_link_remove(struct nhdp_link *);
EXPORT void nhdp_db_link_set_unsymmetric(struct nhdp_link *lnk);
EXPORT struct nhdp_laddr *nhdp_db_link_addr_add(struct nhdp_link *, const struct netaddr *);
EXPORT void nhdp_db_link_addr_remove(struct nhdp_laddr *);
EXPORT void nhdp_db_link_addr_move(struct nhdp_link *, struct nhdp_laddr *);
EXPORT struct nhdp_l2hop *nhdp_db_link_2hop_add(struct nhdp_link *, const struct netaddr *);
EXPORT void nhdp_db_link_2hop_remove(struct nhdp_l2hop *);
EXPORT void nhdp_db_link_connect_dualstack(struct nhdp_link *ipv4, struct nhdp_link *ipv6);
EXPORT void nhdp_db_link_disconnect_dualstack(struct nhdp_link *lnk);

EXPORT int _nhdp_db_link_calculate_status(struct nhdp_link *lnk);
EXPORT void nhdp_db_link_update_status(struct nhdp_link *);

EXPORT const char *nhdp_db_link_status_to_string(struct nhdp_link *);

EXPORT struct list_entity *nhdp_db_get_neigh_list(void);
EXPORT struct list_entity *nhdp_db_get_link_list(void);
EXPORT struct avl_tree *nhdp_db_get_naddr_tree(void);
EXPORT struct avl_tree *nhdp_db_get_neigh_originator_tree(void);

/**
 * @param addr network address
 * @return corresponding neighbor address object, NULL if not found
 */
static INLINE struct nhdp_naddr *
nhdp_db_neighbor_addr_get(const struct netaddr *addr) {
  struct nhdp_naddr *naddr;
  return avl_find_element(nhdp_db_get_naddr_tree(), addr, naddr, _global_node);
}

/**
 * @param originator originator address
 * @return corresponding nhdp neighbor, NULL if not found
 */
static INLINE struct nhdp_neighbor *
nhdp_db_neighbor_get_by_originator(const struct netaddr *originator) {
  struct nhdp_neighbor *neigh;
  return avl_find_element(nhdp_db_get_neigh_originator_tree(), originator, neigh, _originator_node);
}

/**
 * @param lnk nhdp link
 * @param addr network address
 * @return corresponding link address object, NULL if not found
 */
static INLINE struct nhdp_laddr *
nhdp_db_link_addr_get(const struct nhdp_link *lnk, const struct netaddr *addr) {
  struct nhdp_laddr *laddr;
  return avl_find_element(&lnk->_addresses, addr, laddr, _link_node);
}

/**
 * @param lnk nhdp link
 * @param addr network address
 * @return corresponding link two-hop neighbor address, NULL If not found
 */
static INLINE struct nhdp_l2hop *
ndhp_db_link_2hop_get(const struct nhdp_link *lnk, const struct netaddr *addr) {
  struct nhdp_l2hop *l2hop;
  return avl_find_element(&lnk->_2hop, addr, l2hop, _link_node);
}

/**
 * Sets the validity time of a nhdp link
 * @param lnk pointer to nhdp link
 * @param vtime validity time in milliseconds
 */
static INLINE void
nhdp_db_link_set_vtime(struct nhdp_link *lnk, uint64_t vtime) {
  oonf_timer_set(&lnk->vtime, vtime);
}

/**
 * Sets the time until a NHDP link is not considered heard anymore
 * @param lnk pointer to nhdp link
 * @param htime heard time in milliseconds
 */
static INLINE void
nhdp_db_link_set_heardtime(struct nhdp_link *lnk, uint64_t htime) {
  oonf_timer_set(&lnk->heard_time, htime);
}

/**
 * Sets the time until a NHDP link is not considered symmetric anymore
 * @param lnk pointer to nhdp link
 * @param stime symmetric time in milliseconds
 */
static INLINE void
nhdp_db_link_set_symtime(struct nhdp_link *lnk, uint64_t stime) {
  oonf_timer_set(&lnk->sym_time, stime);
  nhdp_db_link_update_status(lnk);
}

/**
 * Set the validity time of a two-hop neighbor
 * @param l2hop nhdp link two-hop neighbor
 * @param vtime new validity time
 */
static INLINE void
nhdp_db_link_2hop_set_vtime(struct nhdp_l2hop *l2hop, uint64_t vtime) {
  oonf_timer_set(&l2hop->_vtime, vtime);
}

/**
 * Define a neighbor address as lost
 * @param naddr nhdp neighbor address
 * @param vtime time until lost address gets purged from the database
 */
static INLINE void
nhdp_db_neighbor_addr_set_lost(struct nhdp_naddr *naddr, uint64_t vtime) {
  oonf_timer_set(&naddr->_lost_vtime, vtime);
}

/**
 * Define a neighbor address as not lost anymore
 * @param naddr nhdp neighbor address
 */
static INLINE void
nhdp_db_neighbor_addr_not_lost(struct nhdp_naddr *naddr) {
  oonf_timer_stop(&naddr->_lost_vtime);
}

/**
 * @param naddr nhdp neighbor address
 * @return true if address is lost, false otherwise
 */
static INLINE bool
nhdp_db_neighbor_addr_is_lost(const struct nhdp_naddr *naddr) {
  return oonf_timer_is_active(&naddr->_lost_vtime);
}

/**
 * @param lnk pointer to NHDP link
 * @return true if link is part of a dualstack link
 */
static INLINE bool
nhdp_db_link_is_dualstack(const struct nhdp_link *lnk) {
  return lnk->dualstack_partner != NULL;
}

/**
 * @param lnk pointer to NHDP link
 * @param af_type address family type
 * @return true if link is part of a dualstack link with the specified
 *   address family
 */
static INLINE bool
nhdp_db_link_is_dualstack_type(const struct nhdp_link *lnk, int af_type) {
  return nhdp_db_link_is_dualstack(lnk) && netaddr_get_address_family(&lnk->neigh->originator) == af_type;
}

/**
 * @param lnk pointer to NHDP link
 * @return true if link is the IPv4 part of a dualstack link
 */
static INLINE bool
nhdp_db_link_is_ipv4_dualstack(const struct nhdp_link *lnk) {
  return nhdp_db_link_is_dualstack_type(lnk, AF_INET);
}

/**
 * @param lnk pointer to NHDP link
 * @return true if link is the IPv6 part of a dualstack link
 */
static INLINE bool
nhdp_db_link_is_ipv6_dualstack(const struct nhdp_link *lnk) {
  return nhdp_db_link_is_dualstack_type(lnk, AF_INET6);
}

/**
 * @param neigh pointer to NHDP neighbor
 * @param af_type address family type
 * @return true if neighbor is part of a dualstack neighbor with the
 *   specified address family
 */
static INLINE bool
nhdp_db_neighbor_is_dualstack_type(const struct nhdp_neighbor *neigh, int af_type) {
  return neigh->dualstack_partner != NULL && netaddr_get_address_family(&neigh->originator) == af_type;
}

/**
 * @param neigh pointer to NHDP neighbor
 * @return true if neighbor is the IPv4 part of a dualstack neighbor
 */
static INLINE bool
nhdp_db_neighbor_is_ipv4_dualstack(const struct nhdp_neighbor *neigh) {
  return nhdp_db_neighbor_is_dualstack_type(neigh, AF_INET);
}

/**
 * @param neigh pointer to NHDP neighbor
 * @return true if neighbor is the IPv6 part of a dualstack neighbor
 */
static INLINE bool
nhdp_db_neighbor_is_ipv6_dualstack(const struct nhdp_neighbor *neigh) {
  return nhdp_db_neighbor_is_dualstack_type(neigh, AF_INET6);
}

static INLINE bool
nhdp_db_2hop_is_lost(const struct nhdp_l2hop *l2hop) {
  return oonf_timer_is_active(&l2hop->_vtime);
}
#endif /* NHDP_DB_H_ */
