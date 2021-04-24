
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

#ifndef OONF_CFG_H_
#define OONF_CFG_H_

#include <oonf/libcommon/autobuf.h>
#include <oonf/oonf.h>
#include <oonf/libcommon/list.h>
#include <oonf/libcommon/netaddr.h>
#include <oonf/libconfig/cfg_schema.h>
#include <oonf/libcore/oonf_subsystem.h>

/**
 * global configuration data for core API
 */
struct oonf_config_global {
  /*! list of plugins to be loaded */
  struct strarray plugin;

  /*! additional path to look for plugins */
  char *plugin_path;

  /*! name/path of pidfile */
  char *pidfile;

  /*! name/path for lockfile */
  char *lockfile;

  /*! true if framework should fork into background */
  bool fork;

  /**
   * true if framework should return an error if configuration
   * contains an unknown section
   */
  bool failfast;
};

/*! section types for configuration */
#define CFG_SECTION_GLOBAL "global"

/*! name of parameter in 'global' section to load plugins */
#define CFG_GLOBAL_PLUGIN "plugin"

extern struct oonf_config_global config_global;

EXPORT int oonf_cfg_apply(void) __attribute__((warn_unused_result));
EXPORT int oonf_cfg_rollback(void);
EXPORT void oonf_cfg_exit(void);
EXPORT bool oonf_cfg_is_running(void);

EXPORT void oonf_cfg_trigger_reload(void);
EXPORT bool oonf_cfg_is_reload_set(void);
EXPORT void oonf_cfg_trigger_commit(void);
EXPORT bool oonf_cfg_is_commit_set(void);

EXPORT struct cfg_instance *oonf_cfg_get_instance(void);
EXPORT struct cfg_db *oonf_cfg_get_db(void);
EXPORT struct cfg_db *oonf_cfg_get_rawdb(void);
EXPORT struct cfg_schema *oonf_cfg_get_schema(void);

EXPORT int oonf_cfg_get_argc(void);
EXPORT char **oonf_cfg_get_argv(void);

/* do not use this in plugins */
int oonf_cfg_init(int argc, char **argv, const char *) __attribute__((warn_unused_result));
void oonf_cfg_cleanup(void);
int oonf_cfg_load_subsystems(void) __attribute__((warn_unused_result));
struct oonf_subsystem *oonf_cfg_load_subsystem(const char *name);
void oonf_cfg_unconfigure_subsystems(void);
void oonf_cfg_initplugins(void);
int oonf_cfg_update_globalcfg(bool) __attribute__((warn_unused_result));
int oonf_cfg_clear_rawdb(void) __attribute__((warn_unused_result));

#endif /* OONF_CFG_H_ */
