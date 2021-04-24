SET(OONF_RFC5444_SRCS rfc5444.c
                      rfc5444_context.c
                      rfc5444_iana.c
                      rfc5444_msg_generator.c
                      rfc5444_pkt_generator.c
                      rfc5444_print.c
                      rfc5444_reader.c
                      rfc5444_tlv_writer.c
                      rfc5444_writer.c)

SET(OONF_RFC5444_INCLUDES rfc5444_api_config.h
                          rfc5444_context.h
                          rfc5444.h
                          rfc5444_iana.h
                          rfc5444_print.h
                          rfc5444_reader.h
                          rfc5444_tlv_writer.h
                          rfc5444_writer.h)

oonf_create_library("librfc5444" "${OONF_RFC5444_SRCS}" "${OONF_RFC5444_INCLUDES}" "oonf_libcommon" "")
