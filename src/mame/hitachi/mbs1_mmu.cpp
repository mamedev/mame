// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Hitachi MB-S1 MMU support

6809 ties A15-A12 with an Address Mapper (74S189 x2) and a System/User physical switch (LS257 x2).

References:
http://haserin09.la.coocan.jp/os9_mmu.html#MB-S1

TODO:
- Default setups for Mark 5 vs. System vs. User modes.
- OS-9 card support
\- extra task switching LS174 + HM6148 x2 in place of the 74S189s (DIY override).
\- Handles 64 pages of memory models

**************************************************************************************************/

#include "emu.h"
#include "mbs1_mmu.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(MBS1_MMU, mbs1_mmu_device, "mbs1_mmu", "Hitachi MB-S1 MMU")

mbs1_mmu_device::mbs1_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MBS1_MMU, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("memory_map", ENDIANNESS_BIG, 8, 20, 0)
{
	std::fill(std::begin(m_bank_latch), std::end(m_bank_latch), 0);
}

mbs1_mmu_device::~mbs1_mmu_device()
{
}

device_memory_interface::space_config_vector mbs1_mmu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void mbs1_mmu_device::device_start()
{
	space().specific(m_space);

	save_pointer(NAME(m_bank_latch), 16);
}

void mbs1_mmu_device::device_reset()
{
}

// TODO: should not reset everything when in user mode
void mbs1_mmu_device::init_banks(bool system_type, bool user_mode)
{
	if (system_type == true)
	{
		for (int i = 0; i < 14; i++)
			m_bank_latch[i] = 0xf0 | i;
		m_bank_latch[0xe] = 0x84;
		m_bank_latch[0xf] = 0xef;
	}
	else
	{
		// Level 3 mode
		for (int i = 0; i < 16; i++)
			m_bank_latch[i] = 0xf0 | i;
	}
}

u8 mbs1_mmu_device::read(offs_t offset)
{
	return m_space.read_byte((offset & 0xfff) | (m_bank_latch[offset >> 12] << 12));
}

void mbs1_mmu_device::write(offs_t offset, u8 data)
{
	m_space.write_byte((offset & 0xfff) | (m_bank_latch[offset >> 12] << 12), data);
}

u8 mbs1_mmu_device::bank_r(offs_t offset)
{
	return m_bank_latch[offset];
}

void mbs1_mmu_device::bank_w(offs_t offset, u8 data)
{
	m_bank_latch[offset] = data;
}
