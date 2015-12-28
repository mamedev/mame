// license:BSD-3-Clause
// copyright-holders:smf
#ifndef __RS232_TERMINAL_H__
#define __RS232_TERMINAL_H__

#include "rs232.h"
#include "machine/terminal.h"


class serial_terminal_device : public generic_terminal_device,
	public device_serial_interface,
	public device_rs232_port_interface
{
public:
	serial_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }

	DECLARE_WRITE_LINE_MEMBER(update_serial);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;
	virtual void send_key(UINT8 code) override;

private:
	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_startbits;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;

	UINT8 m_curr_key;
	bool m_key_valid;
};

extern const device_type SERIAL_TERMINAL;

#endif /* __RS232_TERMINAL_H__ */
