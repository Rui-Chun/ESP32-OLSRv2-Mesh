SET(OONF_COMMON_SRCS  autobuf.c
                      avl_comp.c
                      avl.c
                      bitmap256.c
                      bitstream.c
                      isonumber.c
                      json.c
                      netaddr.c
                      netaddr_acl.c
                      string.c
                      template.c)

SET(OONF_COMMON_INCLUDES autobuf.h
                         avl_comp.h
                         avl.h
                         bitmap256.h
                         bitstream.h
                         common_types.h
                         container_of.h
                         isonumber.h
                         json.h
                         list.h
                         netaddr.h
                         netaddr_acl.h
                         string.h
                         template.h)

oonf_create_library("libcommon" "${OONF_COMMON_SRCS}" "${OONF_COMMON_INCLUDES}" "" "")
