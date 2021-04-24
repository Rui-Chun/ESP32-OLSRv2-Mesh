
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

#ifndef __NEIGHBOR_GRAPH__
#define __NEIGHBOR_GRAPH__

#include <oonf/libcommon/avl.h>
#include <oonf/libcommon/netaddr.h>
#include <oonf/nhdp/nhdp/nhdp_db.h>
#include <oonf/nhdp/nhdp/nhdp_domain.h>

struct neighbor_graph;
struct addr_node;
struct n1_node;

struct neighbor_graph_interface {
  bool (*is_allowed_link_tuple)(
    const struct nhdp_domain *, struct nhdp_interface *current_interface, struct nhdp_link *link);
  uint32_t (*calculate_d1_x_of_n2_addr)(const struct nhdp_domain *, struct neighbor_graph *, struct addr_node *);
  uint32_t (*calculate_d_x_y)(
    const struct nhdp_domain *, struct neighbor_graph *, struct n1_node *, struct addr_node *);
  uint32_t (*calculate_d2_x_y)(const struct nhdp_domain *, struct n1_node *, struct addr_node *);
  uint32_t (*get_willingness_n1)(const struct nhdp_domain *, struct n1_node *);
};

struct neighbor_graph {
  struct avl_tree set_n1;
  struct avl_tree set_n2;
  struct avl_tree set_n;
  struct avl_tree set_mpr;
  struct avl_tree set_mpr_candidates;
  struct neighbor_graph_interface *methods;

  uint32_t *d_x_y_cache;
};

/* FIXME Find a more consistent naming and/or approach to defining the set elements */
struct addr_node {
  struct netaddr addr;
  struct avl_node _avl_node;

  uint32_t table_offset;
  uint32_t min_d_z_y;
};

/* FIXME The link field is only used for flooding, while neigh is only used for routingt MPRs;
 having a void pointer + cast would probably be better (or a union?) */
struct n1_node {
  struct netaddr addr;
  struct nhdp_link *link;
  struct nhdp_neighbor *neigh;
  struct avl_node _avl_node;

  uint32_t table_offset;
};

void mpr_add_n1_node_to_set(struct avl_tree *set, struct nhdp_neighbor *neigh, struct nhdp_link *link, uint32_t offset);

void mpr_add_addr_node_to_set(struct avl_tree *set, const struct netaddr addr, uint32_t offset);

void mpr_init_neighbor_graph(struct neighbor_graph *graph, struct neighbor_graph_interface *methods);

void mpr_clear_addr_set(struct avl_tree *set);

void mpr_clear_n1_set(struct avl_tree *set);

void mpr_clear_neighbor_graph(struct neighbor_graph *graph);

bool mpr_is_mpr(struct neighbor_graph *graph, struct netaddr *addr);

uint32_t mpr_calculate_minimal_d_z_y(const struct nhdp_domain *, struct neighbor_graph *graph, struct addr_node *y);

void mpr_print_addr_set(struct avl_tree *set);

void mpr_print_n1_set(struct nhdp_domain *domain, struct avl_tree *set);

void mpr_print_sets(struct nhdp_domain *, struct neighbor_graph *graph);

uint32_t mpr_calculate_d_of_y_s(
  const struct nhdp_domain *domain, struct neighbor_graph *graph, struct addr_node *y, struct avl_tree *subset_s);

#endif
