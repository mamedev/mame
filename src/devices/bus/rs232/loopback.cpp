// license:BSD-3-Clause
// copyright-holders:smf

#include "loopback.h"

const device_type RS232_LOOPBACK = &device_creator<rs232_loopback_device>;

rs232_loopback_device::rs232_loopback_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RS232_LOOPBACK, "RS232 Loopback", tag, owner, clock, "rs232_loopback", __FILE__),
	device_rs232_port_interface(mconfig, *this)
{
}

void rs232_loopback_device::device_start()
{
}

WRITE_LINE_MEMBER( rs232_loopback_device::input_txd )
{
	if (started())
	{
		output_rxd(state);
	}
}

WRITE_LINE_MEMBER( rs232_loopback_device::input_rts )
{
	if (started())
	{
		output_ri(state);
		output_cts(state);
	}
}

WRITE_LINE_MEMBER( rs232_loopback_device::input_dtr )
{
	if (started())
	{
		output_dsr(state);
		output_dcd(state);
	}
}
