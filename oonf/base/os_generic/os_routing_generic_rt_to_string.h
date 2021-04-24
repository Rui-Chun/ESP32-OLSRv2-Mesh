/*
 * os_routing_generic_rt_to_string.h
 *
 *  Created on: 12.02.2016
 *      Author: rogge
 */

#ifndef _OS_GENERIC_OS_ROUTING_GENERIC_RT_TO_STRING_H_
#define _OS_GENERIC_OS_ROUTING_GENERIC_RT_TO_STRING_H_

#include <oonf/oonf.h>
#include <oonf/base/os_routing.h>

const char *os_routing_generic_rt_to_string(struct os_route_str *buf, const struct os_route_parameter *route_parameter);

#endif /* _OS_GENERIC_OS_ROUTING_GENERIC_RT_TO_STRING_H_ */
