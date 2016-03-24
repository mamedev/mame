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
	typedef std::unique_ptr<imgtool_stream> ptr;

	imgtool_stream(bool wp)
		: imgtype(IMG_FILE)
		, write_protect(wp)
		, name(nullptr)
		, position(0)
		, filesize(0)
		, file()
		, buffer(nullptr)
	{
	}

	imgtool_stream(
			bool wp,
			util::core_file::ptr &&f)
		: imgtype(IMG_FILE)
		, write_protect(wp)
		, name(nullptr)
		, position(0)
		, filesize(f->size())
		, file(std::move(f))
		, buffer(nullptr)
	{
	}

	imgtool_stream(bool wp, std::size_t size)
		: imgtype(IMG_MEM)
		, write_protect(wp)
		, name(nullptr)
		, position(0)
		, filesize(size)
		, file()
		, buffer(reinterpret_cast<std::uint8_t *>(malloc(size)))
	{
	}

	imgtool_stream(bool wp, std::size_t size, void *buf)
		: imgtype(IMG_MEM)
		, write_protect(wp)
		, name(nullptr)
		, position(0)
		, filesize(size)
		, file()
		, buffer(reinterpret_cast<std::uint8_t *>(buf))
	{
	}

	~imgtool_stream()
	{
		free(buffer);
	}

	imgtype_t       imgtype;
	bool            write_protect;
	const char      *name; // needed for clear
	std::uint64_t   position;
	std::uint64_t   filesize;

	util::core_file::ptr file;
	std::uint8_t *buffer;
};



static imgtool_stream *stream_open_zip(const char *zipname, const char *subname, int read_or_write)
{
	if (read_or_write)
		return nullptr;

	/* check to see if the file exists */
	FILE *f = fopen(zipname, "r");
	if (!f)
		return nullptr;
	fclose(f);

	imgtool_stream::ptr imgfile(new imgtool_stream(true));

	imgfile->imgtype = IMG_MEM;

	util::archive_file::ptr z;
	util::archive_file::open_zip(zipname, z);
	if (!z)
		return nullptr;

	int zipent = z->first_file();
	while ((zipent >= 0) && subname && strcmp(subname, z->current_name().c_str()))
		zipent = z->next_file();
	if (zipent < 0)
		return nullptr;

	imgfile->filesize = z->current_uncompressed_length();
	imgfile->buffer = reinterpret_cast<std::uint8_t *>(malloc(z->current_uncompressed_length()));
	if (!imgfile->buffer)
		return nullptr;

	if (z->decompress(imgfile->buffer, z->current_uncompressed_length()) != util::archive_file::error::NONE)
		return nullptr;

	return imgfile.release();
}



imgtool_stream *stream_open(const char *fname, int read_or_write)
{
	static const UINT32 write_modes[] =
	{
		OPEN_FLAG_READ,
		OPEN_FLAG_WRITE | OPEN_FLAG_CREATE,
		OPEN_FLAG_READ | OPEN_FLAG_WRITE,
		OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE
	};
	imgtool_stream *s = nullptr;
	char c;

	/* maybe we are just a ZIP? */
	const char *ext = strrchr(fname, '.');
	if (ext && !core_stricmp(ext, ".zip"))
		return stream_open_zip(fname, nullptr, read_or_write);

	util::core_file::ptr f = nullptr;
	auto const filerr = util::core_file::open(fname, write_modes[read_or_write], f);
	if (filerr != osd_file::error::NONE)
	{
		if (!read_or_write)
		{
			int const len = strlen(fname);

			/* can't open the file; try opening ZIP files with other names */
			std::vector<char> buf(len + 1);
			strcpy(&buf[0], fname);

			for (int i = len-1; !s && (i >= 0); i--)
			{
				if ((buf[i] == '\\') || (buf[i] == '/'))
				{
					c = buf[i];
					buf[i] = '\0';
					s = stream_open_zip(&buf[0], &buf[i + 1], read_or_write);
					buf[i] = c;
				}
			}

			if (s)
				return s;
		}

		/* ah well, it was worth a shot */
		return nullptr;
	}

	imgtool_stream::ptr imgfile(new imgtool_stream(read_or_write ? false : true, std::move(f)));

	/* Normal file */
	imgfile->name = fname;
	return imgfile.release();
}



imgtool_stream *stream_open_write_stream(int size)
{
	imgtool_stream::ptr imgfile(new imgtool_stream(false, size));
	if (!imgfile->buffer)
		return nullptr;

	return imgfile.release();
}



imgtool_stream *stream_open_mem(void *buf, size_t sz)
{
	imgtool_stream::ptr imgfile(new imgtool_stream(false, sz, buf));

	return imgfile.release();
}



void stream_close(imgtool_stream *s)
{
	assert(s != nullptr);

	delete s;
}



util::core_file *stream_core_file(imgtool_stream *stream)
{
	return (stream->imgtype == IMG_FILE) ? stream->file.get() : nullptr;
}



UINT32 stream_read(imgtool_stream *stream, void *buf, UINT32 sz)
{
	UINT32 result = 0;

	switch(stream->imgtype)
	{
		case IMG_FILE:
			assert(sz == (UINT32) sz);
			stream->file->seek(stream->position, SEEK_SET);
			result = stream->file->read(buf, (UINT32) sz);
			break;

		case IMG_MEM:
			/* do we have to limit sz? */
			if (sz > (stream->filesize - stream->position))
				sz = (UINT32) (stream->filesize - stream->position);

			memcpy(buf, stream->buffer + stream->position, sz);
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
					if (s->buffer) free(s->buffer);
					new_buffer = malloc(s->position + sz);
					if (new_buffer)
					{
						s->buffer = (UINT8*)new_buffer;
						s->filesize = s->position + sz;
					}
				}

				/* do we have to limit sz? */
				if (sz > (s->filesize - s->position))
					sz = (UINT32) (s->filesize - s->position);

				memcpy(s->buffer + s->position, buf, sz);
				result = sz;
			}
			break;

		case IMG_FILE:
			s->file->seek(s->position, SEEK_SET);
			result = s->file->write(buf, sz);
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
			ptr = f->buffer;
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
			*result = crc32(0, (unsigned char *) s->buffer, (size_t) s->filesize);
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
