// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony NEWS Memory Management Unit for 68020-based workstations and servers
 *
 * Note: There is very limited documentation about this MMU because starting with the
 *       '030 generation of machines, the custom MMU was no longer needed. Therefore,
 *       this emulation is not yet complete.
 *
 * TODO:
 *  - What is the correct order of operations between checking valid/access bits/etc?
 *  - Are the user/system entries actually split? Or is that part of the tag?
 *  - General accuracy improvements
 */

#include "emu.h"
#include "news_020_mmu.h"

#define LOG_ENTRY (1U << 1)
#define LOG_DATA (1U << 2)
#define LOG_MAP_ERROR (1U << 3)

// #define VERBOSE (LOG_ENTRY | LOG_MAP_ERROR | LOG_GENERAL)
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

	constexpr uint32_t ENTRY_MODIFIED_MASK = 0x04000000;

	// Log messages for mode
	const char KERNEL_MODE[] = "KM";
	const char USER_MODE[] = "UM";

	const char *mode(bool is_supervisor)
	{
		return is_supervisor ? KERNEL_MODE : USER_MODE;
	}

	// Helper class for reading page table entries
	struct news_020_pte
	{
		news_020_pte(uint32_t pte_bits) : pte(pte_bits) {}

		const uint32_t pte;

		bool valid() const
		{
			return pte & 0x80000000;
		}

		bool kernel_writable() const
		{
			return pte & 0x40000000;
		}

		bool kernel_readable() const
		{
			return pte & 0x20000000;
		}

		bool user_writable() const
		{
			return pte & 0x10000000;
		}

		bool user_readable() const
		{
			return pte & 0x08000000;
		}

		bool modified() const
		{
			return pte & ENTRY_MODIFIED_MASK;
		}

		bool fill_on_demand() const
		{
			return pte & 0x02000000;
		}

		// between FOD and pfnum are 5 unused bits for memory, there can be data here for memory-mapped file I/O

		uint32_t pfnum() const
		{
			return pte & 0x000fffff; // 20 bits
		}

		// Additional helper functions
		bool writeable() const
		{
			return kernel_writable() || user_writable();
		}
	};
}

news_020_mmu_device::news_020_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEWS_020_MMU, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_hyperbus_config("hyperbus", ENDIANNESS_BIG, 32, 32, 0),
	m_bus_error(*this),
	m_enabled(false),
	m_romdis(false)
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
	(offset & 0x10000000) ? clear_kernel_entries() : clear_user_entries();
}

void news_020_mmu_device::clear_user_entries()
{
	LOGMASKED(LOG_ENTRY, "(%s) MMU clearing user entries\n", machine().describe_context());

	for (int i = 0; i < MMU_ENTRY_COUNT; ++i)
	{
		m_mmu_user_ram[i] = 0x0;
		m_mmu_user_tag_ram[i] = 0x0;
	}
}

void news_020_mmu_device::clear_kernel_entries()
{
	LOGMASKED(LOG_ENTRY, "(%s) MMU clearing kernel entries\n", machine().describe_context());

	for (int i = 0x0; i < MMU_ENTRY_COUNT; ++i)
	{
		m_mmu_system_ram[i] = 0x0;
		m_mmu_system_tag_ram[i] = 0x0;
	}
}

void news_020_mmu_device::device_start()
{
	space(0).specific(m_hyperbus);
	m_bus_error.resolve();

	save_item(NAME(m_enabled));
	save_item(NAME(m_romdis));

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
	return space_config_vector{
			std::make_pair(AS_PROGRAM, &m_hyperbus_config) };
}

uint32_t news_020_mmu_device::mmu_entry_r(offs_t offset, uint32_t mem_mask)
{
	fatalerror("news_020_mmu: tried to read from mapping RAM!");
	return 0;
}

void news_020_mmu_device::mmu_entry_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const bool system_map = offset & 0x4000000; // user or system mapping
	const uint32_t tag = (offset & ~0x4000000);

	// Actually update map
	offset = offset % MMU_ENTRY_COUNT;
	if (system_map)
	{
		COMBINE_DATA(&m_mmu_system_ram[offset]);
		m_mmu_system_tag_ram[offset] = (m_mmu_system_tag_ram[offset] & ~mem_mask) | (tag & mem_mask);
	}
	else
	{
		COMBINE_DATA(&m_mmu_user_ram[offset]);
		m_mmu_user_tag_ram[offset] = (m_mmu_user_tag_ram[offset] & ~mem_mask) | (tag & mem_mask);
	}

	const news_020_pte modified_entry(system_map ? m_mmu_system_ram[offset] : m_mmu_user_ram[offset]);
	LOGMASKED(LOG_ENTRY, "(%s) MMU %s entry write (0x%08x, 0x%08x, 0x%08x) -> entry 0x%08x: tag = 0x%08x, pfnum = 0x%08x (valid = %d, modified = %d, fill_on_demand = %d, kernel write = %d, kernel read = %d, user write = %d, user read = %d)\n",
			  machine().describe_context(),
			  system_map ? "system" : "user",
			  offset,
			  data,
			  mem_mask,
			  offset % MMU_ENTRY_COUNT,
			  tag,
			  modified_entry.pfnum(),
			  modified_entry.valid(),
			  modified_entry.modified(),
			  modified_entry.fill_on_demand(),
			  modified_entry.kernel_writable(),
			  modified_entry.kernel_readable(),
			  modified_entry.user_writable(),
			  modified_entry.user_readable());
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
			result = this->m_hyperbus.read_dword((0x03000000 + 0x100) + (offset << 2), mem_mask);
		}
		else
		{
			result = this->m_hyperbus.read_dword(offset << 2, mem_mask);
		}
	}
	else
	{
		offset = offset << 2;
		uint32_t vpgnum = (offset & ~0x80000000) >> 12;
		bool system = offset & 0x80000000;

		const uint32_t map_index = vpgnum % MMU_ENTRY_COUNT;
		const news_020_pte pte(system ? m_mmu_system_ram[map_index] : m_mmu_user_ram[map_index]);
		const uint32_t tag = system ? m_mmu_system_tag_ram[map_index] : m_mmu_user_tag_ram[map_index];

		if (!pte.valid() && !pte.fill_on_demand())
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r (offset 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> invalid page\n", machine().describe_context(), offset, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor));
			m_bus_error(offset, mem_mask, false, INVALID_ENTRY);
		}
		else if (tag != vpgnum)
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r (offset 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> tag mismatch 0x%08x != 0x%08x\n", machine().describe_context(), offset, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor), tag, vpgnum);
			m_bus_error(offset, mem_mask, false, TAG_MISMATCH);
		}
		else if (!is_supervisor && !pte.user_readable()) // user memory protection violation
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r (offset 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> user protection violation\n", machine().describe_context(), offset, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor));
			m_bus_error(offset, mem_mask, false, PROTECTION_VIOLATION);
		}
		else if (is_supervisor && !pte.kernel_readable()) // kernel memory protection violation
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r (offset 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> kernel protection violation\n", machine().describe_context(), offset, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor));
			m_bus_error(offset, mem_mask, false, PROTECTION_VIOLATION);
		}
		else if (!pte.valid() && pte.fill_on_demand())
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_r (offset 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> fill on demand page read\n", machine().describe_context(), offset, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor));
			m_bus_error(offset, mem_mask, false, INVALID_ENTRY);
		}
		else
		{
			uint32_t paddr = ((pte.pfnum() << 12) + (offset & 0xfff)) & 0x1fffffff;
			result = this->m_hyperbus.read_dword(paddr, mem_mask); // TODO: unmap lines in the bus instead?
			LOGMASKED(LOG_DATA, "(%s) hyperbus_r (offset 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> 0x%08x = 0x%08x\n", machine().describe_context(), offset, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor), paddr, result);
		}
	}

	return result;
}

void news_020_mmu_device::hyperbus_w(offs_t offset, uint32_t data, uint32_t mem_mask, bool is_supervisor)
{
	if (!m_enabled || (is_supervisor && (offset >= (0xc0000000 >> 2)))) // TODO: is the physical address check here (0xc...) correct?
	{
		this->m_hyperbus.write_dword((offset << 2) & 0x1fffffff, data, mem_mask); // TODO: see above about physical address width
	}
	else
	{
		offset = offset << 2;
		// Get PTE
		uint32_t vpgnum = (offset & ~0x80000000) >> 12;
		bool system = offset & 0x80000000; // TODO: is this correct?

		const uint32_t map_index = vpgnum % MMU_ENTRY_COUNT;
		const news_020_pte pte(system ? m_mmu_system_ram[map_index] : m_mmu_user_ram[map_index]);
		const uint32_t tag = system ? m_mmu_system_tag_ram[map_index] : m_mmu_user_tag_ram[map_index];

		if (!pte.valid() && !pte.fill_on_demand())
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_w (offset 0x%08x, data 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> invalid page\n", machine().describe_context(), offset, data, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor));
			m_bus_error(offset, mem_mask, true, INVALID_ENTRY); // should M be set here too?
		}
		else if (tag != vpgnum)
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_w (offset 0x%08x, data 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> tag mismatch 0x%x != 0x%x\n", machine().describe_context(), offset, data, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor), tag, vpgnum);
			m_bus_error(offset, mem_mask, true, TAG_MISMATCH); // should M be set here too?
		}
		else if (!is_supervisor && !pte.user_writable()) // user memory protection violation
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_w (offset 0x%08x, data 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> user protection violation\n", machine().describe_context(), offset, data, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor));
			m_bus_error(offset, mem_mask, true, PROTECTION_VIOLATION);
		}
		else if (is_supervisor && !pte.kernel_writable()) // kernel memory protection violation
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_w (offset 0x%08x, data 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> kernel protection violation\n", machine().describe_context(), offset, data, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor));
			m_bus_error(offset, mem_mask, true, PROTECTION_VIOLATION);
		}
		else if (pte.writeable())
		{
			if (!pte.valid() && pte.fill_on_demand())
			{
				LOGMASKED(LOG_DATA, "(%s) hyperbus_w (offset 0x%08x, data 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> fill on demand page write\n", machine().describe_context(), offset, data, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor));
				m_bus_error(offset, mem_mask, true, INVALID_ENTRY); // 0x2 (first M?) or 0x10 (invalid page)?
			}
			else
			{
				uint32_t paddr = ((pte.pfnum() << 12) + (offset & 0xfff)) & 0x1fffffff;
				LOGMASKED(LOG_DATA, "(%s) hyperbus_w (offset 0x%08x, data 0x%08x, mask 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> 0x%08x = 0x%08x%s\n", machine().describe_context(), offset, data, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor), paddr, data, pte.modified() ? " (newly modified page)" : "");
				this->m_hyperbus.write_dword(paddr, data, mem_mask);

				if (!pte.modified())
				{
					if (system)
					{
						m_mmu_system_ram[map_index] |= ENTRY_MODIFIED_MASK;
					}
					else
					{
						m_mmu_user_ram[map_index] |= ENTRY_MODIFIED_MASK;
					}
					m_bus_error(offset, mem_mask, true, MODIFIED);
				}
			}
		}
		else
		{
			LOGMASKED(LOG_MAP_ERROR, "(%s) hyperbus_w (offset 0x%08x, data 0x%08x, mask 0x%08x, data 0x%08x, pg 0x%08x, pte 0x%08x, index 0x%08x, %s) -> attempted write to read-only page\n", machine().describe_context(), offset, data, mem_mask, vpgnum, pte.pte, map_index, mode(is_supervisor));
			m_bus_error(offset, mem_mask, true, PROTECTION_VIOLATION);
		}
	}
}
