// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "lpc-pit.h"

const device_type LPC_PIT = &device_creator<lpc_pit_device>;

DEVICE_ADDRESS_MAP_START(map, 32, lpc_pit_device)
	AM_RANGE(0x40, 0x43) AM_READWRITE8(status_r, access_w,  0x00ffffff)
	AM_RANGE(0x40, 0x43) AM_WRITE8    (          control_w, 0xff000000)
	AM_RANGE(0x50, 0x53) AM_READWRITE8(status_r, access_w,  0x00ffffff)
	AM_RANGE(0x50, 0x53) AM_WRITE8    (          control_w, 0xff000000)
ADDRESS_MAP_END

lpc_pit_device::lpc_pit_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: lpc_device(mconfig, LPC_PIT, "LPC PIT", tag, owner, clock, "lpc_pit", __FILE__)
{
}

void lpc_pit_device::map_device(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	io_space->install_device(io_offset, io_window_end, *this, &lpc_pit_device::map);
}

void lpc_pit_device::device_start()
{
}

void lpc_pit_device::device_reset()
{
}

READ8_MEMBER( lpc_pit_device::status_r)
{
	logerror("%s: status_r %d\n", tag().c_str(), offset);
	return 0xff;
}

WRITE8_MEMBER(lpc_pit_device::access_w)
{
	logerror("%s: access_w %d, %02x\n", tag().c_str(), offset, data);
}

WRITE8_MEMBER(lpc_pit_device::control_w)
{
	logerror("%s: control_w %02x\n", tag().c_str(), data);
}
