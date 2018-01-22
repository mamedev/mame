// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#ifndef MAME_BUS_RS232_IE15_H
#define MAME_BUS_RS232_IE15_H

#pragma once

#include "rs232.h"
#include "machine/ie15.h"


class ie15_terminal_device : public ie15_device,
	public device_rs232_port_interface
{
public:
	ie15_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { ie15_device::serial_rx_callback(state); }

	DECLARE_WRITE_LINE_MEMBER(update_serial);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_startbits;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
};

DECLARE_DEVICE_TYPE(SERIAL_TERMINAL_IE15, ie15_terminal_device)

#endif // MAME_BUS_RS232_IE15_H
