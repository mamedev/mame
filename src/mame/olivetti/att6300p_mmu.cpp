// license:BSD-3-Clause
// copyright-holders:D. Donohoe
/**********************************************************************

    AT&T 6300 Plus virtualization emulation

**********************************************************************/

#include "emu.h"

#include "att6300p_mmu.h"

DEFINE_DEVICE_TYPE(ATT6300P_MMU, att6300p_mmu_device, "att6300p_mmu", "AT&T 6300 Plus MMU")

static inline uint32_t remap(uint32_t *table, uint32_t addr, uint32_t imask, uint32_t omask)
{
	return (addr & imask) | (table[(addr>>15) & 0x1f] & omask);
}

att6300p_mmu_device::att6300p_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ATT6300P_MMU, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_trapio(*this),
	m_mem_config("mem", ENDIANNESS_LITTLE, 16, 24, 0),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 14, 0)
{
}

device_memory_interface::space_config_vector att6300p_mmu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_mem_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}

void att6300p_mmu_device::device_start()
{
	space().specific(m_mem16_space);
	m_io = &space(AS_IO);

	save_item(NAME(m_protected_mode));
	save_item(NAME(m_map_table));
	save_item(NAME(m_map_imask));
	save_item(NAME(m_map_omask));
	save_item(NAME(m_mem_prot_limit));
	save_item(NAME(m_mem_wr_fastpath));
	save_item(NAME(m_mem_prot_table));
	save_item(NAME(m_io_prot_table));
	save_item(NAME(m_io_read_traps_enabled));
	save_item(NAME(m_io_write_traps_enabled));
}

void att6300p_mmu_device::device_reset()
{
	set_protected_mode_enabled(false);
	m_mem_wr_fastpath = true;
	m_mem_prot_limit = 0;
	m_io_setup_enabled = false;
	m_mem_setup_enabled = false;
	m_io_read_traps_enabled = false;
	m_io_write_traps_enabled = false;

	for (int i = 0; i < 32; i++)
	{
		m_map_table[i] = i * 32*1024;
	}

	memset(m_mem_prot_table, 0, sizeof m_mem_prot_table);
	memset(m_io_prot_table, 0, sizeof m_mem_prot_table);
}

void att6300p_mmu_device::set_protected_mode_enabled(bool enabled)
{
	m_protected_mode = enabled;

	if (enabled)
	{
		// Replaces A15-A19
		m_map_imask = 0xf07fff;
		m_map_omask = 0x0f8000;
	}
	else
	{
		// Replaces A15-A19, ignores A20-A23 from the CPU, and allows the
		// PROM to select between 0 or 0xf for A20-A23.
		m_map_imask = 0x007fff;
		m_map_omask = 0xff8000;
	}
}

void att6300p_mmu_device::update_fastpath()
{
	m_mem_wr_fastpath = (m_mem_prot_limit == 0 &&
		!m_mem_setup_enabled && !m_io_setup_enabled);
}

void att6300p_mmu_device::set_mem_mapping(uint32_t target_addr[32])
{
	// One target address for each of the 32K regions below 1M
	for (int i = 0; i < 32; i++)
	{
		m_map_table[i] = target_addr[i];
	}
}

void att6300p_mmu_device::set_mem_setup_enabled(bool enabled)
{
	m_mem_setup_enabled = enabled;
	update_fastpath();
}

void att6300p_mmu_device::set_io_setup_enabled(bool enabled)
{
	m_io_setup_enabled = enabled;
	update_fastpath();
}

void att6300p_mmu_device::set_memprot_enabled(bool enabled)
{
	if (enabled)
	{
		// Apply memory protection to addresses below 1M
		m_mem_prot_limit = 0x100000;
	}
	else
	{
		// Never apply memory protection
		m_mem_prot_limit = 0;
	}

	update_fastpath();
}

void att6300p_mmu_device::set_io_read_traps_enabled(bool enabled)
{
	m_io_read_traps_enabled = enabled;
}

void att6300p_mmu_device::set_io_write_traps_enabled(bool enabled)
{
	m_io_write_traps_enabled = enabled;
}

uint16_t att6300p_mmu_device::mem_r(offs_t offset, uint16_t mem_mask)
{
	offset = remap(m_map_table, offset<<1, m_map_imask, m_map_omask);

	return m_mem16_space.read_word(offset, mem_mask);
}

void att6300p_mmu_device::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset = remap(m_map_table, offset<<1, m_map_imask, m_map_omask);

	if (!m_mem_wr_fastpath)
	{
		if (offset < m_mem_prot_limit && m_mem_prot_table[offset>>10])
		{
			// Inhibit memory write
			return;
		}

		if (m_mem_setup_enabled)
		{
			m_mem_prot_table[(offset>>10) & 0x3ff] = (data & 1);
			return;
		}

		if (m_io_setup_enabled && (offset & 0xff8000) == 0xf8000)
		{
			if (mem_mask == 0xff00)
				m_io_prot_table[(offset|1) & 0xfff] = ((data >> 8) & 0xf);
			else
				m_io_prot_table[offset & 0xfff] = (data & 0xf);
		}
	}

	m_mem16_space.write_word(offset, data, mem_mask);
}

uint16_t att6300p_mmu_device::io_r(offs_t offset, uint16_t mem_mask)
{
	offset <<= 1;

	if (m_protected_mode)
	{
		// Skip protection checks
		switch (mem_mask)
		{
			default:
			case 0x00ff:
				return m_io->read_byte(offset);
			case 0xff00:
				return m_io->read_byte(offset | 1) << 8;
			case 0xffff:
				return m_io->read_byte(offset) |
					m_io->read_byte(offset | 1) << 8;
		}
	}
	else
	{
		uint16_t data = 0;
		uint8_t flags;
		int bytes, shift;

		switch (mem_mask)
		{
			default:
			case 0x00ff:
				flags = TRAPIO_FLAG__LBHE;
				bytes = 1;
				shift = 0;
				break;
			case 0xff00:
				flags = TRAPIO_FLAG_LA0;
				bytes = 1;
				offset |= 1;
				shift = 8;
				break;
			case 0xffff:
				flags = 0;
				bytes = 2;
				shift = 8;
				break;
		}

		for (int i = 0; ; )
		{
			uint8_t prot = m_io_prot_table[offset];
			uint8_t val;

			if (m_io_read_traps_enabled && (prot & IO_PROT_INHIBIT_READ))
			{
				val = 0xff;
			}
			else
			{
				val = m_io->read_byte(offset);
			}

			if (m_io_read_traps_enabled && ((prot & IO_PROT_NOTRAP) == 0))
			{
				if (!machine().side_effects_disabled())
					m_trapio(offset | val << 16 | flags << 24);
			}

			data |= val << shift;

			if (++i == bytes)
			{
				break;
			}

			shift = 0;
			offset |= 1;
		}

		return data;
	}
}

void att6300p_mmu_device::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset <<= 1;

	if (m_protected_mode)
	{
		// Skip protection checks
		switch (mem_mask)
		{
			default:
			case 0x00ff:
				m_io->write_byte(offset, data);
				break;
			case 0xff00:
				m_io->write_byte(offset | 1, data >> 8);
				break;
			case 0xffff:
				m_io->write_byte(offset, data & 0xff);
				m_io->write_byte(offset | 1, data >> 8);
				break;
		}
	}
	else
	{
		int bytes;
		uint8_t flags;

		switch (mem_mask)
		{
			default:
			case 0x00ff:
				flags = TRAPIO_FLAG__IORC|TRAPIO_FLAG__LBHE;
				bytes = 1;
				break;
			case 0xff00:
				flags = TRAPIO_FLAG__IORC|TRAPIO_FLAG_LA0;
				bytes = 1;
				offset |= 1;
				data >>= 8;
				break;
			case 0xffff:
				flags = TRAPIO_FLAG__IORC;
				bytes = 2;
				break;
		}

		for (int i = 0; ; )
		{
			uint8_t prot = m_io_prot_table[offset];
			if (!m_io_write_traps_enabled || !(prot & IO_PROT_INHIBIT_WRITE))
			{
				m_io->write_byte(offset, data & 0xff);
			}

			if (m_io_write_traps_enabled && ((prot & IO_PROT_NOTRAP) == 0))
			{
				m_trapio(offset | (data&0xff) << 16 | flags << 24);
			}

			if (++i == bytes)
			{
				break;
			}

			data >>= 8;
			offset |= 1;
		}
	}
}
