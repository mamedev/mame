// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    stream.cpp

    Code for implementing Imgtool streams

***************************************************************************/

#include "stream.h"

#include "imgtool.h"

#include "corefile.h"
#include "ioprocs.h"
#include "unzip.h"

#include <cassert>
#include <cstdio>
#include <cstring>

#include <zlib.h> // for crc32


namespace imgtool {

namespace {

class stream_read_wrapper : public virtual util::random_read
{
public:
	stream_read_wrapper(stream::ptr &&stream, std::uint8_t filler) noexcept
		: m_stream(stream.release())
		, m_filler(filler)
		, m_close(true)
	{
		assert(m_stream);
	}

	stream_read_wrapper(stream &stream, std::uint8_t filler) noexcept
		: m_stream(&stream)
		, m_filler(filler)
		, m_close(false)
	{
	}

	virtual ~stream_read_wrapper()
	{
		if (m_close)
			delete m_stream;
	}

	virtual std::error_condition seek(std::int64_t offset, int whence) noexcept override
	{
		m_stream->seek(offset, whence);
		return std::error_condition();
	}

	virtual std::error_condition tell(std::uint64_t &result) noexcept override
	{
		result = m_stream->tell();
		return std::error_condition();
	}

	virtual std::error_condition length(std::uint64_t &result) noexcept override
	{
		result = m_stream->size();
		return std::error_condition();
	}

	virtual std::error_condition read(void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		actual = m_stream->read(buffer, length);
		if (actual < length)
			std::memset(reinterpret_cast<std::uint8_t *>(buffer) + actual, m_filler, length - actual);
		return std::error_condition();
	}

	virtual std::error_condition read_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::uint64_t const pos = m_stream->tell();
		m_stream->seek(offset, SEEK_SET);
		actual = m_stream->read(buffer, length);
		m_stream->seek(pos, SEEK_SET);
		if (actual < length)
			std::memset(reinterpret_cast<std::uint8_t *>(buffer) + actual, m_filler, length - actual);
		return std::error_condition();
	}

protected:
	stream *const m_stream;
	std::uint8_t m_filler;
	bool const m_close;
};


class stream_read_write_wrapper : public stream_read_wrapper, public util::random_read_write
{
public:
	using stream_read_wrapper::stream_read_wrapper;

	virtual std::error_condition finalize() noexcept override
	{
		return std::error_condition();
	}

	virtual std::error_condition flush() noexcept override
	{
		return std::error_condition();
	}

	virtual std::error_condition write(void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::uint64_t const pos = m_stream->tell();
		std::uint64_t size = m_stream->size();
		if (size < pos)
		{
			m_stream->seek(size, SEEK_SET);
			size += m_stream->fill(m_filler, pos - size);
		}
		actual = (size >= pos) ? m_stream->write(buffer, length) : 0U;
		return (actual == length) ? std::error_condition() : std::errc::io_error;
	}

	virtual std::error_condition write_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override
	{
		std::uint64_t const pos = m_stream->tell();
		std::uint64_t size = m_stream->size();
		if (offset > size)
		{
			m_stream->seek(size, SEEK_SET);
			size += m_stream->fill(m_filler, offset - size);
		}
		else
		{
			m_stream->seek(offset, SEEK_SET);
		}
		actual = (size >= offset) ? m_stream->write(buffer, length) : 0U;
		m_stream->seek(pos, SEEK_SET);
		return (actual == length) ? std::error_condition() : std::errc::io_error;
	}
};

} // anonymous namespace


util::random_read::ptr stream_read(stream::ptr &&s, std::uint8_t filler) noexcept
{
	util::random_read::ptr result;
	if (s)
		result.reset(new (std::nothrow) stream_read_wrapper(std::move(s), filler));
	return result;
}

util::random_read::ptr stream_read(stream &s, std::uint8_t filler) noexcept
{
	util::random_read::ptr result(new (std::nothrow) stream_read_wrapper(s, filler));
	return result;
}

util::random_read_write::ptr stream_read_write(stream::ptr &&s, std::uint8_t filler) noexcept
{
	util::random_read_write::ptr result;
	if (s)
		result.reset(new (std::nothrow) stream_read_write_wrapper(std::move(s), filler));
	return result;
}

util::random_read_write::ptr stream_read_write(stream &s, std::uint8_t filler) noexcept
{
	util::random_read_write::ptr result(new (std::nothrow) stream_read_write_wrapper(s, filler));
	return result;
}



//-------------------------------------------------
//  ctor
//-------------------------------------------------

stream::stream(bool wp)
	: imgtype(IMG_FILE)
	, write_protect(wp)
	, position(0)
	, filesize(0)
	, file()
	, buffer(nullptr)
{
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

stream::stream(bool wp, util::core_file::ptr &&f)
	: imgtype(IMG_FILE)
	, write_protect(wp)
	, position(0)
	, filesize(f->size())
	, file(std::move(f))
	, buffer(nullptr)
{
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

stream::stream(bool wp, std::size_t size)
	: imgtype(IMG_MEM)
	, write_protect(wp)
	, position(0)
	, filesize(size)
	, file()
	, buffer(reinterpret_cast<std::uint8_t *>(malloc(size)))
{
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

stream::stream(bool wp, std::size_t size, void *buf)
	: imgtype(IMG_MEM)
	, write_protect(wp)
	, position(0)
	, filesize(size)
	, file()
	, buffer(reinterpret_cast<std::uint8_t *>(buf))
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

stream::~stream()
{
	if (buffer)
		free(buffer);
}


//-------------------------------------------------
//  open_zip
//-------------------------------------------------

stream::ptr stream::open_zip(const std::string &zipname, const char *subname, int read_or_write)
{
	if (read_or_write)
		return stream::ptr();

	/* check to see if the file exists */
	FILE *f = fopen(zipname.c_str(), "r");
	if (!f)
		return stream::ptr();
	fclose(f);

	stream::ptr imgfile(new stream(true));

	imgfile->imgtype = IMG_MEM;

	util::archive_file::ptr z;
	util::archive_file::open_zip(zipname, z);
	if (!z)
		return stream::ptr();

	int zipent = z->first_file();
	while ((zipent >= 0) && subname && strcmp(subname, z->current_name().c_str()))
		zipent = z->next_file();
	if (zipent < 0)
		return stream::ptr();

	imgfile->filesize = z->current_uncompressed_length();
	imgfile->buffer = reinterpret_cast<std::uint8_t *>(malloc(z->current_uncompressed_length()));
	if (!imgfile->buffer)
		return stream::ptr();

	if (z->decompress(imgfile->buffer, z->current_uncompressed_length()))
		return stream::ptr();

	return imgfile;
}



//-------------------------------------------------
//  open
//-------------------------------------------------

stream::ptr stream::open(const std::string &filename, int read_or_write)
{
	static const uint32_t write_modes[] =
	{
		OPEN_FLAG_READ,
		OPEN_FLAG_WRITE | OPEN_FLAG_CREATE,
		OPEN_FLAG_READ | OPEN_FLAG_WRITE,
		OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE
	};
	stream::ptr s;
	char c;

	// maybe we are just a ZIP?
	if (core_filename_ends_with(filename, ".zip"))
		return open_zip(filename, nullptr, read_or_write);

	util::core_file::ptr f = nullptr;
	auto const filerr = util::core_file::open(filename, write_modes[read_or_write], f);
	if (filerr)
	{
		if (!read_or_write)
		{
			int const len = filename.size();

			// can't open the file; try opening ZIP files with other names
			std::vector<char> buf(len + 1);
			strcpy(&buf[0], filename.c_str());

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

		// ah well, it was worth a shot
		return stream::ptr();
	}

	stream::ptr imgfile(new stream(read_or_write ? false : true, std::move(f)));

	// normal file
	return imgfile;
}


//-------------------------------------------------
//  open_write_stream
//-------------------------------------------------

stream::ptr stream::open_write_stream(int size)
{
	stream::ptr imgfile(new stream(false, size));
	if (!imgfile->buffer)
		return stream::ptr();

	return imgfile;
}


//-------------------------------------------------
//  open_mem
//-------------------------------------------------

stream::ptr stream::open_mem(void *buf, size_t sz)
{
	stream::ptr imgfile(new stream(false, sz, buf));

	return imgfile;
}


//-------------------------------------------------
//  core_file
//-------------------------------------------------

util::core_file *stream::core_file()
{
	return (imgtype == IMG_FILE) ? file.get() : nullptr;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint32_t stream::read(void *buf, uint32_t sz)
{
	uint32_t result = 0;

	switch(imgtype)
	{
		case IMG_FILE:
			assert(sz == (uint32_t) sz);
			file->seek(position, SEEK_SET);
			result = file->read(buf, (uint32_t) sz);
			break;

		case IMG_MEM:
			/* do we have to limit sz? */
			if (sz > (filesize - position))
				sz = (uint32_t) (filesize - position);

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
//  write
//-------------------------------------------------

uint32_t stream::write(const void *buf, uint32_t sz)
{
	void *new_buffer;
	uint32_t result = 0;

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
						buffer = (uint8_t*)new_buffer;
						filesize = position + sz;
					}
				}

				/* do we have to limit sz? */
				if (sz > (filesize - position))
					sz = (uint32_t) (filesize - position);

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
//  size
//-------------------------------------------------

uint64_t stream::size() const
{
	return filesize;
}


//-------------------------------------------------
//  getptr
//-------------------------------------------------

void *stream::getptr()
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
//  seek
//-------------------------------------------------

int stream::seek(int64_t pos, int where)
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
		position = std::min(size(), uint64_t(pos));

	if (position < pos)
		fill('\0', pos - position);

	return 0;
}


//-------------------------------------------------
//  tell
//-------------------------------------------------

uint64_t stream::tell()
{
	return position;
}


//-------------------------------------------------
//  transfer
//-------------------------------------------------

uint64_t stream::transfer(stream &dest, stream &source, uint64_t sz)
{
	uint64_t result = 0;
	uint64_t readsz;
	char buf[1024];

	while(sz && (readsz = source.read(buf, std::min(sz, uint64_t(sizeof(buf))))))
	{
		dest.write(buf, readsz);
		sz -= readsz;
		result += readsz;
	}
	return result;
}


//-------------------------------------------------
//  transfer_all
//-------------------------------------------------

uint64_t stream::transfer_all(stream &dest, stream &source)
{
	return transfer(dest, source, source.size());
}


//-------------------------------------------------
//  crc
//-------------------------------------------------

int stream::crc(unsigned long *result)
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
//  file_crc
//-------------------------------------------------

int stream::file_crc(const char *fname, unsigned long *result)
{
	int err;
	stream::ptr f;

	f = stream::open(fname, OSD_FOPEN_READ);
	if (!f)
		return IMGTOOLERR_FILENOTFOUND;

	err = f->crc(result);
	return err;
}


//-------------------------------------------------
//  fill
//-------------------------------------------------

uint64_t stream::fill(unsigned char b, uint64_t sz)
{
	uint64_t outsz;
	char buf[1024];

	outsz = 0;
	memset(buf, b, (std::min<uint64_t>)(sz, sizeof(buf)));

	while (sz)
	{
		outsz += write(buf, (std::min<uint64_t>)(sz, sizeof(buf)));
		sz -= (std::min<uint64_t>)(sz, sizeof(buf));
	}
	return outsz;
}


//-------------------------------------------------
//  is_read_only
//-------------------------------------------------

bool stream::is_read_only()
{
	return write_protect;
}


//-------------------------------------------------
//  putc
//-------------------------------------------------

uint32_t stream::putc(char c)
{
	return write(&c, 1);
}


//-------------------------------------------------
//  puts
//-------------------------------------------------

uint32_t stream::puts(const char *s)
{
	return write(s, strlen(s));
}


//-------------------------------------------------
//  printf
//-------------------------------------------------

uint32_t stream::printf(const char *fmt, ...)
{
	va_list va;
	char buf[256];

	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);

	return puts(buf);
}

} // namespace imgtool
