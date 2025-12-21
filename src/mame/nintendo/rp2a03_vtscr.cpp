// license:BSD-3-Clause
// copyright-holders:David Shah
/***************************************************************************

    rp2a03_vtscr.cpp

    RP2A03 with VRT VTxx instruction scrambling

    Scrambling in newer NES-based VTxx systems (FC pocket, etc) seems to be
    enabled with a write of 5 to 0x411E, then is activated at the next jump?
    When enabled, opcodes are to be XORed with 0xA1

***************************************************************************/

#include "emu.h"
#include "rp2a03_vtscr.h"

DEFINE_DEVICE_TYPE(RP2A03_VTSCR, rp2a03_vtscr, "rp2a03_vtscr", "RP2A03 with VTxx scrambling")

rp2a03_vtscr::rp2a03_vtscr(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	rp2a03_core_device(mconfig, RP2A03_VTSCR, tag, owner, clock)
{
}

void rp2a03_vtscr::device_start()
{
	mintf = std::make_unique<mi_decrypt>();
	set_scramble(false);
	init();
}

void rp2a03_vtscr::set_next_scramble(bool scr)
{
	downcast<mi_decrypt &>(*mintf).m_next_scramble = scr;
}

void rp2a03_vtscr::set_scramble(bool scr)
{
	downcast<mi_decrypt &>(*mintf).m_next_scramble = scr;
	downcast<mi_decrypt &>(*mintf).m_scramble_en = scr;
}


void rp2a03_vtscr::device_reset()
{
	set_scramble(false);
	rp2a03_core_device::device_reset();
}

bool rp2a03_vtscr::mi_decrypt::toggle_scramble(uint8_t op) {
	return (op == 0x4c) || (op == 0x00) || (op == 0x6c);
}


uint8_t rp2a03_vtscr::mi_decrypt::read_sync(uint16_t adr)
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

uint8_t rp2a03_vtscr::mi_decrypt::descramble(uint8_t op)
{
	return op ^ 0xa1;
}

std::unique_ptr<util::disasm_interface> rp2a03_vtscr::create_disassembler()
{
	return std::make_unique<disassembler>(downcast<mi_decrypt *>(mintf.get()));
}

rp2a03_vtscr::disassembler::disassembler(mi_decrypt *mi) : mintf(mi)
{
}

u32 rp2a03_vtscr::disassembler::interface_flags() const
{
	return SPLIT_DECRYPTION;
}

u8 rp2a03_vtscr::disassembler::decrypt8(u8 value, offs_t pc, bool opcode) const
{
	return opcode && mintf->m_scramble_en ? mintf->descramble(value) : value;
}
