// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "cammu.h"

#define VERBOSE 0

// TODO: decode the cammu registers properly
DEVICE_ADDRESS_MAP_START(map, 32, cammu_device)
	AM_RANGE(0x000, 0xfff) AM_READWRITE(cammu_r, cammu_w)
ADDRESS_MAP_END

const device_type CAMMU = &device_creator<cammu_device>;

cammu_device::cammu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CAMMU, "Cache and MMU", tag, owner, clock, "cammu", __FILE__)
	, device_memory_interface(mconfig, *this),
	m_main_space_config("main", ENDIANNESS_LITTLE, 32, 32, 0),
	m_io_space_config("io", ENDIANNESS_LITTLE, 32, 32, 0),
	m_boot_space_config("boot", ENDIANNESS_LITTLE, 32, 32, 0),
	m_main_space(nullptr),
	m_io_space(nullptr),
	m_boot_space(nullptr)
{ }

void cammu_device::device_start()
{
	m_main_space = &space(AS_0);
	m_io_space = &space(AS_1);
	m_boot_space = &space(AS_2);
}

void cammu_device::device_reset()
{
}

const address_space_config *cammu_device::memory_space_config (address_spacenum spacenum) const
{
	switch (spacenum)
	{
	case AS_0: return &m_main_space_config;
	case AS_1: return &m_io_space_config;
	case AS_2: return &m_boot_space_config;
	}

	return nullptr;
}

READ32_MEMBER(cammu_device::mmu_r)
{
	// handle htlb
	u32 address = offset << 2;

	//if (m_maincpu->supervisor_mode() && (offset & ~0x1FFF) == 0)
	if ((offset & ~0x1FFF) == 0)
	{
		switch (offset & 0x3C00)
		{
		case 0x000:
		case 0x400:
		case 0x800:
		case 0xC00:
			return m_main_space->read_dword(offset << 2, mem_mask);

		case 0x1000:
		case 0x1400:
			return m_io_space->read_dword((offset & 0x7ff) << 2, mem_mask);

		case 0x1800:
		case 0x1C00:
			return m_boot_space->read_dword((offset & 0x7ff) << 2, mem_mask);
		}
	}

	// address with upper bytes 0 or 0x7f1
	if ((offset >> 22) == 0x00 || (offset >> 18) == 0x7f1)
		return m_main_space->read_dword(offset << 2, mem_mask);
	else
		return m_io_space->read_dword(offset << 2, mem_mask);
}

WRITE32_MEMBER(cammu_device::mmu_w)
{
	// handle htlb
	//if (m_maincpu->supervisor_mode() && (offset & ~0x1FFF) == 0)
	if ((offset & ~0x1FFF) == 0)
	{
		switch (offset & 0x3C00)
		{
		case 0x000:
		case 0x400:
		case 0x800:
		case 0xC00:
			// pages 0-3: main space
			m_main_space->write_dword(offset << 2, data, mem_mask);
			return;

		case 0x1000:
		case 0x1400:
			// pages 4-5: pages 0-1 i/o space
			m_io_space->write_dword((offset & 0x7ff) << 2, data, mem_mask);
			return;

		case 0x1800:
		case 0x1C00:
			// pages 6-7: pages 0-1 boot space
			m_boot_space->write_dword((offset & 0x7ff) << 2, data, mem_mask);
			return;
		}
	}

	// address with upper byte 0x00 or upper 3 bytes 0x7f1
	if ((offset >> 22) == 0x00 || (offset >> 18) == 0x7f1)
		m_main_space->write_dword(offset << 2, data, mem_mask);
	else
		m_io_space->write_dword(offset << 2, data, mem_mask);
}
