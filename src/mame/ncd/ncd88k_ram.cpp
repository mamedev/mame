// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * NCD 88k-series X terminal RAM controller
 *
 * This device supports 3 slots of data RAM and 2 slots of code RAM. Each
 * slot may be fitted with up to 32MiB, addressed as up to two banks of
 * memory corresponding to the sides or ranks of the installed module.
 *
 * Control registers allow each bank of memory to be optionally mapped into the
 * code or data address spaces within 1M, 4M or 16M windows, with mirroring
 * where the installed memory is smaller than the window size.
 *
 * Slots, sides, banks and registers are assigned as follows:
 *
 *   Slot  Side  Type   Reg  Bank#  Offset
 *    0     0    data    0     0      18
 *          1    data    0     1      18
 *    1     0    data    1     2      16
 *          1    data    2     3      18
 *    2     0    data    3     4      18
 *          1    data    4     5      18
 *    3     0    code    5     6      16
 *          1    code    5     7      16
 *    4     0    code    6     8      16
 *          1    code    7     9      16
 *
 * The emulation uses a callback to query the amount of memory (in bytes)
 * installed in each slot. Memory is allocated or reallocated on device reset,
 * with 2M, 8M and 32M sizes corresponding to double-sided memory modules and
 * consuming two consecutive banks.
 *
 * Sources:
 *  - system firmware
 *
 * TODO:
 *  - verify offsets
 *  - use ram_device
 */

#include "emu.h"
#include "ncd88k_ram.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NCD88K_RAM, ncd88k_ram_device, "ncd88k_ram", "NCD 88K RAM controller")

ncd88k_ram_device::ncd88k_ram_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NCD88K_RAM, tag, owner, clock)
	, m_code(*this, finder_base::DUMMY_TAG, -1)
	, m_data(*this, finder_base::DUMMY_TAG, -1)
	, m_slot(*this, 0)
	, m_ctrl{}
	, m_bank{}
{
}

void ncd88k_ram_device::device_start()
{
	save_item(NAME(m_ctrl));
}

void ncd88k_ram_device::device_reset()
{
	// reset mapping
	std::ranges::fill(m_ctrl, 0);
	map();

	// allocate configured memory
	for (unsigned slot = 0; slot < std::size(m_slot); slot++)
	{
		u32 const size = m_slot[slot]() & 0x03f0'0000;

		// 2M, 8M and 32M modules have two sides
		if (size & 0x02a0'0000U)
		{
			// two half-size banks
			m_bank[slot * 2 + 0].alloc(size >> 1);
			m_bank[slot * 2 + 1].alloc(size >> 1);
		}
		else
		{
			// one full-size bank
			m_bank[slot * 2 + 0].alloc(size);
			m_bank[slot * 2 + 1].alloc(0);
		}
	}
}

void ncd88k_ram_device::ctrl_w(offs_t offset, u8 data)
{
	unsigned const reg = BIT(offset, 22, 3);
	u32 const val = BIT(offset, 0, 22);

	LOG("%s: ctrl_w reg %u data 0x%06x (window1 %u window0 %u offset 0x%x)\n",
		machine().describe_context(), reg, val, BIT(offset, 20, 2), BIT(offset, 18, 2), offset & 0x3'ffffU);

	if (m_ctrl[reg] != val)
	{
		m_ctrl[reg] = val;

		map();
	}
}

void ncd88k_ram_device::map()
{
	static constexpr u32 window_size[4] = { 0, 0x10'0000, 0x40'0000, 0x100'0000 };

	// unmap all
	m_code->unmap_readwrite(CODE_BASE, CODE_BASE + 0x03ff'ffff);
	m_data->unmap_readwrite(CODE_BASE, CODE_BASE + 0x03ff'ffff);
	m_data->unmap_readwrite(DATA_BASE, DATA_BASE + 0x05ff'ffff);

	for (unsigned reg = 0; reg < std::size(m_ctrl); reg++)
	{
		u32 const &ctrl = m_ctrl[reg];
		unsigned const ws0 = window_size[BIT(ctrl, 18, 2)];
		unsigned const ws1 = window_size[BIT(ctrl, 20, 2)];

		switch (reg)
		{
		case 0: // data: 2 banks
			map<false>(m_bank[0], DATA_BASE, ws0);
			map<false>(m_bank[1], DATA_BASE + ((ctrl & 0x3'ffffU) << 10), ws1);
			break;
		case 1: // data: 1 bank
			map<false>(m_bank[2], DATA_BASE + ((ctrl & 0x3'ffffU) << 10), ws1);
			break;
		case 2: // data: 1 bank, 16 bit offset
			map<false>(m_bank[3], DATA_BASE + ((ctrl & 0xffffU) << 10), ws1);
			break;
		case 3: // data: 1 bank
			map<false>(m_bank[4], DATA_BASE + ((ctrl & 0x3'ffffU) << 10), ws1);
			break;
		case 4: // data: 1 bank
			map<false>(m_bank[5], DATA_BASE + ((ctrl & 0x3'ffffU) << 10), ws1);
			break;

		case 5: // code: 2 banks, 16 bit offset
			map<true>(m_bank[6], CODE_BASE, ws0);
			map<true>(m_bank[7], CODE_BASE + ((ctrl & 0xffffU) << 10), ws1);
			break;
		case 6: // code: 1 bank, 16 bit offset
			map<true>(m_bank[8], CODE_BASE + ((ctrl & 0xffffU) << 10), ws1);
			break;
		case 7: // code: 1 bank, 16 bit offset
			map<true>(m_bank[9], CODE_BASE + ((ctrl & 0xffffU) << 10), ws1);
			break;
		}
	}
}

template <bool Code> void ncd88k_ram_device::map(bank &ram, offs_t base, u32 window_size)
{
	if (offs_t const size = std::min(ram.size, window_size))
	{
		offs_t const mirror = (ram.size < window_size) ? window_size - ram.size : 0;

		LOG("map base 0x%08x size 0x%08x window 0x%08x mirror 0x%08x\n", base, size, window_size, mirror);

		if (Code)
			m_code->install_ram(base, base + size - 1, mirror, ram.ptr.get());

		m_data->install_ram(base, base + size - 1, mirror, ram.ptr.get());
	}
}

void ncd88k_ram_device::bank::alloc(u32 size)
{
	if (this->size != size)
	{
		if (this->size)
			this->ptr.reset();

		if (size)
			this->ptr = std::make_unique<u32[]>(size >> 2);

		this->size = size;
	}
}
