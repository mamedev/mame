// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_RS232_LUAPRINTER_H
#define MAME_BUS_RS232_LUAPRINTER_H

#pragma once

#include "rs232.h"
#include "imagedev/printer.h"
#include "diserial.h"
#include "screen.h"
#include "luaprinter.h"

class serial_luaprinter_device : public device_t,
	public device_serial_interface,
	public device_rs232_port_interface,
	public luaprinter
{
public:
	serial_luaprinter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }

	DECLARE_WRITE_LINE_MEMBER(update_serial);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void rcv_complete() override;

private:
	DECLARE_WRITE_LINE_MEMBER(printer_online);

	required_device<printer_image_device> m_printer;
	required_device<screen_device> m_screen;

	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_startbits;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
	

    bitmap_rgb32 m_bitmap;

	const int xdpi=160;
	const int ydpi=72;
	const int PAPER_WIDTH = 8.5 * xdpi;  // 8.5 inches wide
	const int PAPER_HEIGHT = 11 * ydpi;  // 11  inches high
	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver	
};

DECLARE_DEVICE_TYPE(SERIAL_LUAPRINTER, serial_luaprinter_device)

#endif // MAME_BUS_RS232_LUAPRINTER_H
