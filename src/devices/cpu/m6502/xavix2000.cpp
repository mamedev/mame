// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix2000.cpp (Super XaviX)

    The dies for these are marked

    SSD 2000 NEC 85605-621

    SSD 2002 NEC 85054-611

    6502 with custom opcodes
    integrated gfx / sound

    special notes

    see xavix.cpp for basic notes

    the 2000 chip has more opcodes than the 97/98 chips in xavix.cpp, and
    is a similar die structure to the 2002 chip, but doesn't seem to have any
    additional capabilities.

    the 2002 chip seems to be the one that was officially dubbed 'SuperXaviX'
    and has additional video capabilities on top of the extended opcodes.

***************************************************************************/

#include "emu.h"
#include "xavix2000.h"
#include "xavix2000d.h"

DEFINE_DEVICE_TYPE(XAVIX2000, xavix2000_device, "xavix2000", "XaviX (SSD 2000)")
DEFINE_DEVICE_TYPE(XAVIX2002, xavix2002_device, "xavix2002", "XaviX (SSD 2002) (SuperXaviX)")



xavix2000_device::xavix2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	xavix_device(mconfig, type, tag, owner, clock)
{
	m_use_private_stack_for_extra_callf_byte = false;
}

xavix2000_device::xavix2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix2000_device(mconfig, XAVIX2000, tag, owner, clock)
{
}

xavix2002_device::xavix2002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix2000_device(mconfig, XAVIX2002, tag, owner, clock)
{
}


void xavix2000_device::device_start()
{
	xavix_device::device_start();

	state_add(SXAVIX_J, "J", m_j).callimport().formatstr("%8s");
	state_add(SXAVIX_K, "K", m_k).callimport().formatstr("%8s");
	state_add(SXAVIX_L, "L", m_l).callimport().formatstr("%8s");
	state_add(SXAVIX_M, "M", m_m).callimport().formatstr("%8s");
	state_add(SXAVIX_PA, "PA", m_pa).callimport().formatstr("%8s");
	state_add(SXAVIX_PB, "PB", m_pb).callimport().formatstr("%8s");
}

std::unique_ptr<util::disasm_interface> xavix2000_device::create_disassembler()
{
	return std::make_unique<xavix2000_disassembler>();
}


void xavix2000_device::state_import(const device_state_entry &entry)
{
	xavix_device::state_import(entry);

	switch(entry.index())
	{
	case SXAVIX_J:
		break;
	case SXAVIX_K:
		break;
	case SXAVIX_L:
		break;
	case SXAVIX_M:
		break;
	case SXAVIX_PA:
		break;
	case SXAVIX_PB:
		break;
	}
}

void xavix2000_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	xavix_device::state_string_export(entry, str);

	switch(entry.index())
	{
	case SXAVIX_J:
		str = string_format("%02x", m_j);
		break;
	case SXAVIX_K:
		str = string_format("%02x", m_k);
		break;
	case SXAVIX_L:
		str = string_format("%02x", m_l);
		break;
	case SXAVIX_M:
		str = string_format("%02x", m_m);
		break;
	case SXAVIX_PA:
		str = string_format("%08x", m_pa);
		break;
	case SXAVIX_PB:
		str = string_format("%08x", m_pb);
		break;
	}
}


#include "cpu/m6502/xavix2000.hxx"
