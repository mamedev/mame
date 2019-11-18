// license:BSD-3-Clause
// copyright-holders:David Shah, David Haywood
/***************************************************************************

    m6502_vt1682.cpp

    6502 with instruction scrambling

    This form of scrambling is used in the VRT VT1682
	(it's used for the MiWi2 and InterAct consoles).
    
	This is simpler, permanently activated and consists of swapping opcode bits
    7 and 2.

***************************************************************************/

#include "emu.h"
#include "m6502_vt1682.h"

DEFINE_DEVICE_TYPE(M6502_VT1682, m6502_vt1682, "m6502_vt1682", "VRT VT1682")

m6502_vt1682::m6502_vt1682(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6502_VT1682, tag, owner, clock)
{
}

void m6502_vt1682::device_start()
{
	mintf = std::make_unique<mi_decrypt>();
	init();
}

void m6502_vt1682::device_reset()
{
	m6502_device::device_reset();
}

uint8_t m6502_vt1682::mi_decrypt::descramble(uint8_t op)
{
	return bitswap<8>(op, 2, 6, 5, 4, 3, 7, 1, 0);
}

uint8_t m6502_vt1682::mi_decrypt::read_sync(uint16_t adr)
{
	uint8_t res = cache->read_byte(adr);

	res = descramble(res);

	return res;
}

std::unique_ptr<util::disasm_interface> m6502_vt1682::create_disassembler()
{
	return std::make_unique<disassembler>(downcast<mi_decrypt *>(mintf.get()));
}

m6502_vt1682::disassembler::disassembler(mi_decrypt *mi) : mintf(mi)
{
}

u32 m6502_vt1682::disassembler::interface_flags() const
{
	return SPLIT_DECRYPTION;
}

u8 m6502_vt1682::disassembler::decrypt8(u8 value, offs_t pc, bool opcode) const
{
	return opcode ? mintf->descramble(value) : value;
}
