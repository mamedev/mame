// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LD<zz><.x><.aa><.di> a,[b,c]    0010 0bbb aa11 0ZZX   DBBB CCCC CCAA AAAA
// LD<zz><.x><.aa><.di> 0,[b,c]    0010 0bbb aa11 0ZZX   DBBB CCCC CC11 1110
// PREFETCH<.aa> [b,c]             0010 0bbb aa11 0000   0BBB CCCC CC11 1110    (prefetch is an alias)
//
// LD<zz><.x><.aa><.di> a,[b,limm] 0010 0bbb aa11 0ZZX   DBBB 1111 10AA AAAA (+ Limm)
// LD<zz><.x><.aa><.di> 0,[b,limm] 0010 0bbb aa11 0ZZX   DBBB 1111 1011 1110 (+ Limm)
// PREFETCH<.aa> [b,limm]          0010 0bbb aa11 0000   0BBB 1111 1011 1110 (+ Limm) (prefetch is an alias)
//
// LD<zz><.x><.di> a,[limm,c]      0010 0110 RR11 0ZZX   D111 CCCC CCAA AAAA (+ Limm)
// LD<zz><.x><.di> 0,[limm,c]      0010 0110 RR11 0ZZX   D111 CCCC CC11 1110 (+ Limm)
// PREFETCH [limm,c]               0010 0110 RR11 0000   0111 CCCC CC11 1110 (+ Limm) (prefetch is an alias)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::arcompact_handle04_3x_helper(uint32_t op, int dsize, int extend)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	uint8_t areg = common32_get_areg(op);

	uint8_t X = extend;
	uint32_t s = m_regs[creg];
	int a = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	//int D = (op & 0x00008000) >> 15; op &= ~0x00008000; // D isn't handled
	uint8_t Z = dsize;

	// writeback / increment
	if (a == 1)
	{
		if (breg == LIMM_REG)
			arcompact_fatal("illegal LD 3x %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

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
			arcompact_fatal("illegal LD 3x %08x (data size %d mode %d)", op, Z, a);
		}
	}

	uint32_t readdata = 0;

	// read data
	if (Z == 0)
	{
		readdata = READ32(address);

		if (X) // sign extend is not supported for long reads
			arcompact_fatal("illegal LD 3x %08x (data size %d mode %d with X)", op, Z, a);

	}
	else if (Z == 1)
	{
		readdata = READ8(address);

		if (X)
		{
			if (readdata & 0x80)
				readdata |= 0xffffff00;
		}
	}
	else if (Z == 2)
	{
		readdata = READ16(address);

		if (X)
		{
			if (readdata & 0x8000)
				readdata |= 0xffff0000;
		}
	}
	else if (Z == 3)
	{ // Z == 3 is always illegal
		arcompact_fatal("illegal LD 3x %08x (data size %d mode %d)", op, Z, a);
	}

	m_regs[areg] = readdata;

	// writeback / increment
	if (a == 2)
	{
		if (breg == LIMM_REG)
			arcompact_fatal("illegal LD 3x %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}

	return m_pc + size;
}

uint32_t arcompact_device::handleop32_LD_0(uint32_t op)  { return arcompact_handle04_3x_helper(op,0,0); }
// ZZ value of 0x0 with X of 1 is illegal
uint32_t arcompact_device::handleop32_LD_1(uint32_t op)  { return arcompact_handle04_3x_helper(op,0,1); }
uint32_t arcompact_device::handleop32_LD_2(uint32_t op)  { return arcompact_handle04_3x_helper(op,1,0); }
uint32_t arcompact_device::handleop32_LD_3(uint32_t op)  { return arcompact_handle04_3x_helper(op,1,1); }
uint32_t arcompact_device::handleop32_LD_4(uint32_t op)  { return arcompact_handle04_3x_helper(op,2,0); }
uint32_t arcompact_device::handleop32_LD_5(uint32_t op)  { return arcompact_handle04_3x_helper(op,2,1); }
// ZZ value of 0x3 is illegal
uint32_t arcompact_device::handleop32_LD_6(uint32_t op)  { return arcompact_handle04_3x_helper(op,3,0); }
uint32_t arcompact_device::handleop32_LD_7(uint32_t op)  { return arcompact_handle04_3x_helper(op,3,1); }
