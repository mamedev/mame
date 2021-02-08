// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_RS232_TERMINAL_H
#define MAME_BUS_RS232_TERMINAL_H

#include "rs232.h"
#include "machine/terminal.h"


class serial_terminal_device : public generic_terminal_device,
	public device_buffered_serial_interface<16U>,
	public device_rs232_port_interface
{
public:
	serial_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { device_buffered_serial_interface::rx_w(state); }

	DECLARE_WRITE_LINE_MEMBER(update_serial);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_reset() override;

	virtual void tra_callback() override;
	virtual void send_key(uint8_t code) override;

private:
	virtual void received_byte(uint8_t byte) override;

	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
};

DECLARE_DEVICE_TYPE(SERIAL_TERMINAL, serial_terminal_device)

#endif // MAME_BUS_RS232_TERMINAL_H
