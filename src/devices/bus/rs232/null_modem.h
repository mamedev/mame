// license:BSD-3-Clause
// copyright-holders:smf,Carl
#ifndef MAME_BUS_RS232_NULL_MODEM_H
#define MAME_BUS_RS232_NULL_MODEM_H

#include "rs232.h"
#include "imagedev/bitbngr.h"
#include "diserial.h"

class null_modem_device : public device_t,
	public device_serial_interface,
	public device_rs232_port_interface
{
public:
	null_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }
	virtual WRITE_LINE_MEMBER( input_rts ) override { m_rts = state; }

	DECLARE_WRITE_LINE_MEMBER(update_serial);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	static constexpr int TIMER_POLL = 1;

	void queue();

	required_device<bitbanger_device> m_stream;

	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_startbits;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
	required_ioport m_flow;

	uint8_t m_input_buffer[1000];
	uint32_t m_input_count;
	uint32_t m_input_index;
	emu_timer *m_timer_poll;
	int m_rts;
};

DECLARE_DEVICE_TYPE(NULL_MODEM, null_modem_device)

#endif // MAME_BUS_RS232_NULL_MODEM_H
