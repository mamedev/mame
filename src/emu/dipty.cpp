// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    dipty.h

    Device PTY interface

***************************************************************************/

#include "emu.h"
#include "osdcore.h"

device_pty_interface::device_pty_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "pty")
	, m_pty_master()
	, m_slave_name()
	, m_opened(false)
{
}

device_pty_interface::~device_pty_interface()
{
}

bool device_pty_interface::open()
{
	if (!m_opened)
	{
		if (osd_file::openpty(m_pty_master, m_slave_name) == osd_file::error::NONE)
		{
			m_opened = true;
		}
		else
		{
			m_opened = false;
			m_pty_master.reset();
		}
	}
	return m_opened;
}

void device_pty_interface::close()
{
	m_pty_master.reset();
	m_opened = false;
}

bool device_pty_interface::is_open() const
{
	return m_opened;
}

ssize_t device_pty_interface::read(UINT8 *rx_chars , size_t count) const
{
	std::uint32_t actual_bytes;
	if (m_opened && m_pty_master->read(rx_chars, 0, count, actual_bytes) == osd_file::error::NONE)
		return actual_bytes;
	else
		return -1;
}

void device_pty_interface::write(UINT8 tx_char) const
{
	std::uint32_t actual_bytes;
	if (m_opened)
		m_pty_master->write(&tx_char, 0, 1, actual_bytes);
}

bool device_pty_interface::is_slave_connected() const
{
	// TODO: really check for slave status
	return m_opened;
}

const char *device_pty_interface::slave_name() const
{
	return m_slave_name.c_str();
}
