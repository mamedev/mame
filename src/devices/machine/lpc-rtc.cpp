// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "lpc-rtc.h"

DEFINE_DEVICE_TYPE(LPC_RTC, lpc_rtc_device, "lpc_rpc", "LPC RTC")

void lpc_rtc_device::map(address_map &map)
{
	map(0x70, 0x77).rw(FUNC(lpc_rtc_device::index_r), FUNC(lpc_rtc_device::index_w)).umask32(0x00ff00ff);
	map(0x70, 0x77).rw(FUNC(lpc_rtc_device::target_r), FUNC(lpc_rtc_device::target_w)).umask32(0xff00ff00);
}

void lpc_rtc_device::extmap(address_map &map)
{
	map(0x70, 0x77).rw(FUNC(lpc_rtc_device::extindex_r), FUNC(lpc_rtc_device::extindex_w)).umask32(0x00ff0000);
	map(0x70, 0x77).rw(FUNC(lpc_rtc_device::exttarget_r), FUNC(lpc_rtc_device::exttarget_w)).umask32(0xff000000);
}

lpc_rtc_device::lpc_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: lpc_device(mconfig, LPC_RTC, tag, owner, clock), cur_index(0), cur_extindex(0)
{
}

void lpc_rtc_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(io_offset, io_window_end, *this, &lpc_rtc_device::map);
}

void lpc_rtc_device::map_extdevice(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(io_offset, io_window_end, *this, &lpc_rtc_device::extmap);
}

void lpc_rtc_device::device_start()
{
	memset(ram, 0, 256);
}

void lpc_rtc_device::device_reset()
{
}

READ8_MEMBER(  lpc_rtc_device::index_r)
{
	return cur_index;
}

WRITE8_MEMBER( lpc_rtc_device::index_w)
{
	cur_index = data & 0x7f;
}

READ8_MEMBER(  lpc_rtc_device::target_r)
{
	return ram[cur_index];
}

WRITE8_MEMBER( lpc_rtc_device::target_w)
{
	ram[cur_index] = data;
	logerror("%s: ram[%02x] = %02x\n", tag(), cur_index, data);
}

READ8_MEMBER(  lpc_rtc_device::extindex_r)
{
	return cur_extindex;
}

WRITE8_MEMBER( lpc_rtc_device::extindex_w)
{
	cur_extindex = data & 0x7f;
}

READ8_MEMBER(  lpc_rtc_device::exttarget_r)
{
	return ram[cur_extindex|128];
}

WRITE8_MEMBER( lpc_rtc_device::exttarget_w)
{
	ram[cur_extindex|128] = data;
	logerror("%s: ram[%02x] = %02x\n", tag(), cur_extindex|128, data);
}
