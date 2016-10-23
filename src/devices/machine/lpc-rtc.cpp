// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "lpc-rtc.h"

const device_type LPC_RTC = &device_creator<lpc_rtc_device>;

DEVICE_ADDRESS_MAP_START(map, 32, lpc_rtc_device)
	AM_RANGE(0x70, 0x77) AM_READWRITE8(index_r,     index_w,     0x00ff00ff)
	AM_RANGE(0x70, 0x77) AM_READWRITE8(target_r,    target_w,    0xff00ff00)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(extmap, 32, lpc_rtc_device)
	AM_RANGE(0x70, 0x77) AM_READWRITE8(extindex_r,  extindex_w,  0x00ff0000)
	AM_RANGE(0x70, 0x77) AM_READWRITE8(exttarget_r, exttarget_w, 0xff000000)
ADDRESS_MAP_END

lpc_rtc_device::lpc_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: lpc_device(mconfig, LPC_RTC, "LPC RTC", tag, owner, clock, "lpc_rtc", __FILE__), cur_index(0), cur_extindex(0)
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

uint8_t lpc_rtc_device::index_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return cur_index;
}

void lpc_rtc_device::index_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	cur_index = data & 0x7f;
}

uint8_t lpc_rtc_device::target_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return ram[cur_index];
}

void lpc_rtc_device::target_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	ram[cur_index] = data;
	logerror("%s: ram[%02x] = %02x\n", tag(), cur_index, data);
}

uint8_t lpc_rtc_device::extindex_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return cur_extindex;
}

void lpc_rtc_device::extindex_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	cur_extindex = data & 0x7f;
}

uint8_t lpc_rtc_device::exttarget_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return ram[cur_extindex|128];
}

void lpc_rtc_device::exttarget_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	ram[cur_extindex|128] = data;
	logerror("%s: ram[%02x] = %02x\n", tag(), cur_extindex|128, data);
}
