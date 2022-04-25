// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb


#include "winfile.h"

#include "strconv.h"
#include "winutil.h"

#include <cassert>

#include <windows.h>
#include <cstdlib>
#include <cstring>


namespace {

char const *const winfile_ptty_identifier = "\\\\.\\pipe\\";


class win_osd_ptty : public osd_file
{
public:
	win_osd_ptty(win_osd_ptty const &) = delete;
	win_osd_ptty(win_osd_ptty &&) = delete;
	win_osd_ptty& operator=(win_osd_ptty const &) = delete;
	win_osd_ptty& operator=(win_osd_ptty &&) = delete;

	win_osd_ptty(HANDLE handle) noexcept : m_handle(handle)
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

	virtual std::error_condition read(void *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) noexcept override
	{
		DWORD bytes_read;
		if (!ReadFile(m_handle, buffer, count, &bytes_read, nullptr))
			return win_error_to_file_error(GetLastError());

		actual = bytes_read;
		return std::error_condition();
	}

	virtual std::error_condition write(void const *buffer, std::uint64_t offset, std::uint32_t count, std::uint32_t &actual) noexcept override
	{
		DWORD bytes_written;
		if (!WriteFile(m_handle, buffer, count, &bytes_written, nullptr))
			return win_error_to_file_error(GetLastError());

		actual = bytes_written;
		return std::error_condition();
	}

	virtual std::error_condition truncate(std::uint64_t offset) noexcept override
	{
		// doesn't make sense for a PTTY
		return std::errc::bad_file_descriptor;
	}

	virtual std::error_condition flush() noexcept override
	{
		// don't want to wait for client to read all data as implied by FlushFileBuffers
		return std::error_condition();
	}

private:
	HANDLE m_handle;
};

} // anonymous namespace


bool win_check_ptty_path(std::string const &path) noexcept
{
	if (strncmp(path.c_str(), winfile_ptty_identifier, strlen(winfile_ptty_identifier)) == 0) return true;
	return false;
}


std::error_condition win_open_ptty(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept
{
	osd::text::tstring t_name;
	try { t_name = osd::text::to_tstring(path); }
	catch (...) { return std::errc::not_enough_memory; }

	HANDLE pipe = CreateFileW(t_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

	if (INVALID_HANDLE_VALUE == pipe)
	{
		if (openflags & OPEN_FLAG_CREATE)
			pipe = CreateNamedPipe(t_name.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT, 1, 32, 32, 0, nullptr);
		if (INVALID_HANDLE_VALUE == pipe)
			return win_error_to_file_error(GetLastError());
	}
	else
	{
		DWORD state = PIPE_NOWAIT;
		SetNamedPipeHandleState(pipe, &state, nullptr, nullptr);
		// TODO: check for error?
	}

	osd_file::ptr result(new (std::nothrow) win_osd_ptty(pipe));
	if (!result)
	{
		CloseHandle(pipe);
		return std::errc::not_enough_memory;
	}
	file = std::move(result);
	filesize = 0;
	return std::error_condition();
}
