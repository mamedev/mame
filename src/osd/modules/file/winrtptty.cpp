// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb


#include "winrtfile.h"

#include "strconv.h"

#include <cassert>

#include <windows.h>
#include <cstdlib>


namespace {
char const *const winfile_ptty_identifier = "\\\\.\\pipe\\";


class win_osd_ptty : public osd_file
{
public:
	win_osd_ptty(win_osd_ptty const &) = delete;
	win_osd_ptty(win_osd_ptty &&) = delete;
	win_osd_ptty& operator=(win_osd_ptty const &) = delete;
	win_osd_ptty& operator=(win_osd_ptty &&) = delete;

	win_osd_ptty(HANDLE handle) : m_handle(handle)
	{
		assert(m_handle);
		assert(INVALID_HANDLE_VALUE != m_handle);
	}

	~win_osd_ptty()
	{
	}

	virtual error read(void *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		DWORD bytes_read;
		if (!ReadFile(m_handle, buffer, count, &bytes_read, nullptr))
			return win_error_to_file_error(GetLastError());

		actual = bytes_read;
		return error::NONE;
	}

	virtual error write(void const *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		DWORD bytes_written;
		if (!WriteFile(m_handle, buffer, count, &bytes_written, nullptr))
			return win_error_to_file_error(GetLastError());

		actual = bytes_written;
		return error::NONE;
	}

	virtual error truncate(std::uint64_t offset) override
	{
		// doesn't make sense for a PTTY
		return error::INVALID_ACCESS;
	}

	virtual error flush() override
	{
		// don't want to wait for client to read all data as implied by FlushFileBuffers
		return error::NONE;
	}

private:
	HANDLE m_handle;
};

} // anonymous namespace


bool win_check_ptty_path(std::string const &path)
{
	if (strncmp(path.c_str(), winfile_ptty_identifier, strlen(winfile_ptty_identifier)) == 0) return true;
	return false;
}


osd_file::error win_open_ptty(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize)
{
	return osd_file::error::ACCESS_DENIED;
}
