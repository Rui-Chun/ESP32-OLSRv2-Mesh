/*
 * os_fd_generic_join_mcast.h
 *
 *  Created on: Jan 12, 2016
 *      Author: rogge
 */

#ifndef _OS_FD_GENERIC_JOIN_MCAST_H_
#define _OS_FD_GENERIC_JOIN_MCAST_H_

#include <oonf/oonf.h>
#include <oonf/base/os_fd.h>

EXPORT int os_fd_generic_join_mcast_recv(
  struct os_fd *sock, const struct netaddr *multicast, const struct os_interface *os_if, enum oonf_log_source log_src);

EXPORT int os_fd_generic_join_mcast_send(struct os_fd *sock, const struct netaddr *multicast,
  const struct os_interface *os_if, bool loop, uint8_t ttl, enum oonf_log_source log_src);

#endif /* _OS_FD_GENERIC_JOIN_MCAST_H_ */
