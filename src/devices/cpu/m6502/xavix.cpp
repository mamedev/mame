// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix.c

    6502 with custom opcodes
	integrated gfx / sound / coprocessor?

***************************************************************************/

#include "emu.h"
#include "xavix.h"
#include "xavixd.h"

DEFINE_DEVICE_TYPE(XAVIX, xavix_device, "xavix", "XaviX")

xavix_device::xavix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, XAVIX, tag, owner, clock)
{
}

xavix_device::xavix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock)
{
}

util::disasm_interface *xavix_device::create_disassembler()
{
	return new xavix_disassembler;
}

#include "cpu/m6502/xavix.hxx"
