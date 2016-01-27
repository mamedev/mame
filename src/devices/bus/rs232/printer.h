// license:BSD-3-Clause
// copyright-holders:smf
#ifndef __RS232_PRINTER_H__
#define __RS232_PRINTER_H__

#pragma once

#include "rs232.h"
#include "imagedev/printer.h"

class serial_printer_device : public device_t,
	public device_serial_interface,
	public device_rs232_port_interface
{
public:
	serial_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }

	DECLARE_WRITE_LINE_MEMBER(update_serial);
	DECLARE_WRITE_LINE_MEMBER(printer_online);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void rcv_complete() override;

private:
	required_device<printer_image_device> m_printer;

	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_startbits;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
};

extern const device_type SERIAL_PRINTER;

#endif
