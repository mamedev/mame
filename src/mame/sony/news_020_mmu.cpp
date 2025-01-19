// license:BSD-3-Clause
// copyright-holders:Brice Onken

#include "emu.h"
#include "news_020_mmu.h"

#define LOG_ENTRY     (1U << 1)
#define LOG_DATA      (1U << 2)
#define LOG_MAP_ERROR (1U << 3)

#define VERBOSE (LOG_ENTRY|LOG_MAP_ERROR|LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEWS_020_MMU, news_020_mmu_device, "news_020_mmu", "Sony NEWS 68020 MMU")

namespace
{
	constexpr int MMU_ENTRY_COUNT = 0x800;

	// Bus error status definitions
	constexpr uint8_t INVALID_ENTRY = 1 << 4;
	constexpr uint8_t TAG_MISMATCH = 1 << 3;
	constexpr uint8_t PROTECTION_VIOLATION = 1 << 2;
	constexpr uint8_t MODIFIED = 1 << 1;
}

news_020_mmu_device::news_020_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NEWS_020_MMU, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_hyperbus_config("hyperbus", ENDIANNESS_BIG, 32, 32, 0),
	m_bus_error(*this)
{
}

void news_020_mmu_device::set_mmu_enable(bool enabled)
{
	LOG("(%s) %s MMU!\n", machine().describe_context(), enabled ? "Enabled" : "Disabled");
	m_enabled = enabled;
}

void news_020_mmu_device::set_rom_enable(bool enabled)
{
	m_romdis = !enabled;
}

void news_020_mmu_device::clear_entries(offs_t offset, uint8_t data)
{
	(offset & 0x10000000) ?	clear_kernel_entries() : clear_user_entries();
}

void news_020_mmu_device::clear_user_entries()
{
	LOGMASKED(LOG_ENTRY, "(%s) MMU clearing user entries\n", machine().describe_context());

	// TODO: user/kernel may not be split
	for (int i = 0; i < MMU_ENTRY_COUNT; ++i)
	{
		m_mmu_user_ram[i] = 0x0;
		m_mmu_user_tag_ram[i] = 0x0;
	}
}

void news_020_mmu_device::clear_kernel_entries()
{
	LOGMASKED(LOG_ENTRY, "(%s) MMU clearing kernel entries\n", machine().describe_context());

	// TODO: user/kernel may not be split
	for (int i = 0x0; i < MMU_ENTRY_COUNT; ++i)
	{
		m_mmu_system_ram[i] = 0x0;
		m_mmu_system_tag_ram[i] = 0x0;
	}
}

void news_020_mmu_device::device_start()
{
	m_bus_error.resolve();
	
	save_item(NAME(m_enabled));

	m_mmu_user_ram = std::make_unique<u32[]>(MMU_ENTRY_COUNT);
	save_pointer(NAME(m_mmu_user_ram), MMU_ENTRY_COUNT);
	m_mmu_system_ram = std::make_unique<u32[]>(MMU_ENTRY_COUNT);
	save_pointer(NAME(m_mmu_system_ram), MMU_ENTRY_COUNT);

	m_mmu_user_tag_ram = std::make_unique<u32[]>(MMU_ENTRY_COUNT);
	save_pointer(NAME(m_mmu_user_tag_ram), MMU_ENTRY_COUNT);
	m_mmu_system_tag_ram = std::make_unique<u32[]>(MMU_ENTRY_COUNT);
	save_pointer(NAME(m_mmu_system_tag_ram), MMU_ENTRY_COUNT);
}

device_memory_interface::space_config_vector news_020_mmu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_hyperbus_config)
	};
}

uint32_t news_020_mmu_device::mmu_entry_r(offs_t offset, uint32_t mem_mask)
{
	fatalerror("news_020_mmu: tried to read from mapping RAM!");
	return 0;
}

void news_020_mmu_device::mmu_entry_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const bool system_map = offset & 0x4000000; // user or system mapping
	const uint32_t tag = (offset & ~0x4000000); // TODO: this includes the bits that determine which entry it goes into - is that also in the tag? Should the system bits also be in the tag?

	// Actually update map
	offset = offset % MMU_ENTRY_COUNT;
	if (system_map)
	{
		COMBINE_DATA(&m_mmu_system_ram[offset]);
		m_mmu_system_tag_ram[offset] = (m_mmu_system_tag_ram[offset] & ~mem_mask) | (tag & mem_mask); // COMBINE_DATA but with the tag
	}
	else
	{
		COMBINE_DATA(&m_mmu_user_ram[offset]);
		m_mmu_user_tag_ram[offset] = (m_mmu_user_tag_ram[offset] & ~mem_mask) | (tag & mem_mask);
	}

	#if (VERBOSE & LOG_ENTRY) > 0
	news_020_pte modified_entry = unpack_mmu_entry(system_map ? m_mmu_system_ram[offset] : m_mmu_user_ram[offset]);
	LOGMASKED(LOG_ENTRY, "(%s) mmu %s entry write (0x%x, 0x%x, 0x%x) -> entry 0x%x: tag = 0x%x, pfnum = 0x%x (valid = %d, modified = %d, fill_on_demand = %d, kernel write = %d, kernel read = %d, user write = %d, user read = %d)\n", machine().describe_context(),
			system_map ? "system" : "user",
			offset,
			data,
			mem_mask,
			offset % MMU_ENTRY_COUNT,
			tag,
			modified_entry.pfnum,
			modified_entry.valid,
			modified_entry.modified,
			modified_entry.fill_on_demand,
			modified_entry.kernel_writable,
			modified_entry.kernel_readable,
			modified_entry.user_writable,
			modified_entry.user_readable);
	#endif
}

uint32_t news_020_mmu_device::hyperbus_r(offs_t offset, uint32_t mem_mask, bool is_supervisor)
{
	uint32_t result = 0;
	if (!m_enabled || (is_supervisor && (offset >= (0xc0000000 >> 2)))) // TODO: is the physical address check here (0xc...) correct?
	{
		// TODO: What is the actual physical address width? The 0x1 part might also be an MMU control bit
		offset = offset & (0x1fffffff >> 2);
		if (!m_romdis && offset < ((0xffff - 0x100) >> 2))
		{
			result = this->space(0).read_dword((0x03000000 + 0x100) + (offset << 2), mem_mask);
		}
		else
		{
			result = this->space(0).read_dword(offset << 2, mem_mask);
		}
	}
	else
	{
		offset = offset << 2;
		uint32_t vpgnum = (offset & ~0x80000000) >> 12;
		bool system = offset & 0x80000000; // TODO: is this correct?

		const news_020_pte pte = unpack_mmu_entry(system ? m_mmu_system_ram[vpgnum % MMU_ENTRY_COUNT] : m_mmu_user_ram[vpgnum % MMU_ENTRY_COUNT]);
		const uint32_t tag = system ? m_mmu_system_tag_ram[vpgnum % MMU_ENTRY_COUNT] : m_mmu_user_tag_ram[vpgnum % MMU_ENTRY_COUNT];
		if (!pte.valid && !pte.fill_on_demand)
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r 0x%08x (pg 0x%08x, pte 0x%08x, index 0x%x) -> invalid page\n", machine().describe_context(), offset, vpgnum, pte.raw, (vpgnum % MMU_ENTRY_COUNT) + (system ? MMU_ENTRY_COUNT : 0x0)); // TODO: better log message
			m_bus_error(offset, mem_mask, false, INVALID_ENTRY);
		}
		else if (tag != vpgnum)
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r 0x%08x (pg 0x%08x, pte 0x%08x) -> tag mismatch 0x%x != 0x%x \n", machine().describe_context(), offset, vpgnum, pte.raw, tag, vpgnum, is_supervisor); // TODO: better log message
			m_bus_error(offset, mem_mask, false, TAG_MISMATCH);
		}
		else if (!is_supervisor && !pte.user_readable) // user memory protection violation
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r 0x%08x (pg 0x%08x, pte 0x%08x, index 0x%x) user protection violation\n", machine().describe_context(), offset, vpgnum, pte.raw, (vpgnum % MMU_ENTRY_COUNT) + (system ? MMU_ENTRY_COUNT : 0x0));
			m_bus_error(offset, mem_mask, false, PROTECTION_VIOLATION);
		}
		else if (is_supervisor && !pte.kernel_readable) // kernel memory protection violation
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r 0x%08x (pg 0x%08x, pte 0x%08x, index 0x%x) kernel protection violation\n", machine().describe_context(), offset, vpgnum, pte.raw, (vpgnum % MMU_ENTRY_COUNT) + (system ? MMU_ENTRY_COUNT : 0x0));
			m_bus_error(offset, mem_mask, false, PROTECTION_VIOLATION);
		}
		else if (!pte.valid && pte.fill_on_demand)
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r fill on demand page addr = 0x%x pte = 0x%x\n", machine().describe_context(), offset, pte.raw);
			m_bus_error(offset, mem_mask, false, INVALID_ENTRY);
		}
		else
		{
			uint32_t paddr = ((pte.pfnum << 12) + (offset & 0xfff)) & 0x1fffffff;
			result = this->space(0).read_dword(paddr, mem_mask); // TODO: unmap lines in the bus instead?
			LOGMASKED(LOG_DATA, "(%s) hyperbus_r 0x%08x 0x%08x (pg 0x%08x, pte 0x%08x) -> 0x%08x = 0x%08x\n", machine().describe_context(), offset, mem_mask, vpgnum, pte.raw, paddr, result);
		}
	}

	return result;
}

void news_020_mmu_device::hyperbus_w(offs_t offset, uint32_t data, uint32_t mem_mask, bool is_supervisor)
{
	if (!m_enabled || (is_supervisor && (offset >= (0xc0000000 >> 2)))) // TODO: is the physical address check here (0xc...) correct?
	{
		this->space(0).write_dword((offset << 2) & 0x1fffffff, data, mem_mask); // TODO: see above about physical address width
	}
	else // TODO: other BERR bits
	{
		offset = offset << 2;
		// Get PTE
		uint32_t vpgnum = (offset & ~0x80000000) >> 12;
		bool system = offset & 0x80000000; // TODO: is this correct?

		const news_020_pte pte = unpack_mmu_entry(system ? m_mmu_system_ram[vpgnum % MMU_ENTRY_COUNT] : m_mmu_user_ram[vpgnum % MMU_ENTRY_COUNT]);
		const uint32_t tag = system ? m_mmu_system_tag_ram[vpgnum % MMU_ENTRY_COUNT] : m_mmu_user_tag_ram[vpgnum % MMU_ENTRY_COUNT];
		if (!pte.valid && !pte.fill_on_demand)
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) mmu w 0x%08x (pg 0x%08x) -> invalid page (page % 400 = 0x%x, data = 0x%x pte = 0x%x) as supervisior? %d\n", machine().describe_context(), offset, vpgnum, vpgnum % MMU_ENTRY_COUNT, data, pte.raw, is_supervisor);
			m_bus_error(offset, mem_mask, true, INVALID_ENTRY); // should M be set here too?
		}
		else if (tag != vpgnum)
		{
			// TODO: update below log message
			LOGMASKED(LOG_MAP_ERROR, "(%s) mmu w 0x%08x (pg 0x%08x) -> tag mismatch 0x%x != 0x%x\n", machine().describe_context(), offset, vpgnum, vpgnum % MMU_ENTRY_COUNT, tag, vpgnum);
			m_bus_error(offset, mem_mask, true, TAG_MISMATCH); // should M be set here too?
		}
		// TODO: order of operations between checking valid and access bits?
		else if (!is_supervisor && !pte.user_writable) // user memory protection violation
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) mmu w 0x%08x (pg 0x%08x, pte 0x%08x, index 0x%x) user protection violation\n", machine().describe_context(), offset, vpgnum, pte.raw, (vpgnum % MMU_ENTRY_COUNT) + (system ? MMU_ENTRY_COUNT : 0x0));
			m_bus_error(offset, mem_mask, true, PROTECTION_VIOLATION);
		}
		else if (is_supervisor && !pte.kernel_writable) // kernel memory protection violation
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) mmu w 0x%08x (pg 0x%08x, pte 0x%08x, index 0x%x) kernel protection violation\n", machine().describe_context(), offset, vpgnum, pte.raw, (vpgnum % MMU_ENTRY_COUNT) + (system ? MMU_ENTRY_COUNT : 0x0));
			m_bus_error(offset, mem_mask, true, PROTECTION_VIOLATION);
		}
		else if (pte.kernel_writable || pte.user_writable) // writeable
		{
			if (!pte.valid && pte.fill_on_demand)
			{
				LOGMASKED(LOG_DATA, "(%s) fill on demand page write addr = 0x%x pte = 0x%x\n", machine().describe_context(), offset, pte.raw);
				m_bus_error(offset, mem_mask, true, INVALID_ENTRY); // 0x2 (first M?) or 0x10 (invalid page)?
			}
			else
			{
				uint32_t paddr = ((pte.pfnum << 12) + (offset & 0xfff)) & 0x1fffffff;
				LOGMASKED(LOG_DATA, "(%s) mmu w 0x%08x (pg 0x%08x) -> 0x%08x = 0x%08x\n", machine().describe_context(), offset, vpgnum, paddr, data);
				this->space(0).write_dword(paddr, data, mem_mask); // TODO: fix 1f stuff

				if (!pte.modified)
				{
					LOGMASKED(LOG_DATA, "(%s) write to unmodified page 0x%x!\n", machine().describe_context(), vpgnum);
					if (system)
					{
						m_mmu_system_ram[vpgnum % MMU_ENTRY_COUNT] |= 0x04000000;
					}
					else
					{
						m_mmu_user_ram[vpgnum % MMU_ENTRY_COUNT] |= 0x04000000;
					}
					m_bus_error(offset, mem_mask, true, MODIFIED);
				}
			}
		}
		else
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) write to read-only page 0x%x because it was not set as writable (pte = 0x%x)\n", machine().describe_context(), vpgnum, pte.raw);
			m_bus_error(offset, mem_mask, true, PROTECTION_VIOLATION);
		}
	}
}

news_020_mmu_device::news_020_pte news_020_mmu_device::unpack_mmu_entry(const uint32_t entry)
{
	news_020_pte new_entry;
	new_entry.valid = (entry & 0x80000000) != 0;
	new_entry.user_readable = (entry & 0x08000000) != 0;
	new_entry.user_writable = (entry & 0x10000000) != 0;
	new_entry.kernel_readable = (entry & 0x20000000) != 0;
	new_entry.kernel_writable = (entry & 0x40000000) != 0;
	new_entry.modified = (entry & 0x04000000) != 0;
	new_entry.fill_on_demand = (entry & 0x02000000) != 0;
	new_entry.unused = (entry & 0x01f00000) >> 20;
	new_entry.pfnum =   entry & 0x000fffff;
	new_entry.raw = entry;
	return new_entry;
}
