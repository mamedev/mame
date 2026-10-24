// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX2000 6502-derived CPU core, extending the original XaviX instruction
    set with additional registers and opcodes. SuperXaviX shares this extended
    instruction set, with its own timing and memory interface in superxavix.cpp.
    See xavix.cpp for the original banking and bus-access conventions.

    Known Die Markings:
    SSD 2000 NEC 85605-621

***************************************************************************/

#include "emu.h"
#include "xavix2000.h"
#include "xavix2000d.h"

DEFINE_DEVICE_TYPE(XAVIX2000, xavix2000_device, "xavix2000", "XaviX (SSD 2000)")

xavix2000_device::xavix2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	xavix_device(mconfig, type, tag, owner, clock, true, false)
{
}

xavix2000_device::xavix2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix2000_device(mconfig, XAVIX2000, tag, owner, clock)
{
}


void xavix2000_device::device_start()
{
	xavix_device::device_start();
	m_P &= ~F_B;

	state_add(XAVIX2000_J, "J", m_j).formatstr("%8s");
	state_add(XAVIX2000_K, "K", m_k).formatstr("%8s");
	state_add(XAVIX2000_L, "L", m_l).formatstr("%8s");
	state_add(XAVIX2000_M, "M", m_m).formatstr("%8s");
	state_add(XAVIX2000_PA, "PA", m_pa).formatstr("%8s");
	state_add(XAVIX2000_PB, "PB", m_pb).formatstr("%8s");

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

#include "cpu/m6502/xavix2000.hxx"
