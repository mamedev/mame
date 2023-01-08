// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


#define REG_BLINK (0x1f) // r31
#define REG_SP (0x1c) // r28
#define REG_ILINK1 (0x1d) // r29
#define REG_ILINK2 (0x1e) // r30
#define REG_LP_COUNT (0x3c) // r60

#define ARCOMPACT_LOGGING 1

#define arcompact_fatal if (ARCOMPACT_LOGGING) fatalerror
#define arcompact_log if (ARCOMPACT_LOGGING) fatalerror


void arcompact_device::execute_run()
{
	//uint32_t lres;
	//lres = 0;

	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

//      printf("new pc %04x\n", m_pc);

		if (m_delayactive)
		{
			uint16_t op = READ16((m_pc + 0) >> 1);
			m_pc = get_instruction(op);
			if (m_delaylinks) m_regs[REG_BLINK] = m_pc;

			m_pc = m_delayjump;
			m_delayactive = 0; m_delaylinks = 0;
		}
		else
		{
			uint16_t op = READ16((m_pc + 0) >> 1);
			m_pc = get_instruction(op);
		}

		// hardware loops
		if (m_pc == m_LP_END)
		{
			if (m_regs[REG_LP_COUNT] != 1)
			{
				m_pc = m_LP_START;
			}
			m_regs[REG_LP_COUNT]--;

		}

		m_icount--;
	}

}


#define GET_01_01_01_BRANCH_ADDR \
	int32_t address = (op & 0x00fe0000) >> 17; \
	address |= ((op & 0x00008000) >> 15) << 7; \
	if (address & 0x80) address = -0x80 + (address & 0x7f);

#define GROUP_0e_GET_h \
	h =  ((op & 0x0007) << 3); \
	h |= ((op & 0x00e0) >> 5);
#define COMMON32_GET_breg \
	int b_temp = (op & 0x07000000) >> 24; \
	int B_temp = (op & 0x00007000) >> 12; \
	int breg = b_temp | (B_temp << 3);
#define COMMON32_GET_creg \
	int creg = (op & 0x00000fc0) >> 6;
#define COMMON32_GET_u6 \
	int u = (op & 0x00000fc0) >> 6;
#define COMMON32_GET_areg \
	int areg = (op & 0x0000003f) >> 0;
#define COMMON32_GET_areg_reserved \
	int ares = (op & 0x0000003f) >> 0;
#define COMMON32_GET_F \
	int F = (op & 0x00008000) >> 15;
#define COMMON32_GET_p \
	int p = (op & 0x00c00000) >> 22;

#define COMMON32_GET_s12 \
		int S_temp = (op & 0x0000003f) >> 0; \
		int s_temp = (op & 0x00000fc0) >> 6; \
		int32_t S = s_temp | (S_temp<<6); \
		if (S & 0x800) S = -0x800 + (S&0x7ff); /* sign extend */
#define COMMON32_GET_CONDITION \
		uint8_t condition = op & 0x0000001f;


#define COMMON16_GET_breg \
	breg =  ((op & 0x0700) >>8);
#define COMMON16_GET_creg \
	creg =  ((op & 0x00e0) >>5);
#define COMMON16_GET_areg \
	areg =  ((op & 0x0007) >>0);
#define COMMON16_GET_u3 \
	u =  ((op & 0x0007) >>0);
#define COMMON16_GET_u5 \
	u =  ((op & 0x001f) >>0);
#define COMMON16_GET_u8 \
	u =  ((op & 0x00ff) >>0);
#define COMMON16_GET_u7 \
	u =  ((op & 0x007f) >>0);
#define COMMON16_GET_s9 \
	s =  ((op & 0x01ff) >>0);
// registers used in 16-bit opcodes have a limited range
// and can only address registers r0-r3 and r12-r15

#define REG_16BIT_RANGE(_reg_) \
	if (_reg_>3) _reg_+= 8;

#define GET_LIMM_32 \
	limm = (READ16((m_pc + 4) >> 1) << 16); \
	limm |= READ16((m_pc + 6) >> 1);
#define GET_LIMM_16 \
	limm = (READ16((m_pc + 2) >> 1) << 16); \
	limm |= READ16((m_pc + 4) >> 1);

#define PC_ALIGNED32 \
	(m_pc&0xfffffffc)

int arcompact_device::check_condition(uint8_t condition)
{
	switch (condition & 0x1f)
	{
		case 0x00: return 1; // AL
		case 0x01: return CONDITION_EQ;
		case 0x02: return !CONDITION_EQ; // NE
		case 0x03: fatalerror("unhandled condition check %s", arcompact_disassembler::conditions[condition]); return -1;
		case 0x04: return CONDITION_MI; // MI (N)
		case 0x05: return CONDITION_CS; // CS (Carry Set / Lower than)
		default: fatalerror("unhandled condition check %s", arcompact_disassembler::conditions[condition]); return -1;
	}

	return -1;
}

// handlers

uint32_t arcompact_device::handle_jump_to_addr(int delay, int link, uint32_t address, uint32_t next_addr)
{
	if (delay)
	{
		m_delayactive = 1;
		m_delayjump = address;
		if (link) m_delaylinks = 1;
		else m_delaylinks = 0;
		return next_addr;
	}
	else
	{
		if (link) m_regs[REG_BLINK] = next_addr;
		return address;
	}

}

uint32_t arcompact_device::handle_jump_to_register(int delay, int link, uint32_t reg, uint32_t next_addr, int flag)
{
	if (reg == LIMM_REG)
		arcompact_fatal("handle_jump_to_register called with LIMM register, call handle_jump_to_addr instead");

	if ((reg == REG_ILINK1) || (reg == REG_ILINK2))
	{
		if (flag)
		{
			arcompact_fatal("jump to ILINK1/ILINK2 not supported");
			return next_addr;
		}
		else
		{
			arcompact_fatal("illegal jump to ILINK1/ILINK2 not supported"); // FLAG bit must be set
			return next_addr;
		}
	}
	else
	{
		if (flag)
		{
			arcompact_fatal("illegal jump (flag bit set)"); // FLAG bit must NOT be set
			return next_addr;
		}
		else
		{
			//arcompact_fatal("jump not supported");
			uint32_t target = m_regs[reg];
			return handle_jump_to_addr(delay, link, target, next_addr);
		}
	}

	return 0;
}

uint32_t arcompact_device::handleop32_B_cc_D_s21(uint32_t op)
{
	int size = 4;

	COMMON32_GET_CONDITION

	if (!check_condition(condition))
		return m_pc + (size>>0);

	// Branch Conditionally
	// 0000 0sss ssss sss0 SSSS SSSS SSNQ QQQQ
	int32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x80000) address = -0x80000 + (address & 0x7ffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	uint32_t realaddress = PC_ALIGNED32 + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 0; // don't link
	}
	else
	{
	//  m_regs[REG_BLINK] = m_pc + (size >> 0);  // don't link
		return realaddress;
	}


	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_B_D_s25(uint32_t op)
{
	int size = 4;
	// Branch Unconditionally Far
	int32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address |= ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address & 0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
//  int res =  (op & 0x00000010) >> 4; op &= ~0x00000010; // should be set to 0

	uint32_t realaddress = PC_ALIGNED32 + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 0; // don't link
	}
	else
	{
	//  m_regs[REG_BLINK] = m_pc + (size >> 0);  // don't link
		return realaddress;
	}


	return m_pc + (size>>0);

}

uint32_t arcompact_device::handleop32_BL_cc_d_s21(uint32_t op)
{
	int size = 4;

	// Branch and Link Conditionally
	arcompact_log("unimplemented BLcc %08x", op);
	return m_pc + (size>>0);
}



uint32_t arcompact_device::handleop32_BL_d_s25(uint32_t op)
{
	int size = 4;
	// Branch and Link Unconditionally Far
	// 00001 sssssssss 10  SSSSSSSSSS N R TTTT
	int32_t address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	address |=        ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
//  int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	uint32_t realaddress = PC_ALIGNED32 + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 1;
	}
	else
	{
		m_regs[REG_BLINK] = m_pc + (size >> 0);
		return realaddress;
	}


	return m_pc + (size>>0);
}



uint32_t arcompact_device::arcompact_01_01_00_helper(uint32_t op, const char* optext)
{
	int size;

	// Branch on Compare / Bit Test - Register-Register

	COMMON32_GET_creg
	COMMON32_GET_breg;
	//int n = (op & 0x00000020) >> 5;


	if ((breg != LIMM_REG) && (creg != LIMM_REG))
	{
	}
	else
	{
		//uint32_t limm;
		//GET_LIMM_32;
		size = 8;
	}

	arcompact_log("unimplemented %s %08x (reg-reg)", optext, op);
	return m_pc + (size>>0);
}


// register - register cases

#define BR_REGREG_SETUP \
	/* Branch on Compare / Bit Test - Register-Register */ \
	int size = 4; \
	GET_01_01_01_BRANCH_ADDR; \
	COMMON32_GET_creg; \
	COMMON32_GET_breg; \
	int n = (op & 0x00000020) >> 5; \
	uint32_t b,c; \
	if ((breg != LIMM_REG) && (creg != LIMM_REG)) \
	{ \
		b = m_regs[breg]; \
		c = m_regs[creg]; \
	} \
	else \
	{ \
		uint32_t limm; \
		GET_LIMM_32; \
		size = 8; \
		\
		if (breg == LIMM_REG) \
			b = limm; \
		else \
			b = m_regs[breg]; \
		\
		if (creg == LIMM_REG) \
			c = limm; \
		else \
			c = m_regs[creg]; \
	}
#define BR_TAKEJUMP \
	/* take jump */ \
	uint32_t realaddress = PC_ALIGNED32 + (address * 2); \
		\
	if (n) \
	{ \
		m_delayactive = 1; \
		m_delayjump = realaddress; \
		m_delaylinks = 0; \
	} \
	else \
	{ \
		return realaddress; \
	}


uint32_t arcompact_device::handleop32_BREQ_reg_reg(uint32_t op)  // register - register BREQ
{
	BR_REGREG_SETUP

	// BREQ
	if (b == c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRNE_reg_reg(uint32_t op) // register - register BRNE
{
	BR_REGREG_SETUP

	// BRNE
	if (b != c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRLT_reg_reg(uint32_t op) // regiter - register BRLT
{
	BR_REGREG_SETUP

	// BRLT  (signed operation)
	if ((int32_t)b < (int32_t)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);

}
uint32_t arcompact_device::handleop32_BRGE_reg_reg(uint32_t op) // register - register BRGE
{
	BR_REGREG_SETUP

	// BRGE  (signed operation)
	if ((int32_t)b >= (int32_t)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRLO_reg_reg(uint32_t op) // register - register BRLO
{
	BR_REGREG_SETUP

	// BRLO (unsigned operation)
	if (b < c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_BRHS_reg_reg(uint32_t op) // register - register BRHS
{
	BR_REGREG_SETUP

	// BRHS (unsigned operation)
	if (b >= c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_BBIT0_reg_reg(uint32_t op)  { return arcompact_01_01_00_helper( op, "BBIT0");}
uint32_t arcompact_device::handleop32_BBIT1_reg_reg(uint32_t op)  { return arcompact_01_01_00_helper( op, "BBIT1");}

uint32_t arcompact_device::arcompact_01_01_01_helper(uint32_t op, const char* optext)
{
	int size = 4;
	arcompact_log("unimplemented %s %08x (reg-imm)", optext, op);
	return m_pc + (size>>0);
}

#define BR_REGIMM_SETUP \
	int size = 4; \
	GET_01_01_01_BRANCH_ADDR \
	COMMON32_GET_u6; \
	COMMON32_GET_breg; \
	int n = (op & 0x00000020) >> 5; \
	uint32_t b,c; \
	c = u; \
	/* comparing a LIMM  to an immediate is pointless, is it a valid encoding? */ \
	if ((breg != LIMM_REG)) \
	{ \
		b = m_regs[breg]; \
	} \
	else \
	{ \
		uint32_t limm; \
		GET_LIMM_32; \
		size = 8; \
		b = limm; \
	}

// register -immediate cases
uint32_t arcompact_device::handleop32_BREQ_reg_imm(uint32_t op) // BREQ reg-imm
{
	BR_REGIMM_SETUP

	// BREQ
	if (b == c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRNE_reg_imm(uint32_t op) // BRNE reg-imm
{
	BR_REGIMM_SETUP

	// BRNE
	if (b != c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}



uint32_t arcompact_device::handleop32_BRLT_reg_imm(uint32_t op) // BRLT reg-imm
{
	BR_REGIMM_SETUP

	// BRLT  (signed operation)
	if ((int32_t)b < (int32_t)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);

}
uint32_t arcompact_device::handleop32_BRGE_reg_imm(uint32_t op)
{
	BR_REGIMM_SETUP

	// BRGE  (signed operation)
	if ((int32_t)b >= (int32_t)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRLO_reg_imm(uint32_t op) //  register - immediate BRLO
{
	BR_REGIMM_SETUP

	// BRLO (unsigned operation)
	if (b < c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);

}

uint32_t arcompact_device::handleop32_BRHS_reg_imm(uint32_t op) // register - immediate BRHS
{
	BR_REGIMM_SETUP

	// BRHS (unsigned operation)
	if (b >= c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BBIT0_reg_imm(uint32_t op)  { return arcompact_01_01_01_helper(op, "BBIT0"); }
uint32_t arcompact_device::handleop32_BBIT1_reg_imm(uint32_t op)  { return arcompact_01_01_01_helper(op, "BBIT1"); }


uint32_t arcompact_device::handleop32_LD_r_o(uint32_t op)
{
	int size = 4;
	uint32_t limm;

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

		address = limm;
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
	uint32_t limm = 0;
	int got_limm = 0;
	int S = (op & 0x00008000) >> 15;
	int s = (op & 0x00ff0000) >> 16;
	if (S) s = -0x100 + s;

	COMMON32_GET_breg;
	COMMON32_GET_creg;

//  int R = (op & 0x00000001) >> 0; // bit 0 is reserved
	int Z = (op & 0x00000006) >> 1;
	int a = (op & 0x00000018) >> 3;
//  int D = (op & 0x00000020) >> 5; // we don't use the data cache currently


	uint32_t address = m_regs[breg];

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;

		address = limm;
	}

	uint32_t writedata = m_regs[creg];

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}

		writedata = limm;
	}

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





uint32_t arcompact_device::arcompact_handle04_helper(uint32_t op, const char* optext, int ignore_dst, int b_reserved)
{
	int size;
	//uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_p;
	COMMON32_GET_breg;

	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			//GET_LIMM_32;
			size = 8;
			got_limm = 1;
		}
		else
		{
		}
	}
	else
	{
	}


	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			if (!got_limm)
			{
				//GET_LIMM_32;
				size = 8;
			}
		}
		else
		{
		}
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
		int M = (op & 0x00000020) >> 5;

		if (M == 0)
		{
			COMMON32_GET_creg

			if (creg == LIMM_REG)
			{
				if (!got_limm)
				{
					//GET_LIMM_32;
					size = 8;
				}
			}
			else
			{
			}

		}
		else if (M == 1)
		{
		}

	}

	arcompact_log("unimplemented %s %08x (04 type helper)", optext, op);

	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_ADD_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + c;
	m_regs[areg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if ((b & 0x80000000) == (c & 0x80000000))
		{
			if ((result & 0x80000000) != (b & 0x80000000))
			{
				STATUS32_SET_V;
			}
			else
			{
				STATUS32_CLEAR_V;
			}
		}
		if (b < c)
		{
			STATUS32_SET_C;
		}
		else
		{
			STATUS32_CLEAR_C;
		}
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + c;
	m_regs[areg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if ((b & 0x80000000) == (c & 0x80000000))
		{
			if ((result & 0x80000000) != (b & 0x80000000))
			{
				STATUS32_SET_V;
			}
			else
			{
				STATUS32_CLEAR_V;
			}
		}
		if (b < c)
		{
			STATUS32_SET_C;
		}
		else
		{
			STATUS32_CLEAR_C;
		}
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if ((b & 0x80000000) == (c & 0x80000000))
		{
			if ((result & 0x80000000) != (b & 0x80000000))
			{
				STATUS32_SET_V;
			}
			else
			{
				STATUS32_CLEAR_V;
			}
		}
		if (b < c)
		{
			STATUS32_SET_C;
		}
		else
		{
			STATUS32_CLEAR_C;
		}
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD_p11_m0 (ADD)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b + c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if ((b & 0x80000000) == (c & 0x80000000))
		{
			if ((result & 0x80000000) != (b & 0x80000000))
			{
				STATUS32_SET_V;
			}
			else
			{
				STATUS32_CLEAR_V;
			}
		}
		if (b < c)
		{
			STATUS32_SET_C;
		}
		else
		{
			STATUS32_CLEAR_C;
		}
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB_p11_m0 (SUB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b - c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & c;
	if (areg != LIMM_REG) { m_regs[areg] = result; }

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & c;
	if (areg != LIMM_REG) { m_regs[areg] = result; }

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & c;
	if (breg != LIMM_REG) { m_regs[breg] = result; }

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_AND_p11_m0 (AND)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b & c;
	if (breg != LIMM_REG) { m_regs[breg] = result; }

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_OR_p11_m0 (OR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b | c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & (~c);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & (~c);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & (~c);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BIC_p11_m0 (BIC)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b & (~c);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b ^ c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b ^ c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b ^ c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_XOR_p11_m0 (XOR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b ^ c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p01(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p10(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_MOV_p11_m0 (MOV)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p11_m1(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c - b;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c - b;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c - b;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_RSUB_p11_m0 (RSUB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = c - b;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BSET_p11_m0 (BSET)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BMSK_p11_m0 (BMSK)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD1_p11_m0 (ADD1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b + (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD2_p11_m0 (ADD2)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b + (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b + (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD3_p11_m0 (ADD3)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b + (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB1_p11_m0 (SUB1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b - (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB2_p11_m0 (SUB2)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b - (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b - (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB3_p11_m0 (SUB3)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b - (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p01(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p10(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_LR_p11_m0 (LR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p11_m1(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg is reserved / not used

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SR_p11_m0 (SR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b << (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b << (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b << (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ASL_multiple_p11_m0 (ASL)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ASL_multiple_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b << (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b >> (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_p01(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	COMMON32_GET_areg;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b >> (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_p10(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_s12;

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = (uint32_t)S;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = b >> (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_LSR_multiple_p11_m0 (LSR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_multiple_p11_m1(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as condition code select

	uint32_t c;
	uint32_t b;

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
/*      got_limm = 1; */
		b = limm;
	}
	else
	{
		b = m_regs[breg];
	}

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	COMMON32_GET_CONDITION;
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = b >> (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_single_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c >> 1;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if (c == 0x00000001) { STATUS32_SET_C; }
		else { STATUS32_CLEAR_C; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_single_p01(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c >> 1;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if (c == 0x00000001) { STATUS32_SET_C; }
		else { STATUS32_CLEAR_C; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_single_p10(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_LSR_single_p10 (ares bits already used as opcode select, can't be used as s12) (LSR1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LSR_single_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_LSR_single_p11_m0 (ares bits already used as opcode select, can't be used as Q condition) (LSR1)\n");
	return m_pc + (size >> 0);
}
uint32_t arcompact_device::handleop32_LSR_single_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_LSR_single_p11_m1 (ares bits already used as opcode select, can't be used as Q condition) (LSR1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ROR_single_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	int shift = 1; uint32_t mask = (1 << (shift)) - 1; mask <<= (32-shift); uint32_t result = ((c >> shift) & ~mask) | ((c << (32-shift)) & mask);
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if (c == 0x00000001) { STATUS32_SET_C; }
		else { STATUS32_CLEAR_C; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ROR_single_p01(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	int shift = 1; uint32_t mask = (1 << (shift)) - 1; mask <<= (32-shift); uint32_t result = ((c >> shift) & ~mask) | ((c << (32-shift)) & mask);
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
		if (c == 0x00000001) { STATUS32_SET_C; }
		else { STATUS32_CLEAR_C; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ROR_single_p10(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_ROR_single_p10 (ares bits already used as opcode select, can't be used as s12) (ROR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ROR_single_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_ROR_single_p11_m0 (ares bits already used as opcode select, can't be used as Q condition) (ROR)\n");
	return m_pc + (size >> 0);
}
uint32_t arcompact_device::handleop32_ROR_single_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_ROR_single_p11_m1 (ares bits already used as opcode select, can't be used as Q condition) (ROR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTB_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c & 0x000000ff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTB (EXTB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTB_p01(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c & 0x000000ff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTB (EXTB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTB_p10(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTB_p10 (ares bits already used as opcode select, can't be used as s12) (EXTB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTB_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTB_p11_m0 (ares bits already used as opcode select, can't be used as Q condition) (EXTB)\n");
	return m_pc + (size >> 0);
}
uint32_t arcompact_device::handleop32_EXTB_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTB_p11_m1 (ares bits already used as opcode select, can't be used as Q condition) (EXTB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTW_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_creg;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		c = limm;
	}
	else
	{
		c = m_regs[creg];
	}
	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c & 0x0000ffff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTW (EXTW) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTW_p01(uint32_t op)
{
	int size = 4;
/*  int got_limm = 0; */

	COMMON32_GET_breg;
	COMMON32_GET_F;
	COMMON32_GET_u6;
	 //COMMON32_GET_areg; // areg bits already used as opcode select

	uint32_t c;

	c = u;

	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */
	uint32_t result = c & 0x0000ffff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTW (EXTW) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTW_p10(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTW_p10 (ares bits already used as opcode select, can't be used as s12) (EXTW)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_EXTW_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTW_p11_m0 (ares bits already used as opcode select, can't be used as Q condition) (EXTW)\n");
	return m_pc + (size >> 0);
}
uint32_t arcompact_device::handleop32_EXTW_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTW_p11_m1 (ares bits already used as opcode select, can't be used as Q condition) (EXTW)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop_ADD_S_c_b_u3(uint16_t op)
{
	int u, breg, creg;

	COMMON16_GET_u3;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] + u;
	m_regs[creg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_SUB_S_c_b_u3(uint16_t op)
{
	int u, breg, creg;

	COMMON16_GET_u3;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] - u;
	m_regs[creg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ASL_S_c_b_u3(uint16_t op)
{
	int u, breg, creg;

	COMMON16_GET_u3;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] << u;
	m_regs[creg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_SUB_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] - m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_AND_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] & m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_OR_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] | m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_XOR_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] ^ m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_EXTB_S_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] & 0x000000ff;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_EXTW_S_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] & 0x0000ffff;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_NEG_S_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = 0 - m_regs[creg];
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ADD1_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = m_regs[breg] + (m_regs[creg] <<1);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ADD2_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = m_regs[breg] + (m_regs[creg] <<2);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ADD3_S_b_b_c(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	 uint32_t result = m_regs[breg] + (m_regs[creg] <<3);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_LSR_S_b_b_c_multiple(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[breg] >> (m_regs[creg]&0x1f);
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ASL_S_b_c_single(uint16_t op)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	uint32_t result = m_regs[creg] << 1;
	m_regs[breg] = result;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ASL_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] << (u&0x1f);

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_LSR_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] >> (u&0x1f);

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_ASR_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	int32_t temp = (int32_t)m_regs[breg]; m_regs[breg] = temp >> (u&0x1f); // treat it as a signed value, so sign extension occurs during shift

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_SUB_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] - u;

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_BSET_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] | (1 << (u & 0x1f));

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_BMSK_S_b_b_u5(uint16_t op)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] | ((1 << (u + 1)) - 1);

	return m_pc + (2 >> 0);
}



uint32_t arcompact_device::handleop32_ADC(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x01], /*"ADC"*/ 0,0);
}



uint32_t arcompact_device::handleop32_SBC(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x03], /*"SBC"*/ 0,0);
}

uint32_t arcompact_device::handleop32_MAX(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x08], /*"MAX"*/ 0,0);
}

uint32_t arcompact_device::handleop32_MIN(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x09], /*"MIN"*/ 0,0);
}



uint32_t arcompact_device::handleop32_TST(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x0b], /*"TST"*/ 1,0);
}

uint32_t arcompact_device::handleop32_CMP(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x0c], /*"CMP"*/ 1,0);
}

uint32_t arcompact_device::handleop32_RCMP(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x0d], /*"RCMP"*/ 1,0);
}



uint32_t arcompact_device::handleop32_BCLR(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x10], /*"BCLR"*/ 0,0);
}

uint32_t arcompact_device::handleop32_BTST(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x11], /*"BTST"*/ 0,0);
}

uint32_t arcompact_device::handleop32_BXOR(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x12], /*"BXOR"*/ 0,0);
}






uint32_t arcompact_device::handleop32_MPY(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1a], /*"MPY"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_MPYH(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1b], /*"MPYH"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_MPYHU(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1c], /*"MPYHU"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_MPYU(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1d], /*"MPYU"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_Jcc_p00(uint32_t op)
{
	int size;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_F

	if (creg == LIMM_REG)
	{
		// opcode          iiii i--- ppII IIII F--- CCCC CC-- ----
		// J limm          0010 0RRR 0010 0000 0RRR 1111 10RR RRRR  [LIMM]  (creg = LIMM)

		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}

		return limm;
	}
	else
	{
		// opcode          iiii i--- ppII IIII F--- CCCC CC-- ----
		// J [c]           0010 0RRR 0010 0000 0RRR CCCC CCRR RRRR
		// J.F [ilink1]    0010 0RRR 0010 0000 1RRR 0111 01RR RRRR  (creg = ILINK1, FLAG must be set)
		// J.F [ilink2]    0010 0RRR 0010 0000 1RRR 0111 10RR RRRR  (creg = ILINE2, FLAG must be set)

		if (F)
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				arcompact_log("1 unimplemented J.F %08x", op);
			}
			else
			{
				// should not use .F unless jumping to ILINK1/2
				arcompact_fatal ("illegal 1 unimplemented J.F (F should not be set) %08x", op);
			}

		}
		else
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				// should only jumping to ILINK1/2 if .F is set
				arcompact_fatal("illegal 1 unimplemented J (F not set) %08x", op);
			}
			else
			{
				return m_regs[creg];
			}
		}
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_p01(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uu-- ----
	// J u6            0010 0RRR 0110 0000 0RRR uuuu uuRR RRRR
	int size = 4;
	arcompact_log("2 unimplemented J %08x", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_p10(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- ssss ssSS SSSS
	// J s12           0010 0RRR 1010 0000 0RRR ssss ssSS SSSS
	int size = 4;
	arcompact_log("3 unimplemented J %08x", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_p11_m0(uint32_t op) // Jcc   (no link, no delay)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_CONDITION;
	COMMON32_GET_F

	uint32_t c;

	if (creg == LIMM_REG)
	{
		// opcode          iiii i--- ppII IIII F--- cccc ccmq qqqq
		// Jcc limm        0010 0RRR 1110 0000 0RRR 1111 100Q QQQQ  [LIUMM]
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}

		c = limm;

	}
	else
	{
		// opcode          iiii i--- ppII IIII F--- cccc ccmq qqqq
		// Jcc [c]         0010 0RRR 1110 0000 0RRR CCCC CC0Q QQQQ
		// no conditional links to ILINK1, ILINK2?

		c = m_regs[creg];
	}

	if (!check_condition(condition))
		return m_pc + (size>>0);

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_fatal ("fatal handleop32_Jcc_p11_m0 J %08x (F not set but ILINK1 or ILINK2 used as dst)", op);
		}
		else
		{
			uint32_t realaddress = c;
			return realaddress;
		}
	}

	if (F)
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented handleop32_Jcc_p11_m0 J %08x (F set)", op);
		}
		else
		{
			arcompact_fatal ("fatal handleop32_Jcc_p11_m0 J %08x (F set but not ILINK1 or ILINK2 used as dst)", op);

		}
	}


	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_p11_m1(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uumq qqqq
	// Jcc u6          0010 0RRR 1110 0000 0RRR uuuu uu1Q QQQQ
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_p11_m1 J %08x (u6)", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_D_p00(uint32_t op)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_F

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}

		handle_jump_to_addr(1,0,limm, m_pc + (size>>0));
	}
	else
	{
		return handle_jump_to_register(1,0,creg, m_pc + (size>>0), F); // delay, no link
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_p01(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (u6 type) %08x", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_p10(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (s12 type) %08x", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_D_p11_m0(uint32_t op) // Jcc.D   (no link, delay)
{
	int size = 4;
	[[maybe_unused]] uint32_t limm;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_CONDITION;
	COMMON32_GET_F

	//uint32_t c = 0;

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}

	//  c = limm;

	}
	else
	{
		// opcode          iiii i--- ppII IIII F--- cccc ccmq qqqq
		// Jcc [c]         0010 0RRR 1110 0000 0RRR CCCC CC0Q QQQQ
		// no conditional links to ILINK1, ILINK2?

	//  c = m_regs[creg];
	}

	if (!check_condition(condition))
		return m_pc + (size>>0);

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented Jcc.D (p11_m0 type, illegal) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented Jcc.D (p11_m0 type, unimplemented) %08x", op);
		}
	}

	if (F)
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented Jcc.D.F (p11_m0 type, unimplemented) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented Jcc.D.F (p11_m0 type, illegal) %08x", op);
		}
	}


	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_D_p11_m1 J.D %08x (u6)", op);
	return m_pc + (size>>0);
}



uint32_t arcompact_device::handleop32_JLcc(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x22], /*"JL"*/ 1,1);
}

uint32_t arcompact_device::handleop32_JLcc_D(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x23], /*"JL.D"*/ 1,1);
}




uint32_t arcompact_device::handleop32_LP(uint32_t op) // LPcc (loop setup)
{
	int size = 4;
//  COMMON32_GET_breg; // breg is reserved
	COMMON32_GET_p;

	if (p == 0x00)
	{
		arcompact_fatal("<illegal LPcc, p = 0x00)");
	}
	else if (p == 0x01)
	{
		arcompact_fatal("<illegal LPcc, p = 0x01)");
	}
	else if (p == 0x02) // Loop unconditional
	{ // 0010 0RRR 1010 1000 0RRR ssss ssSS SSSS
		COMMON32_GET_s12
		if (S & 0x800) S = -0x800 + (S&0x7ff);

		arcompact_fatal("Lp unconditional not supported %d", S);
	}
	else if (p == 0x03) // Loop conditional
	{ // 0010 0RRR 1110 1000 0RRR uuuu uu1Q QQQQ
		COMMON32_GET_u6
		COMMON32_GET_CONDITION
		//arcompact_fatal("Lp conditional %s not supported %d", arcompact_disassembler::conditions[condition], u);

		// if the loop condition fails then just jump to after the end of the loop, don't set any registers
		if (!check_condition(condition))
		{
			uint32_t realoffset = PC_ALIGNED32 + (u * 2);
			return realoffset;
		}
		else
		{
			// otherwise set up the loop positions
			m_LP_START = m_pc + (size >> 0);
			m_LP_END = PC_ALIGNED32 + (u * 2);
			return m_pc + (size>>0);
		}

	}

	return m_pc + (size>>0);

}


uint32_t arcompact_device::handleop32_FLAG(uint32_t op)
{
	// leapster bios uses formats for FLAG that are not defined, bug I guess work anyway (P modes 0 / 1)
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x29], /*"FLAG"*/ 1,1);
}


uint32_t arcompact_device::arcompact_handle04_2f_helper(uint32_t op, const char* optext)
{
	int size;

	COMMON32_GET_p;
	//COMMON32_GET_breg;

	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			//uint32_t limm;
			//GET_LIMM_32;
			size = 8;
		}
		else
		{
		}
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
	}

	arcompact_log("unimplemented %s %08x (type 04_2f)", optext, op);
	return m_pc + (size>>0);
}



uint32_t arcompact_device::handleop32_ASL_single(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ASL"); } // ASL
uint32_t arcompact_device::handleop32_ASR_single(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ASR"); } // ASR

uint32_t arcompact_device::handleop32_RRC(uint32_t op)  { return arcompact_handle04_2f_helper(op, "RCC"); } // RCC
uint32_t arcompact_device::handleop32_SEXB(uint32_t op)  { return arcompact_handle04_2f_helper(op, "SEXB"); } // SEXB
uint32_t arcompact_device::handleop32_SEXW(uint32_t op)  { return arcompact_handle04_2f_helper(op, "SEXW"); } // SEXW


uint32_t arcompact_device::handleop32_ABS(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ABS"); } // ABS
uint32_t arcompact_device::handleop32_NOT(uint32_t op)  { return arcompact_handle04_2f_helper(op, "NOT"); } // NOT
uint32_t arcompact_device::handleop32_RLC(uint32_t op)  { return arcompact_handle04_2f_helper(op, "RCL"); } // RLC
uint32_t arcompact_device::handleop32_EX(uint32_t op)  { return arcompact_handle04_2f_helper(op, "EX"); } // EX


uint32_t arcompact_device::handleop32_SLEEP(uint32_t op)  { arcompact_log("SLEEP (%08x)", op); return m_pc + (4 >> 0);}
uint32_t arcompact_device::handleop32_SWI(uint32_t op)  { arcompact_log("SWI / TRAP0 (%08x)", op); return m_pc + (4 >> 0);}
uint32_t arcompact_device::handleop32_SYNC(uint32_t op)  { arcompact_log("SYNC (%08x)", op); return m_pc + (4 >> 0);}
uint32_t arcompact_device::handleop32_RTIE(uint32_t op)  { arcompact_log("RTIE (%08x)", op); return m_pc + (4 >> 0);}
uint32_t arcompact_device::handleop32_BRK(uint32_t op)  { arcompact_log("BRK (%08x)", op); return m_pc + (4 >> 0);}




uint32_t arcompact_device::arcompact_handle04_3x_helper(uint32_t op, int dsize, int extend)
{
	int size;
	//uint32_t limm=0;
	int got_limm = 0;


	COMMON32_GET_breg;
	COMMON32_GET_creg



	if (breg == LIMM_REG)
	{
		//GET_LIMM_32;
		size = 8;
		got_limm = 1;

	}
	else
	{
	}

	if (creg == LIMM_REG)
	{
		if (!got_limm)
		{
			//GET_LIMM_32;
			size = 8;
		}

	}
	else
	{
	}

	arcompact_log("unimplemented LD %08x (type 04_3x)", op);
	return m_pc + (size>>0);
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








uint32_t arcompact_device::handleop32_ASR_multiple(uint32_t op)  { return arcompact_handle04_helper(op, "ASR", 0,0); }
uint32_t arcompact_device::handleop32_ROR_multiple(uint32_t op)  { return arcompact_handle04_helper(op, "ROR", 0,0); }
uint32_t arcompact_device::handleop32_MUL64(uint32_t op)  { return arcompact_handle04_helper(op, "MUL64", 2,0); } // special
uint32_t arcompact_device::handleop32_MULU64(uint32_t op)  { return arcompact_handle04_helper(op, "MULU64", 2,0);} // special
uint32_t arcompact_device::handleop32_ADDS(uint32_t op)  { return arcompact_handle04_helper(op, "ADDS", 0,0); }
uint32_t arcompact_device::handleop32_SUBS(uint32_t op)  { return arcompact_handle04_helper(op, "SUBS", 0,0); }
uint32_t arcompact_device::handleop32_DIVAW(uint32_t op)  { return arcompact_handle04_helper(op, "DIVAW", 0,0); }



uint32_t arcompact_device::handleop32_ASLS(uint32_t op)  { return arcompact_handle04_helper(op, "ASLS", 0,0); }
uint32_t arcompact_device::handleop32_ASRS(uint32_t op)  { return arcompact_handle04_helper(op, "ASRS", 0,0); }

uint32_t arcompact_device::handleop32_ADDSDW(uint32_t op)  { return arcompact_handle04_helper(op, "ADDSDW", 0,0); }
uint32_t arcompact_device::handleop32_SUBSDW(uint32_t op)  { return arcompact_handle04_helper(op, "SUBSDW", 0,0); }



uint32_t arcompact_device::arcompact_handle05_2f_0x_helper(uint32_t op, const char* optext)
{
	int size;

	COMMON32_GET_p;
	//COMMON32_GET_breg;

	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			//uint32_t limm;
			//GET_LIMM_32;
			size = 8;

		}
		else
		{
		}
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
	}

	arcompact_log("unimplemented %s %08x", optext, op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_SWAP(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "SWAP");  }
uint32_t arcompact_device::handleop32_NORM(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "NORM");  }
uint32_t arcompact_device::handleop32_SAT16(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "SAT16"); }
uint32_t arcompact_device::handleop32_RND16(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "RND16"); }
uint32_t arcompact_device::handleop32_ABSSW(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "ABSSW"); }
uint32_t arcompact_device::handleop32_ABSS(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "ABSS");  }
uint32_t arcompact_device::handleop32_NEGSW(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "NEGSW"); }
uint32_t arcompact_device::handleop32_NEGS(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "NEGS");  }
uint32_t arcompact_device::handleop32_NORMW(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "NORMW"); }


uint32_t arcompact_device::handleop32_ARC_EXT06(uint32_t op)
{
	arcompact_log("op a,b,c (06 ARC ext) (%08x)", op );
	return m_pc + (4 >> 0);
}

uint32_t arcompact_device::handleop32_USER_EXT07(uint32_t op)
{
	arcompact_log("op a,b,c (07 User ext) (%08x)", op );
	return m_pc + (4 >> 0);
}

uint32_t arcompact_device::handleop32_USER_EXT08(uint32_t op)
{
	arcompact_log("op a,b,c (08 User ext) (%08x)", op );
	return m_pc + (4 >> 0);
}

uint32_t arcompact_device::handleop32_MARKET_EXT09(uint32_t op)
{
	arcompact_log("op a,b,c (09 Market ext) (%08x)", op );
	return m_pc + (4 >> 0);
}

uint32_t arcompact_device::handleop32_MARKET_EXT0a(uint32_t op)
{
	arcompact_log("op a,b,c (0a Market ext) (%08x)",  op );
	return m_pc + (4 >> 0);
}

uint32_t arcompact_device::handleop32_MARKET_EXT0b(uint32_t op)
{
	arcompact_log("op a,b,c (0b Market ext) (%08x)",  op );
	return m_pc + (4 >> 0);
}



uint32_t arcompact_device::arcompact_handle0c_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x (0x0c group)", optext, op);
	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_LD_S_a_b_c(uint16_t op)
{
	return arcompact_handle0c_helper(op, "LD_S");
}

uint32_t arcompact_device::handleop_LDB_S_a_b_c(uint16_t op)
{
	return arcompact_handle0c_helper(op, "LDB_S");
}

uint32_t arcompact_device::handleop_LDW_S_a_b_c(uint16_t op)
{
	return arcompact_handle0c_helper(op, "LDW_S");
}

uint32_t arcompact_device::handleop_ADD_S_a_b_c(uint16_t op) // ADD_S a <- b + c
{
	int areg, breg, creg;

	COMMON16_GET_areg;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(areg);
	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	m_regs[areg] = m_regs[breg] + m_regs[creg];

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::arcompact_handle0d_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x (0x0d group)", optext, op);
	return m_pc + (2 >> 0);
}



uint32_t arcompact_device::handleop_ASR_S_c_b_u3(uint16_t op)
{
	return arcompact_handle0d_helper(op, "ASR_S");
}



uint32_t arcompact_device::arcompact_handle0e_0x_helper(uint16_t op, const char* optext, int revop)
{
	int h;// , breg;
	int size;

	GROUP_0e_GET_h;

	if (h == LIMM_REG)
	{
		//uint32_t limm;
		//GET_LIMM;
		size = 6;
	}
	else
	{
	}

	arcompact_log("unimplemented %s %04x (0x0e_0x group)", optext, op);

	return m_pc+ (size>>0);

}

uint32_t arcompact_device::handleop_ADD_S_b_b_h_or_limm(uint16_t op) // ADD_s b, b, h
{
	int h,breg;
	int size = 2;

	GROUP_0e_GET_h;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (h == LIMM_REG)
	{
		uint32_t limm;
		GET_LIMM_16;
		size = 6;

		m_regs[breg] = m_regs[breg] + limm;

	}
	else
	{
		m_regs[breg] = m_regs[breg] + m_regs[h];
	}

	return m_pc+ (size>>0);
}

// 16-bit MOV with extended register range
uint32_t arcompact_device::handleop_MOV_S_b_h_or_limm(uint16_t op) // MOV_S b <- h
{
	int h,breg;
	int size = 2;

	GROUP_0e_GET_h;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (h == LIMM_REG)
	{
		// opcode        iiii ibbb hhhI Ihhh
		// MOV_S b, limm 0111 0bbb 1100 1111 [LIMM]   (h == LIMM)

		uint32_t limm;
		GET_LIMM_16;
		size = 6;

		m_regs[breg] = limm;

	}
	else
	{
		// opcode        iiii ibbb hhhI Ihhh
		// MOV_S b,h     0111 0bbb hhh0 1HHH
		m_regs[breg] = m_regs[h];
	}

	return m_pc+ (size>>0);
}

uint32_t arcompact_device::handleop_CMP_S_b_h_or_limm(uint16_t op)
{
	return arcompact_handle0e_0x_helper(op, "CMP_S", 0);
}

uint32_t arcompact_device::handleop_MOV_S_hob(uint16_t op) // MOV_S h <- b
{
	int h,breg;
	int size = 2;

	GROUP_0e_GET_h;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (h == LIMM_REG) // no result..
	{
	}

	m_regs[h] = m_regs[breg];

	return m_pc+ (size>>0);
}



uint32_t arcompact_device::arcompact_handle0f_00_0x_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);
}



uint32_t arcompact_device::handleop_J_S_b(uint16_t op)  { return arcompact_handle0f_00_0x_helper(op, "J_S"); }
uint32_t arcompact_device::handleop_J_S_D_b(uint16_t op)  { return arcompact_handle0f_00_0x_helper(op, "J_S.D"); }

uint32_t arcompact_device::handleop_JL_S_b(uint16_t op) // JL_S
{
	int breg;

	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_regs[REG_BLINK] = m_pc + (2 >> 0);

	return m_regs[breg];
}


uint32_t arcompact_device::handleop_JL_S_D_b(uint16_t op) // JL_S.D
{
	int breg;

	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_delayactive = 1;
	m_delayjump = m_regs[breg];
	m_delaylinks = 1;

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_SUB_S_NE_b_b_b(uint16_t op)  { return arcompact_handle0f_00_0x_helper(op, "SUB_S.NE"); }




// Zero parameters (ZOP)
uint32_t arcompact_device::handleop_NOP_S(uint16_t op)  { /*arcompact_log("NOP_S");*/ return m_pc + (2 >> 0);}
uint32_t arcompact_device::handleop_UNIMP_S(uint16_t op)  { arcompact_log("UNIMP_S"); return m_pc + (2 >> 0);} // Unimplemented Instruction, same as illegal, but recommended to fill blank space
uint32_t arcompact_device::handleop_JEQ_S_blink(uint16_t op)  { arcompact_log("JEQ_S [blink]"); return m_pc + (2 >> 0);}
uint32_t arcompact_device::handleop_JNE_S_blink(uint16_t op)  { arcompact_log("JNE_S [blink]"); return m_pc + (2 >> 0);}

uint32_t arcompact_device::handleop_J_S_blink(uint16_t op) // J_S [blink]
{
	return m_regs[REG_BLINK];
}


uint32_t arcompact_device::handleop_J_S_D_blink(uint16_t op) // J_S.D [blink]
{
	m_delayactive = 1;
	m_delayjump = m_regs[REG_BLINK];
	m_delaylinks = 0;

	return m_pc + (2 >> 0);
}





uint32_t arcompact_device::arcompact_handle0f_0x_helper(uint16_t op, const char* optext, int nodst)
{
	arcompact_log("unimplemented %s %04x (0xf_0x group)", optext, op);
	return m_pc + (2 >> 0);
}






uint32_t arcompact_device::handleop_BIC_S_b_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "BIC_S",0);  }

uint32_t arcompact_device::handleop_TST_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "TST_S",1);  }
uint32_t arcompact_device::handleop_MUL64_S_0_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "MUL64_S",2);  } // actual destination is special multiply registers
uint32_t arcompact_device::handleop_SEXB_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "SEXB_S",0); }
uint32_t arcompact_device::handleop_SEXW_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "SEXW_S",0); }




uint32_t arcompact_device::handleop_ABS_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "ABS_S",0);  }
uint32_t arcompact_device::handleop_NOT_S_b_c(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "NOT_S",0);  }


uint32_t arcompact_device::handleop_ASL_S_b_b_c_multiple(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "ASL_S",0);  }

uint32_t arcompact_device::handleop_ASR_S_b_b_c_multiple(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "ASR_S",0);  }


uint32_t arcompact_device::handleop_ASR_S_b_c_single(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "ASR1_S",0); }
uint32_t arcompact_device::handleop_LSR_S_b_c_single(uint16_t op)  { return arcompact_handle0f_0x_helper(op, "LSR1_S",0); }


uint32_t arcompact_device::handleop_TRAP_S_u6(uint16_t op)  // special
{
	arcompact_log("unimplemented TRAP_S %04x",  op);
	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BRK_S(uint16_t op)  // special
{
	arcompact_log("unimplemented BRK_S %04x",  op);
	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::arcompact_handle_ld_helper(uint16_t op, const char* optext, int shift, int swap)
{
	arcompact_log("unimplemented %s %04x (ld/st group %d %d)", optext, op, shift, swap);
	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_LD_S_c_b_u7(uint16_t op)
{ // LD_S c, [b, u7]
	int breg, creg, u;

	COMMON16_GET_breg;
	COMMON16_GET_creg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	u <<= 2; // check
	m_regs[creg] = READ32((m_regs[breg] + u) >> 2);

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_LDB_S_c_b_u5(uint16_t op)
{
	// LDB_S c, [b, u5]
	int breg, creg, u;

	COMMON16_GET_breg;
	COMMON16_GET_creg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

//  u <<= 0; // check
	m_regs[creg] = READ8((m_regs[breg] + u) >> 0);

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_LDW_S_c_b_u6(uint16_t op)
{
	// LDB_W c, [b, u6]
	int breg, creg, u;

	COMMON16_GET_breg;
	COMMON16_GET_creg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	u <<= 1;
	m_regs[creg] = READ16((m_regs[breg] + u) >> 1);

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_LDW_S_X_c_b_u6(uint16_t op)
{
	return arcompact_handle_ld_helper(op, "LDW_S.X", 1, 0);
}

uint32_t arcompact_device::handleop_ST_S_c_b_u7(uint16_t op) // ST_S c, [b, u7]
{
	int breg, creg, u;

	COMMON16_GET_breg;
	COMMON16_GET_creg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	u <<= 2;

	WRITE32((m_regs[breg] + u) >> 2, m_regs[creg]);

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_STB_S_c_b_u5(uint16_t op) // STB_S c. [b, u6]
{
	int breg, creg, u;

	COMMON16_GET_breg;
	COMMON16_GET_creg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

//  u <<= 0;

	WRITE8((m_regs[breg] + u) >> 0, m_regs[creg]);

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_STW_S_c_b_u6(uint16_t op) // STW_S c. [b, u6]
{
	int breg, creg, u;

	COMMON16_GET_breg;
	COMMON16_GET_creg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	u <<= 1;

	WRITE16((m_regs[breg] + u) >> 1, m_regs[creg]);

	return m_pc + (2 >> 0);

}


uint32_t arcompact_device::arcompact_handle_l7_0x_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x (l7_0x group)", optext, op);
	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BCLR_S_b_b_u5(uint16_t op)
{
	return arcompact_handle_l7_0x_helper(op, "BCLR_S");
}

uint32_t arcompact_device::handleop_BTST_S_b_u5(uint16_t op)
{
	return arcompact_handle_l7_0x_helper(op, "BTST_S");
}



uint32_t arcompact_device::arcompact_handle18_0x_helper(uint16_t op, const char* optext, int st)
{
	arcompact_log("unimplemented %s %04x (0x18_0x group)", optext, op);
	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_LD_S_b_sp_u7(uint16_t op)   // LD_S b, [SP, u7]
{
	int breg;
	uint32_t u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	uint32_t address = m_regs[REG_SP] + (u << 2);

	m_regs[breg] = READ32(address >> 2);

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_LDB_S_b_sp_u7(uint16_t op)
{
	return arcompact_handle18_0x_helper(op, "LDB_S (SP)", 0);
}

uint32_t arcompact_device::handleop_ST_S_b_sp_u7(uint16_t op)  // ST_S b, [SP, u7]
{
	int breg;
	uint32_t u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	uint32_t address = m_regs[REG_SP] + (u << 2);

	WRITE32(address >> 2, m_regs[breg]);

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_STB_S_b_sp_u7(uint16_t op)
{
	return arcompact_handle18_0x_helper(op, "STB_S (SP)", 1);
}

uint32_t arcompact_device::handleop_ADD_S_b_sp_u7(uint16_t op)  // ADD_S b, SP, u7
{
	int breg;
	uint32_t u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[REG_SP] + (u << 2);

	return m_pc + (2 >> 0);
}

// op bits remaining for 0x18_05_xx subgroups 0x001f
uint32_t arcompact_device::handleop_ADD_S_sp_sp_u7(uint16_t op)
{
	int u;
	COMMON16_GET_u5;

	m_regs[REG_SP] = m_regs[REG_SP] + (u << 2);

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_SUB_S_sp_sp_u7(uint16_t op)
{
	int u;
	COMMON16_GET_u5;

	m_regs[REG_SP] = m_regs[REG_SP] - (u << 2);

	return m_pc + (2 >> 0);
}

// op bits remaining for 0x18_06_xx subgroups 0x0700
uint32_t arcompact_device::handleop_POP_S_b(uint16_t op) // POP_S b
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_regs[breg] = READ32(m_regs[REG_SP] >> 2);
	m_regs[REG_SP] += 4;

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_POP_S_blink(uint16_t op) // POP_S blink
{
	// breg bits are reserved
	m_regs[REG_BLINK] = READ32(m_regs[REG_SP] >> 2 );
	m_regs[REG_SP] += 4;

	return m_pc + (2 >> 0);
}

// op bits remaining for 0x18_07_xx subgroups 0x0700
uint32_t arcompact_device::handleop_PUSH_S_b(uint16_t op) // PUSH_S b
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_regs[REG_SP] -= 4;

	WRITE32(m_regs[REG_SP] >> 2, m_regs[breg]);

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_PUSH_S_blink(uint16_t op) // PUSH_S [blink]
{
	// breg bits are reserved

	m_regs[REG_SP] -= 4;

	WRITE32(m_regs[REG_SP] >> 2, m_regs[REG_BLINK]);

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::arcompact_handle19_0x_helper(uint16_t op, const char* optext, int shift, int format)
{
	arcompact_log("unimplemented %s %04x (0x19_0x group)", optext, op);
	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_LD_S_r0_gp_s11(uint16_t op)  { return arcompact_handle19_0x_helper(op, "LD_S", 2, 0); }
uint32_t arcompact_device::handleop_LDB_S_r0_gp_s9(uint16_t op)  { return arcompact_handle19_0x_helper(op, "LDB_S", 0, 0); }
uint32_t arcompact_device::handleop_LDW_S_r0_gp_s10(uint16_t op)  { return arcompact_handle19_0x_helper(op, "LDW_S", 1, 0);  }
uint32_t arcompact_device::handleop_ADD_S_r0_gp_s11(uint16_t op)  { return arcompact_handle19_0x_helper(op, "ADD_S", 2, 1); }

uint32_t arcompact_device::handleop_LD_S_b_pcl_u10(uint16_t op)
{
	arcompact_log("unimplemented MOV_S x, [PCL, x] %04x",  op);
	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_MOV_S_b_u8(uint16_t op) // MOV_S b, u8
{
	int breg;
	uint32_t u;
	COMMON16_GET_breg;
	COMMON16_GET_u8;
	REG_16BIT_RANGE(breg);

	m_regs[breg] = u;

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_ADD_S_b_b_u7(uint16_t op) // ADD_S b, b, u7
{
	int breg;
	uint32_t u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] + u;

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_CMP_S_b_u7(uint16_t op) // CMP b, u7
{
	int breg;
	uint32_t u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	// flag setting ALWAYS occurs on CMP operations, even 16-bit ones even without a .F opcode type

	// TODO: verify this flag setting logic

	// unsigned checks
	if (m_regs[breg] == u)
	{
		STATUS32_SET_Z;
	}
	else
	{
		STATUS32_CLEAR_Z;
	}

	if (m_regs[breg] < u)
	{
		STATUS32_SET_C;
	}
	else
	{
		STATUS32_CLEAR_C;
	}
	// signed checks
	int32_t temp = (int32_t)m_regs[breg] - (int32_t)u;

	if (temp < 0)
	{
		STATUS32_SET_N;
	}
	else
	{
		STATUS32_CLEAR_N;
	}

	// if signs of source values don't match, and sign of result doesn't match the first source value, then we've overflowed?
	if ((m_regs[breg] & 0x80000000) != (u & 0x80000000))
	{
		if ((m_regs[breg] & 0x80000000) != (temp & 0x80000000))
		{
			STATUS32_SET_V;
		}
		else
		{
			STATUS32_CLEAR_V;
		}
	}

	// only sets flags, no result written

	return m_pc + (2 >> 0);
}



uint32_t arcompact_device::handleop_BREQ_S_b_0_s8(uint16_t op) // BREQ_S b,0,s8
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (!m_regs[breg])
	{
		int s = (op & 0x007f) >> 0; op &= ~0x007f;
		if (s & 0x40) s = -0x40 + (s & 0x3f);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::handleop_BRNE_S_b_0_s8(uint16_t op) // BRNE_S b,0,s8
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (m_regs[breg])
	{
		int s = (op & 0x007f) >> 0; op &= ~0x007f;
		if (s & 0x40) s = -0x40 + (s & 0x3f);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}


uint32_t arcompact_device::arcompact_handle1e_0x_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x (1e_0x type)", optext, op);
	return m_pc + (2 >> 0);
}



uint32_t arcompact_device::handleop_B_S_s10(uint16_t op) // B_S s10  (branch always)
{
	int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
	if (s & 0x100) s = -0x100 + (s & 0xff);
	uint32_t realaddress = PC_ALIGNED32 + (s * 2);
	//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
	return realaddress;
}

uint32_t arcompact_device::handleop_BEQ_S_s10(uint16_t op) // BEQ_S s10 (branch is zero bit is set)
{
	if (STATUS32_CHECK_Z)
	{
		int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
		if (s & 0x100) s = -0x100 + (s & 0xff);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BNE_S_s10(uint16_t op) // BNE_S s10  (branch if zero bit isn't set)
{
	if (!STATUS32_CHECK_Z)
	{
		int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
		if (s & 0x100) s = -0x100 + (s & 0xff);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::arcompact_handle1e_03_0x_helper(uint16_t op, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BGT_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BGT_S"); }
uint32_t arcompact_device::handleop_BGE_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BGE_S"); }

uint32_t arcompact_device::handleop_BLT_S_s7(uint16_t op) // BLT_S
{
	if (CONDITION_LT)
	{
		int s = (op & 0x003f) >> 0; op &= ~0x003f;
		if (s & 0x020) s = -0x20 + (s & 0x1f);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BLE_S_s7(uint16_t op) // BLE_S
{
	if (CONDITION_LE)
	{
		int s = (op & 0x003f) >> 0; op &= ~0x003f;
		if (s & 0x020) s = -0x20 + (s & 0x1f);
		uint32_t realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

uint32_t arcompact_device::handleop_BHI_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BHI_S"); }
uint32_t arcompact_device::handleop_BHS_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BHS_S"); }
uint32_t arcompact_device::handleop_BLO_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BLO_S"); }
uint32_t arcompact_device::handleop_BLS_S_s7(uint16_t op)  { return arcompact_handle1e_03_0x_helper(op, "BLS_S"); }

uint32_t arcompact_device::handleop_BL_S_s13(uint16_t op) // BL_S s13
{
	int s = (op & 0x07ff) >> 0; op &= ~0x07ff;
	if (s & 0x400) s = -0x400 + (s & 0x3ff);

	uint32_t realaddress = PC_ALIGNED32 + (s * 4);

	m_regs[REG_BLINK] = m_pc + (2 >> 0);
	return realaddress;
}

/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers                                                                                            *
*                                                                                                                                   *
************************************************************************************************************************************/

uint32_t arcompact_device::arcompact_handle_reserved(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op)  { logerror("<reserved 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>"); return m_pc + (4 >> 0);}

uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint16_t op) { logerror("<illegal 0x%02x_%02x> (%04x)\n", param1, param2, op); fatalerror("<illegal op>");  return m_pc + (2 >> 0); }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint16_t op) { logerror("<illegal 0x%02x_%02x_%02x> (%04x)\n", param1, param2, param3, op); fatalerror("<illegal op>");  return m_pc + (2 >> 0); }
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint16_t op) { logerror("<illegal 0x%02x_%02x_%02x_%02x> (%04x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>");  return m_pc + (2 >> 0); }

uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint32_t op)  { logerror("<illegal 0x%02x_%02x> (%08x)\n", param1, param2, op); fatalerror("<illegal op>"); return m_pc + (4 >> 0);}
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint32_t op)  { logerror("<illegal 0x%02x_%02x_%02x> (%08x)\n", param1, param2, param3, op); fatalerror("<illegal op>"); return m_pc + (4 >> 0);}
uint32_t arcompact_device::arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op)  { logerror("<illegal 0x%02x_%02x_%02x_%02x> (%08x)\n", param1, param2, param3, param4, op); fatalerror("<illegal op>"); return m_pc + (4 >> 0);}
