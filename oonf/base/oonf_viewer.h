
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

#ifndef OONF_VIEWER_H_
#define OONF_VIEWER_H_

#include <oonf/libcommon/autobuf.h>
#include <oonf/oonf.h>
#include <oonf/libcommon/json.h>
#include <oonf/libcommon/template.h>

#include <oonf/libcore/oonf_subsystem.h>
#include <oonf/base/oonf_telnet.h> /* compile-time dependency */

/*! subsystem identifier */
#define OONF_VIEWER_SUBSYSTEM "viewer"

/*! viewer should return raw numbers, not iso-prefixes */
#define OONF_VIEWER_RAW_FORMAT "raw"

/*! viewer should only return the names of the fields */
#define OONF_VIEWER_HEAD_FORMAT "head"

/*! viewer should output json format */
#define OONF_VIEWER_JSON_FORMAT "json"

/*! viewer should output json format with raw numbers */
#define OONF_VIEWER_JSON_RAW_FORMAT "jsonraw"

/*! viewer should output json format without surrounding object brackets */
#define OONF_VIEWER_DATA_FORMAT "data"

/**
 * viewer should output json format with raw numbers
 * and without surrounding object brackets
 */
#define OONF_VIEWER_DATA_RAW_FORMAT "dataraw"

/**
 * This struct defines a template engine command that can output both
 * table and JSON.
 */
struct oonf_viewer_template {
  /*! output buffer */
  struct autobuf *out;

  /*! true if output should be in JSON format */
  bool create_json;

  /*! true if isonumbers should be raw */
  bool create_raw;

  /*! true if skips the enclosing JSON brackets */
  bool create_only_data;

  /*! pointer to template data array to get key/value pairs */
  struct abuf_template_data *data;

  /*! size of template data array */
  size_t data_size;

  /*! name of the JSON object which contains the key*/
  const char *json_name;

  /*! single line help text for overview about command */
  const char *help_line;

  /*! full help text about this template output */
  const char *help;

  /**
   * Callback triggered to generate the content of the template
   * @param this viewer template
   * @return -1 if an error happened, 0 otherwise
   */
  int (*cb_function)(struct oonf_viewer_template *);

  /*! internal variable for template engine storage array */
  struct abuf_template_storage *_storage;

  /*! internal variable for JSON generation */
  struct json_session _json;
};

EXPORT void oonf_viewer_output_prepare(struct oonf_viewer_template *template, struct abuf_template_storage *storage,
  struct autobuf *out, const char *format);
EXPORT void oonf_viewer_output_print_line(struct oonf_viewer_template *template);
EXPORT void oonf_viewer_output_finish(struct oonf_viewer_template *template);

EXPORT void oonf_viewer_print_help(
  struct autobuf *out, const char *parameter, struct oonf_viewer_template *template, size_t count);
EXPORT int oonf_viewer_call_subcommands(struct autobuf *out, struct abuf_template_storage *storage, const char *param,
  struct oonf_viewer_template *templates, size_t count);
EXPORT enum oonf_telnet_result oonf_viewer_telnet_handler(struct autobuf *out, struct abuf_template_storage *storage,
  const char *cmd, const char *param, struct oonf_viewer_template *templates, size_t count);
EXPORT enum oonf_telnet_result oonf_viewer_telnet_help(
  struct autobuf *out, const char *cmd, const char *parameter, struct oonf_viewer_template *template, size_t count);

#endif /* OONF_VIEWER_H_ */
