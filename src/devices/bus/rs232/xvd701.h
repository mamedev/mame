// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_RS232_XVD701_H
#define MAME_BUS_RS232_XVD701_H

#include "rs232.h"
#include "diserial.h"

class jvc_xvd701_device : public device_t,
		public device_serial_interface,
		public device_rs232_port_interface
{
public:
	jvc_xvd701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }
protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	static constexpr int TIMER_RESPONSE = 1;

	void send_response();
	unsigned char sum(unsigned char *buffer, int length);

	unsigned char m_command[11];
	unsigned char m_response[11];
	int m_response_index;
	emu_timer *m_timer_response;
};

DECLARE_DEVICE_TYPE(JVC_XVD701, jvc_xvd701_device)

#endif // MAME_BUS_RS232_XVD701_H
