
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

#ifndef CFG_SCHEMA_H_
#define CFG_SCHEMA_H_

struct cfg_schema;
struct cfg_schema_section;
struct cfg_schema_entry;

#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <oonf/libcommon/autobuf.h>
#include <oonf/libcommon/avl.h>
#include <oonf/libcommon/netaddr.h>
#include <oonf/libcommon/string.h>

#include <oonf/libconfig/cfg_db.h>

/* macros for creating schema entries */
#if !defined(REMOVE_HELPTEXT)
/**
 * Helper macro to produce a cfg_schema_entry.
 * Help text can be switched off by preprocessor.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define _CFG_VALIDATE(p_name, p_def, p_help, args...)                                                                  \
  { .key.entry = (p_name), .def = { .value = (p_def), .length = sizeof(p_def) }, .help = (p_help), ##args }
#else
#define _CFG_VALIDATE(p_name, p_def, p_help, args...)                                                                  \
  { .key.entry = (p_name), .def = { .value = (p_def), .length = sizeof(p_def) }, ##args }
#endif

/*
 * Example of a section schema definition.
 *
 * All CFG_VALIDATE_xxx macros follow a similar pattern.
 * - the first parameter is the name of the key in the configuration file
 * - the second parameter is the default value (as a string!)
 * - the third parameter is the help text
 *
 * static struct cfg_schema_section section =
 * {
 *     .type = "testsection",
 *     .mode = CFG_SSMODE_NAMED
 * };
 *
 * static struct cfg_schema_entry entries[] = {
 *     CFG_VALIDATE_PRINTABLE("text", "defaulttext", "help for text parameter"),
 *     CFG_VALIDATE_INT32_MINMAX("number", "0", "help for number parameter", 0, 10),
 * };
 */

/**
 * Helper macro to create a cfg_schema_entry for an integer parameter
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param size number of bytes for integer storage
 * @param fraction number of fractional digits
 * @param min minimal allowed parameter value
 * @param max maximum allowed parameter value
 * @param args variable list of additional arguments
 */
#define _CFG_VALIDATE_INT(p_name, p_def, p_help, size, fraction, min, max, args...)                                    \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_int, .cb_valhelp = cfg_schema_help_int,      \
    .validate_param = { { .i64 = (min) }, { .i64 = (max) }, { .u16 = { size, fraction } } }, ##args)

/**
 * Creates a cfg_schema_entry for a parameter that does not need to be validated
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_STRING(p_name, p_def, p_help, args...) _CFG_VALIDATE(p_name, p_def, p_help, ##args)

/**
 * Creates a cfg_schema_entry for a string parameter with a maximum length
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param maxlen maximum number of characters in string
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_STRING_LEN(p_name, p_def, p_help, maxlen, args...)                                                \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_strlen,                                      \
    .cb_valhelp = cfg_schema_help_strlen, .validate_param = { { .s = (maxlen) } }, ##args)

/**
 * Creates a cfg_schema_entry for a string parameter with
 * only printable characters
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_PRINTABLE(p_name, p_def, p_help, args...)                                                         \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_printable,                                   \
    .cb_valhelp = cfg_schema_help_printable, .validate_param = { { .s = INT32_MAX } }, ##args)

/**
 * Creates a cfg_schema_entry for a string parameter with a maximum length
 * and only printable characters
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param maxlen maximum number of characters in string
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_PRINTABLE_LEN(p_name, p_def, p_help, maxlen, args...)                                             \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_printable,                                   \
    .cb_valhelp = cfg_schema_help_printable, .validate_param = { { .s = (maxlen) } }, ##args)

/**
 * Creates a cfg_schema_entry for a parameter that can be choosen
 * from a fixed list defined by a callback.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param p_choice_cb reference a function with a size_t argument and a
 *   const void pointer argument stored in the schema entry
 *   that returns a const char pointer, the choice with the given index.
 *   see cfg_schema_get_choice_value() function as an example
 * @param p_choice_count number of elements in the list
 * @param p_choice_arg argument for choice callback function
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_CHOICE_CB_ARG(p_name, p_def, p_help, p_choice_cb, p_choice_count, p_choice_arg, args...)          \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_choice,                                      \
    .cb_valhelp = cfg_schema_help_choice,                                                                              \
    .validate_param = { { .ptr = (p_choice_cb) }, { .s = (p_choice_count) }, { .ptr = (p_choice_arg) } }, ##args)

/**
 * Creates a cfg_schema_entry for a parameter that can be choosen
 * from a fixed list.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param p_list reference to an array of string pointers with the options
 *   (not a pointer to the array, ARRAYSIZE() would not work)
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_CHOICE(p_name, p_def, p_help, p_list, args...)                                                    \
  CFG_VALIDATE_CHOICE_CB_ARG(p_name, p_def, p_help, cfg_schema_get_choice_value, ARRAYSIZE(p_list), p_list, ##args)

/**
 * Creates a cfg_schema_entry for a parameter that can be choosen
 * from a fixed list defined by a callback.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param p_choice_cb reference a function with a size_t argument and a
 *   const void pointer argument stored in the schema entry
 *   that returns a const char pointer, the choice with the given index.
 *   see cfg_schema_get_choice_value() function as an example
 * @param p_choice_count number of elements in the list
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_CHOICE_CB(p_name, p_def, p_help, p_choice_cb, p_choice_count, args...)                            \
  CFG_VALIDATE_CHOICE_CB_ARG(p_name, p_def, p_help, p_choice_cb, p_choice_count, NULL, ##args)

/**
 * Creates a cfg_schema_entry for a 32 bit signed integer parameter
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param fraction number of fractional digits
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_INT32(p_name, p_def, p_help, fraction, base2, args...)                                            \
  _CFG_VALIDATE_INT(p_name, p_def, p_help, 4, fraction, INT32_MIN, INT32_MAX, ##args)

/**
 * Creates a cfg_schema_entry for a 64 bit signed integer parameter
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param fraction number of fractional digits
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_INT64(p_name, p_def, p_help, fraction, base2, args...)                                            \
  _CFG_VALIDATE_INT(p_name, p_def, p_help, 8, fraction, INT64_MIN, INT64_MAX, ##args)

/**
 * Creates a cfg_schema_entry for a 32 bit signed integer parameter
 * with a fixed range of possible values
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param fraction number of fractional digits
 * @param min minimal allowed value
 * @param max maximal allowed value
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_INT32_MINMAX(p_name, p_def, p_help, fraction, base2, min, max, args...)                           \
  _CFG_VALIDATE_INT(p_name, p_def, p_help, 4, fraction, min, max, ##args)

/**
 * Creates a cfg_schema_entry for a 64 bit signed integer parameter
 * with a fixed range of possible values
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param fraction number of fractional digits
 * @param min minimal allowed value
 * @param max maximal allowed value
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_INT64_MINMAX(p_name, p_def, p_help, fraction, base2, min, max, args...)                           \
  _CFG_VALIDATE_INT(p_name, p_def, p_help, 8, fraction, min, max, ##args)

/**
 * Creates a cfg_schema_entry for a network address of any type
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_NETADDR(p_name, p_def, p_help, prefix, unspec, args...)                                           \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_netaddr,                                     \
    .cb_valhelp = cfg_schema_help_netaddr,                                                                             \
    .validate_param = { { .i8 = { AF_MAC48, AF_EUI64, AF_INET, AF_INET6, !!(unspec) ? AF_UNSPEC : -1 } },              \
      { .b = !!(prefix) } },                                                                                           \
    ##args)

/**
 * Creates a cfg_schema_entry for a layer-2 network address,
 * either MAC or EUI-64
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_NETADDR_HWADDR(p_name, p_def, p_help, prefix, unspec, args...)                                    \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_netaddr,                                     \
    .cb_valhelp = cfg_schema_help_netaddr,                                                                             \
    .validate_param = { { .i8 = { AF_MAC48, AF_EUI64, -1, -1, !!(unspec) ? AF_UNSPEC : -1 } }, { .b = !!(prefix) } },  \
    ##args)

/**
 * Creates a cfg_schema_entry for a MAC (ethernet) network address
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_NETADDR_MAC48(p_name, p_def, p_help, prefix, unspec, args...)                                     \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_netaddr,                                     \
    .cb_valhelp = cfg_schema_help_netaddr,                                                                             \
    .validate_param = { { .i8 = { AF_MAC48, -1, -1, -1, !!(unspec) ? AF_UNSPEC : -1 } }, { .b = !!(prefix) } },        \
    ##args)

/**
 * Creates a cfg_schema_entry for an EUI-64 network address
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_NETADDR_EUI64(p_name, p_def, p_help, prefix, unspec, args...)                                     \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_netaddr,                                     \
    .cb_valhelp = cfg_schema_help_netaddr,                                                                             \
    .validate_param = { { .i8 = { AF_EUI64, -1, -1, -1, !!(unspec) ? AF_UNSPEC : -1 } }, { .b = !!(prefix) } },        \
    ##args)

/**
 * Creates a cfg_schema_entry for an IPv4 network address
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_NETADDR_V4(p_name, p_def, p_help, prefix, unspec, args...)                                        \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_netaddr,                                     \
    .cb_valhelp = cfg_schema_help_netaddr,                                                                             \
    .validate_param = { { .i8 = { AF_INET, -1, -1, -1, !!(unspec) ? AF_UNSPEC : -1 } }, { .b = !!(prefix) } }, ##args)

/**
 * Creates a cfg_schema_entry for an IPv6 network address
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_NETADDR_V6(p_name, p_def, p_help, prefix, unspec, args...)                                        \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_netaddr,                                     \
    .cb_valhelp = cfg_schema_help_netaddr,                                                                             \
    .validate_param = { { .i8 = { AF_INET6, -1, -1, -1, !!(unspec) ? AF_UNSPEC : -1 } }, { .b = !!(prefix) } },        \
    ##args)

/**
 * Creates a cfg_schema_entry for an IP network address
 * (either IPv4 or IPv6)
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_NETADDR_V46(p_name, p_def, p_help, prefix, unspec, args...)                                       \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_netaddr,                                     \
    .cb_valhelp = cfg_schema_help_netaddr,                                                                             \
    .validate_param = { { .i8 = { AF_INET, AF_INET6, -1, -1, !!(unspec) ? AF_UNSPEC : -1 } }, { .b = !!(prefix) } },   \
    ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_ACL(p_name, p_def, p_help, args...)                                                               \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_acl, .cb_valhelp = cfg_schema_help_acl,      \
    .list = true, .validate_param = { { .i8 = { AF_MAC48, AF_EUI64, AF_INET, AF_INET6, -1 } }, { .b = true } },        \
    ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for MAC (ethernet) or EUI-64 addresses.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_ACL_HWADDR(p_name, p_def, p_help, args...)                                                        \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_acl, .cb_valhelp = cfg_schema_help_acl,      \
    .list = true, .validate_param = { { .i8 = { AF_MAC48, AF_EUI64, -1, -1, -1 } }, { .b = true } }, ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for MAC (ethernet) addresses.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_ACL_MAC48(p_name, p_def, p_help, args...)                                                         \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_acl, .cb_valhelp = cfg_schema_help_acl,      \
    .list = true, .validate_param = { { .i8 = { AF_MAC48, -1, -1, -1, -1 } }, { .b = true } }, ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for EUI-64 addresses.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_ACL_EUI64(p_name, p_def, p_help, args...)                                                         \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_acl, .cb_valhelp = cfg_schema_help_acl,      \
    .list = true, .validate_param = { { .i8 = { AF_EUI64, -1, -1, -1, -1 } }, { .b = true } }, ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for IPv4 addresses.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_ACL_V4(p_name, p_def, p_help, args...)                                                            \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_acl, .cb_valhelp = cfg_schema_help_acl,      \
    .list = true, .validate_param = { { .i8 = { AF_INET, -1, -1, -1, -1 } }, { .b = true } }, ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for IPv6 addresses.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_ACL_V6(p_name, p_def, p_help, args...)                                                            \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_acl, .cb_valhelp = cfg_schema_help_acl,      \
    .list = true, .validate_param = { { .i8 = { AF_INET6, -1, -1, -1, -1 } }, { .b = true } }, ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for IP (either IPv4 or IPv6) addresses.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_ACL_V46(p_name, p_def, p_help, args...)                                                           \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_acl, .cb_valhelp = cfg_schema_help_acl,      \
    .list = true, .validate_param = { { .i8 = { AF_INET, AF_INET6, -1, -1, -1 } }, { .b = true } }, ##args)

/**
 * Creates a cfg_schema_entry for 256 bit bitmap.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_BITMAP256(p_name, p_def, p_help, args...)                                                         \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_bitmap256,                                   \
    .cb_valhelp = cfg_schema_help_bitmap256, .list = true, ##args)

/**
 * Creates a cfg_schema_entry for a boolean parameter
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_BOOL(p_name, p_def, p_help, args...)                                                              \
  CFG_VALIDATE_CHOICE(p_name, p_def, p_help, CFGLIST_BOOL, ##args)

/**
 * Creates a cfg_schema_entry for a list of parameters, split by spaces
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param schema_entries array of schema entries for tokens
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_TOKENS(p_name, p_def, p_help, schema_entries, args...)                                            \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_tokens, .cb_valhelp = cfg_schema_help_token, \
    .validate_param = { { .ptr = (schema_entries) }, { .s = ARRAYSIZE(schema_entries) } }, ##args)

/**
 * Creates a cfg_schema_entry for a list of parameters, split by spaces.
 * Contains a pointer to a customizer for validation, binary conversion
 * and help output.
 * @param p_help help text for configuration entry
 * @param schema_entries array of schema entries for tokens
 * @param custom pointer to customizer for tokens
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_TOKENS_CUSTOM(p_name, p_def, p_help, schema_entries, custom, args...)                             \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = cfg_schema_validate_tokens, .cb_valhelp = cfg_schema_help_token, \
    .validate_param = { { .ptr = (schema_entries) }, { .s = ARRAYSIZE(schema_entries) }, { .ptr = &(custom) } },       \
    ##args)

/*
 * Example of a section schema definition with binary mapping
 *
 * All CFG_MAP_xxx macros follow a similar pattern.
 * - the first parameter is the name of the struct the data will be mapped into
 * - the second parameter is the name of the field the data will be mapped into
 * - the third parameter is the name of the key in the configuration file
 * - the fourth parameter is the default value (as a string!)
 * - the fifth parameter is the help text
 *
 * struct bin_data {
 *   char *string;
 *   int int_value;
 * };
 *
 * static struct cfg_schema_section section =
 * {
 *     .type = "testsection",
 *     .mode = CFG_SSMODE_NAMED
 * };
 *
 * static struct cfg_schema_entry entries[] = {
 *     CFG_MAP_PRINTABLE(bin_data, string, "text", "defaulttext", "help for text parameter"),
 *     CFG_MAP_INT_MINMAX(bin_data, int_value, "number", "0", "help for number parameter", 0, 10),
 * };
 */

/**
 * Helper macro that calculates the size in bytes of a struct field
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 */
#define calculate_size(p_reference, p_field) sizeof(((struct p_reference *)0)->p_field)

/**
 * Helper macro to create a cfg_schema_entry for an integer parameter
 * that can be mapped into a binary struct.
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be an interger
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param size number of bytes for integer storage
 * @param fraction number of fractional digits
 * @param base2 true if iso prefixes should use factor 1024
 * @param min minimal allowed parameter value
 * @param max maximum allowed parameter value
 * @param args variable list of additional arguments
 */
#define _CFG_MAP_INT(p_reference, p_field, p_name, p_def, p_help, size, fraction, min, max, args...)                   \
  _CFG_VALIDATE_INT(p_name, p_def, p_help, size, fraction, min, max, .cb_to_binary = cfg_schema_tobin_int,             \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a parameter that does not need to be validated
 * and can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a string pointer
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_STRING(p_reference, p_field, p_name, p_def, p_help, args...)                                           \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_strptr,                                        \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a string parameter with a maximum length
 * and can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a string pointer
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param maxlen maximum number of characters in string
 * @param args variable list of additional arguments
 */
#define CFG_MAP_STRING_LEN(p_reference, p_field, p_name, p_def, p_help, maxlen, args...)                               \
  CFG_VALIDATE_STRING_LEN(p_name, p_def, p_help, maxlen, .cb_to_binary = cfg_schema_tobin_strptr,                      \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a string parameter with a maximum length
 * and can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a string array
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param maxlen maximum number of characters in string
 * @param args variable list of additional arguments
 */
#define CFG_MAP_STRING_ARRAY(p_reference, p_field, p_name, p_def, p_help, maxlen, args...)                             \
  CFG_VALIDATE_STRING_LEN(p_name, p_def, p_help, maxlen, .cb_to_binary = cfg_schema_tobin_strarray,                    \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a string parameter with
 * only printable characters and can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a string pointer
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_PRINTABLE(p_reference, p_field, p_name, p_def, p_help, args...)                                        \
  CFG_VALIDATE_PRINTABLE(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_strptr,                               \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a string parameter with a maximum length,
 * only printable characters and can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a string pointer
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param maxlen maximum number of characters in string
 * @param args variable list of additional arguments
 */
#define CFG_MAP_PRINTABLE_LEN(p_reference, p_field, p_name, p_def, p_help, maxlen, args...)                            \
  CFG_VALIDATE_PRINTABLE_LEN(p_name, p_def, p_help, maxlen, .cb_to_binary = cfg_schema_tobin_strptr,                   \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a string parameter with a maximum length,
 * only printable characters and can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a string array
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param maxlen maximum number of characters in string
 * @param args variable list of additional arguments
 */
#define CFG_MAP_PRINTABLE_ARRAY(p_reference, p_field, p_name, p_def, p_help, maxlen, args...)                          \
  CFG_VALIDATE_PRINTABLE_LEN(p_name, p_def, p_help, maxlen, .cb_to_binary = cfg_schema_tobin_strarray,                 \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a parameter that can be chosen
 * from a fixed list defined by a callback.
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param p_choice_cb reference a function with a size_t argument and a
 *   const void pointer argument stored in the schema entry
 *   that returns a const char pointer, the choice with the given index.
 *   see cfg_schema_get_choice_value() function as an example
 * @param p_choice_count number of elements in the list
 * @param p_choice_arg argument for choice callback function
 * @param args variable list of additional arguments
 */
#define CFG_MAP_CHOICE_CB_ARG(                                                                                         \
  p_reference, p_field, p_name, p_def, p_help, p_choice_cb, p_choice_count, p_choice_arg, args...)                     \
  CFG_VALIDATE_CHOICE_CB_ARG(p_name, p_def, p_help, p_choice_cb, p_choice_count, p_choice_arg,                         \
    .cb_to_binary = cfg_schema_tobin_choice, .bin_size = calculate_size(p_reference, p_field),                         \
    .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a parameter that can be choosen
 * from a fixed list.
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be an integer for the index of the selected choice
 * @param p_name parameter name
 * from a fixed list and can be mapped into a binary struct
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param p_list reference to an array of string pointers with the options
 *   (not a pointer to the array, ARRAYSIZE() would not work)
 * @param args variable list of additional arguments
 */
#define CFG_MAP_CHOICE_CB(p_reference, p_field, p_name, p_def, p_help, p_choice_cb, p_choice_count, args...)           \
  CFG_MAP_CHOICE_CB_ARG(p_reference, p_field, p_name, p_def, p_help, p_choice_cb, p_choice_count, NULL, ##args)

/**
 * Creates a cfg_schema_entry for a parameter that can be choosen
 * from a fixed list.
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be an integer for the index of the selected choice
 * @param p_name parameter name
 * from a fixed list and can be mapped into a binary struct
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param p_list reference to an array of string pointers with the options
 *   (not a pointer to the array, ARRAYSIZE() would not work)
 * @param args variable list of additional arguments
 */
#define CFG_MAP_CHOICE(p_reference, p_field, p_name, p_def, p_help, p_list, args...)                                   \
  CFG_MAP_CHOICE_CB_ARG(                                                                                               \
    p_reference, p_field, p_name, p_def, p_help, cfg_schema_get_choice_value, ARRAYSIZE(p_list), p_list, ##args)

/**
 * Creates a cfg_schema_entry for a 32 bit signed integer parameter
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a 32 bit integer
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param fraction number of fractional digits
 * @param args variable list of additional arguments
 */
#define CFG_MAP_INT32(p_reference, p_field, p_name, p_def, p_help, fraction, args...)                                  \
  _CFG_MAP_INT(p_reference, p_field, p_name, p_def, p_help, 4, fraction, INT32_MIN, INT32_MAX, ##args)

/**
 * Creates a cfg_schema_entry for a 64 bit signed integer parameter
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a 64 bit integer
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param fraction number of fractional digits
 * @param args variable list of additional arguments
 */
#define CFG_MAP_INT64(p_reference, p_field, p_name, p_def, p_help, fraction, args...)                                  \
  _CFG_MAP_INT(p_reference, p_field, p_name, p_def, p_help, 8, fraction, INT64_MIN, INT64_MAX, ##args)

/**
 * Creates a cfg_schema_entry for a 32 bit signed integer parameter
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a 32 bit integer
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param fraction number of fractional digits
 * @param min minimal allowed value
 * @param max maximal allowed value
 * @param args variable list of additional arguments
 */
#define CFG_MAP_INT32_MINMAX(p_reference, p_field, p_name, p_def, p_help, fraction, min, max, args...)                 \
  _CFG_MAP_INT(p_reference, p_field, p_name, p_def, p_help, 4, fraction, min, max, ##args)

/**
 * Creates a cfg_schema_entry for a 64 bit signed integer parameter
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a 64 bit integer
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param fraction number of fractional digits
 * @param min minimal allowed value
 * @param max maximal allowed value
 * @param args variable list of additional arguments
 */
#define CFG_MAP_INT64_MINMAX(p_reference, p_field, p_name, p_def, p_help, fraction, min, max, args...)                 \
  _CFG_MAP_INT(p_reference, p_field, p_name, p_def, p_help, 8, fraction, min, max, ##args)

/**
 * Creates a cfg_schema_entry for a network address of any type
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_MAP_NETADDR(p_reference, p_field, p_name, p_def, p_help, prefix, unspec, args...)                          \
  CFG_VALIDATE_NETADDR(p_name, p_def, p_help, prefix, unspec, .cb_to_binary = cfg_schema_tobin_netaddr,                \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a layer-2 network address,
 * either MAC or EUI-64 that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_MAP_NETADDR_HWADDR(p_reference, p_field, p_name, p_def, p_help, prefix, unspec, args...)                   \
  CFG_VALIDATE_NETADDR_HWADDR(p_name, p_def, p_help, prefix, unspec, .cb_to_binary = cfg_schema_tobin_netaddr,         \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a MAC (ethernet) network address
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_MAP_NETADDR_MAC48(p_reference, p_field, p_name, p_def, p_help, prefix, unspec, args...)                    \
  CFG_VALIDATE_NETADDR_MAC48(p_name, p_def, p_help, prefix, unspec, .cb_to_binary = cfg_schema_tobin_netaddr,          \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for an EUI-64 network address
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_MAP_NETADDR_EUI64(p_reference, p_field, p_name, p_def, p_help, prefix, unspec, args...)                    \
  CFG_VALIDATE_NETADDR_EUI64(p_name, p_def, p_help, prefix, unspec, .cb_to_binary = cfg_schema_tobin_netaddr,          \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for an IPv4 network address
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_MAP_NETADDR_V4(p_reference, p_field, p_name, p_def, p_help, prefix, unspec, args...)                       \
  CFG_VALIDATE_NETADDR_V4(p_name, p_def, p_help, prefix, unspec, .cb_to_binary = cfg_schema_tobin_netaddr,             \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for an IPv6 network address
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_MAP_NETADDR_V6(p_reference, p_field, p_name, p_def, p_help, prefix, unspec, args...)                       \
  CFG_VALIDATE_NETADDR_V6(p_name, p_def, p_help, prefix, unspec, .cb_to_binary = cfg_schema_tobin_netaddr,             \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for an IP network address
 * (either IPv4 or IPv6) that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param prefix true if the parameter also allows prefixes
 * @param unspec true if the parameter also allows an unspecified address
 * @param args variable list of additional arguments
 */
#define CFG_MAP_NETADDR_V46(p_reference, p_field, p_name, p_def, p_help, prefix, unspec, args...)                      \
  CFG_VALIDATE_NETADDR_V46(p_name, p_def, p_help, prefix, unspec, .cb_to_binary = cfg_schema_tobin_netaddr,            \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr_acl struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_ACL(p_reference, p_field, p_name, p_def, p_help, args...)                                              \
  CFG_VALIDATE_ACL(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_acl,                                        \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for MAC (ethernet) or EUI-64 addresses
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr_acl struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_ACL_HWADDR(p_reference, p_field, p_name, p_def, p_help, args...)                                       \
  CFG_VALIDATE_ACL_HWADDR(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_acl,                                 \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for MAC (ethernet) addresses
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr_acl struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_ACL_MAC48(p_reference, p_field, p_name, p_def, p_help, args...)                                        \
  CFG_VALIDATE_ACL_MAC48(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_acl,                                  \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for EUI-64 addresses
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr_acl struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_ACL_EUI64(p_reference, p_field, p_name, p_def, p_help, args...)                                        \
  CFG_VALIDATE_ACL_EUI64(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_acl,                                  \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for IPv4 addresses
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr_acl struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_ACL_V4(p_reference, p_field, p_name, p_def, p_help, args...)                                           \
  CFG_VALIDATE_ACL_V4(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_acl,                                     \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for IPv6 addresses
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr_acl struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_ACL_V6(p_reference, p_field, p_name, p_def, p_help, args...)                                           \
  CFG_VALIDATE_ACL_V6(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_acl,                                     \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a network address based
 * access control list for IP (either IPv4 or IPv6) addresses
 * that can be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a netaddr_acl struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_ACL_V46(p_reference, p_field, p_name, p_def, p_help, args...)                                          \
  CFG_VALIDATE_ACL_V46(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_acl,                                    \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for 256 bit bitmap that can be
 * mapped into a binary struct.
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a bitmap256 struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_BITMAP256(p_reference, p_field, p_name, p_def, p_help, args...)                                        \
  CFG_VALIDATE_BITMAP256(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_bitmap256,                            \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for a boolean parameter
 * that can be mapped into a binary struct.
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a bool
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_BOOL(p_reference, p_field, p_name, p_def, p_help, args...)                                             \
  CFG_VALIDATE_BOOL(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_bool,                                      \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field), ##args)

/**
 * Creates a cfg_schema_entry for list of strings that can
 * be mapped into a binary struct
 * @param p_reference reference to instance of struct
 * @param p_field name of field in the struct for the parameter,
 *   it must be a strarray struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_MAP_STRINGLIST(p_reference, p_field, p_name, p_def, p_help, args...)                                       \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_to_binary = cfg_schema_tobin_stringlist,                                    \
    .bin_size = calculate_size(p_reference, p_field), .bin_offset = offsetof(struct p_reference, p_field),             \
    .list = true, ##args)

/**
 * Creates a cfg_schema_entry for a list of parameters, split by spaces
 * that can be mapped into a binary struct
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param schema_entries array of schema entries for tokens
 * @param args variable list of additional arguments
 */
#define CFG_MAP_TOKENS(p_name, p_def, p_help, schema_entries, args...)                                                 \
  CFG_VALIDATE_TOKENS(p_name, p_def, p_help, schema_entries, .cb_to_binary = cfg_schema_tobin_tokens, ##args)
/**
 * Creates a cfg_schema_entry for a list of parameters, split by spaces
 * that can be mapped into a binary struct. Contains a pointer to
 * a customizer for validation, binary conversion and help output.
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param schema_entries array of schema entries for tokens
 * @param custom pointer to customizer for tokens
 * @param args variable list of additional arguments
 */
#define CFG_MAP_TOKENS_CUSTOM(p_name, p_def, p_help, schema_entries, custom, args...)                                  \
  CFG_VALIDATE_TOKENS(p_name, p_def, p_help, schema_entries, custom, .cb_to_binary = cfg_schema_tobin_tokens, ##args)

/*! convenience definition for configuration that only allows access from loopback */
#define ACL_LOCALHOST_ONLY "+127.0.0.1/8\0+::1/128\0" ACL_DEFAULT_REJECT

/**
 * Definition of a configuration schema
 */
struct cfg_schema {
  /*! tree of sections of this schema */
  struct avl_tree sections;

  /*! tree of schema entries of this schema */
  struct avl_tree entries;

  /*! list of delta handlers of this schema */
  struct list_entity handlers;
};

/**
 * Possible modes for a schema section
 */
enum cfg_schema_section_mode
{
  /**
   * normal unnamed section, delta handlers will be triggered at
   * startup even it does not exist in the configuration file
   *
   * default setting
   */
  CFG_SSMODE_UNNAMED = 0, //!< CFG_SSMODE_UNNAMED

  /**
   * named section, delta handlers will always trigger for this
   */
  CFG_SSMODE_NAMED, //!< CFG_SSMODE_NAMED

  /**
   * named section, configuration demands at least one existing
   * section of this type to be valid.
   */
  CFG_SSMODE_NAMED_MANDATORY, //!< CFG_SSMODE_NAMED_MANDATORY

  /**
   * named section, if none exists the configuration will create
   * a temporary (and empty) section with the defined default name.
   */
  CFG_SSMODE_NAMED_WITH_DEFAULT, //!< CFG_SSMODE_NAMED_WITH_DEFAULT

  /*! number of configuration section modes */
  CFG_SSMODE_MAX, //!< CFG_SSMODE_MAX
};

/**
 * Represents the schema of all named sections within
 * a certain type
 */
struct cfg_schema_section {
  /*! node for global section tree, initialized by schema_add() */
  struct avl_node _section_node;

  /*! name of section type, key for section_node */
  const char *type;

  /*! name of default section if mode is CFG_SSMODE_NAMED_WITH_DEFAULT */
  const char *def_name;

  /*! mode of this section, see above */
  enum cfg_schema_section_mode mode;

  /*! help text for section */
  const char *help;

  /**
   * callback for checking configuration of section
   * @param section_name name of the configuration section including type
   * @param named configuration section to validate
   * @param log buffer for text output of validator
   * @return -1 if section was invalid, 0 otherwise
   */
  int (*cb_validate)(const char *section_name, struct cfg_named_section *named, struct autobuf *log);

  /*! node for global delta handler tree, initialized by delta_add() */
  struct list_entity _delta_node;

  /**
   * callback for delta handling, NULL if not interested
   */
  void (*cb_delta_handler)(void);

  /**
   * pointer to former version of changed section, only valid
   * during call of cb_delta_handler
   */
  struct cfg_named_section *pre;

  /**
   * pointer to later version of changed section, only valid
   * during call of cb_delta_handler
   */
  struct cfg_named_section *post;

  /**
   * Name of configured section (or NULL if unnamed section), only valid
   * during call of cb_delta_handler
   */
  const char *section_name;

  /*! array of entries in section */
  struct cfg_schema_entry *entries;

  /*! number of entries in section */
  size_t entry_count;

  /*! pointer to next section for subsystem initialization */
  struct cfg_schema_section *next_section;
};

/**
 * Key (used for avl) of a schema entry
 */
struct cfg_schema_entry_key {
  /*! section type of entry */
  const char *type;

  /*! key of entry */
  const char *entry;
};

/**
 * Represents the schema of a configuration entry
 */
struct cfg_schema_entry {
  /*! node for global section tree */
  struct avl_node _node;

  /*! pointer to schema section */
  struct cfg_schema_section *_parent;

  /*! name of entry */
  struct cfg_schema_entry_key key;

  /*! default value */
  struct const_strarray def;

  /*! help text for entry */
  const char *help;

  /*! value is a list of parameters instead of a single one */
  bool list;

  /**
   * callback for checking value and giving help for an entry
   * @param entry pointer to schema entry
   * @param section_name name of section including type
   * @param value value of entry
   * @param out buffer for text output of validation
   * @return -1 if entry was invalid, 0 otherwise
   */
  int (*cb_validate)(
    const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);

  /**
   * Outputs a help string for this schema entry
   * @param entry pointer to schema entry
   * @param out buffer for help text
   */
  void (*cb_valhelp)(const struct cfg_schema_entry *entry, struct autobuf *out);

  /*! parameters for validator functions */
  union {
    int8_t i8[8];
    uint8_t u8[8];
    int16_t i16[4];
    uint16_t u16[4];
    int32_t i32[2];
    uint32_t u32[2];
    int64_t i64;
    uint64_t u64;
    bool b;
    size_t s;
    void *ptr;
  } validate_param[3];

  /**
   * callback for converting the text value of an entry into binary
   * @param s_entry pointer to schema entry
   * @param value text value of the entry
   * @param ptr pointer to binary data structure to put data into
   * @return -1 if an error happened, 0 otherwise
   */
  int (*cb_to_binary)(const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *ptr);

  /*! offset of current binary data compared to reference pointer */
  size_t bin_offset;

  /*! size in bytes of the target buffer to allow sanity checks */
  size_t bin_size;

  /*! pointer to value before change (for delta calculation) */
  const struct const_strarray *pre;

  /*! pointer to value after change (for delta calculation) */
  const struct const_strarray *post;

  /*! true if the value changed (for delta calculation) */
  bool delta_changed;
};

/**
 * Callback table to customize the behavior of a TOKEN configuration
 * entry, which might be necessary if the tokens depend on each other
 * for validation and conversion to binary.
 */
struct cfg_schema_token_customizer {
  /**
   * Additional validator, will be called after all schema entries have
   * been successfully validated with their own validator.
   * @param out buffer for text output of validation
   * @param section_name name of configuration section
   * @param entry_name name of configuration entry
   * @param value complete token string defined by the user
   * @param entries pointer to array of schema entries for token
   * @param entry_count number of schema entries for token
   * @return -1 if validation failed, 0 otherwise
   */
  int (*cb_validator)(struct autobuf *out, const char *section_name, const char *entry_name, const char *value,
    struct cfg_schema_entry *entries, size_t entry_count);

  /**
   * Additional binary converter for TOKENs
   * @param entries pointer to array of schema entries for token
   * @param entry_count number of schema entries for token
   * @param value string array of values specified by the user, use only first
   *   string if converter does not need multiple entries
   * @param ptr pointer to the beginning of the data structure used for
   *   placing the content of the tokens
   * @return -1 if an error happened, 0 otherwise
   */
  int (*cb_tobin)(struct cfg_schema_entry *entries, size_t entry_count, const char *value, void *ptr);

  /**
   * Additional help output generator for TOKENs
   * @param entry token schema entry
   * @param out buffer for help text output
   */
  void (*cb_valhelp)(const struct cfg_schema_entry *entry, struct autobuf *out);

  /*! number of optional arguments at the end of the chain of tokens */
  uint32_t optional;
};

/*! List of strings that are considered valid boolean options */
#define CFGLIST_BOOL_VALUES "true", "1", "on", "yes", "false", "0", "off", "no"

/*! List of string that are boolean options with value true */
#define CFGLIST_BOOL_TRUE_VALUES "true", "1", "on", "yes"

EXPORT extern const char *CFGLIST_BOOL_TRUE[4];
EXPORT extern const char *CFGLIST_BOOL[8];
EXPORT extern const char *CFG_SCHEMA_SECTIONMODE[CFG_SSMODE_MAX];

EXPORT void cfg_schema_add(struct cfg_schema *schema);

EXPORT void cfg_schema_add_section(struct cfg_schema *schema, struct cfg_schema_section *section);
EXPORT void cfg_schema_remove_section(struct cfg_schema *schema, struct cfg_schema_section *section);

EXPORT int cfg_schema_validate(struct cfg_db *db, bool cleanup, bool ignore_unknown_elements, struct autobuf *out);

EXPORT int cfg_schema_tobin(
  void *target, struct cfg_named_section *named, const struct cfg_schema_entry *entries, size_t count);

EXPORT int cfg_schema_handle_db_changes(struct cfg_db *pre_change, struct cfg_db *post_change);
EXPORT int cfg_schema_handle_db_startup_changes(struct cfg_db *db);

EXPORT int cfg_avlcmp_schemaentries(const void *p1, const void *p2);

EXPORT const char *cfg_schema_get_choice_value(size_t idx, const void *ptr);

EXPORT int cfg_schema_validate_printable(
  const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);
EXPORT int cfg_schema_validate_strlen(
  const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);
EXPORT int cfg_schema_validate_choice(
  const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);
EXPORT int cfg_schema_validate_int(
  const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);
EXPORT int cfg_schema_validate_netaddr(
  const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);
EXPORT int cfg_schema_validate_acl(
  const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);
EXPORT int cfg_schema_validate_bitmap256(
  const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);
EXPORT int cfg_schema_validate_tokens(
  const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);

EXPORT void cfg_schema_help_printable(const struct cfg_schema_entry *entry, struct autobuf *out);
EXPORT void cfg_schema_help_strlen(const struct cfg_schema_entry *entry, struct autobuf *out);
EXPORT void cfg_schema_help_choice(const struct cfg_schema_entry *entry, struct autobuf *out);
EXPORT void cfg_schema_help_int(const struct cfg_schema_entry *entry, struct autobuf *out);
EXPORT void cfg_schema_help_netaddr(const struct cfg_schema_entry *entry, struct autobuf *out);
EXPORT void cfg_schema_help_acl(const struct cfg_schema_entry *entry, struct autobuf *out);
EXPORT void cfg_schema_help_bitmap256(const struct cfg_schema_entry *entry, struct autobuf *out);
EXPORT void cfg_schema_help_token(const struct cfg_schema_entry *entry, struct autobuf *out);

EXPORT int cfg_schema_tobin_strptr(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);
EXPORT int cfg_schema_tobin_strarray(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);
EXPORT int cfg_schema_tobin_choice(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);
EXPORT int cfg_schema_tobin_int(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);
EXPORT int cfg_schema_tobin_netaddr(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);
EXPORT int cfg_schema_tobin_bool(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);
EXPORT int cfg_schema_tobin_stringlist(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);
EXPORT int cfg_schema_tobin_acl(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);
EXPORT int cfg_schema_tobin_bitmap256(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);
EXPORT int cfg_schema_tobin_tokens(
  const struct cfg_schema_entry *s_entry, const struct const_strarray *value, void *reference);

/**
 * Finds a section in a schema
 * @param schema pointer to schema
 * @param type type of section
 * @return pointer to section, NULL if not found
 */
static INLINE struct cfg_schema_section *
cfg_schema_find_section(struct cfg_schema *schema, const char *type) {
  struct cfg_schema_section *section;

  return avl_find_element(&schema->sections, type, section, _section_node);
}

/**
 * Finds an entry in a schema section
 * @param section pointer to section
 * @param name name of entry
 * @return pointer of entry, NULL if not found
 */
static INLINE struct cfg_schema_entry *
cfg_schema_find_section_entry(struct cfg_schema_section *section, const char *name) {
  size_t i;

  for (i = 0; i < section->entry_count; i++) {
    if (strcmp(section->entries[i].key.entry, name) == 0) {
      return &section->entries[i];
    }
  }
  return NULL;
}

#endif /* CFG_SCHEMA_H_ */
