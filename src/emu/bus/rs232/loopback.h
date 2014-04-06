// license:MAME
// copyright-holders:smf

#ifndef RS232_LOOPBACK_H_
#define RS232_LOOPBACK_H_

#include "bus/rs232/rs232.h"

class rs232_loopback_device : public device_t,
	public device_rs232_port_interface
{
public:
	rs232_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual WRITE_LINE_MEMBER( input_txd ) { output_rxd(state); }
	virtual WRITE_LINE_MEMBER( input_rts ) { output_ri(state); output_cts(state); }
	virtual WRITE_LINE_MEMBER( input_dtr ) { output_dsr(state); output_dcd(state); }

protected:
	virtual void device_start();
};

extern const device_type RS232_LOOPBACK;

#endif
