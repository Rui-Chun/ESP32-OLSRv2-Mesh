/*
 * bitstream.h
 *
 *  Created on: 04.10.2016
 *      Author: rogge
 */

#ifndef SRC_COMMON_BITSTREAM_H_
#define SRC_COMMON_BITSTREAM_H_

#include <oonf/oonf.h>

struct bitstream_r {
  const void *ptr;
  size_t length;

  size_t bit_offset;
};

struct bitstream_w {
  void *ptr;
  size_t length;

  size_t bit_offset;
};

EXPORT void bitstream_r_init(struct bitstream_r *stream, const void *src, size_t bytes);
EXPORT int bitstream_r_memcpy(struct bitstream_r *stream, void *dst, size_t bytes);
EXPORT int bitstream_r_read(struct bitstream_r *stream, uint64_t *dst, int32_t bits);
EXPORT void bitstream_r_pad(struct bitstream_r *stream);

EXPORT void bitstream_w_init(struct bitstream_w *stream, void *buffer, size_t bytes);
EXPORT int bitstream_w_memcpy(struct bitstream_w *stream, const void *src, size_t bytes);
EXPORT int bitstream_w_write(struct bitstream_w *stream, uint64_t src, int32_t bits);
EXPORT void bitstream_w_pad(struct bitstream_w *stream);

/**
 * @param stream writing bitstream
 * @return the number of bytes already written into the stream
 */
static INLINE size_t
bitstream_w_get_length(const struct bitstream_w *stream) {
  return (stream->bit_offset + 7) / 8;
}

#endif /* SRC_COMMON_BITSTREAM_H_ */
