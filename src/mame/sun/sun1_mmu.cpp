// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * This device emulates the Sun-1 memory management unit.
 *
 * Sources:
 *  - Sun-1 System Reference Manual, Draft Version 1.0, July 27, 1982, Sun Microsystems, Inc.
 *  - Sun 68000 Board User's Manual, Revision B, February 1983, Sun Microsystems Inc.
 *
 * TODO:
 *  - everything
 */

#include "emu.h"

#include "sun1_mmu.h"

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

enum segment_mask : u16
{
	SEGMENT_PPTR = 0x003f,
	SEGMENT_PROT = 0x0f00,
	SEGMENT_CTXT = 0xf000,
};
enum page_mask : u16
{
	PAGE_ADDR = 0x0fff,
	PAGE_TYPE = 0x3000, // 0=on-board memory, 1=nonexistent, 2=multibus memory, 3=multibus i/o
	PAGE_MOD  = 0x4000,
	PAGE_ACC  = 0x8000,
};
enum mode_mask : unsigned
{
	P_R   = 0x04, // read
	P_W   = 0x02, // write
	P_X   = 0x01, // execute

	P_RX  = P_R | P_X,
	P_RW  = P_R | P_W,
	P_RWX = P_R | P_W | P_X,
};

DEFINE_DEVICE_TYPE(SUN1MMU, sun1mmu_device, "sun1mmu", "Sun-1 MMU")

sun1mmu_device::sun1mmu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SUN1MMU, tag, owner, clock)
	, m_space{
		{*this, finder_base::DUMMY_TAG, 0},
		{*this, finder_base::DUMMY_TAG, 1},
		{*this, finder_base::DUMMY_TAG, 2},
		{*this, finder_base::DUMMY_TAG, 3},
	}
	, m_error(*this)
	, m_context(0)
	, m_segment{}
	, m_page{}
	, m_super(true)
{
}

void sun1mmu_device::device_start()
{
	save_item(NAME(m_context));
	save_item(NAME(m_segment));
	save_item(NAME(m_page));
	save_item(NAME(m_stall));
	save_item(NAME(m_super));

	m_space[0]->specific(m_cpu_mem);
	m_space[1]->specific(m_cpu_spc);
	m_space[2]->specific(m_bus_mem);
	m_space[3]->specific(m_bus_pio);
}

void sun1mmu_device::device_reset()
{
	m_stall = false;
}

static bool access(bool const super, unsigned const code, unsigned const mode)
{
	static constexpr unsigned protection[2][16] =
	{
		// user mode
		{
			0    , 0    , 0    , 0    , 0    , 0    , P_R  , P_R  ,
			P_RW , P_RW , P_RX , P_RWX, P_RX , P_RX , P_X  , P_RWX,
		},
		// supervisor mode
		{
			0    , P_X  , P_R  , P_RX , P_RW , P_RWX, P_R  , P_RW ,
			P_R  , P_RW , P_RW , P_RW , P_RX , P_RWX, P_RWX, P_RWX,
		},
	};

	return protection[super][code] & mode;
}

void sun1mmu_device::context_w(u16 data)
{
	LOG("context 0x%04x (%s)\n", data, machine().describe_context());

	m_context = data & 0xf000U;
}

u16 sun1mmu_device::segment_r(offs_t offset)
{
	return m_context | m_segment[m_context >> 12][BIT(offset, 14, 6)];
}

void sun1mmu_device::segment_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("segment[0x%x][0x%02x] 0x%04x (%s)\n", m_context >> 12, BIT(offset, 14, 6), data, machine().describe_context());

	m_segment[m_context >> 12][BIT(offset, 14, 6)] = data & 0x0fffU;
}

u16 sun1mmu_device::page_r(offs_t offset)
{
	u16 const segment = m_segment[m_context >> 12][BIT(offset, 14, 6)];

	return m_page[(segment & SEGMENT_PPTR) << 4 | BIT(offset, 10, 4)];
}

void sun1mmu_device::page_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const segment = m_segment[m_context >> 12][BIT(offset, 14, 6)];

	LOG("page[0x%03x] 0x%04x (%s)\n", (segment & SEGMENT_PPTR) << 4 | BIT(offset, 10, 4), data, machine().describe_context());

	m_page[(segment & SEGMENT_PPTR) << 4 | BIT(offset, 10, 4)] = data;
}

std::optional<std::pair<unsigned, offs_t>> sun1mmu_device::translate(offs_t const logical, unsigned const mode)
{
	// check for mapped address
	if (logical < 0x20'0000)
	{
		u16 const segment = m_segment[m_context >> 12][BIT(logical, 15, 6)];

		// check segment map error
		if (access(m_super, BIT(segment, 8, 4), mode) || machine().side_effects_disabled())
		{
			u16 &page = m_page[(segment & SEGMENT_PPTR) << 4 | BIT(logical, 11, 4)];

			// update reference and modify bits
			if (!machine().side_effects_disabled())
			{
				if (mode & P_W)
					page |= PAGE_ACC | PAGE_MOD;
				else
					page |= PAGE_ACC;
			}

			return std::pair<unsigned, offs_t>(BIT(page, 12, 2), (page & PAGE_ADDR) << 11 | BIT(logical, 0, 11));
		}
		else
			LOG("segment map error 0x%08x context %d segment 0x%04x mode %c (%s)\n",
				logical, m_context, segment, (mode == P_R) ? 'R' : (mode == P_W) ? 'W' : 'X', machine().describe_context());
	}
	else if (m_super || machine().side_effects_disabled())
		// unmapped supervisor access
		return std::pair<unsigned, offs_t>(0, logical);
	else
		LOG("system space error 0x%08x (%s)\n", logical, machine().describe_context());

	// protection error
	return std::nullopt;
}

template <bool Execute> u16 sun1mmu_device::mmu_read(offs_t logical, u16 mem_mask)
{
	u16 data = 0;

	if (m_stall)
	{
		m_error(MMU_DEFER);

		return data;
	}

	auto const t = translate(logical, Execute ? P_X : P_R);

	if (t.has_value())
	{
		auto const [type, physical] = t.value();
		u16 flags = 0;

		switch (type)
		{
		case 0:
			// on-board memory
			data = m_cpu_mem.read_word(physical, mem_mask);
			break;
		case 1:
			// nonexistent
			LOG("nonexistent_r 0x%08x translated 0x%08x (%s)\n", logical, physical, machine().describe_context());
			m_error(MMU_ERROR);
			break;
		case 2:
			// multibus memory
			std::tie(data, flags) = m_bus_mem.read_word_flags(physical, mem_mask);
			if (flags)
			{
				LOG("multibus_r mem 0x%08x translated 0x%08x (%s)\n", logical, physical, machine().describe_context());
				m_stall = true;
				m_error(MMU_DEFER);
			}
			break;
		case 3:
			// mutibus i/o
			std::tie(data, flags) = m_bus_pio.read_word_flags(physical, mem_mask);
			if (flags)
			{
				LOG("multibus_r i/o 0x%08x translated 0x%08x (%s)\n", logical, physical, machine().describe_context());
				m_stall = true;
				m_error(MMU_DEFER);
			}
			break;
		}
	}
	else
		m_error(MMU_ERROR);

	return data;
}

void sun1mmu_device::mmu_write(offs_t logical, u16 data, u16 mem_mask)
{
	if (m_stall)
	{
		m_error(MMU_DEFER);

		return;
	}

	auto const t = translate(logical, P_W);

	if (t.has_value())
	{
		auto const [type, physical] = t.value();

		switch (type)
		{
		case 0:
			// on-board memory
			m_cpu_mem.write_word(physical, data, mem_mask);
			break;
		case 1:
			// nonexistent
			LOG("nonexistent write 0x%08x translated 0x%08x (%s)\n", logical, physical, machine().describe_context());
			m_error(MMU_ERROR);
			break;
		case 2:
			// multibus memory
			if (m_bus_mem.write_word_flags(physical, data, mem_mask))
			{
				LOG("multibus_w mem 0x%08x translated 0x%08x (%s)\n", logical, physical, machine().describe_context());
				m_stall = true;
				m_error(MMU_DEFER);
			}
			break;
		case 3:
			// mutibus i/o
			if (m_bus_pio.write_word_flags(physical, data, mem_mask))
			{
				LOG("multibus_w i/o 0x%08x translated 0x%08x (%s)\n", logical, physical, machine().describe_context());
				m_stall = true;
				m_error(MMU_DEFER);
			}
			break;
		}
	}
	else
		m_error(MMU_ERROR);
}

u16 sun1mmu_device::read_program(offs_t logical, u16 mem_mask)
{
	return mmu_read<true>(logical, mem_mask);
}

void sun1mmu_device::write_program(offs_t logical, u16 data, u16 mem_mask)
{
	mmu_write(logical, data, mem_mask);
}

u16 sun1mmu_device::read_data(offs_t logical, u16 mem_mask)
{
	return mmu_read<false>(logical, mem_mask);
}

void sun1mmu_device::write_data(offs_t logical, u16 data, u16 mem_mask)
{
	mmu_write(logical, data, mem_mask);
}

u16 sun1mmu_device::read_cpu(offs_t logical, u16 mem_mask)
{
	return m_cpu_spc.read_word(logical, mem_mask);
}

void sun1mmu_device::set_super(bool super)
{
	m_super = super;
}
