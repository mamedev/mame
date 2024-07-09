// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************
    Z80N
***************************************************************************/

#include "emu.h"
#include "z80n.h"
#include "z80ndasm.h"

#include "z80.inc"

DEFINE_DEVICE_TYPE(Z80N, z80n_device, "z80n", "Z80N")

std::unique_ptr<util::disasm_interface> z80n_device::create_disassembler()
{
	return std::make_unique<z80n_disassembler>();
}

z80n_device::z80n_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, Z80N, tag, owner, clock)
	, m_out_retn_seen_cb(*this)
	, m_in_nextreg_cb(*this, 0)
	, m_out_nextreg_cb(*this)
{
}

void z80n_device::do_op()
{
	#include "cpu/z80/z80n.hxx"
}

void z80n_device::device_start()
{
	z80_device::device_start();
	save_item(NAME(m_stackless));
}

void z80n_device::device_reset()
{
	z80_device::device_reset();
	m_stackless = 0;
}
