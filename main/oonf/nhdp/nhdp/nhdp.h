
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

#ifndef NHDP_H_
#define NHDP_H_

#include <oonf/oonf.h>
#include <oonf/libcommon/netaddr.h>
#include <oonf/libconfig/cfg_schema.h>
#include <oonf/libcore/oonf_subsystem.h>
#include <oonf/base/oonf_rfc5444.h>

/*! subsystem identifier */
#define OONF_NHDP_SUBSYSTEM "nhdp"

/*! configuration section for NHDP */
#define CFG_NHDP_SCHEMA_DOMAIN_SECTION_INIT .type = "domain", .mode = CFG_SSMODE_NAMED_WITH_DEFAULT, .def_name = "0"

/*! configuration section for NHDP domains */
#define CFG_NHDP_SCHEMA_NHDP_SECTION_INIT .type = "nhdp", .mode = CFG_SSMODE_UNNAMED

/*! special name for 'no' metric/mpr */
#define CFG_DOMAIN_NO_METRIC_MPR "-"

/*! special name for 'any' metric/mpr */
#define CFG_DOMAIN_ANY_METRIC_MPR "*"

enum
{
  /*! maximum number of metric domains */
  NHDP_MAXIMUM_DOMAINS = 4,

  /*! message tlv for transporting IPv4 originator in ipv6 messages */
  NHDP_MSGTLV_IPV4ORIGINATOR = 226,

  /*! message tlv for transporting mac address */
  NHDP_MSGTLV_MAC = 227,

  /*! Address TLV for custom link metric data */
  NHDP_ADDRTLV_LQ_CUSTOM = 228,
};

EXPORT void nhdp_set_originator(const struct netaddr *);
EXPORT void nhdp_reset_originator(int af_type);
EXPORT const struct netaddr *nhdp_get_originator(int af_type);

EXPORT bool nhdp_flooding_selector(
  struct rfc5444_writer *writer, struct rfc5444_writer_target *rfc5444_target, void *ptr);
EXPORT bool nhdp_forwarding_selector(
  struct rfc5444_writer_target *rfc5444_target, struct rfc5444_reader_tlvblock_context *context);

#endif /* NHDP_H_ */
