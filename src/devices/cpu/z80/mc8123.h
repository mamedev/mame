// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Widel
/***************************************************************************

  NEC MC-8123 encryption emulation

***************************************************************************/

#ifndef MAME_CPU_Z80_MC8123_H
#define MAME_CPU_Z80_MC8123_H

#pragma once

#include "z80.h"

class mc8123_device : public z80_device
{
public:
	// construction/destruction
	mc8123_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// this function assumes a fixed portion of ROM at 0000-7FFF, and
	// an arbitrary amount of banks at 8000-BFFF.
	void decode(u8 *rom, u8 *opcodes, unsigned length);

private:
	static u8 decrypt_type0(u8 val, u8 param, unsigned swap);
	static u8 decrypt_type1a(u8 val, u8 param, unsigned swap);
	static u8 decrypt_type1b(u8 val, u8 param, unsigned swap);
	static u8 decrypt_type2a(u8 val, u8 param, unsigned swap);
	static u8 decrypt_type2b(u8 val, u8 param, unsigned swap);
	static u8 decrypt_type3a(u8 val, u8 param, unsigned swap);
	static u8 decrypt_type3b(u8 val, u8 param, unsigned swap);
	static u8 decrypt_internal(u8 val, u8 key, bool opcode);

	u8 decrypt(offs_t addr, u8 val, bool opcode);

	required_region_ptr<u8> m_key;
};

DECLARE_DEVICE_TYPE(MC8123, mc8123_device)

#endif // MAME_CPU_Z80_MC8123_H
