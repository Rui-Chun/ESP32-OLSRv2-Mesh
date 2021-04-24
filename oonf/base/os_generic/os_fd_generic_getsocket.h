/*
 * os_fd_generic_getsocket.h
 *
 *  Created on: Jan 12, 2016
 *      Author: rogge
 */

#ifndef _OS_FD_GENERIC_GETSOCKET_H_
#define _OS_FD_GENERIC_GETSOCKET_H_

#include <oonf/oonf.h>
#include <oonf/base/os_fd.h>

EXPORT int os_fd_generic_getsocket(struct os_fd *sock, const union netaddr_socket *bind_to, bool tcp, size_t recvbuf,
  const struct os_interface *os_if, enum oonf_log_source log_src);

#endif /* _OS_FD_GENERIC_GETSOCKET_H_ */
