// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

MIF-201 MIDI interface

Bundled with Micro Musician VA

References:
- https://mamedev.emulab.it/kale/fast/files/micromusician_va.jpg

TODO:
- evasive, needs manual, dip-switch sheet and PCB pictures;

**************************************************************************************************/

#include "emu.h"
#include "mif201.h"

DEFINE_DEVICE_TYPE(MIF201, mif201_device, "mif201", "Micro Musician VA MIF-201 MIDI Interface")

mif201_device::mif201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIF201, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_uart(*this, "uart%u", 1U)
	, m_pit(*this, "pit")
{
}

//void mif201_device::irq_out(int state)
//{
//  m_bus->int_w(2, state);
//}


void mif201_device::device_add_mconfig(machine_config &config)
{
	// TODO: unknown clocks
	I8251(config, m_uart[0], 1021800);

	I8251(config, m_uart[1], 1021800);

	PIT8253(config, m_pit, 1021800);
}

void mif201_device::device_start()
{
}

void mif201_device::device_reset()
{
}

void mif201_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &mif201_device::io_map);
	}
}

void mif201_device::io_map(address_map &map)
{
	// sheet claims i8251-1 having swapped ports compared to i8251-2 but micromus just uses
	// $e2d2 for control, assume typo.
	map(0xe2d0, 0xe2d3).lrw8(
		NAME([this](offs_t offset) { return m_uart[0]->read(offset); }),
		NAME([this](offs_t offset, u8 data) { m_uart[0]->write(offset, data); })
	).umask16(0x00ff);
	map(0xe4d0, 0xe4d3).lrw8(
		NAME([this](offs_t offset) { return m_uart[1]->read(offset); }),
		NAME([this](offs_t offset, u8 data) { m_uart[1]->write(offset, data); })
	).umask16(0x00ff);
	map(0xe6d0, 0xe6d3).lrw8(
		NAME([this](offs_t offset) { return m_pit->read(offset); }),
		NAME([this](offs_t offset, u8 data) { m_pit->write(offset, data); })
	).umask16(0x00ff);
	map(0xe7d0, 0xe7d3).lrw8(
		NAME([this](offs_t offset) { return m_pit->read(offset | 2); }),
		NAME([this](offs_t offset, u8 data) { m_pit->write(offset | 2, data); })
	).umask16(0x00ff);
}
