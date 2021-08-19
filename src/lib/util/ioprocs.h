// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocs.h

    I/O interfaces

***************************************************************************/
#ifndef MAME_LIB_UTIL_IOPROCS_H
#define MAME_LIB_UTIL_IOPROCS_H

#pragma once

#include "utilfwd.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <system_error>


// FIXME: make a proper place for OSD forward declarations
class osd_file;


namespace util {

class read_stream
{
public:
	using ptr = std::unique_ptr<read_stream>;

	virtual ~read_stream() = default;

	virtual std::error_condition read(void *buffer, std::size_t length, std::size_t &actual) noexcept = 0;
};


class write_stream
{
public:
	using ptr = std::unique_ptr<write_stream>;

	virtual ~write_stream() = default;

	virtual std::error_condition finalize() noexcept = 0;
	virtual std::error_condition flush() noexcept = 0;
	virtual std::error_condition write(void const *buffer, std::size_t length, std::size_t &actual) noexcept = 0;
};


class read_write_stream : public virtual read_stream, public virtual write_stream
{
public:
	using ptr = std::unique_ptr<read_write_stream>;
};


class random_access
{
public:
	virtual ~random_access() = default;

	virtual std::error_condition seek(std::int64_t offset, int whence) noexcept = 0;
	virtual std::error_condition tell(std::uint64_t &result) noexcept = 0;
	virtual std::error_condition length(std::uint64_t &result) noexcept = 0;
};


class random_read : public virtual read_stream, public virtual random_access
{
public:
	using ptr = std::unique_ptr<random_read>;

	virtual std::error_condition read_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept = 0;
};


class random_write : public virtual write_stream, public virtual random_access
{
public:
	using ptr = std::unique_ptr<random_write>;

	virtual std::error_condition write_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept = 0;
};


class random_read_write : public read_write_stream, public virtual random_read, public virtual random_write
{
public:
	using ptr = std::unique_ptr<random_read_write>;
};


random_read::ptr ram_read(void const *data, std::size_t size) noexcept;
random_read::ptr ram_read(void const *data, std::size_t size, std::uint8_t filler) noexcept;
random_read::ptr ram_read_copy(void const *data, std::size_t size) noexcept;
random_read::ptr ram_read_copy(void const *data, std::size_t size, std::uint8_t filler) noexcept;

random_read::ptr stdio_read(FILE *file) noexcept;
random_read::ptr stdio_read(FILE *file, std::uint8_t filler) noexcept;
random_read::ptr stdio_read_noclose(FILE *file) noexcept;
random_read::ptr stdio_read_noclose(FILE *file, std::uint8_t filler) noexcept;

random_read_write::ptr stdio_read_write(FILE *file) noexcept;
random_read_write::ptr stdio_read_write(FILE *file, std::uint8_t filler) noexcept;
random_read_write::ptr stdio_read_write_noclose(FILE *file) noexcept;
random_read_write::ptr stdio_read_write_noclose(FILE *file, std::uint8_t filler) noexcept;

random_read::ptr osd_file_read(std::unique_ptr<osd_file> &&file) noexcept;
random_read::ptr osd_file_read(osd_file &file) noexcept;

random_read_write::ptr osd_file_read_write(std::unique_ptr<osd_file> &&file) noexcept;
random_read_write::ptr osd_file_read_write(osd_file &file) noexcept;

random_read::ptr core_file_read(std::unique_ptr<core_file> &&file) noexcept;
random_read::ptr core_file_read(std::unique_ptr<core_file> &&file, std::uint8_t filler) noexcept;
random_read::ptr core_file_read(core_file &file) noexcept;
random_read::ptr core_file_read(core_file &file, std::uint8_t filler) noexcept;

random_read_write::ptr core_file_read_write(std::unique_ptr<core_file> &&file) noexcept;
random_read_write::ptr core_file_read_write(std::unique_ptr<core_file> &&file, std::uint8_t filler) noexcept;
random_read_write::ptr core_file_read_write(core_file &file) noexcept;
random_read_write::ptr core_file_read_write(core_file &file, std::uint8_t filler) noexcept;

} // namespace util

#endif // MAME_LIB_UTIL_IOPROCS_H
