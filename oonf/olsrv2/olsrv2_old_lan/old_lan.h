/*
 * old_lan.h
 *
 *  Created on: 18.10.2017
 *      Author: rogge
 */

#ifndef _OLD_LAN_H_
#define _OLD_LAN_H_

#include <oonf/libconfig/cfg_schema.h>

#define OONF_OLD_LAN_SUBSYSTEM "olsrv2_old_lan"

/**
 * Creates a cfg_schema_entry for a locally attached network
 * @param p_name parameter name
 * @param p_def parameter default value
 * @param p_help help text for configuration entry
 * @param args variable list of additional arguments
 */
#define CFG_VALIDATE_LAN(p_name, p_def, p_help, args...)                                                               \
  _CFG_VALIDATE(p_name, p_def, p_help, .cb_validate = olsrv2_validate_lan,                                             \
    .validate_param = { { .i8 = { AF_INET, AF_INET6, -1, -1, -1 } }, { .b = true } }, ##args)

EXPORT int olsrv2_validate_lan(
  const struct cfg_schema_entry *entry, const char *section_name, const char *value, struct autobuf *out);

#endif /* _OLD_LAN_H_ */
