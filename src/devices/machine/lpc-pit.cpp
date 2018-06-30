// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "lpc-pit.h"

DEFINE_DEVICE_TYPE(LPC_PIT, lpc_pit_device, "lpc_pit", "LPC PIT")

void lpc_pit_device::map(address_map &map)
{
	map(0x40, 0x42).rw(FUNC(lpc_pit_device::status_r), FUNC(lpc_pit_device::access_w));
	map(0x43, 0x43).w(FUNC(lpc_pit_device::control_w));
	map(0x50, 0x52).rw(FUNC(lpc_pit_device::status_r), FUNC(lpc_pit_device::access_w));
	map(0x53, 0x53).w(FUNC(lpc_pit_device::control_w));
}

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
