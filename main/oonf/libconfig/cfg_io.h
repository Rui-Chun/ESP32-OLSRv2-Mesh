
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

#ifndef CFG_IO_H_
#define CFG_IO_H_

/* forward declaration */
struct cfg_io;

#include <oonf/libcommon/autobuf.h>
#include <oonf/libcommon/avl.h>
#include <oonf/oonf.h>

#include <oonf/libconfig/cfg.h>

/**
 *  Represents a single IO-Handler
 */
struct cfg_io {
  /*! node for global tree in cfg_io.c */
  struct avl_node node;

  /*! name of io handler */
  const char *name;

  /*! true if this is the default handler */
  bool def;

  /**
   * callback to load a configuration
   * @param param user parameter for configuration loader
   * @param log buffer for text output of the loader
   * @return configuration database, NULL if an error happened
   */
  struct cfg_db *(*load)(const char *param, struct autobuf *log);

  /**
   * callback to save a configuration
   * @param param user parameter for configuration storage
   * @param src configuration database to save
   * @param log buffer for text output while storing the data
   * @return -1 if an error happened, 0 otherwise
   */
  int (*save)(const char *param, struct cfg_db *src, struct autobuf *log);
};

/**
 * Macro to iterate over all IO handlers of a configuration instance
 * This macro should be used similar to a for() or while() construct
 * @param instance configuration instance
 * @param io reference of io handler variable,
 *   will be used as the iterator variable
 * @param safeit helper reference of of io handler variable
 */
#define CFG_FOR_ALL_IO(instance, io, safeit) avl_for_each_element_safe(&(instance)->io_tree, io, node, safeit)

EXPORT void cfg_io_add(struct cfg_instance *, struct cfg_io *);
EXPORT void cfg_io_remove(struct cfg_instance *, struct cfg_io *);

EXPORT struct cfg_db *cfg_io_load(struct cfg_instance *instance, const char *url, struct autobuf *log);
EXPORT int cfg_io_save(struct cfg_instance *instance, const char *url, struct cfg_db *src, struct autobuf *log);

#endif /* CFG_IO_H_ */
