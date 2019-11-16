// license:BSD-3-Clause
// copyright-holders:David Shah, David Haywood
/***************************************************************************

    m6502_vh2009.cpp

    6502 with instruction scrambling

    Die is marked VH2009, used on polmega, silv35

***************************************************************************/

#include "emu.h"
#include "m6502_vh2009.h"

DEFINE_DEVICE_TYPE(M6502_VH2009, m6502_vh2009, "m6502_vh2009", "VRT VH2009")

m6502_vh2009::m6502_vh2009(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6502_VH2009, tag, owner, clock)
{
}

void m6502_vh2009::device_start()
{
	mintf = std::make_unique<mi_decrypt>();
	init();
}

void m6502_vh2009::device_reset()
{
	m6502_device::device_reset();
}

uint8_t m6502_vh2009::mi_decrypt::descramble(uint8_t op)
{
	return bitswap<8>(op, 7, 5, 6, 4, 3, 2, 1, 0);
}

uint8_t m6502_vh2009::mi_decrypt::read_sync(uint16_t adr)
{
	uint8_t res = cache->read_byte(adr);

	res = descramble(res);

	return res;
}

std::unique_ptr<util::disasm_interface> m6502_vh2009::create_disassembler()
{
	return std::make_unique<disassembler>(downcast<mi_decrypt *>(mintf.get()));
}

m6502_vh2009::disassembler::disassembler(mi_decrypt *mi) : mintf(mi)
{
}

u32 m6502_vh2009::disassembler::interface_flags() const
{
	return SPLIT_DECRYPTION;
}

u8 m6502_vh2009::disassembler::decrypt8(u8 value, offs_t pc, bool opcode) const
{
	return opcode ? mintf->descramble(value) : value;
}
