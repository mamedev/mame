// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"

#include "simm.h"

DEFINE_DEVICE_TYPE(PC9801_54_2MB,  pc9801_54_2mb_device,  "pc9801_54_2mb",  "PC-9801-54 2MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_54_4MB,  pc9801_54_4mb_device,  "pc9801_54_4mb",  "PC-9801-54 4MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_54_7MB,  pc9801_54_7mb_device,  "pc9801_54_7mb",  "PC-9801-54 7MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_54_8MB,  pc9801_54_8mb_device,  "pc9801_54_8mb",  "PC-9801-54 8MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_54_9MB,  pc9801_54_9mb_device,  "pc9801_54_9mb",  "PC-9801-54 9MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_54_15MB, pc9801_54_15mb_device, "pc9801_54_15mb", "PC-9801-54 15MB SIMM")

pc9801_54_2mb_device::pc9801_54_2mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_54_2MB, tag, owner, clock)
	, device_pc9801_54_interface(mconfig, *this)
{
}

void pc9801_54_2mb_device::device_start()
{
	m_ram.resize(0x200000 / 2);
	m_slot->install_ram(0x000000, 0x1fffff, &m_ram[0]);
}

pc9801_54_4mb_device::pc9801_54_4mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_54_4MB, tag, owner, clock)
	, device_pc9801_54_interface(mconfig, *this)
{
}

void pc9801_54_4mb_device::device_start()
{
	m_ram.resize(0x400000 / 2);
	m_slot->install_ram(0x000000, 0x3fffff, &m_ram[0]);
}


pc9801_54_7mb_device::pc9801_54_7mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_54_7MB, tag, owner, clock)
	, device_pc9801_54_interface(mconfig, *this)
{
}

void pc9801_54_7mb_device::device_start()
{
	m_ram.resize(0x700000 / 2);
	m_slot->install_ram(0x000000, 0x6fffff, &m_ram[0]);
}


pc9801_54_8mb_device::pc9801_54_8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_54_8MB, tag, owner, clock)
	, device_pc9801_54_interface(mconfig, *this)
{
}

void pc9801_54_8mb_device::device_start()
{
	m_ram.resize(0x800000 / 2);
	m_slot->install_ram(0x000000, 0x7fffff, &m_ram[0]);
}


pc9801_54_9mb_device::pc9801_54_9mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_54_9MB, tag, owner, clock)
	, device_pc9801_54_interface(mconfig, *this)
{
}

void pc9801_54_9mb_device::device_start()
{
	m_ram.resize(0x900000 / 2);
	m_slot->install_ram(0x000000, 0x8fffff, &m_ram[0]);
}


pc9801_54_15mb_device::pc9801_54_15mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_54_15MB, tag, owner, clock)
	, device_pc9801_54_interface(mconfig, *this)
{
}

void pc9801_54_15mb_device::device_start()
{
	m_ram.resize(0xf00000 / 2);
	m_slot->install_ram(0x000000, 0xefffff, &m_ram[0]);
}

