
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

#ifndef OONF_TELNET_H_
#define OONF_TELNET_H_

#include <oonf/libcommon/avl.h>
#include <oonf/oonf.h>
#include <oonf/libcommon/list.h>
#include <oonf/libcommon/netaddr.h>
#include <oonf/libcommon/netaddr_acl.h>
#include <oonf/base/oonf_stream_socket.h>

/*! subsystem identifier */
#define OONF_TELNET_SUBSYSTEM "telnet"

/**
 * telnet session status
 */
enum oonf_telnet_result
{
  /*! active and waiting for the next command */
  TELNET_RESULT_ACTIVE,

  /*! will output continuous data until stopped */
  TELNET_RESULT_CONTINOUS,

  /*! an error happened with the telnet command */
  TELNET_RESULT_INTERNAL_ERROR,

  /*! telnet session should end */
  TELNET_RESULT_QUIT,

  /**
   * this one is used internally for the telnet API,
   * it should not be returned by a command handler
   */
  _TELNET_RESULT_UNKNOWN_COMMAND,
};

/**
 * represents a cleanup handler that must be called when the
 * telnet core is shut down.
 */
struct oonf_telnet_cleanup {
  /*! pointer to telnet data */
  struct oonf_telnet_data *data;

  /**
   * Callback that a telnet session is to be cleaned up
   * @param cleanup registered cleanup handler
   */
  void (*cleanup_handler)(struct oonf_telnet_cleanup *cleanup);

  /*! custom data pointer for cleanup handler */
  void *custom;

  /*! node for list of cleanup handlers */
  struct list_entity node;
};

/**
 * represents the data part of a telnet connection to a client
 */
struct oonf_telnet_data {
  /*! address of remote communication partner */
  struct netaddr *remote;

  /*! output buffer for telnet commands */
  struct autobuf *out;

  /*! current telnet command */
  const char *command;

  /*! current telnet parameters */
  const char *parameter;

  /*! true if echo mode is active */
  bool show_echo;

  /*! millisecond timeout between commands */
  uint32_t timeout_value;

  /**
   * Callback triggered to stop continuous data output
   * @param data this telnet data object
   */
  void (*stop_handler)(struct oonf_telnet_data *data);

  /*! custom data for stop handler */
  void *stop_data[4];

  /*! custom timer for stop handler */
  struct oonf_timer_instance stop_timer;

  /*! list of cleanup handlers */
  struct list_entity cleanup_list;
};

/**
 * represents a full telnet session including socket
 */
struct oonf_telnet_session {
  /*! telnet TCP stream session */
  struct oonf_stream_session session;

  /*! telnet data */
  struct oonf_telnet_data data;
};

#if !defined(REMOVE_HELPTEXT)
/**
 * define an array entry for a telnet command
 * @param cmd command name
 * @param cb callback for command
 * @param helptext help text for command
 * @param args additional arguments
 */
#define TELNET_CMD(cmd, cb, helptext, args...)                                                                         \
  { .command = (cmd), .handler = (cb), .help = helptext, ##args }
#else
#define TELNET_CMD(cmd, cb, helptext, args...)                                                                         \
  { .command = (cmd), .handler = (cb), .help = "", ##args }
#endif

/**
 * represents a telnet command
 */
struct oonf_telnet_command {
  /*! name of telnet command */
  const char *command;

  /*! help text for telnet command, NULL if it uses a custom help handler */
  const char *help;

  /*! access control list for telnet command, NULL if not used */
  struct netaddr_acl *acl;

  /**
   * callback triggered when telnet command is called
   * @param con telnet data
   * @return telnet result
   */
  enum oonf_telnet_result (*handler)(struct oonf_telnet_data *con);

  /**
   * callback triggered when help command for telnet command is called
   * @param con telnet data
   * @return telnet result
   */
  enum oonf_telnet_result (*help_handler)(struct oonf_telnet_data *con);

  /*! node for tree of telnet commands */
  struct avl_node _node;
};

EXPORT int oonf_telnet_add(struct oonf_telnet_command *command);
EXPORT void oonf_telnet_remove(struct oonf_telnet_command *command);

EXPORT void oonf_telnet_stop(struct oonf_telnet_data *data, bool print_prompt);

EXPORT enum oonf_telnet_result oonf_telnet_execute(
  const char *cmd, const char *para, struct autobuf *out, struct netaddr *remote);

/**
 * Add a cleanup handler to a telnet session
 * @param data pointer to telnet data
 * @param cleanup pointer to initialized cleanup handler
 */
static INLINE void
oonf_telnet_add_cleanup(struct oonf_telnet_data *data, struct oonf_telnet_cleanup *cleanup) {
  cleanup->data = data;
  list_add_tail(&data->cleanup_list, &cleanup->node);
}

/**
 * Removes a cleanup handler to a telnet session
 * @param cleanup pointer to cleanup handler
 */
static INLINE void
oonf_telnet_remove_cleanup(struct oonf_telnet_cleanup *cleanup) {
  list_remove(&cleanup->node);
}

/**
 * Flushs the output stream of a telnet session. This will be only
 * necessary for continous output.
 * @param data pointer to telnet data
 */
static INLINE void
oonf_telnet_flush_session(struct oonf_telnet_data *data) {
  struct oonf_telnet_session *session;

  session = container_of(data, struct oonf_telnet_session, data);
  if (session->session.state != STREAM_SESSION_INACTIVE && session->session.state != STREAM_SESSION_CLEANUP) {
    oonf_stream_flush(&session->session);
  }
}

#endif /* OONF_TELNET_H_ */
