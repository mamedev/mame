// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "lpc-pit.h"

DEFINE_DEVICE_TYPE(LPC_PIT, lpc_pit_device, "lpc_pit", "LPC PIT")

ADDRESS_MAP_START(lpc_pit_device::map)
	AM_RANGE(0x40, 0x43) AM_READWRITE8(status_r, access_w,  0x00ffffff)
	AM_RANGE(0x40, 0x43) AM_WRITE8    (          control_w, 0xff000000)
	AM_RANGE(0x50, 0x53) AM_READWRITE8(status_r, access_w,  0x00ffffff)
	AM_RANGE(0x50, 0x53) AM_WRITE8    (          control_w, 0xff000000)
ADDRESS_MAP_END

lpc_pit_device::lpc_pit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: lpc_device(mconfig, LPC_PIT, tag, owner, clock)
{
}

void lpc_pit_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
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
	logerror("%s: status_r %d\n", tag(), offset);
	return 0xff;
}

WRITE8_MEMBER(lpc_pit_device::access_w)
{
	logerror("%s: access_w %d, %02x\n", tag(), offset, data);
}

WRITE8_MEMBER(lpc_pit_device::control_w)
{
	logerror("%s: control_w %02x\n", tag(), data);
}
