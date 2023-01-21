// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

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

	h = group_0e_get_h(op);

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

	h = group_0e_get_h(op);
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (h == LIMM_REG)
	{
		GET_LIMM_16;
		size = 6;
	}

	m_regs[breg] = m_regs[breg] + m_regs[h];

	return m_pc+ (size>>0);
}

// 16-bit MOV with extended register range
uint32_t arcompact_device::handleop_MOV_S_b_h_or_limm(uint16_t op) // MOV_S b <- h
{
	int h,breg;
	int size = 2;

	h = group_0e_get_h(op);
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (h == LIMM_REG)
	{
		// opcode        iiii ibbb hhhI Ihhh
		// MOV_S b, limm 0111 0bbb 1100 1111 [LIMM]   (h == LIMM)
		GET_LIMM_16;
		size = 6;

		m_regs[breg] = m_regs[h];

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

	h = group_0e_get_h(op);
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


