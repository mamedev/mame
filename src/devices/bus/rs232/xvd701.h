// license:BSD-3-Clause
// copyright-holders:smf
#ifndef JVC_XV701_H_
#define JVC_XV701_H_

#include "rs232.h"

class jvc_xvd701_device : public device_t,
	public device_serial_interface,
	public device_rs232_port_interface
{
public:
	jvc_xvd701_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }
protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	static const int TIMER_RESPONSE = 1;

	void send_response();
	unsigned char sum(unsigned char *buffer, int length);

	unsigned char m_command[11];
	unsigned char m_response[11];
	int m_response_index;
	emu_timer *m_timer_response;
};

extern const device_type JVC_XVD701;

#endif
