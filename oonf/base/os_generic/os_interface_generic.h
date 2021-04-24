/*
 * os_interface_generic_get_bindaddress.h
 *
 *  Created on: 07.03.2016
 *      Author: rogge
 */

#ifndef OS_INTERFACE_GENERIC_H_
#define OS_INTERFACE_GENERIC_H_

#include <oonf/oonf.h>
#include <oonf/libcommon/netaddr_acl.h>
#include <oonf/base/os_interface.h>

EXPORT const struct netaddr *os_interface_generic_get_bindaddress(
  int af_type, struct netaddr_acl *filter, struct os_interface *os_if);
EXPORT struct os_interface *os_interface_generic_get_data_by_ifbaseindex(unsigned ifindex);
EXPORT struct os_interface *os_interface_generic_get_data_by_ifindex(unsigned ifindex);
EXPORT const struct os_interface_ip *os_interface_generic_get_prefix_from_dst(
  struct netaddr *destination, struct os_interface *os_if);

#endif /* OS_INTERFACE_GENERIC_H_ */
