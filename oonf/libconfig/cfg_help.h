
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

#ifndef CFG_HELP_H_
#define CFG_HELP_H_

#include <oonf/libcommon/autobuf.h>
#include <oonf/oonf.h>
#include <oonf/libconfig/cfg_schema.h>

/*! prefix before help text parameter explanation */
#define CFG_HELP_INDENT_PREFIX "    "

EXPORT void cfg_help_printable(struct autobuf *out, size_t len);
EXPORT void cfg_help_strlen(struct autobuf *out, size_t len);
EXPORT void cfg_help_choice(struct autobuf *out, bool preamble, const char *(*callback)(size_t idx, const void *ptr),
  size_t choice_count, const void *ptr);
EXPORT void cfg_help_int(struct autobuf *out, int64_t min, int64_t max, uint16_t bytelen, uint16_t fraction);
EXPORT void cfg_help_netaddr(
  struct autobuf *out, bool preamble, bool prefix, const int8_t *af_types, size_t af_types_count);
EXPORT void cfg_help_acl(
  struct autobuf *out, bool preamble, bool prefix, const int8_t *af_types, size_t af_types_count);
EXPORT void cfg_help_bitmap256(struct autobuf *out, bool preamble);
EXPORT void cfg_help_token(struct autobuf *out, bool preamble, const struct cfg_schema_entry *token_entry,
  const struct cfg_schema_entry *sub_entries, size_t entry_count, const struct cfg_schema_token_customizer *customizer);

#endif /* CFG_HELP_H_ */
