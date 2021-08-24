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
char const *const winfile_tty_identifier = "\\\\.\\COM";


class win_osd_tty : public osd_file
{
public:
	win_osd_tty(win_osd_tty const &) = delete;
	win_osd_tty(win_osd_tty &&) = delete;
	win_osd_tty& operator=(win_osd_tty const &) = delete;
	win_osd_tty& operator=(win_osd_tty &&) = delete;

	win_osd_tty(HANDLE handle) noexcept : m_handle(handle)
	{
		assert(m_handle);
		assert(INVALID_HANDLE_VALUE != m_handle);
	}

	~win_osd_tty()
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


bool win_check_tty_path(std::string const &path) noexcept
{
	if (strncmp(path.c_str(), winfile_tty_identifier, strlen(winfile_tty_identifier)) == 0) return true;
	return false;
}


std::error_condition win_open_tty(std::string const &path, std::uint32_t openflags, osd_file::ptr &file, std::uint64_t &filesize) noexcept
{
	osd::text::tstring t_name;
	try { t_name = osd::text::to_tstring(path); }
	catch (...) { return std::errc::not_enough_memory; }

	DCB dcb;

	HANDLE hSerial = CreateFileW(t_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

	if (INVALID_HANDLE_VALUE == hSerial)
	{
		return win_error_to_file_error(GetLastError());
	}
	memset(&dcb, 0, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	if (!GetCommState(hSerial, &dcb)) 
	{
		return win_error_to_file_error(GetLastError());
	}
	dcb.BaudRate        = 2000000;       //  Bit rate. Don't care for serial over USB.
	dcb.ByteSize        = 8;             //  Data size, xmit and rcv
	dcb.Parity          = NOPARITY;      //  Parity bit
	dcb.StopBits        = ONESTOPBIT;    //  Stop bit
	dcb.fOutX           = FALSE;
	dcb.fInX            = FALSE;
	dcb.fOutxCtsFlow    = FALSE;
	dcb.fOutxDsrFlow    = FALSE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fRtsControl     = RTS_CONTROL_ENABLE;
	dcb.fDtrControl     = DTR_CONTROL_ENABLE;
	if (!SetCommState(hSerial, &dcb)) 
	{
		return win_error_to_file_error(GetLastError());
	}

	osd_file::ptr result(new (std::nothrow) win_osd_tty(hSerial));
	if (!result)
	{
		CloseHandle(hSerial);
		return std::errc::not_enough_memory;
	}
	file = std::move(result);
	filesize = 0;
	return std::error_condition();
}
