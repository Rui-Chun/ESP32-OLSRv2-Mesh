
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

#ifndef CFG_H_
#define CFG_H_

/* forward declaration */
struct cfg_instance;

#include <ctype.h>

#include <oonf/libcommon/autobuf.h>
#include <oonf/libcommon/avl.h>
#include <oonf/oonf.h>

#include <oonf/libconfig/cfg_cmd.h>
#include <oonf/libconfig/cfg_io.h>
#include <oonf/libconfig/cfg_schema.h>

/*! Separator for configuration URL */
#define CFG_IO_URL_SPLITTER "://"

/**
 * Instance of a configuration handler with default io handler
 */
struct cfg_instance {
  /*! tree of io handlers and format parsers */
  struct avl_tree io_tree;

  /*! name of default io file handler */
  const char *default_io;
};

EXPORT void cfg_add(struct cfg_instance *);
EXPORT void cfg_remove(struct cfg_instance *);

EXPORT int cfg_append_printable_line(struct autobuf *autobuf, const char *fmt, ...)
  __attribute__((format(printf, 2, 3)));
EXPORT bool cfg_is_allowed_key(const char *key);
EXPORT const char *cfg_get_choice_array_value(size_t idx, const void *ptr);
EXPORT int cfg_get_choice_index(
  const char *key, const char *(*callback)(size_t idx, const void *ptr), size_t choices_count, const void *ptr);

EXPORT int cfg_avlcmp_keys(const void *p1, const void *p2);

EXPORT void cfg_set_ifname_handler(int (*get_phy_if)(char *phy_ifname, const char *ifname));
EXPORT const char *cfg_get_phy_if(char *phy_if, const char *ifname);

/**
 * Compares to keys/names of two section types/names or entry names.
 * A NULL pointer is considered larger than any valid string.
 * @param str1 first key
 * @param str2 second key
 * @return similar to strcmp()
 */
static INLINE int
cfg_cmp_keys(const char *str1, const char *str2) {
  return cfg_avlcmp_keys(str1, str2);
}

/**
 * Checks if a string value represents a positive boolean value
 * @param value string representation of boolean
 * @return boolean value of the string representation
 */
static INLINE bool
cfg_get_bool(const char *value) {
  return cfg_get_choice_index(value, cfg_get_choice_array_value, ARRAYSIZE(CFGLIST_BOOL_TRUE), CFGLIST_BOOL_TRUE) >= 0;
}

/**
 * Checks if a string value represents a boolean value
 * @param value string value that might contain a boolean text representation
 * @return true if string is a boolean, false otherwise
 */
static INLINE bool
cfg_is_bool(const char *value) {
  return cfg_get_choice_index(value, cfg_get_choice_array_value, ARRAYSIZE(CFGLIST_BOOL), CFGLIST_BOOL) >= 0;
}

/**
 * Checks if a section name is valid
 * @param name name of a configuration section
 * @return true if name contains no space, false otherwise
 */
static INLINE bool
cfg_is_allowed_section_name(const char *name) {
  return strchr(name, ' ') == NULL;
}

#endif /* CFG_H_ */
