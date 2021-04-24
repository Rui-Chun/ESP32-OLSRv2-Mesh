/*
 * cfg_tobin.h
 *
 *  Created on: 05.10.2017
 *      Author: rogge
 */

#ifndef CFG_TOBIN_H_
#define CFG_TOBIN_H_

#include <oonf/oonf.h>
#include <oonf/libcommon/string.h>

EXPORT int cfg_tobin_strptr(void *reference, size_t bin_size, const struct const_strarray *value);
EXPORT int cfg_tobin_strarray(void *reference, size_t bin_size, const struct const_strarray *value, size_t array_size);
EXPORT int cfg_tobin_choice(void *reference, size_t bin_size, const struct const_strarray *value,
  const char *(*callback)(size_t idx, const void *ptr), size_t choices_count, const void *ptr);
EXPORT int cfg_tobin_int(
  void *reference, size_t bin_size, const struct const_strarray *value, uint16_t fractions, size_t int_size);
EXPORT int cfg_tobin_netaddr(void *reference, size_t bin_size, const struct const_strarray *value);
EXPORT int cfg_tobin_acl(void *reference, size_t bin_size, const struct const_strarray *value);
EXPORT int cfg_tobin_bitmap256(void *reference, size_t bin_size, const struct const_strarray *value);
EXPORT int cfg_tobin_bool(void *reference, size_t bin_size, const struct const_strarray *value);
EXPORT int cfg_tobin_stringlist(void *reference, size_t bin_size, const struct const_strarray *value);
EXPORT int cfg_tobin_tokens(void *reference, const char *value, struct cfg_schema_entry *entries, size_t entry_count,
  struct cfg_schema_token_customizer *custom);

#endif /* CFG_TOBIN_H_ */
