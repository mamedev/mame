// license:BSD-3-Clause
// copyright-holders:smf

#ifndef RS232_LOOPBACK_H_
#define RS232_LOOPBACK_H_

#include "rs232.h"

class rs232_loopback_device : public device_t,
	public device_rs232_port_interface
{
public:
	rs232_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_txd(int state) override;
	virtual void input_rts(int state) override;
	virtual void input_dtr(int state) override;

protected:
	virtual void device_start() override;
};

extern const device_type RS232_LOOPBACK;

#endif
