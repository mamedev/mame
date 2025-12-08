// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"

#include "simm.h"

DEFINE_DEVICE_TYPE(PC9801_61_2MB,  pc9801_61_2mb_device,  "pc9801_61_2mb",  "PC-9801-61 2MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_61_4MB,  pc9801_61_4mb_device,  "pc9801_61_4mb",  "PC-9801-61 4MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_61_8MB,  pc9801_61_8mb_device,  "pc9801_61_8mb",  "PC-9801-61 8MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_61_16MB, pc9801_61_16mb_device, "pc9801_61_16mb", "PC-9801-61 16MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_61_20MB, pc9801_61_20mb_device, "pc9801_61_20mb", "PC-9801-61 20MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_61_32MB, pc9801_61_32mb_device, "pc9801_61_32mb", "PC-9801-61 32MB SIMM")
DEFINE_DEVICE_TYPE(PC9801_61_64MB, pc9801_61_64mb_device, "pc9801_61_64mb", "PC-9801-61 64MB SIMM")

pc9801_61_2mb_device::pc9801_61_2mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_61_2MB, tag, owner, clock)
	, device_pc9801_61_interface(mconfig, *this)
{
}

void pc9801_61_2mb_device::device_start()
{
	m_ram.resize(0x200000 / 4);
	m_slot->install_ram(0x000000, 0x1fffff, &m_ram[0]);
}

pc9801_61_4mb_device::pc9801_61_4mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_61_4MB, tag, owner, clock)
	, device_pc9801_61_interface(mconfig, *this)
{
}

void pc9801_61_4mb_device::device_start()
{
	m_ram.resize(0x400000 / 4);
	m_slot->install_ram(0x000000, 0x3fffff, &m_ram[0]);
}

pc9801_61_8mb_device::pc9801_61_8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_61_8MB, tag, owner, clock)
	, device_pc9801_61_interface(mconfig, *this)
{
}

void pc9801_61_8mb_device::device_start()
{
	m_ram.resize(0x800000 / 4);
	m_slot->install_ram(0x000000, 0x7fffff, &m_ram[0]);
}

pc9801_61_16mb_device::pc9801_61_16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_61_16MB, tag, owner, clock)
	, device_pc9801_61_interface(mconfig, *this)
{
}

void pc9801_61_16mb_device::device_start()
{
	m_ram.resize(0x1000000 / 4);
	m_slot->install_ram(0x000000, 0xffffff, &m_ram[0]);
}

pc9801_61_20mb_device::pc9801_61_20mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_61_20MB, tag, owner, clock)
	, device_pc9801_61_interface(mconfig, *this)
{
}

void pc9801_61_20mb_device::device_start()
{
	m_ram.resize(0x1400000 / 4);
	m_slot->install_ram(0x000000, 0x13fffff, &m_ram[0]);
}

pc9801_61_32mb_device::pc9801_61_32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_61_32MB, tag, owner, clock)
	, device_pc9801_61_interface(mconfig, *this)
{
}

void pc9801_61_32mb_device::device_start()
{
	m_ram.resize(0x2000000 / 4);
	m_slot->install_ram(0x000000, 0x1ffffff, &m_ram[0]);
}

pc9801_61_64mb_device::pc9801_61_64mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_61_64MB, tag, owner, clock)
	, device_pc9801_61_interface(mconfig, *this)
{
}

void pc9801_61_64mb_device::device_start()
{
	m_ram.resize(0x4000000 / 4);
	m_slot->install_ram(0x000000, 0x3ffffff, &m_ram[0]);
}

