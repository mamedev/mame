// license:BSD-3-Clause
// copyright-holders:David Shah, David Haywood
/***************************************************************************

    m6502_swap_op_d5_d6.cpp

    6502 with instruction scrambling

    Seen on die marked VH2009, used on polmega, silv35
    but also elsewhere

***************************************************************************/

#include "emu.h"
#include "m6502_swap_op_d5_d6.h"

DEFINE_DEVICE_TYPE(M6502_SWAP_OP_D5_D6, m6502_swap_op_d5_d6, "m6502_swap_op_d5_d6", "M6502 swapped D5/D6")

m6502_swap_op_d5_d6::m6502_swap_op_d5_d6(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6502_SWAP_OP_D5_D6, tag, owner, clock)
{
}

void m6502_swap_op_d5_d6::device_start()
{
	mintf = std::make_unique<mi_decrypt>();
	init();
}

void m6502_swap_op_d5_d6::device_reset()
{
	m6502_device::device_reset();
}

uint8_t m6502_swap_op_d5_d6::mi_decrypt::descramble(uint8_t op)
{
	return bitswap<8>(op, 7, 5, 6, 4, 3, 2, 1, 0);
}

uint8_t m6502_swap_op_d5_d6::mi_decrypt::read_sync(uint16_t adr)
{
	uint8_t res = cache->read_byte(adr);

	res = descramble(res);

	return res;
}

std::unique_ptr<util::disasm_interface> m6502_swap_op_d5_d6::create_disassembler()
{
	return std::make_unique<disassembler>(downcast<mi_decrypt *>(mintf.get()));
}

m6502_swap_op_d5_d6::disassembler::disassembler(mi_decrypt *mi) : mintf(mi)
{
}

u32 m6502_swap_op_d5_d6::disassembler::interface_flags() const
{
	return SPLIT_DECRYPTION;
}

u8 m6502_swap_op_d5_d6::disassembler::decrypt8(u8 value, offs_t pc, bool opcode) const
{
	return opcode ? mintf->descramble(value) : value;
}
