// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    dipty.h

    Device PTY interface

***************************************************************************/

#include "emu.h"
#include "osdcore.h"

device_pty_interface::device_pty_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "pty"),
		m_pty_master(NULL),
		m_slave_name(),
		m_opened(false)
{
}

device_pty_interface::~device_pty_interface()
{
}

bool device_pty_interface::open(void)
{
	if (!m_opened) {
		char buffer[ 128 ];

		if (osd_openpty(&m_pty_master , buffer , sizeof(buffer)) == FILERR_NONE) {
			m_opened = true;
			m_slave_name.assign(buffer);
		} else {
			m_opened = false;
			m_pty_master = NULL;
		}
	}

	return m_opened;
}

void device_pty_interface::close(void)
{
	if (m_opened) {
		osd_close(m_pty_master);
		m_opened = false;
	}
}

bool device_pty_interface::is_open(void) const
{
	return m_opened;
}

ssize_t device_pty_interface::read(UINT8 *rx_chars , size_t count) const
{
	UINT32 actual_bytes;

	if (m_opened && osd_read(m_pty_master, rx_chars, 0, count, &actual_bytes) == FILERR_NONE) {
		return actual_bytes;
	} else {
		return -1;
	}
}

void device_pty_interface::write(UINT8 tx_char) const
{
	UINT32 actual_bytes;

	if (m_opened) {
		osd_write(m_pty_master, &tx_char, 0, 1, &actual_bytes);
	}
}

bool device_pty_interface::is_slave_connected(void) const
{
	// TODO: really check for slave status
	return m_opened;
}

const char *device_pty_interface::slave_name(void) const
{
	return m_slave_name.c_str();
}
