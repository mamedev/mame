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
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <cstdio>  // for fileno
#include <unistd.h> // for ftruncate


namespace {
class std_osd_file : public osd_file
{
public:

	std_osd_file(FILE *f) : m_file(f) { assert(m_file); }

	//============================================================
	//  osd_close
	//============================================================

	virtual ~std_osd_file() override
	{
		// close the file handle
		if (m_file) std::fclose(m_file);
	}

	//============================================================
	//  osd_read
	//============================================================

	virtual error read(void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) override
	{
		// seek to the new location; note that most fseek implementations are limited to 32 bits
		if (std::fseek(m_file, offset, SEEK_SET) < 0)
			return error::FAILURE;

		// perform the read
		std::size_t const count = std::fread(buffer, 1, length, m_file);
		actual = count;

		return error::NONE;
	}

	//============================================================
	//  osd_write
	//============================================================

	virtual error write(const void *buffer, std::uint64_t offset, std::uint32_t length, std::uint32_t &actual) override
	{
		// seek to the new location; note that most fseek implementations are limited to 32 bits
		if (std::fseek(m_file, offset, SEEK_SET) < 0)
			return error::FAILURE;

		// perform the write
		std::size_t const count = std::fwrite(buffer, 1, length, m_file);
		actual = count;

		return error::NONE;
	}

	//============================================================
	//  osd_truncate
	//============================================================

	error truncate(std::uint64_t offset) override
	{
		return (ftruncate(fileno(m_file), offset) < 0) ? error::FAILURE : error::NONE;
	}

	//============================================================
	//  osd_fflush
	//============================================================

	virtual error flush() override
	{
		return (std::fflush(m_file) == EOF) ? error::FAILURE : error::NONE;
	}

private:
	FILE *m_file;
};

} // anonymous namespace


//============================================================
//  osd_open
//============================================================

osd_file::error osd_file::open(std::string const &path, std::uint32_t openflags, ptr &file, std::uint64_t &filesize)
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
		return error::INVALID_ACCESS;

	// open the file
	FILE *const fileptr = std::fopen(path.c_str(), mode);
	if (!fileptr)
		return error::NOT_FOUND;

	// get the size -- note that most fseek/ftell implementations are limited to 32 bits
	long length;
	if ((std::fseek(fileptr, 0, SEEK_END) < 0) ||
		((length = std::ftell(fileptr)) < 0) ||
		(std::fseek(fileptr, 0, SEEK_SET) < 0))
	{
		std::fclose(fileptr);
		return error::FAILURE;
	}

	try
	{
		file = std::make_unique<std_osd_file>(fileptr);
		filesize = std::int64_t(length);
		return error::NONE;
	}
	catch (...)
	{
		std::fclose(fileptr);
		return error::OUT_OF_MEMORY;
	}
}


//============================================================
//  osd_openpty
//============================================================

osd_file::error osd_file::openpty(ptr &file, std::string &name)
{
	return error::FAILURE;
}


//============================================================
//  osd_rmfile
//============================================================

osd_file::error osd_file::remove(std::string const &filename)
{
	return (std::remove(filename.c_str()) < 0) ? error::FAILURE : error::NONE;
}


//============================================================
//  osd_get_physical_drive_geometry
//============================================================

bool osd_get_physical_drive_geometry(const char *filename, uint32_t *cylinders, uint32_t *heads, uint32_t *sectors, uint32_t *bps)
{
	// there is no standard way of doing this, so we always return false, indicating
	// that a given path is not a physical drive
	return false;
}


//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(char32_t *uchar, const char *osdchar, size_t count)
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

osd_file::error osd_get_full_path(std::string &dst, std::string const &path)
{
	// derive the full path of the file in an allocated string
	// for now just fake it since we don't presume any underlying file system
	dst = path;

	return osd_file::error::NONE;
}


//============================================================
//  osd_is_absolute_path
//============================================================

bool osd_is_absolute_path(std::string const &path)
{
	// assume no for everything
	return false;
}


//============================================================
//  osd_get_volume_name
//============================================================

const char *osd_get_volume_name(int idx)
{
	// we don't expose volumes
	return nullptr;
}
