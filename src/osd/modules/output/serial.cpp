// license:BSD-3-Clause
// copyright-holders:Nadav Weiss
/***************************************************************************

    serial.cpp

    Serial port output interface.

    Forwards MAME output notifications as "name = value\n" lines over a
    serial port.  Intended for driving external arcade hardware (ticket
    dispensers, coin hoppers, lamp boards) through a user-space bridge.

***************************************************************************/

#include "output_module.h"

#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"

#include "osdcore.h"
#include "strformat.h"

#include <cstddef>
#include <cstdint>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif


namespace osd {

namespace {

#if defined(_WIN32)

class serial_port
{
public:
	serial_port() : m_handle(INVALID_HANDLE_VALUE) { }
	~serial_port() { close(); }

	bool open(const std::string &port, unsigned baud)
	{
		// Windows requires "\\.\COMn" form for COM ports above COM9, and it
		// also works for COM1-COM9, so always use it.
		std::string path = "\\\\.\\" + port;
		m_handle = CreateFileA(
				path.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				0,
				nullptr,
				OPEN_EXISTING,
				0,
				nullptr);
		if (m_handle == INVALID_HANDLE_VALUE)
			return false;

		DCB dcb = {};
		dcb.DCBlength = sizeof(dcb);
		if (!GetCommState(m_handle, &dcb))
		{
			close();
			return false;
		}
		dcb.BaudRate = baud;
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.fBinary = TRUE;
		dcb.fParity = FALSE;
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
		if (!SetCommState(m_handle, &dcb))
		{
			close();
			return false;
		}

		// No blocking: zero write timeouts so a stuck host can't stall MAME.
		COMMTIMEOUTS timeouts = {};
		timeouts.ReadIntervalTimeout = MAXDWORD;
		timeouts.WriteTotalTimeoutConstant = 0;
		timeouts.WriteTotalTimeoutMultiplier = 0;
		SetCommTimeouts(m_handle, &timeouts);

		return true;
	}

	void close()
	{
		if (m_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
		}
	}

	bool write(const char *data, std::size_t length)
	{
		if (m_handle == INVALID_HANDLE_VALUE)
			return false;
		DWORD written = 0;
		return WriteFile(m_handle, data, DWORD(length), &written, nullptr) != 0;
	}

private:
	HANDLE m_handle;
};

#else

class serial_port
{
public:
	serial_port() : m_fd(-1) { }
	~serial_port() { close(); }

	bool open(const std::string &port, unsigned baud)
	{
		m_fd = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (m_fd < 0)
			return false;

		termios tio = {};
		if (tcgetattr(m_fd, &tio) != 0)
		{
			close();
			return false;
		}

		cfmakeraw(&tio);
		tio.c_cflag |= CLOCAL | CREAD;
		tio.c_cflag &= ~CRTSCTS;
		tio.c_cflag = (tio.c_cflag & ~CSIZE) | CS8;
		tio.c_cflag &= ~PARENB;
		tio.c_cflag &= ~CSTOPB;
		tio.c_iflag &= ~(IXON | IXOFF | IXANY);

		speed_t speed = baud_to_speed(baud);
		cfsetispeed(&tio, speed);
		cfsetospeed(&tio, speed);

		if (tcsetattr(m_fd, TCSANOW, &tio) != 0)
		{
			close();
			return false;
		}

		return true;
	}

	void close()
	{
		if (m_fd >= 0)
		{
			::close(m_fd);
			m_fd = -1;
		}
	}

	bool write(const char *data, std::size_t length)
	{
		if (m_fd < 0)
			return false;
		ssize_t written = ::write(m_fd, data, length);
		return written == ssize_t(length);
	}

private:
	static speed_t baud_to_speed(unsigned baud)
	{
		switch (baud)
		{
			case 1200:   return B1200;
			case 2400:   return B2400;
			case 4800:   return B4800;
			case 9600:   return B9600;
			case 19200:  return B19200;
			case 38400:  return B38400;
			case 57600:  return B57600;
			case 115200: return B115200;
			default:     return B9600;
		}
	}

	int m_fd;
};

#endif


class output_serial : public osd_module, public output_module
{
public:
	output_serial() :
		osd_module(OSD_OUTPUT_PROVIDER, "serial"),
		output_module()
	{
	}

	virtual int init(osd_interface &osd, const osd_options &options) override
	{
		const char *port = options.value(OSDOPTION_OUTPUT_SERIAL_PORT);
		if (port == nullptr || port[0] == '\0')
		{
			osd_printf_error("serial output: no port configured (set -%s)\n", OSDOPTION_OUTPUT_SERIAL_PORT);
			return -1;
		}

		unsigned baud = 9600;
		if (options.exists(OSDOPTION_OUTPUT_SERIAL_BAUD))
			baud = unsigned(options.int_value(OSDOPTION_OUTPUT_SERIAL_BAUD));

		if (!m_port.open(port, baud))
		{
			osd_printf_error("serial output: failed to open %s at %u baud\n", port, baud);
			return -1;
		}

		osd_printf_verbose("serial output: opened %s at %u baud\n", port, baud);
		return 0;
	}

	virtual void exit() override
	{
		m_port.close();
	}

	// output_module

	virtual void notify(const char *outname, int32_t value) override
	{
		auto msg = util::string_format("%s = %d\n", ((outname == nullptr) ? "none" : outname), value);
		m_port.write(msg.data(), msg.size());
	}

private:
	serial_port m_port;
};

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(OUTPUT_SERIAL, osd::output_serial)
