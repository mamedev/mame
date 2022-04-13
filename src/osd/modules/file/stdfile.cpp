// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  minifile.c - Minimal core file access functions
//
//============================================================

#include "osdcore.h"
#include "osdfile.h"

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>

#include <stdio.h>  // for fileno
#include <unistd.h> // for ftruncate


namespace {

class std_osd_file : public osd_file
{
public:

	std_osd_file(FILE *f) noexcept : m_file(f)
	{
		assert(m_file);
	}

	//============================================================
	//  osd_close
	//============================================================

	virtual ~std_osd_file() override
	{
		// close the file handle
		if (m_file)
			std::fclose(m_file);
	}

	//============================================================
	//  osd_read
	//============================================================

	virtual std::error_condition read(void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) noexcept override
	{
		// seek to the new location; note that most fseek implementations are limited to the range of long int
		if (std::numeric_limits<long>::max() < offset)
			return std::errc::invalid_argument;
		if (std::fseek(m_file, offset, SEEK_SET) < 0)
			return std::error_condition(errno, std::generic_category());

		// perform the read
		std::size_t const count = std::fread(buffer, 1, length, m_file);
		if ((count < length) && std::ferror(m_file))
		{
			std::clearerr(m_file);
			return std::error_condition(errno, std::generic_category());
		}
		actual = count;

		return std::error_condition();
	}

	//============================================================
	//  osd_write
	//============================================================

	virtual std::error_condition write(const void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) noexcept override
	{
		// seek to the new location; note that most fseek implementations are limited to the range of long int
		if (std::numeric_limits<long>::max() < offset)
			return std::errc::invalid_argument;
		if (std::fseek(m_file, offset, SEEK_SET) < 0)
			return std::error_condition(errno, std::generic_category());

		// perform the write
		std::size_t const count = std::fwrite(buffer, 1, length, m_file);
		if (count < length)
		{
			std::clearerr(m_file);
			return std::error_condition(errno, std::generic_category());
		}
		actual = count;

		return std::error_condition();
	}

	//============================================================
	//  osd_truncate
	//============================================================

	virtual std::error_condition truncate(std::uint64_t offset) noexcept override
	{
		// this is present in POSIX but not C/C++
		if (::ftruncate(::fileno(m_file), offset) < 0)
			return std::error_condition(errno, std::generic_category());
		else
			return std::error_condition();
	}

	//============================================================
	//  osd_fflush
	//============================================================

	virtual std::error_condition flush() noexcept override
	{
		if (!std::fflush(m_file))
			return std::error_condition();
		else
			return std::error_condition(errno, std::generic_category());
	}

private:
	FILE *m_file;
};

} // anonymous namespace


//============================================================
//  osd_open
//============================================================

std::error_condition osd_file::open(std::string const &path, std::uint32_t openflags, ptr &file, std::uint64_t &filesize) noexcept
{
	// based on the flags, choose a mode
	const char *mode;
	if (openflags & OPEN_FLAG_WRITE)
	{
		if (openflags & OPEN_FLAG_READ)
			mode = (openflags & OPEN_FLAG_CREATE) ? "w+b" : "r+b";
		else
			mode = "wb";
	}
	else if (openflags & OPEN_FLAG_READ)
		mode = "rb";
	else
		return std::errc::invalid_argument;

	// open the file
	FILE *const fileptr = std::fopen(path.c_str(), mode);
	if (!fileptr)
		return std::error_condition(errno, std::generic_category());

	// get the size -- note that most fseek/ftell implementations are limited to 32 bits
	long length;
	if ((std::fseek(fileptr, 0, SEEK_END) < 0) ||
		((length = std::ftell(fileptr)) < 0) ||
		(std::fseek(fileptr, 0, SEEK_SET) < 0))
	{
		std::error_condition err(errno, std::generic_category());
		std::fclose(fileptr);
		return err;
	}

	osd_file::ptr result(new (std::nothrow) std_osd_file(fileptr));
	if (!result)
	{
		std::fclose(fileptr);
		return std::errc::not_enough_memory;
	}
	file = std::move(result);
	filesize = std::int64_t(length);
	return std::error_condition();
}


//============================================================
//  osd_openpty
//============================================================

std::error_condition osd_file::openpty(ptr &file, std::string &name) noexcept
{
	return std::errc::not_supported;
}


//============================================================
//  osd_rmfile
//============================================================

std::error_condition osd_file::remove(std::string const &filename) noexcept
{
	if (!std::remove(filename.c_str()))
		return std::error_condition();
	else
		return std::error_condition(errno, std::generic_category());
}


//============================================================
//  osd_get_physical_drive_geometry
//============================================================

bool osd_get_physical_drive_geometry(const char *filename, uint32_t *cylinders, uint32_t *heads, uint32_t *sectors, uint32_t *bps) noexcept
{
	// there is no standard way of doing this, so we always return false, indicating
	// that a given path is not a physical drive
	return false;
}


//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(char32_t *uchar, const char *osdchar, size_t count) noexcept
{
	// we assume a standard 1:1 mapping of characters to the first 256 unicode characters
	*uchar = (uint8_t)*osdchar;
	return 1;
}


//============================================================
//  osd_stat
//============================================================

osd_directory_entry *osd_stat(const std::string &path)
{
	osd_directory_entry *result = nullptr;

	// create an osd_directory_entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd_directory_entry
	result = (osd_directory_entry *)malloc(sizeof(*result) + path.length() + 1);
	strcpy((char *)(result + 1), path.c_str());
	result->name = (char *)(result + 1);
	result->type = ENTTYPE_NONE;
	result->size = 0;

	FILE *f = std::fopen(path.c_str(), "rb");
	if (f != nullptr)
	{
		std::fseek(f, 0, SEEK_END);
		result->type = ENTTYPE_FILE;
		result->size = std::ftell(f);
		std::fclose(f);
	}
	return result;
}


//============================================================
//  osd_get_full_path
//============================================================

std::error_condition osd_get_full_path(std::string &dst, std::string const &path) noexcept
{
	// derive the full path of the file in an allocated string
	// for now just fake it since we don't presume any underlying file system
	try { dst = path; }
	catch (...) { return std::errc::not_enough_memory; }

	return std::error_condition();
}


//============================================================
//  osd_is_absolute_path
//============================================================

bool osd_is_absolute_path(std::string const &path) noexcept
{
	// assume no for everything
	return false;
}


//============================================================
//  osd_get_volume_name
//============================================================

std::string osd_get_volume_name(int idx)
{
	// we don't expose volumes
	return std::string();
}


//============================================================
//  osd_get_volume_names
//============================================================

std::vector<std::string> osd_get_volume_names()
{
	// we don't expose volumes
	return std::vector<std::string>();
}
