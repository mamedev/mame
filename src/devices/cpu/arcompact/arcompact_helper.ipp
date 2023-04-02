// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_CPU_ARCOMPACT_ARCOMPACT_HELPER_IPP
#define MAME_CPU_ARCOMPACT_ARCOMPACT_HELPER_IPP

#pragma once

#include "arcompact.h"


inline bool arcompact_device::check_condition(uint8_t condition)
{
	switch (condition & 0x1f)
	{
	case 0x00: return condition_AL();
	case 0x01: return condition_EQ();
	case 0x02: return condition_NE();
	case 0x03: return condition_PL();
	case 0x04: return condition_MI();
	case 0x05: return condition_CS();
	case 0x06: return condition_HS();
	case 0x07: return condition_VS();
	case 0x08: return condition_VC();
	case 0x09: return condition_GT();
	case 0x0a: return condition_GE();
	case 0x0b: return condition_LT();
	case 0x0c: return condition_LE();
	case 0x0d: return condition_HI();
	case 0x0e: return condition_LS();
	case 0x0f: return condition_PNZ();

	default: fatalerror("unhandled condition check %02x", condition & 0x1f); return false;
	}
	return false;
}


inline void arcompact_device::do_flags_overflow(uint32_t result, uint32_t b, uint32_t c)
{
	if ((b & 0x80000000) == (c & 0x80000000))
	{
		if ((result & 0x80000000) != (b & 0x80000000))
		{
			status32_set_v();
		}
		else
		{
			status32_clear_v();
		}
	}
}

inline void arcompact_device::do_flags_add(uint32_t result, uint32_t b, uint32_t c)
{
	do_flags_nz(result);
	do_flags_overflow(result, b, c);

	if (result < b)
	{
		status32_set_c();
	}
	else
	{
		status32_clear_c();
	}
}

inline void arcompact_device::do_flags_sub(uint32_t result, uint32_t b, uint32_t c)
{
	do_flags_nz(result);
	do_flags_overflow(result, b, c);

	if (result > b)
	{
		status32_set_c();
	}
	else
	{
		status32_clear_c();
	}
}

inline void arcompact_device::do_flags_nz(uint32_t result)
{
	if (result & 0x80000000) { status32_set_n(); }
	else { status32_clear_n(); }
	if (result == 0x00000000) { status32_set_z(); }
	else { status32_clear_z(); }
}

inline void arcompact_device::arcompact_handle_ld_helper(uint32_t op, uint8_t areg, uint8_t breg, uint32_t s, uint8_t X, uint8_t Z, uint8_t a)
{
	// writeback / increment
	if (a == 1)
	{
		if (breg == REG_LIMM)
			fatalerror("illegal LD helper %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

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
			fatalerror("illegal LD helper %08x (data size %d mode %d)", op, Z, a);
		}
	}

	uint32_t readdata = 0;

	// read data
	if (Z == 0)
	{
		readdata = READ32(address);
		m_regs[areg] = readdata;
		if (X) // sign extend is not supported for long reads
			fatalerror("illegal LD helper %08x (data size %d mode %d with X)", op, Z, a);
	}
	else if (Z == 1)
	{
		readdata = READ8(address);

		if (X)
		{
			readdata = util::sext(readdata, 8);
			m_regs[areg] = readdata;
		}
		else
		{
			m_regs[areg] = readdata;
		}
	}
	else if (Z == 2)
	{
		readdata = READ16(address);

		if (X)
		{
			readdata = util::sext(readdata, 16);
			m_regs[areg] = readdata;
		}
		else
		{
			m_regs[areg] = readdata;
		}
	}
	else if (Z == 3)
	{ // Z == 3 is always illegal
		fatalerror("illegal LD helper %08x (data size %d mode %d)", op, Z, a);
	}

	// writeback / increment
	if (a == 2)
	{
		if (breg == REG_LIMM)
			fatalerror("illegal LD helper %08x (data size %d mode %d)", op, Z, a); // using the LIMM as the base register and an increment mode is illegal

		m_regs[breg] = m_regs[breg] + s;
	}
}

inline uint32_t arcompact_device::handleop32_general(uint32_t op, ophandler32 ophandler)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(breg, creg);
		m_regs[common32_get_areg(op)] = ophandler(*this, m_regs[breg], m_regs[creg], common32_get_F(op));
		return m_pc + size;
	}

	case 0x01:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		m_regs[common32_get_areg(op)] = ophandler(*this, m_regs[breg], common32_get_u6(op), common32_get_F(op));
		return m_pc + size;
	}
	case 0x02:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		m_regs[breg] = ophandler(*this, m_regs[breg], common32_get_s12(op), common32_get_F(op));
		return m_pc + size;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			uint8_t breg = common32_get_breg(op);
			uint8_t creg = common32_get_creg(op);
			int size = check_limm(breg, creg);
			if (check_condition(common32_get_condition(op)))
				m_regs[breg] = ophandler(*this, m_regs[breg], m_regs[creg], common32_get_F(op));
			return m_pc + size;
		}
		case 0x01:
		{
			uint8_t breg = common32_get_breg(op);
			int size = check_limm(breg);
			if (check_condition(common32_get_condition(op)))
				m_regs[breg] = ophandler(*this, m_regs[breg], common32_get_u6(op), common32_get_F(op));
			return m_pc + size;
		}
		}
	}
	}
	return 0;
}

inline uint32_t arcompact_device::handleop32_general_MULx64(uint32_t op, ophandler32_mul ophandler)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(breg, creg);
		ophandler(*this, m_regs[breg], m_regs[creg]);
		return m_pc + size;
	}

	case 0x01:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		ophandler(*this, m_regs[breg], common32_get_u6(op));
		return m_pc + size;
	}
	case 0x02:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		ophandler(*this, m_regs[breg], common32_get_s12(op));
		return m_pc + size;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			uint8_t breg = common32_get_breg(op);
			uint8_t creg = common32_get_creg(op);
			int size = check_limm(breg, creg);
			if (!check_condition(common32_get_condition(op)))
				return m_pc + size;
			ophandler(*this, m_regs[breg], m_regs[creg]);
			return m_pc + size;
		}

		case 0x01:
		{
			uint8_t breg = common32_get_breg(op);
			int size = check_limm(breg);
			if (!check_condition(common32_get_condition(op)))
				return m_pc + size;
			ophandler(*this, m_regs[breg], common32_get_u6(op));
			return m_pc + size;
		}
		}
	}
	}

	return 0;
}

inline uint32_t arcompact_device::handleop32_general_nowriteback_forced_flag(uint32_t op, ophandler32_ff ophandler)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(breg, creg);
		ophandler(*this, m_regs[breg], m_regs[creg]);
		return m_pc + size;
	}
	case 0x01:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		ophandler(*this, m_regs[breg], common32_get_u6(op));
		return m_pc + size;
	}
	case 0x02:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		ophandler(*this, m_regs[breg], common32_get_s12(op));
		return m_pc + size;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			uint8_t breg = common32_get_breg(op);
			uint8_t creg = common32_get_creg(op);
			int size = check_limm(breg, creg);
			if (check_condition(common32_get_condition(op)))
				ophandler(*this, m_regs[breg], m_regs[creg]);
			return m_pc + size;
		}
		case 0x01:
		{
			uint8_t breg = common32_get_breg(op);
			int size = check_limm(breg);
			if (check_condition(common32_get_condition(op)))
				ophandler(*this, m_regs[breg], common32_get_u6(op));
			return m_pc + size;
		}
		}
	}
	}

	return 0;
}

inline uint32_t arcompact_device::handleop32_general_SOP_group(uint32_t op, ophandler32_sop ophandler)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(breg, creg);
		m_regs[breg] = ophandler(*this, m_regs[creg], common32_get_F(op));
		return m_pc + size;
	}
	case 0x01:
	{
		uint8_t breg = common32_get_breg(op);
		int size = check_limm(breg);
		m_regs[breg] = ophandler(*this, common32_get_u6(op), common32_get_F(op));
		return m_pc + size;
	}
	case 0x02:
	case 0x03:
		fatalerror("SOP Group: illegal mode 02/03 specifying use of bits already assigned to opcode select: opcode %04x\n", op);
		return 0;
	}
	return 0;
}

#endif // MAME_CPU_ARCOMPACT_ARCOMPACT_HELPER_IPP
