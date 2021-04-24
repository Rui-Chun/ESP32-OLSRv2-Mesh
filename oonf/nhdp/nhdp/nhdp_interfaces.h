
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

#ifndef NHDP_INTERFACES_H_
#define NHDP_INTERFACES_H_

struct nhdp_interface;
struct nhdp_interface_addr;
struct nhdp_interface_domaindata;

#include <oonf/libcommon/avl.h>
#include <oonf/oonf.h>
#include <oonf/libcommon/list.h>
#include <oonf/libcommon/netaddr.h>
#include <oonf/libcommon/netaddr_acl.h>
#include <oonf/base/oonf_rfc5444.h>
#include <oonf/base/oonf_timer.h>
#include <oonf/base/os_interface.h>

#include <oonf/nhdp/nhdp/nhdp_db.h>

/*! memory class for NHDP interface */
#define NHDP_CLASS_INTERFACE "nhdp_interf"

/*! memory class for NHDP interface address */
#define NHDP_CLASS_INTERFACE_ADDRESS "nhdp_iaddr"

/**
 * nhdp_interface represents a local interface
 * participating in the mesh network
 */
struct nhdp_interface {
  /*! listener for interface events */
  struct oonf_rfc5444_interface_listener rfc5444_if;

  /*! make sure interface data is available */
  struct os_interface_listener os_if_listener;

  /*! interval between two hellos sent through this interface */
  uint64_t hello_interval;

  /*! 'corrected' hello interval */
  uint64_t refresh_interval;

  /*! interval until a hello is considered lost */
  uint64_t validity_time;

  /*! See RFC 6130, 5.3.2 and 5.4.1 */
  uint64_t h_hold_time;

  /*! See RFC 6130, 5.3.2 and 5.4.1 */
  uint64_t l_hold_time;

  /*! See RFC 6130, 5.3.2 and 5.4.1 */
  uint64_t n_hold_time;

  /*! See RFC 6130, 5.3.2 and 5.4.1 */
  uint64_t i_hold_time;

  /*! variables to store hello interval overwritten by a plugin */
  uint64_t overwrite_hello_interval;

  /*! variables to store hello validity overwritten by a plugin */
  uint64_t overwrite_hello_validity;

  /*! ACL for interface addresses that should be included into HELLOs */
  struct netaddr_acl ifaddr_filter;

  /*! local ipv4 address of the NHDP rfc5444 socket */
  struct netaddr local_ipv4;

  /*! local ipv6 address of the NHDP rfc5444 socket */
  struct netaddr local_ipv6;

  /**
   * true if this interface has a neighbor that should be reached through
   * IPv4 for flooding. This is set by the nhdp interface code.
   */
  bool use_ipv4_for_flooding;

  /**
   * true if this interface has a neighbor that should be reached through
   * IPv6 for flooding. This is set by the nhdp interface code.
   */
  bool use_ipv6_for_flooding;

  /*! address family used for dualstack purpose */
  int dualstack_af_type;

  /*! timer for hello generation */
  struct oonf_timer_instance _hello_timer;

  /*! member entry for global interface tree */
  struct avl_node _node;

  /*! tree of interface addresses */
  struct avl_tree _if_addresses;

  /*! list of interface nhdp links (nhdp_link objects) */
  struct list_entity _links;

  /*! tree of addresses of links (nhdp_laddr objects) */
  struct avl_tree _link_addresses;

  /*! tree of originator addresses of links (nhdp_link objects) */
  struct avl_tree _link_originators;

  /*! tree of two-hop entries of links of this interface (nhdp_l2hop) */
  struct avl_tree _if_twohops;

  /*! interface has been registered */
  bool registered;

  /**
   * reference count, some plugins might want to attach data to
   * this structure before its generated by the nhdp subsystem
   */
  int _refcount;
};

/**
 * nhdp_interface_addr represents a single address of a nhdp interface
 */
struct nhdp_interface_addr {
  /*! interface address */
  struct netaddr if_addr;

  /*! pointer to nhdp interface */
  struct nhdp_interface *interf;

  /*! true if address was removed */
  bool removed;

  /*! true if address will be removed soon */
  bool _to_be_removed;

  /*! validity time until entry should be removed from database */
  struct oonf_timer_instance _vtime;

  /*! member entry for interfaces tree of addresses */
  struct avl_node _if_node;

  /*! member entry for global address tree */
  struct avl_node _global_node;
};

void nhdp_interfaces_init(struct oonf_rfc5444_protocol *);
void nhdp_interfaces_cleanup(void);

EXPORT struct nhdp_interface *nhdp_interface_add(const char *name);
EXPORT void nhdp_interface_remove(struct nhdp_interface *interf);
EXPORT void nhdp_interface_apply_settings(struct nhdp_interface *interf);
EXPORT void nhdp_interface_update_status(struct nhdp_interface *);

EXPORT struct avl_tree *nhdp_interface_get_tree(void);
EXPORT struct avl_tree *nhdp_interface_get_address_tree(void);

EXPORT uint64_t nhdp_interface_set_hello_interval(struct nhdp_interface *interf, uint64_t new_interval);
EXPORT uint64_t nhdp_set_hello_validity(struct nhdp_interface *interf, uint64_t new_interval);

/**
 * @param name interface name
 * @return nhdp interface, NULL if not found
 */
static INLINE struct nhdp_interface *
nhdp_interface_get(const char *name) {
  struct nhdp_interface *interf;

  return avl_find_element(nhdp_interface_get_tree(), name, interf, _node);
}

/**
 * @param interf nhdp interface
 * @return name of interface (e.g. wlan0)
 */
static INLINE const char *
nhdp_interface_get_name(const struct nhdp_interface *interf) {
  return interf->_node.key;
}

/**
 * @param interf nhdp interface
 * @param addr network address
 * @return nhdp interface address
 */
static INLINE struct nhdp_interface_addr *
nhdp_interface_addr_if_get(const struct nhdp_interface *interf, const struct netaddr *addr) {
  struct nhdp_interface_addr *iaddr;

  return avl_find_element(&interf->_if_addresses, addr, iaddr, _if_node);
}

/**
 * @param addr network address
 * @return nhdp interface address
 */
static INLINE struct nhdp_interface_addr *
nhdp_interface_addr_global_get(const struct netaddr *addr) {
  struct nhdp_interface_addr *iaddr;

  return avl_find_element(nhdp_interface_get_address_tree(), addr, iaddr, _global_node);
}

/**
 * Add a link to a nhdp interface
 * @param interf nhdp interface
 * @param lnk nhdp link
 */
static INLINE void
nhdp_interface_add_link(struct nhdp_interface *interf, struct nhdp_link *lnk) {
  lnk->local_if = interf;

  list_add_tail(&interf->_links, &lnk->_if_node);
}

/**
 * Remove a nhdp link from a nhdp interface
 * @param lnk nhdp link
 */
static INLINE void
nhdp_interface_remove_link(struct nhdp_link *lnk) {
  list_remove(&lnk->_if_node);
  lnk->local_if = NULL;
}

/**
 * Add a twohop link to a nhdp interface
 * @param interf nhdp interface
 * @param twohop twohop link
 */
static INLINE void
nhdp_interface_add_l2hop(struct nhdp_interface *interf, struct nhdp_l2hop *twohop) {
  twohop->_if_node.key = &twohop->twohop_addr;
  avl_insert(&interf->_if_twohops, &twohop->_if_node);
}

/**
 * Remove a nhdp twohop link from a nhdp interface
 * @param twohop twohop link
 */
static INLINE void
nhdp_interface_remove_l2hop(struct nhdp_l2hop *twohop) {
  avl_remove(&twohop->link->local_if->_if_twohops, &twohop->_if_node);
}

/**
 * Attach a link address to the local nhdp interface
 * @param laddr NHDP link address
 */
static INLINE void
nhdp_interface_add_laddr(struct nhdp_laddr *laddr) {
  avl_insert(&laddr->link->local_if->_link_addresses, &laddr->_if_node);
}

/**
 * Detach a link address from the local nhdp interface
 * @param laddr NHDP link address
 */
static INLINE void
nhdp_interface_remove_laddr(struct nhdp_laddr *laddr) {
  avl_remove(&laddr->link->local_if->_link_addresses, &laddr->_if_node);
}

/**
 * @param interf local nhdp interface
 * @param addr network address
 * @return link address object fitting the network address, NULL if not found
 */
static INLINE struct nhdp_laddr *
nhdp_interface_get_link_addr(const struct nhdp_interface *interf, const struct netaddr *addr) {
  struct nhdp_laddr *laddr;

  return avl_find_element(&interf->_link_addresses, addr, laddr, _if_node);
}

/**
 * @param interf local nhdp interface
 * @param originator originator address
 * @return corresponding nhdp link, NULL if not found
 */
static INLINE struct nhdp_link *
nhdp_interface_link_get_by_originator(const struct nhdp_interface *interf, const struct netaddr *originator) {
  struct nhdp_link *lnk;
  return avl_find_element(&interf->_link_originators, originator, lnk, _originator_node);
}

/**
 * @param nhdp_if pointer to nhdp interface
 * @return pointer to corresponding os_interface
 */
static INLINE struct os_interface_listener *
nhdp_interface_get_if_listener(struct nhdp_interface *nhdp_if) {
  return &nhdp_if->os_if_listener;
}

/**
 * @param nhdp_if NHDP interface
 * @param af_family address family
 * @return socket address of NHDP interface
 */
static INLINE const struct netaddr *
nhdp_interface_get_socket_address(struct nhdp_interface *nhdp_if, int af_family) {
  switch (af_family) {
    case AF_INET:
      return &nhdp_if->local_ipv4;
    case AF_INET6:
      return &nhdp_if->local_ipv6;
    default:
      return &NETADDR_UNSPEC;
  }
}

#endif /* NHDP_INTERFACES_H_ */
