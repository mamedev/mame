// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    stream.h

    Code for implementing Imgtool streams

***************************************************************************/
#ifndef MAME_TOOLS_IMGTOOL_STREAM_H
#define MAME_TOOLS_IMGTOOL_STREAM_H

#pragma once

#include "utilfwd.h"

#include "osdcomm.h"

#include <cstdint>
#include <memory>
#include <string>


namespace imgtool {

class stream
{
public:
	typedef std::unique_ptr<stream> ptr;

	~stream();

	static imgtool::stream::ptr open(const std::string &filename, int read_or_write);  /* similar params to mame_fopen */
	static imgtool::stream::ptr open_write_stream(int filesize);
	static imgtool::stream::ptr open_mem(void *buf, size_t sz);

	util::core_file *core_file();
	uint32_t read(void *buf, uint32_t sz);
	uint32_t write(const void *buf, uint32_t sz);
	uint64_t size() const;
	int seek(int64_t pos, int where);
	uint64_t tell();
	void *getptr();
	uint32_t putc(char c);
	uint32_t puts(const char *s);
	uint32_t printf(const char *fmt, ...) ATTR_PRINTF(2, 3);

	// transfers sz bytes from source to dest
	static uint64_t transfer(imgtool::stream &dest, imgtool::stream &source, uint64_t sz);
	static uint64_t transfer_all(imgtool::stream &dest, imgtool::stream &source);

	// fills sz bytes with b
	uint64_t fill(unsigned char b, uint64_t sz);

	// returns the CRC of a file
	int crc(unsigned long *result);
	static int file_crc(const char *fname, unsigned long *result);

	// returns whether a stream is read only or not
	bool is_read_only();

private:
	enum imgtype_t
	{
		IMG_FILE,
		IMG_MEM
	};

	imgtype_t       imgtype;
	bool            write_protect;
	std::uint64_t   position;
	std::uint64_t   filesize;

	std::unique_ptr<util::core_file> file;
	std::uint8_t *buffer;

	// ctors
	stream(bool wp);
	stream(bool wp, std::unique_ptr<util::core_file> &&f);
	stream(bool wp, std::size_t size);
	stream(bool wp, std::size_t size, void *buf);

	// private methods
	static stream::ptr open_zip(const std::string &zipname, const char *subname, int read_or_write);
};


std::unique_ptr<util::random_read> stream_read(stream::ptr &&s, std::uint8_t filler) noexcept;
std::unique_ptr<util::random_read> stream_read(stream &s, std::uint8_t filler) noexcept;
std::unique_ptr<util::random_read_write> stream_read_write(stream::ptr &&s, std::uint8_t filler) noexcept;
std::unique_ptr<util::random_read_write> stream_read_write(stream &s, std::uint8_t filler) noexcept;

} // namespace imgtool

#endif // MAME_TOOLS_IMGTOOL_STREAM_H
