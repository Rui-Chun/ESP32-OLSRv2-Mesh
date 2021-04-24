
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

#ifndef NHDP_HYSTERESIS_H_
#define NHDP_HYSTERESIS_H_

#include <oonf/oonf.h>
#include <oonf/base/oonf_rfc5444.h>

#include <oonf/nhdp/nhdp/nhdp_db.h>

/**
 * Output buffer for hysteresis data string representation
 */
struct nhdp_hysteresis_str {
  /*! text buffer for maximum length string representation */
  char buf[128];
};

/**
 * NHDP link hysteresis handler
 */
struct nhdp_hysteresis_handler {
  /*! name of the handler */
  const char *name;

  /**
   * callback to update link hysteresis pending and lost status.
   * Some implementations might also change their internal state
   * @param lnk nhdp link
   * @param context rfc5444 reader context
   */
  void (*update_hysteresis)(struct nhdp_link *lnk, struct rfc5444_reader_tlvblock_context *context);

  /**
   * callback to check if link is pending
   * @param lnk nhdp link
   * @return true if link is pending
   */
  bool (*is_pending)(struct nhdp_link *lnk);

  /**
   * callback to check if link is lost
   * @param lnk nhdp link
   * @return true if link is lost
   */
  bool (*is_lost)(struct nhdp_link *lnk);

  /**
   * callback to convert hysteresis state into string representation
   * @param buf buffer for text output
   * @param lnk nhdp link
   * @return pointer to buffer
   */
  const char *(*to_string)(struct nhdp_hysteresis_str *buf, struct nhdp_link *lnk);
};

EXPORT void nhdp_hysteresis_set_handler(struct nhdp_hysteresis_handler *);
EXPORT struct nhdp_hysteresis_handler *nhdp_hysteresis_get_handler(void);

/**
 * Update a links hysteresis because of an incoming NHDP Hello message
 * @param lnk pointer to NHDP link
 * @param context pointer to rfc5444 context (message context)
 */
static INLINE void
nhdp_hysteresis_update(struct nhdp_link *lnk, struct rfc5444_reader_tlvblock_context *context) {
  nhdp_hysteresis_get_handler()->update_hysteresis(lnk, context);
}

/**
 * @param lnk pointer to nhdp link
 * @return true if link is pending
 */
static INLINE bool
nhdp_hysteresis_is_pending(struct nhdp_link *lnk) {
  return nhdp_hysteresis_get_handler()->is_pending(lnk);
}

/**
 * @param lnk pointer to nhdp link
 * @return true if link is lost
 */
static INLINE bool
nhdp_hysteresis_is_lost(struct nhdp_link *lnk) {
  return nhdp_hysteresis_get_handler()->is_lost(lnk);
}

/**
 * @param buf pointer to nhdp hysteresis text buffer
 * @param lnk pointer to nhdp link
 * @return pointer to text description of nhdp links hysteresis status
 */
static INLINE const char *
nhdp_hysteresis_to_string(struct nhdp_hysteresis_str *buf, struct nhdp_link *lnk) {
  return nhdp_hysteresis_get_handler()->to_string(buf, lnk);
}

#endif /* NHDP_HYSTERESIS_H_ */
