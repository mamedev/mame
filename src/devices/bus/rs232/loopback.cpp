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
		output_si(state);
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

DEFINE_DEVICE_TYPE(DEC_RS232_LOOPBACK, dec_rs232_loopback_device, "dec_rs232_loopback", "RS232 Loopback (DEC 12-15336-00)")

dec_rs232_loopback_device::dec_rs232_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DEC_RS232_LOOPBACK, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
{
}

void dec_rs232_loopback_device::device_start()
{
}

WRITE_LINE_MEMBER( dec_rs232_loopback_device::input_txd )
{
	// Pin 2 (Transmitted Data) connected to Pin 3 (Received Data) and Pin 15 (Transmission Clock)
	if (started())
	{
		output_rxd(state);
		output_txc(state);
	}
}

WRITE_LINE_MEMBER( dec_rs232_loopback_device::input_rts )
{
	// Pin 4 (Request to Send) connected to Pin 5 (Clear to Send) and Pin 8 (Carrier Detect)
	if (started())
	{
		output_cts(state);
		output_dcd(state);
	}
}

WRITE_LINE_MEMBER( dec_rs232_loopback_device::input_dtr )
{
	// Pin 20 (Data Terminal Ready) connected to Pin 6 (Data Set Ready) and 22 (Ring Indicator)
	if (started())
	{
		output_dsr(state);
		output_ri(state);
	}
}

WRITE_LINE_MEMBER( dec_rs232_loopback_device::input_spds )
{
	// Pin 19 (Speed Select) connected to Pin 12 (Speed Indicator) and 17 (Receive Clock)
	if (started())
	{
		output_si(state);
		output_rxc(state);
	}
}
