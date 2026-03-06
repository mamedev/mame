// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * This device emulates the SGI PM2 memory management unit.
 *
 * Sources:
 *  -
 *
 * TODO:
 *  - multibus stall
 */

#include "emu.h"

#include "pm2_mmu.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

/*
 * PROTMAP: udmM pPPP cccc cccc (binary)
 *  u = used, d = dirty
 *  mM = mem type: 00->Multibus I/O, 01->Multibus memory,
 *             10->page fault,   11->local RAM.
 *  PPP = protection, cccccccc = context
 *
 * protection codes are:    rwx supervisor, rwx user:
 *   0  ------  No access
 *   1  r-x---
 *   2  rwx---
 *   3  rwx--x
 *   4  rwxr-x
 *   5  rwxrwx
 *   6  ------  reserved for future use
 *   7  ------  like maybe the context doesn't need to match?
 */

enum prot_mask : u16
{
	PROT_CONTEXT = 0x00ff,
	PROT_0       = 0x0000, // no access
	PROT_1       = 0x0100, // r-x---
	PROT_2       = 0x0200, // rwx---
	PROT_3       = 0x0300, // rwx--x
	PROT_4       = 0x0400, // rwxr-x
	PROT_5       = 0x0500, // rwxrwx
	PROT_6       = 0x0600, // reserved
	PROT_7       = 0x0700, // reserved
	PROT_IO      = 0x1000, // 0=memory, 1=I/O
	PROT_OB      = 0x2000, // 0=Multibus, 1=on-board
	PROT_DIRTY   = 0x4000,
	PROT_USED    = 0x8000,
};

static constexpr u8 R = 1U << device_memory_interface::TR_READ;
static constexpr u8 W = 1U << device_memory_interface::TR_WRITE;
static constexpr u8 X = 1U << device_memory_interface::TR_FETCH;

static u8 constexpr access[2][8] =
{
	{      0,     0,     0,     X, R  |X, R|W|X,     0,     0 }, // user
	{      0, R  |X, R|W|X, R|W|X, R|W|X, R|W|X,     0,     0 }, // supervisor
};

DEFINE_DEVICE_TYPE(SGI_PM2_MMU, pm2_mmu_device, "sgi_pm2_mmu", "SGI PM2 MMU")

pm2_mmu_device::pm2_mmu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_PM2_MMU, tag, owner, clock)
	, m_space{
		{*this, finder_base::DUMMY_TAG, 0},
		{*this, finder_base::DUMMY_TAG, 1},
		{*this, finder_base::DUMMY_TAG, 2},
		{*this, finder_base::DUMMY_TAG, 3},
	}
	, m_error(*this)
	, m_page(nullptr)
	, m_prot(nullptr)
	, m_context(0)
{
}

void pm2_mmu_device::device_start()
{
	m_page = std::make_unique<u16[]>(4096); // IMS1420P55 4Kx4 SRAM (x3)
	m_prot = std::make_unique<u16[]>(4096); // IMS1420P55 4Kx4 SRAM (x4)

	save_pointer(NAME(m_page), 4096);
	save_pointer(NAME(m_prot), 4096);

	save_item(NAME(m_context));
	save_item(NAME(m_super));
	save_item(NAME(m_boot));

	m_space[0]->specific(m_bus_mem);
	m_space[1]->specific(m_bus_pio);
	m_space[2]->specific(m_cpu_mem);
	m_space[3]->specific(m_cpu_spc);
}

void pm2_mmu_device::device_reset()
{
	m_super = true;
	m_boot = true;
}

u16 pm2_mmu_device::context_r()
{
	return m_context;
}
void pm2_mmu_device::context_w(u16 data)
{
	LOG("%s: context_w 0x%04x\n", machine().describe_context(), data);
	m_context = data;
}

u16 pm2_mmu_device::prot_r(offs_t offset)
{
	return m_prot[offset];
}

void pm2_mmu_device::prot_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("%s: prot_w[0x%03x] 0x%04x\n", machine().describe_context(), offset, data);
	m_prot[offset] = data;
}

u16 pm2_mmu_device::page_r(offs_t offset)
{
	return m_page[offset];
}

void pm2_mmu_device::page_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("%s: page_w[0x%03x] 0x%04x\n", machine().describe_context(), offset, data);
	m_page[offset] = data & 0x0fff;
}

void pm2_mmu_device::boot_w(int state)
{
	m_boot = !state;
}

std::optional<std::pair<unsigned, offs_t>> pm2_mmu_device::translate(offs_t const logical, int const intention)
{
	if (logical < 0xf8'0000)
	{
		if (m_boot)
			return std::pair<unsigned, offs_t>(2, logical | 0xf8'0000);

		u16 const prot = m_prot[logical >> 12];
		u16 const page = m_page[logical >> 12];

		if (!machine().side_effects_disabled())
		{
			// check context
			if ((prot & PROT_CONTEXT) != m_context)
				return std::nullopt;

			// check protection
			if (!BIT(access[m_super][BIT(prot, 8, 3)], intention))
				return std::nullopt;

			// update reference and modify bits
			if (intention == device_memory_interface::TR_WRITE)
				m_prot[logical >> 12] |= PROT_USED | PROT_DIRTY;
			else
				m_prot[logical >> 12] |= PROT_USED;
		}

		return std::pair<unsigned, offs_t>(BIT(prot, 12, 2), u32(page) << 12 | (logical & 0xfff));
	}
	else if (m_super || machine().side_effects_disabled())
		// unmapped supervisor access
		return std::pair<unsigned, offs_t>(2, logical);

	// protection error
	return std::nullopt;
}

bool pm2_mmu_device::translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	if (spacenum == m68000_base_device::AS_CPU_SPACE)
	{
		target_space = m_space[3];
		return true;
	}

	if (spacenum != AS_PROGRAM)
		return false;

	auto const t = translate(address, intention);
	if (!t.has_value())
		return false;

	auto const [type, physical] = t.value();
	if (type == 3)
		return false;

	address = physical;
	target_space = m_space[type];
	return true;
}

template <bool Execute> u16 pm2_mmu_device::mmu_read(offs_t logical, u16 mem_mask)
{
	u16 data = 0;

	auto const t = translate(logical, Execute ? device_memory_interface::TR_FETCH : device_memory_interface::TR_READ);

	if (t.has_value())
	{
		auto const [type, physical] = t.value();
		u16 flags = 0;

		switch (type)
		{
		case 0:
			// Multibus memory
			std::tie(data, flags) = m_bus_mem.read_word_flags(physical, mem_mask);
			if (flags)
			{
				LOG("%s: multibus_r mem 0x%08x translated 0x%08x\n", machine().describe_context(), logical, physical);
				m_error(MMU_ERROR);
			}
			break;
		case 1:
			// Multibus I/O
			std::tie(data, flags) = m_bus_pio.read_word_flags(physical, mem_mask);
			if (flags)
			{
				LOG("%s: multibus_r i/o 0x%08x translated 0x%08x\n", machine().describe_context(), logical, physical);
				m_error(MMU_ERROR);
			}
			break;
		case 2:
			// on-board memory
			data = m_cpu_mem.read_word(physical, mem_mask);
			break;
		case 3:
			// nonexistent
			LOG("%s: nonexistent_r 0x%08x translated 0x%08x\n", machine().describe_context(), logical, physical);
			m_error(MMU_FAULT);
			break;
		}
	}
	else
		m_error(MMU_ERROR);

	return data;
}

void pm2_mmu_device::mmu_write(offs_t logical, u16 data, u16 mem_mask)
{
	auto const t = translate(logical, device_memory_interface::TR_WRITE);

	if (t.has_value())
	{
		auto const [type, physical] = t.value();

		switch (type)
		{
		case 0:
			// Multibus memory
			if (m_bus_mem.write_word_flags(physical, data, mem_mask))
			{
				LOG("%s: multibus_w mem 0x%08x translated 0x%08x\n", machine().describe_context(), logical, physical);
				m_error(MMU_ERROR);
			}
			break;
		case 1:
			// Multibus I/O
			if (m_bus_pio.write_word_flags(physical, data, mem_mask))
			{
				LOG("%s: multibus_w i/o 0x%08x translated 0x%08x\n", machine().describe_context(), logical, physical);
				m_error(MMU_ERROR);
			}
			break;
		case 2:
			// on-board memory
			m_cpu_mem.write_word(physical, data, mem_mask);
			break;
		case 3:
			// nonexistent
			LOG("%s: nonexistent write 0x%08x translated 0x%08x\n", machine().describe_context(), logical, physical);
			m_error(MMU_FAULT);
			break;
		}
	}
	else
		m_error(MMU_ERROR);
}

u16 pm2_mmu_device::read_program(offs_t logical, u16 mem_mask)
{
	return mmu_read<true>(logical, mem_mask);
}

void pm2_mmu_device::write_program(offs_t logical, u16 data, u16 mem_mask)
{
	mmu_write(logical, data, mem_mask);
}

u16 pm2_mmu_device::read_data(offs_t logical, u16 mem_mask)
{
	return mmu_read<false>(logical, mem_mask);
}

void pm2_mmu_device::write_data(offs_t logical, u16 data, u16 mem_mask)
{
	mmu_write(logical, data, mem_mask);
}

u16 pm2_mmu_device::read_cpu(offs_t logical, u16 mem_mask)
{
	return m_cpu_spc.read_word(logical, mem_mask);
}

void pm2_mmu_device::set_super(bool super)
{
	m_super = super;
}
