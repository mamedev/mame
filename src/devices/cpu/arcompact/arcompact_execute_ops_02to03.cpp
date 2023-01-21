// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


uint32_t arcompact_device::handleop32_LD_r_o(uint32_t op)
{
	int size = 4;

	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;
	if (S) s = -0x100 + s;

	COMMON32_GET_breg;
	COMMON32_GET_areg

	int X = (op & 0x00000040) >> 6;  //op &= ~0x00000040;
	int Z = (op & 0x00000180) >> 7;  //op &= ~0x00000180;
	int a = (op & 0x00000600) >> 9;  //op &= ~0x00000600;
//  int D = (op & 0x00000800) >> 11;// op &= ~0x00000800; // we don't use the data cache currently

	uint32_t address = m_regs[breg];

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;

		address = m_regs[LIMM_REG];
	}

	// address manipulation
	if ((a == 0) || (a == 1))
	{
		address = address + s;
	}
	else if (a == 2)
	{
		//address = address;
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
		readdata = READ32(address >> 2);

		if (X) // sign extend is not supported for long reads
			arcompact_fatal("illegal LD %08x (data size %d mode %d with X)", op, Z, a);

	}
	else if (Z == 1)
	{
		readdata = READ8(address >> 0);

		if (X) // todo
			arcompact_fatal("illegal LD %08x (data size %d mode %d with X)", op, Z, a);

	}
	else if (Z == 2)
	{
		readdata = READ16(address >> 1);

		if (X) // todo
			arcompact_fatal("illegal LD %08x (data size %d mode %d with X)", op, Z, a);

	}
	else if (Z == 3)
	{ // Z == 3 is always illegal
		arcompact_fatal("xx_ illegal LD %08x (data size %d mode %d)", op, Z, a);
	}

	m_regs[areg] = readdata;

	// writeback / increment
	if ((a == 1) || (a == 2))
	{
		if (breg==LIMM_REG)
			arcompact_fatal("yy_ illegal LD %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}

	return m_pc + (size>>0);

}

uint32_t arcompact_device::handleop32_ST_r_o(uint32_t op)
{
	int size = 4;
	int S = (op & 0x00008000) >> 15;
	int s = (op & 0x00ff0000) >> 16;
	if (S) s = -0x100 + s;

	COMMON32_GET_breg;
	COMMON32_GET_creg;

//  int R = (op & 0x00000001) >> 0; // bit 0 is reserved
	int Z = (op & 0x00000006) >> 1;
	int a = (op & 0x00000018) >> 3;
//  int D = (op & 0x00000020) >> 5; // we don't use the data cache currently

	if ((breg == LIMM_REG) || (creg == LIMM_REG))
	{
		GET_LIMM_32;
		size = 8;
	}

	uint32_t address = m_regs[breg];
	uint32_t writedata = m_regs[creg];

	// are LIMM addresses with 's' offset non-0 ('a' mode 0 / 3) legal?
	// not mentioned in docs..

	// address manipulation
	if ((a == 0) || (a == 1))
	{
		address = address + s;
	}
	else if (a == 2)
	{
		//address = address;
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
		WRITE32(address >> 2, writedata);
	}
	else if (Z == 1)
	{
		WRITE8(address >> 0, writedata);
	}
	else if (Z == 2)
	{
		WRITE16(address >> 1, writedata);
	}
	else if (Z == 3)
	{ // Z == 3 is always illegal
		arcompact_fatal("illegal ST %08x (data size %d mode %d)", op, Z, a);
	}

	// writeback / increment
	if ((a == 1) || (a == 2))
	{
		if (breg==LIMM_REG)
			arcompact_fatal("illegal ST %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}

	return m_pc + (size>>0);

}
