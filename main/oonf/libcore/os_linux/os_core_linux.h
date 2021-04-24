
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

#ifndef OS_CORE_LINUX_H_
#define OS_CORE_LINUX_H_

#include <stdlib.h>
#include <sys/time.h>

#include <oonf/libcore/os_core.h>
#include <oonf/libcore/os_generic/os_core_generic_syslog.h>

/*! default folder for Linux lockfile */
#define OS_CORE_LOCKFILE_FOLDER "/var/run/"

EXPORT int os_core_linux_get_random(void *dst, size_t length);
EXPORT int os_core_linux_create_lockfile(const char *path);

/**
 * Initialize core
 * @param appname name of the application
 */
static INLINE int
os_core_init(const char *appname) {
  os_core_generic_syslog_init(appname);
  return 0;
}

/**
 * Cleanup core
 */
static INLINE void
os_core_cleanup(void) {
  os_core_generic_syslog_cleanup();
}

static INLINE int
os_core_syslog(enum oonf_log_severity sev, const char *msg) {
  os_core_generic_syslog(sev, msg);
  return 0;
}

/**
 * Create a lock file of a certain name
 * @param path name of lockfile including path
 * @return 0 if the lock was created successfully, false otherwise
 */
static INLINE int
os_core_create_lockfile(const char *path) {
  return os_core_linux_create_lockfile(path);
}

/**
 * Get some random data
 * @param dst pointer to destination buffer
 * @param length number of random bytes requested
 * @return 0 if the random data was generated, -1 if an error happened
 */
static INLINE int
os_core_get_random(void *dst, size_t length) {
  return os_core_linux_get_random(dst, length);
}

/**
 * Inline wrapper around gettimeofday
 * @param tv pointer to target timeval object
 * @return -1 if an error happened, 0 otherwise
 */
static INLINE int
os_core_gettimeofday(struct timeval *tv) {
  return gettimeofday(tv, NULL);
}

#endif /* OS_CORE_GENERIC_H_ */
