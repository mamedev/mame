// license:BSD-3-Clause
// copyright-holders:David Shah
/***************************************************************************

    m6502_vtscr.cpp

    6502 with VRT VTxx instruction scrambling

    Scrambling in newer NES-based VTxx systems (FC pocket, etc) seems to be
    enabled with a write of 5 to 0x411E, then is activated at the next jump?
    When enabled, opcodes are to be XORed with 0xA1

    Another form of scrambling is used in the VRT VT1682, this is not yet
    implemented at all in MAME (it's used for the MiWi2 and InterAct consoles).
    This is simpler, permanently activated and consists of swapping opcode bits
    7 and 2.

***************************************************************************/

#include "emu.h"
#include "m6502_vtscr.h"

DEFINE_DEVICE_TYPE(M6502_VTSCR, m6502_vtscr, "m6502_vtscr", "M6502 with VTxx scrambling")

m6502_vtscr::m6502_vtscr(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6502_VTSCR, tag, owner, clock)
{
}

void m6502_vtscr::device_start()
{
	mintf = std::make_unique<mi_decrypt>();
	set_scramble(false);
	init();
}

void m6502_vtscr::set_next_scramble(bool scr)
{
	downcast<mi_decrypt &>(*mintf).m_next_scramble = scr;
}

void m6502_vtscr::set_scramble(bool scr)
{
	downcast<mi_decrypt &>(*mintf).m_next_scramble = scr;
	downcast<mi_decrypt &>(*mintf).m_scramble_en = scr;
}


void m6502_vtscr::device_reset()
{
	set_scramble(false);
	m6502_device::device_reset();
}

bool m6502_vtscr::mi_decrypt::toggle_scramble(uint8_t op) {
	return (op == 0x4C) || (op == 0x00) || (op == 0x6C);
}


uint8_t m6502_vtscr::mi_decrypt::read_sync(uint16_t adr)
{
	uint8_t res = cprogram.read_byte(adr);
	if(m_scramble_en)
	{
		res = descramble(res);
	}

	if(toggle_scramble(res))
	{
		m_scramble_en = m_next_scramble;
	}
	return res;
}

uint8_t m6502_vtscr::mi_decrypt::descramble(uint8_t op)
{
	return op ^ 0xA1;
}

std::unique_ptr<util::disasm_interface> m6502_vtscr::create_disassembler()
{
	return std::make_unique<disassembler>(downcast<mi_decrypt *>(mintf.get()));
}

m6502_vtscr::disassembler::disassembler(mi_decrypt *mi) : mintf(mi)
{
}

u32 m6502_vtscr::disassembler::interface_flags() const
{
	return SPLIT_DECRYPTION;
}

u8 m6502_vtscr::disassembler::decrypt8(u8 value, offs_t pc, bool opcode) const
{
	return opcode && mintf->m_scramble_en ? mintf->descramble(value) : value;
}
