// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    dipty.h

    Device PTY interface

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIPTY_H__
#define __DIPTY_H__

class device_pty_interface : public device_interface
{
public:
		// construction/destruction
		device_pty_interface(const machine_config &mconfig, device_t &device);
		virtual ~device_pty_interface();

		bool open(void);
		void close(void);

		bool is_open(void) const;

		ssize_t read(UINT8 *rx_chars , size_t count);
		void write(UINT8 tx_char);

		bool is_slave_connected(void) const;

		const char *slave_name(void) const;

protected:
		osd_file *m_pty_master;
		std::string m_slave_name;
		bool m_opened;
};

// iterator
typedef device_interface_iterator<device_pty_interface> pty_interface_iterator;

#endif  /* __DIPTY_H__ */
