// license:BSD-3-Clause
// copyright-holders:smf

#include "emu.h"
#include "loopback.h"

DEFINE_DEVICE_TYPE(RS232_LOOPBACK, rs232_loopback_device, "rs232_loopback", "RS232 Loopback")

rs232_loopback_device::rs232_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RS232_LOOPBACK, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
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
