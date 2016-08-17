// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    stream.h

    Code for implementing Imgtool streams

***************************************************************************/

#ifndef STREAM_H
#define STREAM_H

#include "imgterrs.h"
#include "corefile.h"

struct imgtool_stream;

imgtool_stream *stream_open(const char *fname, int read_or_write);  /* similar params to mame_fopen */
imgtool_stream *stream_open_write_stream(int filesize);
imgtool_stream *stream_open_mem(void *buf, size_t sz);
void stream_close(imgtool_stream *stream);
util::core_file *stream_core_file(imgtool_stream *stream);
UINT32 stream_read(imgtool_stream *stream, void *buf, UINT32 sz);
UINT32 stream_write(imgtool_stream *stream, const void *buf, UINT32 sz);
UINT64 stream_size(imgtool_stream *stream);
int stream_seek(imgtool_stream *stream, INT64 pos, int where);
UINT64 stream_tell(imgtool_stream *stream);
void *stream_getptr(imgtool_stream *stream);
UINT32 stream_putc(imgtool_stream *stream, char c);
UINT32 stream_puts(imgtool_stream *stream, const char *s);
UINT32 stream_printf(imgtool_stream *stream, const char *fmt, ...) ATTR_PRINTF(2,3);

/* Transfers sz bytes from source to dest */
UINT64 stream_transfer(imgtool_stream *dest, imgtool_stream *source, UINT64 sz);
UINT64 stream_transfer_all(imgtool_stream *dest, imgtool_stream *source);

/* Fills sz bytes with b */
UINT64 stream_fill(imgtool_stream *f, unsigned char b, UINT64 sz);

/* Returns the CRC of a file */
int stream_crc(imgtool_stream *f, unsigned long *result);
int file_crc(const char *fname,  unsigned long *result);

/* Returns whether a stream is read only or not */
int stream_isreadonly(imgtool_stream *f);

#endif /* STREAM_H */
