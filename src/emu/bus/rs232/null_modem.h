// license:BSD-3-Clause
// copyright-holders:smf,Carl
#ifndef NULL_MODEM_H_
#define NULL_MODEM_H_

#include "rs232.h"
#include "imagedev/bitbngr.h"

class null_modem_device : public device_t,
	public device_serial_interface,
	public device_rs232_port_interface
{
public:
	null_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual WRITE_LINE_MEMBER( input_txd ) { device_serial_interface::rx_w(state); }

	DECLARE_WRITE_LINE_MEMBER(update_serial);

protected:
	virtual ioport_constructor device_input_ports() const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_complete();

private:
	void queue();

	static const int TIMER_POLL = 1;

	required_device<bitbanger_device> m_stream;

	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_startbits;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;

	UINT8 m_input_buffer[1000];
	UINT32 m_input_count;
	UINT32 m_input_index;
	emu_timer *m_timer_poll;
};

extern const device_type NULL_MODEM;

#endif
