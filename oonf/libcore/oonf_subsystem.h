
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

#ifndef OONF_SUBSYSTEM_H_
#define OONF_SUBSYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oonf/oonf.h>
#include <oonf/libconfig/cfg_schema.h>

#include <oonf/libcore/oonf_logging.h>

/*! maximum size of subsystem name */
#define OONF_SUBSYSTEM_NAMESIZE 32

/**
 * @def DECLARE_OONF_PLUGIN
 * Declaration of a subsystem. Creates a constructor that hooks the subsystem into the core.
 * @param subsystem pointer to subsystem definition
 */
#define DECLARE_OONF_PLUGIN(subsystem)                                                                                 \
  EXPORT void hookup_subsystem_##subsystem(void) __attribute__((constructor));                                         \
  void hookup_subsystem_##subsystem(void) {                                                                            \
    oonf_subsystem_hook(&subsystem);                                                                                   \
  }

/**
 * description of a subsystem of the OONF-API.
 * In theory, ALL fields except for name are optional.
 */
struct oonf_subsystem {
  /*! name of the subsystem */
  const char *name;

  /*! array of dependencies of this subsystem */
  const char **dependencies;

  /*! number of dependencies of this subsystem */
  size_t dependencies_count;

  /*! description of the subsystem */
  const char *descr;

  /*! author of the subsystem */
  const char *author;

  /*! First configuration section of subsystem, might be NULL */
  struct cfg_schema_section *cfg_section;

  /**
   * Will be called once during the initialization of the subsystem.
   * Other subsystems may not be initialized during this time.
   * @return -1 if an error happened, 0 otherwise
   */
  int (*init)(void);

  /**
   * Will be called when the application begins to shut down.
   * Subsystems should stop sending normal network traffic and begin
   * to shutdown, but they will run for a few more hundred milliseconds
   * until the cleanup() callback tells them to finally shut down.
   *
   * This allows a subsystem to send out a couple of network events
   * to shut down properly.
   */
  void (*initiate_shutdown)(void);

  /**
   * Will be called once during the cleanup of the subsystem.
   * Other subsystems might already be cleanup up during this time.
   */
  void (*cleanup)(void);

  /**
   * Will be called early during initialization, even before command
   * line arguments are parsed and the configuration is loaded. The
   * callback is meant for cfgio/cfgparser implementations to hook
   * themselves into the core.
   */
  void (*early_cfg_init)(void);

  /*! true if the subsystem can be (de)activated during runtime */
  bool can_cleanup;

  /*! true if this subsystem does not need a logging source */
  bool no_logging;

  /*! logging source for subsystem */
  enum oonf_log_source logging;

  /*! true if the subsystem is initialized */
  bool _initialized;

  /*! true if unload of subsystem is in progress */
  bool _unload_initiated;

  /*! temporary variable to detect circular dependencies */
  bool _dependency_missing;

  /*! pointer to dlopen handle */
  void *_dlhandle;

  /*! which template was used to load subsystem */
  int _dlpath_index;

  /*! hook into subsystem tree */
  struct avl_node _node;
};

/**
 * Buffer for text representation of subsystem name
 */
struct oonf_subsystem_namebuf {
  /*! text array for maxmimum length subsystem name */
  char name[OONF_SUBSYSTEM_NAMESIZE];
};

EXPORT extern struct avl_tree oonf_plugin_tree;

EXPORT void oonf_subsystem_hook(struct oonf_subsystem *subsystem);
EXPORT struct oonf_subsystem *oonf_subsystem_load(const char *);
EXPORT int oonf_subsystem_unload(struct oonf_subsystem *);

int oonf_subsystem_init(void);
void oonf_subsystem_initiate_shutdown(void);
void oonf_subsystem_cleanup(void);

void oonf_subsystem_set_path(const char *path);

void oonf_subsystem_configure(struct cfg_schema *schema, struct oonf_subsystem *subsystem);
void oonf_subsystem_unconfigure(struct cfg_schema *schema, struct oonf_subsystem *subsystem);

void oonf_subsystem_initiate_unload(struct oonf_subsystem *);
int oonf_subsystem_call_init(struct oonf_subsystem *plugin);
void oonf_subsystem_extract_name(struct oonf_subsystem_namebuf *buf, const char *libname);

/**
 * Query for a certain subsystem name
 * @param libname name of plugin
 * @return pointer to plugin db entry, NULL if not found
 */
static INLINE struct oonf_subsystem *
oonf_subsystem_get(const char *libname) {
  struct oonf_subsystem *plugin;

  return avl_find_element(&oonf_plugin_tree, libname, plugin, _node);
}

static INLINE bool
oonf_subsystem_is_initialized(struct oonf_subsystem *subsystem) {
  return subsystem->_initialized;
}

/**
 * @param subsystem pointer to subsystem
 * @return true if its a plugin loaded at runtime through dlopen(),
 *   false otherwise.
 */
static INLINE bool
oonf_subsystem_is_dynamic(struct oonf_subsystem *subsystem) {
  return subsystem->_dlhandle != NULL;
}

#endif /* OONF_SUBSYSTEM_H_ */
