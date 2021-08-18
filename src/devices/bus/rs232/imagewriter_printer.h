// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_IMAGEWRITER_PRINTER_H
#define MAME_BUS_IMAGEWRITER_PRINTER_H

#pragma once

#include "rs232.h"
#include "imagedev/printer.h"
#include "diserial.h"
#include "cpu/i8085/i8085.h"
#include "bus/a2bus/bitmap_printer.h"
#include "machine/i8155.h"
#include "machine/i8251.h"

class apple_imagewriter_printer_device : public device_t,
	public device_serial_interface,
	public device_rs232_port_interface
{
public:
	apple_imagewriter_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }

	DECLARE_WRITE_LINE_MEMBER(update_serial);

protected:
	apple_imagewriter_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;  // gotta be const and const!
	virtual void rcv_complete() override;
	int m_initial_rx_state;

	void mem_map(address_map &map);
	void io_map(address_map &map);

private:
	DECLARE_WRITE_LINE_MEMBER(printer_online);

	required_device<printer_image_device> m_printer;
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<i8155_device> m_8155head;
	required_device<i8155_device> m_8155switch;
	required_device<bitmap_printer_device> m_bitmap_printer;

	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;

	const int dpi = 144;
	const int PAPER_WIDTH = 8.5 * dpi;  // 8.5 inches wide
	const int PAPER_HEIGHT = 11 * dpi;   // 11  inches high
};

DECLARE_DEVICE_TYPE(APPLE_IMAGEWRITER_PRINTER, apple_imagewriter_printer_device)

#endif // MAME_BUS_IMAGEWRITER_PRINTER_H
