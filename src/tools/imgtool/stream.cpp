// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    stream.cpp

    Code for implementing Imgtool streams

***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "unzip.h"
#include "osdcore.h"
#include "imgtool.h"


//-------------------------------------------------
//	ctor
//-------------------------------------------------

imgtool::stream::stream(bool wp)
	: imgtype(IMG_FILE)
	, write_protect(wp)
	, name(nullptr)
	, position(0)
	, filesize(0)
	, file()
	, buffer(nullptr)
{
}


//-------------------------------------------------
//	ctor
//-------------------------------------------------

imgtool::stream::stream(bool wp, util::core_file::ptr &&f)
	: imgtype(IMG_FILE)
	, write_protect(wp)
	, name(nullptr)
	, position(0)
	, filesize(f->size())
	, file(std::move(f))
	, buffer(nullptr)
{
}


//-------------------------------------------------
//	ctor
//-------------------------------------------------

imgtool::stream::stream(bool wp, std::size_t size)
	: imgtype(IMG_MEM)
	, write_protect(wp)
	, name(nullptr)
	, position(0)
	, filesize(size)
	, file()
	, buffer(reinterpret_cast<std::uint8_t *>(malloc(size)))
{
}


//-------------------------------------------------
//	ctor
//-------------------------------------------------

imgtool::stream::stream(bool wp, std::size_t size, void *buf)
	: imgtype(IMG_MEM)
	, write_protect(wp)
	, name(nullptr)
	, position(0)
	, filesize(size)
	, file()
	, buffer(reinterpret_cast<std::uint8_t *>(buf))
{
}


//-------------------------------------------------
//	dtor
//-------------------------------------------------

imgtool::stream::~stream()
{
	if (buffer)
		free(buffer);
}


//-------------------------------------------------
//	open_zip
//-------------------------------------------------

imgtool::stream *imgtool::stream::open_zip(const char *zipname, const char *subname, int read_or_write)
{
	if (read_or_write)
		return nullptr;

	/* check to see if the file exists */
	FILE *f = fopen(zipname, "r");
	if (!f)
		return nullptr;
	fclose(f);

	imgtool::stream::ptr imgfile(new imgtool::stream(true));

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



//-------------------------------------------------
//	open
//-------------------------------------------------

imgtool::stream *imgtool::stream::open(const char *fname, int read_or_write)
{
	static const UINT32 write_modes[] =
	{
		OPEN_FLAG_READ,
		OPEN_FLAG_WRITE | OPEN_FLAG_CREATE,
		OPEN_FLAG_READ | OPEN_FLAG_WRITE,
		OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE
	};
	imgtool::stream *s = nullptr;
	char c;

	/* maybe we are just a ZIP? */
	const char *ext = strrchr(fname, '.');
	if (ext && !core_stricmp(ext, ".zip"))
		return open_zip(fname, nullptr, read_or_write);

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
					s = open_zip(&buf[0], &buf[i + 1], read_or_write);
					buf[i] = c;
				}
			}

			if (s)
				return s;
		}

		/* ah well, it was worth a shot */
		return nullptr;
	}

	imgtool::stream::ptr imgfile(new imgtool::stream(read_or_write ? false : true, std::move(f)));

	/* Normal file */
	imgfile->name = fname;
	return imgfile.release();
}


//-------------------------------------------------
//	open_write_stream
//-------------------------------------------------

imgtool::stream *imgtool::stream::open_write_stream(int size)
{
	imgtool::stream::ptr imgfile(new imgtool::stream(false, size));
	if (!imgfile->buffer)
		return nullptr;

	return imgfile.release();
}


//-------------------------------------------------
//	open_mem
//-------------------------------------------------

imgtool::stream *imgtool::stream::open_mem(void *buf, size_t sz)
{
	imgtool::stream::ptr imgfile(new imgtool::stream(false, sz, buf));

	return imgfile.release();
}


//-------------------------------------------------
//	core_file
//-------------------------------------------------

util::core_file *imgtool::stream::core_file()
{
	return (imgtype == IMG_FILE) ? file.get() : nullptr;
}


//-------------------------------------------------
//	read
//-------------------------------------------------

UINT32 imgtool::stream::read(void *buf, UINT32 sz)
{
	UINT32 result = 0;

	switch(imgtype)
	{
		case IMG_FILE:
			assert(sz == (UINT32) sz);
			file->seek(position, SEEK_SET);
			result = file->read(buf, (UINT32) sz);
			break;

		case IMG_MEM:
			/* do we have to limit sz? */
			if (sz > (filesize - position))
				sz = (UINT32) (filesize - position);

			memcpy(buf, buffer + position, sz);
			result = sz;
			break;

		default:
			assert(0);
			break;
	}
	position += result;
	return result;
}


//-------------------------------------------------
//	write
//-------------------------------------------------

UINT32 imgtool::stream::write(const void *buf, UINT32 sz)
{
	void *new_buffer;
	UINT32 result = 0;

	switch(imgtype)
	{
		case IMG_MEM:
			if (!write_protect)
			{
				/* do we have to expand the buffer? */
				if (filesize < position + sz)
				{
					/* try to expand the buffer */
					new_buffer = realloc(buffer , position + sz);
					if (new_buffer)
					{
						buffer = (UINT8*)new_buffer;
						filesize = position + sz;
					}
				}

				/* do we have to limit sz? */
				if (sz > (filesize - position))
					sz = (UINT32) (filesize - position);

				memcpy(buffer + position, buf, sz);
				result = sz;
			}
			break;

		case IMG_FILE:
			file->seek(position, SEEK_SET);
			result = file->write(buf, sz);
			break;

		default:
			assert(0);
			break;
	}

	/* advance the file pointer */
	position += result;

	/* did we grow the file */
	if (position > filesize)
		filesize = position;
	return result;
}


//-------------------------------------------------
//	size
//-------------------------------------------------

UINT64 imgtool::stream::size() const
{
	return filesize;
}


//-------------------------------------------------
//	getptr
//-------------------------------------------------

void *imgtool::stream::getptr()
{
	void *ptr;

	switch(imgtype)
	{
		case IMG_MEM:
			ptr = buffer;
			break;

		default:
			ptr = nullptr;
			break;
	}
	return ptr;
}


//-------------------------------------------------
//	seek
//-------------------------------------------------

int imgtool::stream::seek(INT64 pos, int where)
{
	switch(where)
	{
		case SEEK_CUR:
			pos += position;
			break;
		case SEEK_END:
			pos += size();
			break;
	}

	if (pos < 0)
		position = 0;
	else
		position = std::min(size(), UINT64(pos));

	if (position < pos)
		fill('\0', pos - position);

	return 0;
}


//-------------------------------------------------
//	tell
//-------------------------------------------------

UINT64 imgtool::stream::tell()
{
	return position;
}


//-------------------------------------------------
//	transfer
//-------------------------------------------------

UINT64 imgtool::stream::transfer(imgtool::stream &dest, imgtool::stream &source, UINT64 sz)
{
	UINT64 result = 0;
	UINT64 readsz;
	char buf[1024];

	while(sz && (readsz = source.read(buf, std::min(sz, UINT64(sizeof(buf))))))
	{
		dest.write(buf, readsz);
		sz -= readsz;
		result += readsz;
	}
	return result;
}


//-------------------------------------------------
//	transfer_all
//-------------------------------------------------

UINT64 imgtool::stream::transfer_all(imgtool::stream &dest, imgtool::stream &source)
{
	return transfer(dest, source, source.size());
}


//-------------------------------------------------
//	crc
//-------------------------------------------------

int imgtool::stream::crc(unsigned long *result)
{
	size_t sz;
	void *ptr;

	switch(imgtype)
	{
		case IMG_MEM:
			*result = crc32(0, (unsigned char *) buffer, (size_t) filesize);
			break;

		default:
			sz = size();
			ptr = malloc(sz);
			if (!ptr)
				return IMGTOOLERR_OUTOFMEMORY;
			seek(0, SEEK_SET);
			if (read(ptr, sz) != sz)
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


//-------------------------------------------------
//	file_crc
//-------------------------------------------------

int imgtool::stream::file_crc(const char *fname, unsigned long *result)
{
	int err;
	imgtool::stream *f;

	f = imgtool::stream::open(fname, OSD_FOPEN_READ);
	if (!f)
		return IMGTOOLERR_FILENOTFOUND;

	err = f->crc(result);
	delete f;
	return err;
}


//-------------------------------------------------
//	fill
//-------------------------------------------------

UINT64 imgtool::stream::fill(unsigned char b, UINT64 sz)
{
	UINT64 outsz;
	char buf[1024];

	outsz = 0;
	memset(buf, b, (std::min<UINT64>)(sz, sizeof(buf)));

	while (sz)
	{
		outsz += write(buf, (std::min<UINT64>)(sz, sizeof(buf)));
		sz -= (std::min<UINT64>)(sz, sizeof(buf));
	}
	return outsz;
}


//-------------------------------------------------
//	is_read_only
//-------------------------------------------------

int imgtool::stream::is_read_only()
{
	return write_protect;
}


//-------------------------------------------------
//	putc
//-------------------------------------------------

UINT32 imgtool::stream::putc(char c)
{
	return write(&c, 1);
}


//-------------------------------------------------
//	puts
//-------------------------------------------------

UINT32 imgtool::stream::puts(const char *s)
{
	return write(s, strlen(s));
}


//-------------------------------------------------
//	printf
//-------------------------------------------------

UINT32 imgtool::stream::printf(const char *fmt, ...)
{
	va_list va;
	char buf[256];

	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);

	return puts(buf);
}
