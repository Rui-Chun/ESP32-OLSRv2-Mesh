/*
 * bitstream.c
 *
 *  Created on: 04.10.2016
 *      Author: rogge
 */

#include <string.h>

#include <oonf/libcommon/bitstream.h>
#include <oonf/oonf.h>

/**
 * Initialize reading bitstream
 * @param stream bitstream
 * @param src source buffer
 * @param bytes length of buffer in bytes
 */
void
bitstream_r_init(struct bitstream_r *stream, const void *src, size_t bytes) {
  stream->ptr = src;
  stream->length = bytes;
  stream->bit_offset = 0;
}

/**
 * Copy a number of bytes from a bitstream to a buffer.
 * The bitstreams pointer will be padded to a whole byte before the copy.
 * @param stream bitstream
 * @param dst destination buffer
 * @param bytes number of bytes to copy
 * @return -1 if source buffer is out of bytes, 0 otherwise
 */
int
bitstream_r_memcpy(struct bitstream_r *stream, void *dst, size_t bytes) {
  const uint8_t *buffer;
  size_t offset;
  bitstream_r_pad(stream);

  offset = stream->bit_offset / 8;
  if (offset + bytes > stream->length) {
    return -1;
  }

  buffer = stream->ptr;
  memcpy(dst, &buffer[offset], bytes);

  stream->bit_offset += bytes * 8;
  return 0;
}

/**
 * Read up to 56 bits from a bitstream into a 64 bit integer
 * @param stream bitstream
 * @param dst storage buffer for result
 * @param bits number of bits to copy
 * @return -1 if buffer is out of bits, 0 otherwise
 */
int
bitstream_r_read(struct bitstream_r *stream, uint64_t *dst, int32_t bits) {
  size_t offset, size, bit_start, bit_end;
  const uint8_t *buffer;
  uint64_t result;

  if (bits <= 0) {
    *dst = 0;
    return 0;
  }

  if (bits > 56) {
    return -1;
  }

  buffer = stream->ptr;

  /* copy range */
  bit_start = stream->bit_offset;
  bit_end = bit_start + bits;

  /* calculate offset */
  offset = bit_start / 8;
  size = (bit_end + 7) / 8 - offset;

  /* check range */
  if (offset + size > stream->length) {
    return -1;
  }

  /* advance pointer */
  stream->bit_offset += bits;

  /* truncate bit positions */
  bit_start = bit_start & 7;
  bit_end = bit_end & 7;

  /* copy first byte */
  result = buffer[offset];
  if (bit_start != 0) {
    result &= (0xFF >> bit_start);
  }

  while (size > 1) {
    offset++;
    size--;

    result = (result << 8) | buffer[offset];
  }

  if (bit_end != 0) {
    result >>= (8 - bit_end);
  }

  *dst = result;
  return 0;
}

/**
 * Pad a reading bitstream to a whole byte
 * @param stream bitstream
 */
void
bitstream_r_pad(struct bitstream_r *stream) {
  size_t bits;

  bits = stream->bit_offset & 7;
  if (bits) {
    stream->bit_offset -= bits;
    stream->bit_offset += 8;
  }
}

/**
 * Initialize a writing bitstream. This will also initialize
 * the whole buffer with 0 bytes.
 * @param stream bitstream
 * @param buffer output buffer
 * @param bytes output buffer length in bytes
 */
void
bitstream_w_init(struct bitstream_w *stream, void *buffer, size_t bytes) {
  stream->ptr = buffer;
  stream->length = bytes;
  stream->bit_offset = 0;

  memset(buffer, 0, bytes);
}

/**
 * Copy a number of bytes into a bitstream.
 * The bitstreams pointer will be padded to a whole byte before the copy.
 * @param stream bitstream
 * @param src source buffer
 * @param bytes number of bytes in source buffer
 * @return -1 if buffer is out of bytes, 0 otherwise
 */
int
bitstream_w_memcpy(struct bitstream_w *stream, const void *src, size_t bytes) {
  size_t offset;
  uint8_t *buffer;

  bitstream_w_pad(stream);
  offset = stream->bit_offset / 8;

  if (offset + bytes > stream->length) {
    return -1;
  }

  buffer = stream->ptr;

  memcpy(&buffer[offset], src, bytes);
  offset += bytes * 8;
  return 0;
}

/**
 * Copy up to 56 bits into a bitstream.
 * @param stream bitstream
 * @param src input source
 * @param bits number of bits to copy
 * @return -1 if an error happened, 0 otherwise
 */
int
bitstream_w_write(struct bitstream_w *stream, uint64_t src, int32_t bits) {
  uint32_t bit_start, bit_end;
  size_t offset, size;
  uint8_t *buffer;

  if (bits <= 0) {
    return 0;
  }

  if (bits > 56) {
    return -1;
  }

  buffer = stream->ptr;

  /* copy range */
  bit_start = stream->bit_offset;
  bit_end = bit_start + bits;

  /* calculate offset */
  offset = bit_start / 8;
  size = (bit_end + 7) / 8 - offset;

  /* check range */
  if (offset + size > stream->length) {
    return -1;
  }

  /* advance pointer */
  stream->bit_offset += bits;

  /* mask input */
  src &= ((1ull << bits) - 1ull);

  /* shift source value */
  bit_end = bit_end & 7;
  if (bit_end != 0) {
    src <<= (8 - bit_end);
  }

  /* write data to buffer */
  while (size > 0) {
    size--;

    buffer[offset + size] |= (src & 0xFF);
    src >>= 8;
  }

  return 0;
}

/**
 * Pad a writing bitstream to a whole byte
 * @param stream bitstream
 */
void
bitstream_w_pad(struct bitstream_w *stream) {
  size_t bits;

  bits = stream->bit_offset & 7;
  if (bits) {
    stream->bit_offset -= bits;
    stream->bit_offset += 8;
  }
}
