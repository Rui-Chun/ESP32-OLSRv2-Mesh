/*
 * os_fd_generic_set_nonblocking.h
 *
 *  Created on: Jan 12, 2016
 *      Author: rogge
 */

#ifndef _OS_FD_GENERIC_SET_NONBLOCKING_H_
#define _OS_FD_GENERIC_SET_NONBLOCKING_H_

#include <oonf/oonf.h>
#include <oonf/base/os_fd.h>

EXPORT int os_fd_generic_set_nonblocking(struct os_fd *sock);

#endif /* _OS_FD_GENERIC_SET_NONBLOCKING_H_ */
