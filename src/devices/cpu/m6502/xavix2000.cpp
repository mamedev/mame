// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX2000 and SuperXaviX 6502-derived CPU cores, extending the original
    XaviX instruction set with additional opcodes. Graphics, sound and the
    extra SuperXaviX video/I/O hardware are modelled outside these CPU devices.
    See xavix.cpp for the original banking and bus-access conventions.

    Both devices share the extended instruction set. SuperXaviX uses separate
    instruction timing and buffered memory accesses.

    Known Die Markings:
    SSD 2000 NEC 85605-621
    SSD 2002 NEC 85054-611

    TODO:
    - Account for competing audio, video and DMA memory requests.

***************************************************************************/

#include "emu.h"
#include "xavix2000.h"
#include "xavix2000d.h"

#include <algorithm>
#include <iterator>

DEFINE_DEVICE_TYPE(XAVIX2000, xavix2000_device, "xavix2000", "XaviX (SSD 2000)")
DEFINE_DEVICE_TYPE(SUPERXAVIX, superxavix_device, "superxavix", "SuperXaviX (SSD 2002)")



xavix2000_device::xavix2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	xavix_device(mconfig, type, tag, owner, clock, true, false)
{
}

xavix2000_device::xavix2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix2000_device(mconfig, XAVIX2000, tag, owner, clock)
{
}

superxavix_device::superxavix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix2000_device(mconfig, SUPERXAVIX, tag, owner, clock),
	m_memory_control_cb(*this, 0)
{
}


void xavix2000_device::device_start()
{
	xavix_device::device_start();
	m_P &= ~F_B;

	state_add(XAVIX2000_J, "J", m_j).callimport().formatstr("%8s");
	state_add(XAVIX2000_K, "K", m_k).callimport().formatstr("%8s");
	state_add(XAVIX2000_L, "L", m_l).callimport().formatstr("%8s");
	state_add(XAVIX2000_M, "M", m_m).callimport().formatstr("%8s");
	state_add(XAVIX2000_PA, "PA", m_pa).callimport().formatstr("%8s");
	state_add(XAVIX2000_PB, "PB", m_pb).callimport().formatstr("%8s");

	save_item(NAME(m_j));
	save_item(NAME(m_k));
	save_item(NAME(m_l));
	save_item(NAME(m_m));
	save_item(NAME(m_pa));
	save_item(NAME(m_pb));
	save_item(NAME(m_interrupt_is_nmi));
}

void xavix2000_device::device_reset()
{
	xavix_device::device_reset();
	m_interrupt_is_nmi = false;

	m_j = 0;
	m_k = 0;
	m_l = 0;
	m_m = 0;
	m_pa = 0;
	m_pb = 0;
}

void superxavix_device::device_start()
{
	xavix2000_device::device_start();
	save_item(NAME(m_instruction_tag));
	save_item(NAME(m_instruction_cache));
	save_item(NAME(m_instruction_valid));
	save_item(NAME(m_instruction_burst));
	save_item(NAME(m_data_tag));
	save_item(NAME(m_data_cache));
	save_item(NAME(m_data_valid));
	save_item(NAME(m_bus_address));
	save_item(NAME(m_bus_cycle));
	save_item(NAME(m_bus_available));
	save_item(NAME(m_bus_kind));
	save_item(NAME(m_bus_last_kind));
	save_item(NAME(m_bus_remaining));
	save_item(NAME(m_bus_control));
	save_item(NAME(m_bus_write_data));
	save_item(NAME(m_bus_bytes));
	save_item(NAME(m_bus_lanes));
	save_item(NAME(m_access_address));
	save_item(NAME(m_access_start));
	save_item(NAME(m_access_cycle));
	save_item(NAME(m_access_kind));
	save_item(NAME(m_access_data));
	save_item(NAME(m_access_started));
	save_item(NAME(m_operand));
}

void superxavix_device::device_reset()
{
	xavix2000_device::device_reset();
	m_A = m_X = m_Y = 0;
	m_SP = 0x100;
	m_P &= ~F_B;
	m_instruction_tag = m_data_tag = 0;
	std::fill(std::begin(m_instruction_cache), std::end(m_instruction_cache), 0);
	std::fill(std::begin(m_data_cache), std::end(m_data_cache), 0);
	m_instruction_valid = m_data_valid = 0;
	m_instruction_burst = false;
	m_bus_kind = m_bus_last_kind = m_access_kind = access_kind::NONE;
	m_bus_address = m_access_address = 0;
	m_bus_cycle = m_bus_available = m_access_start = m_access_cycle = 0;
	m_bus_remaining = m_bus_control = m_bus_write_data = m_bus_lanes = 0;
	m_bus_bytes[0] = m_bus_bytes[1] = 0;
	m_access_data = m_operand = 0;
	m_access_started = false;
}

std::unique_ptr<util::disasm_interface> xavix2000_device::create_disassembler()
{
	return std::make_unique<xavix2000_disassembler>();
}

void xavix2000_device::state_import(const device_state_entry &entry)
{
	if (entry.index() == STATE_GENFLAGS || entry.index() == M6502_P)
		m_P |= F_E;
	else
		xavix_device::state_import(entry);
}


void xavix2000_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	xavix_device::state_string_export(entry, str);

	switch(entry.index())
	{
	case XAVIX2000_J:
		str = string_format("%02x", m_j);
		break;
	case XAVIX2000_K:
		str = string_format("%02x", m_k);
		break;
	case XAVIX2000_L:
		str = string_format("%02x", m_l);
		break;
	case XAVIX2000_M:
		str = string_format("%02x", m_m);
		break;
	case XAVIX2000_PA:
		str = string_format("%08x", m_pa);
		break;
	case XAVIX2000_PB:
		str = string_format("%08x", m_pb);
		break;
	}
}

bool superxavix_device::wait_for_cycle(u64 cycle)
{
	u64 const now = total_cycles();
	return cycle > now && access_before_time(cycle, now);
}

void superxavix_device::observe_address(u32 address)
{
	// The data buffer follows the shared address, including instruction accesses.
	if ((address & ~1U) != m_data_tag)
		m_data_valid = 0;
}

u8 superxavix_device::raw_instruction(u32 address)
{
	if (address == 0xfe)
		return m_codebank;
	if (address == 0xff)
		return m_databank;
	return m_mintf->m_cprogram.read_interruptible(address);
}

bool superxavix_device::advance_external_bus(u64 cycle)
{
	// TODO: account for competing audio, video and DMA memory requests.
	while (m_bus_kind != access_kind::NONE && m_bus_cycle <= cycle)
	{
		if (m_bus_kind == access_kind::WRITE)
		{
			xavix_device::write_banked_data(m_bus_address, m_bus_write_data);
			if (access_to_be_redone_noclear())
				return false;
			m_bus_kind = access_kind::NONE;
			continue;
		}

		unsigned const lanes = BIT(m_bus_control, 0) ? 3 : (1U << BIT(m_bus_address, 0));
		for (unsigned lane = 0; lane != 2; ++lane)
		{
			if (BIT(lanes, lane) && !BIT(m_bus_lanes, lane))
			{
				u32 const address = (m_bus_address & ~1U) | lane;
				u8 const data = m_bus_kind == access_kind::INSTRUCTION
					? raw_instruction(address)
					: xavix_device::read_banked_data(address);
				if (access_to_be_redone_noclear())
					return false;
				m_bus_bytes[lane] = data;
				m_bus_lanes |= 1U << lane;
				if (m_icount <= 0 && (m_bus_lanes & lanes) != lanes)
				{
					defer_access();
					return false;
				}
			}
		}

		if (m_bus_kind == access_kind::INSTRUCTION)
		{
			for (unsigned lane = 0; lane != 2; ++lane)
				if (BIT(lanes, lane))
				{
					unsigned const index = (m_bus_address & 2) | lane;
					m_instruction_cache[index] = m_bus_bytes[lane];
					m_instruction_valid |= 1U << index;
				}
		}
		else
		{
			m_data_tag = m_bus_address & ~1U;
			for (unsigned lane = 0; lane != 2; ++lane)
				if (BIT(lanes, lane))
					m_data_cache[lane] = m_bus_bytes[lane];
			m_data_valid |= lanes;
		}

		unsigned const count = BIT(m_bus_control, 0) ? 2 - BIT(m_bus_address, 0) : 1;
		if (m_bus_remaining <= count)
		{
			m_bus_kind = access_kind::NONE;
			m_bus_remaining = 0;
		}
		else
		{
			unsigned const page = BIT(m_bus_control, 4, 2);
			unsigned const mask = page ? (2U << page) - 1 : 0;
			u32 const next = m_bus_address + count;
			unsigned const cycles = mask && !((m_bus_address ^ next) & ~mask)
				? page_cycles(m_bus_control) : access_cycles(m_bus_control);
			m_bus_address = next;
			m_bus_remaining -= count;
			m_bus_cycle += cycles;
			m_bus_available = m_bus_cycle + 1;
			m_bus_lanes = 0;
		}
	}
	return true;
}

void superxavix_device::start_external_access()
{
	unsigned const region_bits = BIT(m_access_address, 21, 2);
	m_bus_control = m_memory_control_cb(region_bits < 2 ? 0 : region_bits - 1);
	u64 const grant = std::max(m_access_start + 1,
		m_bus_available + (m_bus_last_kind == access_kind::INSTRUCTION && m_access_kind == access_kind::INSTRUCTION ? 1 : 0));
	m_bus_address = m_access_address;
	m_bus_kind = m_bus_last_kind = m_access_kind;
	m_bus_remaining = 1;
	m_bus_lanes = 0;
	m_bus_write_data = m_access_data;
	m_bus_cycle = grant + access_cycles(m_bus_control) - 1;
	m_bus_available = m_bus_cycle + 1;
	m_access_started = true;
	if (m_access_kind == access_kind::INSTRUCTION)
	{
		// A miss replaces the whole buffer, even when its tag is unchanged.
		m_instruction_tag = m_access_address & ~3U;
		m_instruction_valid = 0;
		m_bus_remaining = m_instruction_burst ? 4 - (m_access_address & 3) : 1;
		m_instruction_burst = BIT(m_memory_control_cb(3), 0);
	}
	else if (m_access_kind == access_kind::DATA)
	{
		// Data becomes available only after the external response is registered.
		++m_bus_cycle;
	}
	else
	{
		m_access_cycle = grant;
		m_bus_cycle = m_bus_available;
	}
}

u8 superxavix_device::memory_access(u32 address, access_kind kind, u8 data)
{
	address &= 0xffffff;
	if (m_access_kind == access_kind::NONE)
	{
		m_access_kind = kind;
		m_access_address = address;
		m_access_start = total_cycles();
		m_access_data = data;
		m_access_started = false;
		observe_address(address);
		if (internal_address(address))
			m_access_cycle = m_access_start + 1;
	}
	assert(m_access_kind == kind && m_access_address == address);

	for (;;)
	{
		if (m_bus_kind != access_kind::NONE && !advance_external_bus(total_cycles()))
			return 0;
		if (internal_address(address))
		{
			if (wait_for_cycle(m_access_cycle))
				return 0;
			if (m_bus_kind != access_kind::NONE && !advance_external_bus(total_cycles()))
				return 0;
			u8 result = 0;
			if (kind == access_kind::WRITE)
				xavix_device::write_banked_data(address, m_access_data);
			else if (kind == access_kind::INSTRUCTION)
				result = raw_instruction(address);
			else
				result = xavix_device::read_banked_data(address);
			if (!access_to_be_redone_noclear())
				m_access_kind = access_kind::NONE;
			return result;
		}

		if (kind == access_kind::INSTRUCTION)
		{
			if (m_instruction_tag == (address & ~3U) && BIT(m_instruction_valid, address & 3))
			{
				m_access_kind = access_kind::NONE;
				return m_instruction_cache[address & 3];
			}
		}
		// A valid data-buffer byte also completes same-word writes without an external transfer.
		else if (m_data_tag == (address & ~1U) && BIT(m_data_valid, address & 1))
		{
			m_access_kind = access_kind::NONE;
			return m_data_cache[address & 1];
		}

		if (kind == access_kind::WRITE && m_access_started)
		{
			if (wait_for_cycle(m_access_cycle))
				return 0;
			m_access_kind = access_kind::NONE;
			return 0;
		}
		if (!m_access_started && m_bus_kind == access_kind::NONE)
			start_external_access();
		u64 const next = kind == access_kind::WRITE && m_access_started ? m_access_cycle : m_bus_cycle;
		if (wait_for_cycle(next))
			return 0;
	}
}

u8 superxavix_device::read_vector(bool nmi, bool low)
{
	bool const internal = !m_interrupt_vector_callback.isnull() && m_interrupt_vector_callback(nmi ? 0 : 1, low ? 1 : 0) != -1;
	u32 const address = (internal ? 0x7ffa : 0xfffa) + (nmi ? 0 : 4) + (low ? 0 : 1);
	return memory_access(address, access_kind::DATA);
}

void superxavix_device::read_idle(u32 address)
{
	observe_address(address);
	if (m_bus_kind != access_kind::NONE)
		advance_external_bus(total_cycles());
}

#include "cpu/m6502/xavix2000.hxx"
#include "cpu/m6502/superxavix.hxx"
