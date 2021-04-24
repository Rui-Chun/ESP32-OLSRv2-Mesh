
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

#ifndef TEST_RFC5444_INTEROP_H_
#define TEST_RFC5444_INTEROP_H_

#include <oonf/oonf.h>
#include <oonf/libcommon/avl.h>

struct test_tlv {
  uint8_t flags;
  uint8_t type, exttype;

  uint8_t *value;
  uint16_t length;

  bool okay;
};

struct test_address {
  uint8_t *addr;
  uint8_t plen;

  struct test_tlv *tlvs;
  uint16_t tlv_count;

  bool okay;
};

struct test_message {
  uint8_t type;
  uint8_t addrlen;
  uint8_t flags;

  bool has_originator;
  uint8_t *originator;

  bool has_hoplimit;
  uint8_t hoplimit;

  bool has_hopcount;
  uint8_t hopcount;

  bool has_seqno;
  uint16_t seqno;

  struct test_tlv *tlvs;
  uint16_t tlv_count;

  struct test_address *addrs;
  uint16_t address_count;

  bool okay;
};

struct test_packet {
  const char *test;
  struct avl_node _node;

  uint8_t *binary;
  size_t binlen;

  uint8_t version;
  uint8_t flags;

  bool has_seq;
  uint8_t seqno;

  struct test_tlv *tlvs;
  size_t tlv_count;

  struct test_message *msgs;
  size_t msg_count;
};

#define ADD_TEST(x) __attribute__((constructor)) void __add_test_ ## x (void); \
                     void __add_test_ ## x (void){ add_test(&x); }

EXPORT void add_test(struct test_packet *);

#endif /* TEST_RFC5444_INTEROP_H_ */
