// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb

#define WIN32_LEAN_AND_MEAN

#include "winfile.h"

#include "strconv.h"
#include "winutil.h"

#include <cassert>

#include <windows.h>
#include <stdlib.h>


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
		FlushFileBuffers(m_handle);
		DisconnectNamedPipe(m_handle);
		CloseHandle(m_handle);
	}

	virtual error read(void *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		DWORD bytes_read;
		if (!ReadFile(m_handle, buffer, count, &bytes_read, NULL))
			return win_error_to_file_error(GetLastError());

		actual = bytes_read;
		return error::NONE;
	}

	virtual error write(void const *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) override
	{
		DWORD bytes_written;
		if (!WriteFile(m_handle, buffer, count, &bytes_written, NULL))
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
	TCHAR *t_name = tstring_from_utf8(path.c_str());
	if (!t_name)
		return osd_file::error::OUT_OF_MEMORY;

	HANDLE pipe = CreateNamedPipe(t_name, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT, 1, 32, 32, 0, NULL);
	osd_free(t_name);

	if (INVALID_HANDLE_VALUE == pipe)
		return osd_file::error::ACCESS_DENIED;

	try
	{
		file = std::make_unique<win_osd_ptty>(pipe);
		filesize = 0;
		return osd_file::error::NONE;
	}
	catch (...)
	{
		CloseHandle(pipe);
		return osd_file::error::OUT_OF_MEMORY;
	}
}
