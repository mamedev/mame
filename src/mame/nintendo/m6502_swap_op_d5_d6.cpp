// license:BSD-3-Clause
// copyright-holders:David Shah, David Haywood
/***************************************************************************

    m6502_swap_op_d5_d6.cpp

    6502 / RP2A03 with instruction scrambling

    Seen on die marked VH2009, used on polmega, silv35
    these are RP2A03 derived CPUs used on VTxx systems

    VT1682 systems with this scrambling currently derive from M6502 type
    but this might be incorrect

***************************************************************************/

#include "emu.h"
#include "m6502_swap_op_d5_d6.h"

DEFINE_DEVICE_TYPE(M6502_SWAP_OP_D5_D6, m6502_swap_op_d5_d6, "m6502_swap_op_d5_d6", "M6502 swapped D5/D6")
DEFINE_DEVICE_TYPE(RP2A03_CORE_SWAP_OP_D5_D6, rp2a03_core_swap_op_d5_d6, "rp2a03_core_swap_op_d5_d6", "RP2A03 core with optional code bitswap")

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
	downcast<mi_decrypt &>(*mintf).m_encryption_enabled = true;
	m6502_device::device_reset();
}

void m6502_swap_op_d5_d6::set_encryption_state(bool state)
{
	downcast<mi_decrypt &>(*mintf).m_encryption_enabled = state;
}

uint8_t m6502_swap_op_d5_d6::mi_decrypt::descramble(uint8_t op)
{
	if (m_encryption_enabled)
		return bitswap<8>(op, 7, 5, 6, 4, 3, 2, 1, 0);
	else
		return op;
}

uint8_t m6502_swap_op_d5_d6::mi_decrypt::read_sync(uint16_t adr)
{
	uint8_t res = cprogram.read_byte(adr);

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





rp2a03_core_swap_op_d5_d6::rp2a03_core_swap_op_d5_d6(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	rp2a03_core_device(mconfig, RP2A03_CORE_SWAP_OP_D5_D6, tag, owner, clock),
	m_encryption_enabled_on_reset(true),
	m_cryptconfig(0)
{
}

void rp2a03_core_swap_op_d5_d6::device_start()
{
	mintf = std::make_unique<mi_decrypt>();
	init();
	save_item(downcast<mi_decrypt &>(*mintf).m_encryption_enabled, "m_encryption_enabled");
}

void rp2a03_core_swap_op_d5_d6::device_reset()
{
	downcast<mi_decrypt &>(*mintf).m_encryption_enabled = m_encryption_enabled_on_reset;
	downcast<mi_decrypt &>(*mintf).m_whichcrypt = m_cryptconfig;
	rp2a03_core_device::device_reset();
}

void rp2a03_core_swap_op_d5_d6::set_encryption_state(bool state)
{
	downcast<mi_decrypt &>(*mintf).m_encryption_enabled = state;
}

uint8_t rp2a03_core_swap_op_d5_d6::mi_decrypt::descramble(uint8_t op)
{
	if (m_encryption_enabled)
	{
		switch (m_whichcrypt)
		{
		case 0: return bitswap<8>(op, 7, 5, 6, 4, 3, 2, 1, 0); break; // most sets (d5/d6 swap)
		case 1: return bitswap<8>(op, 7, 6, 5, 1, 3, 2, 4, 0); break; // red5mam/dgun2593/gcs2mgp/240in1ar
		case 2: return bitswap<8>(op, 7, 6, 4, 5, 3, 2, 1, 0); break; // vibes240
		case 3: return bitswap<8>(op, 6, 7, 5, 4, 3, 1, 2, 0); break; // gbox2020
		case 4: return bitswap<8>(op, 7, 6, 4, 5, 3, 1, 2, 0); break; // s10fake
		}
	}

	return op;
}

uint8_t rp2a03_core_swap_op_d5_d6::mi_decrypt::read_sync(uint16_t adr)
{
	uint8_t res = cprogram.read_byte(adr);

	res = descramble(res);

	return res;
}

std::unique_ptr<util::disasm_interface> rp2a03_core_swap_op_d5_d6::create_disassembler()
{
	return std::make_unique<disassembler>(downcast<mi_decrypt *>(mintf.get()));
}

rp2a03_core_swap_op_d5_d6::disassembler::disassembler(mi_decrypt *mi) : mintf(mi)
{
}

u32 rp2a03_core_swap_op_d5_d6::disassembler::interface_flags() const
{
	return SPLIT_DECRYPTION;
}

u8 rp2a03_core_swap_op_d5_d6::disassembler::decrypt8(u8 value, offs_t pc, bool opcode) const
{
	return opcode ? mintf->descramble(value) : value;
}

