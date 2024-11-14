// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_RS232_PRINTER_H
#define MAME_BUS_RS232_PRINTER_H

#pragma once

#include "rs232.h"
#include "imagedev/printer.h"
#include "diserial.h"

class serial_printer_device : public device_t,
	public device_serial_interface,
	public device_rs232_port_interface
{
public:
	serial_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_txd(int state) override { device_serial_interface::rx_w(state); }

	void update_serial(int state);

protected:
	serial_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override;
	int m_initial_rx_state;

private:
	void printer_online(int state);

	required_device<printer_image_device> m_printer;

	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
};

class radio_shack_serial_printer_device : public serial_printer_device
{
public:
	radio_shack_serial_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(SERIAL_PRINTER, serial_printer_device)
DECLARE_DEVICE_TYPE(RADIO_SHACK_SERIAL_PRINTER, radio_shack_serial_printer_device)

#endif // MAME_BUS_RS232_PRINTER_H
