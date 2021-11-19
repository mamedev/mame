// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    dipty.h

    Device PTY interface

***************************************************************************/
#ifndef MAME_EMU_DIPTY_H
#define MAME_EMU_DIPTY_H

#pragma once

#include <string>


class device_pty_interface : public device_interface
{
public:
	// construction/destruction
	device_pty_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pty_interface();

	bool open();
	void close();

	bool is_open() const;

	ssize_t read(u8 *rx_chars , size_t count) const;
	void write(u8 tx_char) const;

	bool is_slave_connected() const;

	const std::string &slave_name() const { return m_slave_name; }

protected:
	osd_file::ptr m_pty_master;
	std::string m_slave_name;
	bool m_opened;
};

// iterator
typedef device_interface_enumerator<device_pty_interface> pty_interface_enumerator;

#endif // MAME_EMU_DIPTY_H
