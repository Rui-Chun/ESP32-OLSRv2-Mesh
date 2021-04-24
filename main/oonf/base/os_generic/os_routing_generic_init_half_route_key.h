/*
 * os_routing_generic_init_half_route_key.h
 *
 *  Created on: 12.02.2016
 *      Author: rogge
 */

#ifndef _OS_ROUTING_GENERIC_INIT_HALF_ROUTE_KEY_H_
#define _OS_ROUTING_GENERIC_INIT_HALF_ROUTE_KEY_H_

#include <oonf/oonf.h>
#include <oonf/base/os_routing.h>

EXPORT void os_routing_generic_init_half_os_route_key(
  struct netaddr *ipany, struct netaddr *specific, const struct netaddr *source);

#endif /* _OS_ROUTING_GENERIC_INIT_HALF_ROUTE_KEY_H_ */
