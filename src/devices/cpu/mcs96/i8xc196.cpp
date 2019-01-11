// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8xc196.h

    MCS96, c196 branch, the enhanced 16 bits bus version

***************************************************************************/

#include "emu.h"
#include "i8xc196.h"
#include "i8xc196d.h"

i8xc196_device::i8xc196_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mcs96_device(mconfig, type, tag, owner, clock, 16, address_map_constructor(FUNC(i8xc196_device::internal_regs), this))
{
}

std::unique_ptr<util::disasm_interface> i8xc196_device::create_disassembler()
{
	return std::make_unique<i8xc196_disassembler>();
}

void i8xc196_device::internal_regs(address_map &map)
{
	map(0x00, 0x01).lr16("r0", []() -> u16 { return 0; }).nopw();
	map(0x08, 0x08).rw(FUNC(i8xc196_device::int_mask_r), FUNC(i8xc196_device::int_mask_w));
	map(0x09, 0x09).rw(FUNC(i8xc196_device::int_pending_r), FUNC(i8xc196_device::int_pending_w));
	map(0x18, 0xff).ram().share("register_file");
}

void i8xc196_device::do_exec_partial()
{
}

#include "cpu/mcs96/i8xc196.hxx"
