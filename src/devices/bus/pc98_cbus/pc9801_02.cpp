// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Conventional memory for vanilla PC-9801

**************************************************************************************************/

#include "emu.h"

#include "pc9801_02.h"

DEFINE_DEVICE_TYPE(PC9801_02_128KB, pc9801_02_128kb_device, "pc9801_02_128kb", "NEC PC-9801-02 conventional RAM")

pc9801_02_128kb_device::pc9801_02_128kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_02_128KB, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
{
}

void pc9801_02_128kb_device::device_start()
{
	m_ram.resize((128*1024) / 2);
}

void pc9801_02_128kb_device::device_reset()
{
}

void pc9801_02_128kb_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_bus->space(AS_PROGRAM).install_ram(0x000000, 0x01ffff, &m_ram[0]);
	}
}

DEFINE_DEVICE_TYPE(PC9801_02_256KB, pc9801_02_256kb_device, "pc9801_02_256kb", "NEC PC-9801-41 conventional RAM")

pc9801_02_256kb_device::pc9801_02_256kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_02_256KB, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
{
}

void pc9801_02_256kb_device::device_start()
{
	m_ram.resize((256*1024) / 2);
}

void pc9801_02_256kb_device::device_reset()
{
}

void pc9801_02_256kb_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_bus->space(AS_PROGRAM).install_ram(0x000000, 0x03ffff, &m_ram[0]);
	}
}


DEFINE_DEVICE_TYPE(PC9801_02_384KB, pc9801_02_384kb_device, "pc9801_02_384kb", "NEC PC-9801-02 x 1 + PC-9801-41 x 1 conventional RAMs")

pc9801_02_384kb_device::pc9801_02_384kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_02_384KB, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
{
}

void pc9801_02_384kb_device::device_start()
{
	m_ram.resize((384*1024) / 2);
}

void pc9801_02_384kb_device::device_reset()
{
}

void pc9801_02_384kb_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_bus->space(AS_PROGRAM).install_ram(0x000000, 0x05ffff, &m_ram[0]);
	}
}

DEFINE_DEVICE_TYPE(PC9801_02_512KB, pc9801_02_512kb_device, "pc9801_02_512kb", "NEC PC-9801-41 x 2 conventional RAMs")

pc9801_02_512kb_device::pc9801_02_512kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_02_512KB, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
{
}

void pc9801_02_512kb_device::device_start()
{
	m_ram.resize((512*1024) / 2);
}

void pc9801_02_512kb_device::device_reset()
{
}

void pc9801_02_512kb_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_bus->space(AS_PROGRAM).install_ram(0x000000, 0x07ffff, &m_ram[0]);
	}
}



DEFINE_DEVICE_TYPE(PC9801_02_640KB, pc9801_02_640kb_device, "pc9801_02_640kb", "NEC PC-9801-41 x 2 + PC-9801-02 x 1 conventional RAMs")

pc9801_02_640kb_device::pc9801_02_640kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_02_640KB, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
{
}

void pc9801_02_640kb_device::device_start()
{
	m_ram.resize((640*1024) / 2);
}

void pc9801_02_640kb_device::device_reset()
{
}

void pc9801_02_640kb_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_bus->space(AS_PROGRAM).install_ram(0x000000, 0x09ffff, &m_ram[0]);
	}
}

