
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

#ifndef OS_VIF_LINUX_H_
#define OS_VIF_LINUX_H_

#include <oonf/oonf.h>
#include <oonf/base/os_vif.h>

/**
 * Linux internal data for handling virtual interfaces (tun/tap)
 */
struct os_vif_internal {
  /* no internal data necessary */
};

EXPORT int os_vif_linux_open(struct os_fd *fd, struct os_vif *vif);
EXPORT void os_vif_linux_close(struct os_vif *vif);

EXPORT struct avl_tree *os_vif_linux_get_tree(void);

/**
 * Open a new virtual interface
 * @param vif pointer to virtual interface object
 * @return -1 if an error happened, 0 otherwise
 */
static INLINE int
os_vif_open(struct os_fd *fd, struct os_vif *vif) {
  return os_vif_linux_open(fd, vif);
}

/**
 * Close a virtual interface
 * @param vif pointer to virtual interface object
 */
static INLINE void
os_vif_close(struct os_vif *vif) {
  os_vif_linux_close(vif);
}

/**
 * get virtual interface tree
 * @return vif tree
 */
static INLINE struct avl_tree *
os_vif_get_tree(void) {
  return os_vif_linux_get_tree();
}

#endif /* OS_VIF_LINUX_H_ */
