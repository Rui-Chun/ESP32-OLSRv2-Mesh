/*
 * os_fd_generic_configsocket.h
 *
 *  Created on: Jan 12, 2016
 *      Author: rogge
 */

#ifndef _OS_FD_GENERIC_CONFIGSOCKET_H_
#define _OS_FD_GENERIC_CONFIGSOCKET_H_

#include <oonf/oonf.h>
#include <oonf/libcore/oonf_logging.h>
#include <oonf/base/os_fd.h>
#include <oonf/base/os_interface.h>

EXPORT int os_fd_generic_configsocket(struct os_fd *sock, const union netaddr_socket *bind_to, size_t recvbuf,
  bool rawip, const struct os_interface *os_if, enum oonf_log_source log_src);

#endif /* _OS_FD_GENERIC_CONFIGSOCKET_H_ */
