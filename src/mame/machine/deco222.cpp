// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Data East encrypted CPU 222, aka C10707?
 also sometimes implemented as basic logic outside the CPU on early revs and bootlegs */


#include "emu.h"
#include "deco222.h"

DEFINE_DEVICE_TYPE(DECO_222,    deco_222_device,    "deco222",    "DECO 222")
DEFINE_DEVICE_TYPE(DECO_C10707, deco_c10707_device, "decoc10707", "DECO C10707")


deco_222_device::deco_222_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, DECO_222, tag, owner, clock)
{
}

void deco_222_device::device_start()
{
	mintf = std::make_unique<mi_decrypt>();
	init();
}

void deco_222_device::device_reset()
{
	m6502_device::device_reset();
	downcast<mi_decrypt &>(*mintf).had_written = false;
}

uint8_t deco_222_device::mi_decrypt::read_sync(uint16_t adr)
{
	return bitswap<8>(cache->read_byte(adr) ,7,5,6,4,3,2,1,0);
}

std::unique_ptr<util::disasm_interface> deco_222_device::create_disassembler()
{
	return std::make_unique<disassembler>();
}

u32 deco_222_device::disassembler::interface_flags() const
{
	return SPLIT_DECRYPTION;
}

u8 deco_222_device::disassembler::decrypt8(u8 value, offs_t pc, bool opcode) const
{
	return opcode ? bitswap<8>(value,7,5,6,4,3,2,1,0) : value;
}

deco_c10707_device::deco_c10707_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, DECO_C10707, tag, owner, clock)
{
}

void deco_c10707_device::device_start()
{
	mintf = std::make_unique<mi_decrypt>();
	init();
}

void deco_c10707_device::device_reset()
{
	m6502_device::device_reset();
	downcast<mi_decrypt &>(*mintf).had_written = false;
}

uint8_t deco_c10707_device::mi_decrypt::read_sync(uint16_t adr)
{
	return bitswap<8>(cache->read_byte(adr) ,7,5,6,4,3,2,1,0);
}

std::unique_ptr<util::disasm_interface> deco_c10707_device::create_disassembler()
{
	return std::make_unique<disassembler>();
}

u32 deco_c10707_device::disassembler::interface_flags() const
{
	return SPLIT_DECRYPTION;
}

u8 deco_c10707_device::disassembler::decrypt8(u8 value, offs_t pc, bool opcode) const
{
	return opcode ? bitswap<8>(value,7,5,6,4,3,2,1,0) : value;
}
