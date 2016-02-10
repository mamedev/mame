// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    stream.c

    Code for implementing Imgtool streams

***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "unzip.h"
#include "osdcore.h"
#include "imgtool.h"

enum imgtype_t
{
	IMG_FILE,
	IMG_MEM
};

struct imgtool_stream
{
	imgtype_t imgtype;
	int write_protect;
	const char *name; // needed for clear
	UINT64 position;
	UINT64 filesize;

	union
	{
		core_file *file;
		UINT8 *buffer;
	} u;
};



static imgtool_stream *stream_open_zip(const char *zipname, const char *subname, int read_or_write)
{
	imgtool_stream *imgfile = nullptr;
//  zip_error ziperr;
	zip_file *z = nullptr;
	const zip_file_header *zipent;
	FILE *f;

	if (read_or_write)
		goto error;

	/* check to see if the file exists */
	f = fopen(zipname, "r");
	if (!f)
		goto error;
	fclose(f);

	imgfile = (imgtool_stream *)malloc(sizeof(imgtool_stream));
	if (!imgfile)
		goto error;

	memset(imgfile, 0, sizeof(*imgfile));
	imgfile->imgtype = IMG_MEM;
	imgfile->write_protect = 1;
	imgfile->position = 0;

//  ziperr =
	zip_file_open(zipname, &z);
	if (!z)
		goto error;

	zipent = zip_file_first_file(z);
	while(zipent && subname && strcmp(subname, zipent->filename))
		zipent = zip_file_next_file(z);
	if (!zipent)
		goto error;

	imgfile->filesize = zipent->uncompressed_length;
	imgfile->u.buffer = (UINT8*)malloc(zipent->uncompressed_length);
	if (!imgfile->u.buffer)
		goto error;

	if (zip_file_decompress(z, imgfile->u.buffer, zipent->uncompressed_length))
		goto error;

	zip_file_close(z);
	return imgfile;

error:
	if (z)
		zip_file_close(z);
	if (imgfile)
	{
		if (imgfile->u.buffer)
			free(imgfile->u.buffer);
		free(imgfile);
	}
	return nullptr;
}



imgtool_stream *stream_open(const char *fname, int read_or_write)
{
	file_error filerr;
	const char *ext;
	imgtool_stream *imgfile = nullptr;
	static const UINT32 write_modes[] =
	{
		OPEN_FLAG_READ,
		OPEN_FLAG_WRITE | OPEN_FLAG_CREATE,
		OPEN_FLAG_READ | OPEN_FLAG_WRITE,
		OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE
	};
	core_file *f = nullptr;
	char *buf = nullptr;
	int len, i;
	imgtool_stream *s = nullptr;
	char c;

	/* maybe we are just a ZIP? */
	ext = strrchr(fname, '.');
	if (ext && !core_stricmp(ext, ".zip"))
		return stream_open_zip(fname, nullptr, read_or_write);

	filerr = core_fopen(fname, write_modes[read_or_write], &f);
	if (filerr != FILERR_NONE)
	{
		if (!read_or_write)
		{
			len = strlen(fname);

			/* can't open the file; try opening ZIP files with other names */
			buf = (char*)malloc(len + 1);
			if (!buf)
				goto error;
			strcpy(buf, fname);

			for(i = len-1; !s && (i >= 0); i--)
			{
				if ((buf[i] == '\\') || (buf[i] == '/'))
				{
					c = buf[i];
					buf[i] = '\0';
					s = stream_open_zip(buf, buf + i + 1, read_or_write);
					buf[i] = c;
				}
			}
			free(buf);
			buf = nullptr;

			if (s)
				return s;
		}

		/* ah well, it was worth a shot */
		goto error;
	}

	imgfile = (imgtool_stream *)malloc(sizeof(imgtool_stream));
	if (!imgfile)
		goto error;

	/* Normal file */
	memset(imgfile, 0, sizeof(*imgfile));
	imgfile->imgtype = IMG_FILE;
	imgfile->position = 0;
	imgfile->filesize = core_fsize(f);
	imgfile->write_protect = read_or_write ? 0 : 1;
	imgfile->u.file = f;
	imgfile->name = fname;
	return imgfile;

error:
	if (imgfile != nullptr)
		free((void *) imgfile);
	if (f != nullptr)
		core_fclose(f);
	if (buf)
		free(buf);
	return (imgtool_stream *) nullptr;
}



imgtool_stream *stream_open_write_stream(int size)
{
	imgtool_stream *imgfile;

	imgfile = (imgtool_stream *)malloc(sizeof(imgtool_stream));
	if (!imgfile)
		return nullptr;

	imgfile->imgtype = IMG_MEM;
	imgfile->write_protect = 0;
	imgfile->position = 0;
	imgfile->filesize = size;
	imgfile->u.buffer = (UINT8*)malloc(size);

	if (!imgfile->u.buffer)
	{
		free(imgfile);
		return nullptr;
	}

	return imgfile;
}



imgtool_stream *stream_open_mem(void *buf, size_t sz)
{
	imgtool_stream *imgfile;

	imgfile = (imgtool_stream *)malloc(sizeof(imgtool_stream));
	if (!imgfile)
		return nullptr;

	memset(imgfile, 0, sizeof(*imgfile));
	imgfile->imgtype = IMG_MEM;
	imgfile->position = 0;
	imgfile->write_protect = 0;

	imgfile->filesize = sz;
	imgfile->u.buffer = (UINT8*)buf;
	return imgfile;
}



void stream_close(imgtool_stream *s)
{
	assert(s != nullptr);

	switch(s->imgtype)
	{
		case IMG_FILE:
			if (s->u.file != nullptr)
			{
				core_fclose(s->u.file);
				s->u.file = nullptr;
			}
			break;

		case IMG_MEM:
			if (s->u.buffer != nullptr)
			{
				free(s->u.buffer);
				s->u.buffer = nullptr;
			}
			break;

		default:
			assert(0);
			break;
	}
	free((void *) s);
}



core_file *stream_core_file(imgtool_stream *stream)
{
	return (stream->imgtype == IMG_FILE) ? stream->u.file : nullptr;
}



UINT32 stream_read(imgtool_stream *stream, void *buf, UINT32 sz)
{
	UINT32 result = 0;

	switch(stream->imgtype)
	{
		case IMG_FILE:
			assert(sz == (UINT32) sz);
			core_fseek(stream->u.file, stream->position, SEEK_SET);
			result = core_fread(stream->u.file, buf, (UINT32) sz);
			break;

		case IMG_MEM:
			/* do we have to limit sz? */
			if (sz > (stream->filesize - stream->position))
				sz = (UINT32) (stream->filesize - stream->position);

			memcpy(buf, stream->u.buffer + stream->position, sz);
			result = sz;
			break;

		default:
			assert(0);
			break;
	}
	stream->position += result;
	return result;
}



UINT32 stream_write(imgtool_stream *s, const void *buf, UINT32 sz)
{
	void *new_buffer;
	UINT32 result = 0;

	switch(s->imgtype)
	{
		case IMG_MEM:
			if (!s->write_protect)
			{
				/* do we have to expand the buffer? */
				if (s->filesize < s->position + sz)
				{
					/* try to expand the buffer */
					if (s->u.buffer) free(s->u.buffer);
					new_buffer = malloc(s->position + sz);
					if (new_buffer)
					{
						s->u.buffer = (UINT8*)new_buffer;
						s->filesize = s->position + sz;
					}
				}

				/* do we have to limit sz? */
				if (sz > (s->filesize - s->position))
					sz = (UINT32) (s->filesize - s->position);

				memcpy(s->u.buffer + s->position, buf, sz);
				result = sz;
			}
			break;

		case IMG_FILE:
			core_fseek(s->u.file, s->position, SEEK_SET);
			result = core_fwrite(s->u.file, buf, sz);
			break;

		default:
			assert(0);
			break;
	}

	/* advance the file pointer */
	s->position += result;

	/* did we grow the file */
	if (s->position > s->filesize)
		s->filesize = s->position;
	return result;
}



UINT64 stream_size(imgtool_stream *s)
{
	return s->filesize;
}



void *stream_getptr(imgtool_stream *f)
{
	void *ptr;

	switch(f->imgtype)
	{
		case IMG_MEM:
			ptr = f->u.buffer;
			break;

		default:
			ptr = nullptr;
			break;
	}
	return ptr;
}



int stream_seek(imgtool_stream *s, INT64 pos, int where)
{
	UINT64 size;

	size = stream_size(s);

	switch(where)
	{
		case SEEK_CUR:
			pos += s->position;
			break;
		case SEEK_END:
			pos += size;
			break;
	}

	if (pos < 0)
		s->position = 0;
	else
		s->position = MIN(size, pos);

	if (s->position < pos)
		stream_fill(s, '\0', pos - s->position);

	return 0;
}



UINT64 stream_tell(imgtool_stream *s)
{
	return s->position;
}



UINT64 stream_transfer(imgtool_stream *dest, imgtool_stream *source, UINT64 sz)
{
	UINT64 result = 0;
	UINT64 readsz;
	char buf[1024];

	while(sz && (readsz = stream_read(source, buf, MIN(sz, sizeof(buf)))))
	{
		stream_write(dest, buf, readsz);
		sz -= readsz;
		result += readsz;
	}
	return result;
}



UINT64 stream_transfer_all(imgtool_stream *dest, imgtool_stream *source)
{
	return stream_transfer(dest, source, stream_size(source));
}



int stream_crc(imgtool_stream *s, unsigned long *result)
{
	size_t sz;
	void *ptr;

	switch(s->imgtype)
	{
		case IMG_MEM:
			*result = crc32(0, (unsigned char *) s->u.buffer, (size_t) s->filesize);
			break;

		default:
			sz = stream_size(s);
			ptr = malloc(sz);
			if (!ptr)
				return IMGTOOLERR_OUTOFMEMORY;
			stream_seek(s, 0, SEEK_SET);
			if (stream_read(s, ptr, sz) != sz)
			{
				free(ptr);
				return IMGTOOLERR_READERROR;
			}
			*result = crc32(0, (const Bytef*)ptr, sz);
			free(ptr);
			break;
	}
	return 0;
}



int file_crc(const char *fname,  unsigned long *result)
{
	int err;
	imgtool_stream *f;

	f = stream_open(fname, OSD_FOPEN_READ);
	if (!f)
		return IMGTOOLERR_FILENOTFOUND;

	err = stream_crc(f, result);
	stream_close(f);
	return err;
}



UINT64 stream_fill(imgtool_stream *f, unsigned char b, UINT64 sz)
{
	UINT64 outsz;
	char buf[1024];

	outsz = 0;
	memset(buf, b, MIN(sz, sizeof(buf)));

	while(sz)
	{
		outsz += stream_write(f, buf, MIN(sz, sizeof(buf)));
		sz -= MIN(sz, sizeof(buf));
	}
	return outsz;
}



int stream_isreadonly(imgtool_stream *s)
{
	return s->write_protect;
}



UINT32 stream_putc(imgtool_stream *stream, char c)
{
	return stream_write(stream, &c, 1);
}



UINT32 stream_puts(imgtool_stream *stream, const char *s)
{
	return stream_write(stream, s, strlen(s));
}



UINT32 stream_printf(imgtool_stream *stream, const char *fmt, ...)
{
	va_list va;
	char buf[256];

	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);

	return stream_puts(stream, buf);
}
