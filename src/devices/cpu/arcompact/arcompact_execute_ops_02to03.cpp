// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I
// LD<zz><.x><.aa><.di> a,[b,s9]   0001 0bbb ssss ssss   SBBB DaaZ ZXAA AAAA
// LD<zz><.x><.di> a,[limm]        0001 0110 0000 0000   0111 DRRZ ZXAA AAAA (+ Limm)
// LD<zz><.x><.aa><.di> 0,[b,s9]   0001 0bbb ssss ssss   SBBB DaaZ ZX11 1110
// LD<zz><.x><.di> 0,[limm]        0001 0110 0000 0000   0111 DRRZ ZX11 1110 (+ Limm)
//
// PREFETCH<.aa> [b,s9]            0001 0bbb ssss ssss   SBBB 0aa0 0011 1110
// PREFETCH [limm]                 0001 0110 0000 0000   0111 0RR0 0011 1110 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_LD_r_o(uint32_t op)
{
	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;
	if (S) s = -0x100 + s;

	uint8_t breg = common32_get_breg(op);
	uint8_t areg = common32_get_areg(op);

	int X = (op & 0x00000040) >> 6;  //op &= ~0x00000040;
	int Z = (op & 0x00000180) >> 7;  //op &= ~0x00000180;
	int a = (op & 0x00000600) >> 9;  //op &= ~0x00000600;
//  int D = (op & 0x00000800) >> 11;// op &= ~0x00000800; // we don't use the data cache currently


	int size = check_b_limm(breg);

	// writeback / increment
	if (a == 1)
	{
		if (breg==LIMM_REG)
			arcompact_fatal("yy_ illegal LD %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}

	uint32_t address = m_regs[breg];

	// address manipulation
	if (a == 0)
	{
		address = address + s;
	}
	else if (a == 3)
	{
		if (Z == 0)
		{
			address = address + (s << 2);
		}
		else if (Z == 2)
		{
			address = address + (s << 1);
		}
		else // Z == 1 and Z == 3 are invalid here
		{
			arcompact_fatal("zz_ illegal LD %08x (data size %d mode %d)", op, Z, a);
		}
	}

	uint32_t readdata = 0;

	// read data
	if (Z == 0)
	{
		readdata = READ32(address);

		if (X) // sign extend is not supported for long reads
			arcompact_fatal("illegal LD %08x (data size %d mode %d with X)", op, Z, a);

	}
	else if (Z == 1)
	{
		readdata = READ8(address);

		if (X) // todo
			arcompact_fatal("illegal LD %08x (data size %d mode %d with X)", op, Z, a);

	}
	else if (Z == 2)
	{
		readdata = READ16(address);

		if (X) // todo
			arcompact_fatal("illegal LD %08x (data size %d mode %d with X)", op, Z, a);

	}
	else if (Z == 3)
	{ // Z == 3 is always illegal
		arcompact_fatal("xx_ illegal LD %08x (data size %d mode %d)", op, Z, a);
	}

	m_regs[areg] = readdata;

	// writeback / increment
	if (a == 2)
	{
		if (breg==LIMM_REG)
			arcompact_fatal("yy_ illegal LD %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}

	return m_pc + (size>>0);

}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I
// ST<zz><.aa><.di> c,[b,s9]       0001 1bbb ssss ssss   SBBB CCCC CCDa aZZR
// ST<zz><.di> c,[limm]            0001 1110 0000 0000   0111 CCCC CCDR RZZR (+ Limm)
// ST<zz><.aa><.di> limm,[b,s9]    0001 1bbb ssss ssss   SBBB 1111 10Da aZZR (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// can be used as a PUSH when breg is stack register (28), s is -4 (0x1fc) Z is 0, D is 0, and a is 1
uint32_t arcompact_device::handleop32_ST_r_o(uint32_t op)
{
	int S = (op & 0x00008000) >> 15;
	int s = (op & 0x00ff0000) >> 16;
	if (S) s = -0x100 + s;

	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);

//  int R = (op & 0x00000001) >> 0; // bit 0 is reserved
	int Z = (op & 0x00000006) >> 1;
	int a = (op & 0x00000018) >> 3;
//  int D = (op & 0x00000020) >> 5; // we don't use the data cache currently

	int size = check_b_c_limm(breg, creg);

	// writeback / increment
	if (a == 1)
	{
		if (breg==LIMM_REG)
			arcompact_fatal("illegal ST %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}

	uint32_t address = m_regs[breg];
	uint32_t writedata = m_regs[creg];

	// are LIMM addresses with 's' offset non-0 ('a' mode 0 / 3) legal?
	// not mentioned in docs..


	// address manipulation
	if (a == 0)
	{
		address = address + s;
	}
	else if (a == 3)
	{
		if (Z == 0)
			address = address + (s << 2);
		else if (Z==2)
			address = address + (s << 1);
		else // Z == 1 and Z == 3 are invalid here
			arcompact_fatal("illegal ST %08x (data size %d mode %d)", op, Z, a);
	}

	// write data
	if (Z == 0)
	{
		WRITE32(address, writedata);
	}
	else if (Z == 1)
	{
		WRITE8(address, writedata);
	}
	else if (Z == 2)
	{
		WRITE16(address, writedata);
	}
	else if (Z == 3)
	{ // Z == 3 is always illegal
		arcompact_fatal("illegal ST %08x (data size %d mode %d)", op, Z, a);
	}

	// writeback / increment
	if (a == 2)
	{
		if (breg==LIMM_REG)
			arcompact_fatal("illegal ST %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}

	return m_pc + (size>>0);

}
