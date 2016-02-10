// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "debugger.h"
#include "arcompact.h"
#include "arcompact_common.h"

#define ARCOMPACT_LOGGING 1

#define arcompact_fatal if (ARCOMPACT_LOGGING) fatalerror
#define arcompact_log if (ARCOMPACT_LOGGING) fatalerror


void arcompact_device::execute_run()
{
	//UINT32 lres;
	//lres = 0;

	while (m_icount > 0)
	{
		debugger_instruction_hook(this, m_pc);

//      printf("new pc %04x\n", m_pc);

		if (m_delayactive)
		{
			UINT16 op = READ16((m_pc + 0) >> 1);
			m_pc = get_insruction(op);
			if (m_delaylinks) m_regs[REG_BLINK] = m_pc;

			m_pc = m_delayjump;
			m_delayactive = 0; m_delaylinks = 0;
		}
		else
		{
			UINT16 op = READ16((m_pc + 0) >> 1);
			m_pc = get_insruction(op);
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
	INT32 address = (op & 0x00fe0000) >> 17; \
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
		INT32 S = s_temp | (S_temp<<6); \
		if (S & 0x800) S = -0x800 + (S&0x7ff); /* sign extend */
#define COMMON32_GET_CONDITION \
		UINT8 condition = op & 0x0000001f;


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
// registers used in 16-bit opcodes hae a limited range
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

int arcompact_device::check_condition(UINT8 condition)
{
	switch (condition & 0x1f)
	{
		case 0x00: return 1; // AL
		case 0x01: return CONDITION_EQ;
		case 0x02: return !CONDITION_EQ; // NE
		case 0x03: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x04: return CONDITION_MI; // MI (N)
		case 0x05: return CONDITION_CS; // CS (Carry Set / Lower than)
		case 0x06: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x07: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x08: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x09: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x0a: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x0b: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x0c: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x0d: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x0e: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x0f: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x10: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x11: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x12: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x13: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x14: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x15: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x16: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x17: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x18: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x19: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x1a: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x1b: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x1c: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x1d: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x1e: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
		case 0x1f: fatalerror("unhandled condition check %s", conditions[condition]); return -1;
	}

		return -1;

}


ARCOMPACT_RETTYPE arcompact_device::get_insruction(OPS_32)
{
	UINT8 instruction = ARCOMPACT_OPERATION;

	if (instruction < 0x0c)
	{
		op <<= 16;
		op |= READ16((m_pc + 2) >> 1);

		switch (instruction) // 32-bit instructions (with optional extra dword for immediate data)
		{
			case 0x00: return arcompact_handle00(PARAMS);    // Bcc
			case 0x01: return arcompact_handle01(PARAMS);    // BLcc/BRcc
			case 0x02: return arcompact_handle02(PARAMS);    // LD r+o
			case 0x03: return arcompact_handle03(PARAMS);    // ST r+o
			case 0x04: return arcompact_handle04(PARAMS);    // op a,b,c (basecase)
			case 0x05: return arcompact_handle05(PARAMS);    // op a,b,c (05 ARC ext)
			case 0x06: return arcompact_handle06(PARAMS);    // op a,b,c (06 ARC ext)
			case 0x07: return arcompact_handle07(PARAMS);    // op a,b,c (07 User ext)
			case 0x08: return arcompact_handle08(PARAMS);    // op a,b,c (08 User ext)
			case 0x09: return arcompact_handle09(PARAMS);    // op a,b,c (09 Market ext)
			case 0x0a: return arcompact_handle0a(PARAMS);    // op a,b,c (0a Market ext)
			case 0x0b: return arcompact_handle0b(PARAMS);    // op a,b,c (0b Market ext)
		}
	}
	else
	{
		switch (instruction) // 16-bit instructions
		{
			case 0x0c: return arcompact_handle0c(PARAMS);    // Load/Add reg-reg
			case 0x0d: return arcompact_handle0d(PARAMS);    // Add/Sub/Shft imm
			case 0x0e: return arcompact_handle0e(PARAMS);    // Mov/Cmp/Add
			case 0x0f: return arcompact_handle0f(PARAMS);    // op_S b,b,c (single 16-bit ops)
			case 0x10: return arcompact_handle10(PARAMS);    // LD_S
			case 0x11: return arcompact_handle11(PARAMS);    // LDB_S
			case 0x12: return arcompact_handle12(PARAMS);    // LDW_S
			case 0x13: return arcompact_handle13(PARAMS);    // LSW_S.X
			case 0x14: return arcompact_handle14(PARAMS);    // ST_S
			case 0x15: return arcompact_handle15(PARAMS);    // STB_S
			case 0x16: return arcompact_handle16(PARAMS);    // STW_S
			case 0x17: return arcompact_handle17(PARAMS);    // Shift/Sub/Bit
			case 0x18: return arcompact_handle18(PARAMS);    // Stack Instr
			case 0x19: return arcompact_handle19(PARAMS);    // GP Instr
			case 0x1a: return arcompact_handle1a(PARAMS);    // PCL Instr
			case 0x1b: return arcompact_handle1b(PARAMS);    // MOV_S
			case 0x1c: return arcompact_handle1c(PARAMS);    // ADD_S/CMP_S
			case 0x1d: return arcompact_handle1d(PARAMS);    // BRcc_S
			case 0x1e: return arcompact_handle1e(PARAMS);    // Bcc_S
			case 0x1f: return arcompact_handle1f(PARAMS);    // BL_S
		}
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle00(OPS_32)
{
	UINT8 subinstr = (op & 0x00010000) >> 16;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle00_00(PARAMS);  // Branch Conditionally
		case 0x01: return arcompact_handle00_01(PARAMS);  // Branch Unconditionally Far
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01(OPS_32)
{
	UINT8 subinstr = (op & 0x00010000) >> 16;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle01_00(PARAMS);  // Branh & Link
		case 0x01: return arcompact_handle01_01(PARAMS);  // Branch on Compare
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_00(OPS_32)
{
	UINT8 subinstr2 = (op & 0x00020000) >> 17;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle01_00_00dasm(PARAMS);  // Branch and Link Conditionally
		case 0x01: return arcompact_handle01_00_01dasm(PARAMS);  // Branch and Link Unconditional Far
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01(OPS_32)
{
	UINT8 subinstr2 = (op & 0x00000010) >> 4;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle01_01_00(PARAMS);  // Branch on Compare Register-Register
		case 0x01: return arcompact_handle01_01_01(PARAMS);  // Branch on Compare/Bit Test Register-Immediate
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00(OPS_32)
{
	UINT8 subinstr3 = (op & 0x0000000f) >> 0;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle01_01_00_00(PARAMS);  // BREQ (reg-reg)
		case 0x01: return arcompact_handle01_01_00_01(PARAMS);  // BRNE (reg-reg)
		case 0x02: return arcompact_handle01_01_00_02(PARAMS);  // BRLT (reg-reg)
		case 0x03: return arcompact_handle01_01_00_03(PARAMS);  // BRGE (reg-reg)
		case 0x04: return arcompact_handle01_01_00_04(PARAMS);  // BRLO (reg-reg)
		case 0x05: return arcompact_handle01_01_00_05(PARAMS);  // BRHS (reg-reg)
		case 0x06: return arcompact_handle01_01_00_06(PARAMS);  // reserved
		case 0x07: return arcompact_handle01_01_00_07(PARAMS);  // reserved
		case 0x08: return arcompact_handle01_01_00_08(PARAMS);  // reserved
		case 0x09: return arcompact_handle01_01_00_09(PARAMS);  // reserved
		case 0x0a: return arcompact_handle01_01_00_0a(PARAMS);  // reserved
		case 0x0b: return arcompact_handle01_01_00_0b(PARAMS);  // reserved
		case 0x0c: return arcompact_handle01_01_00_0c(PARAMS);  // reserved
		case 0x0d: return arcompact_handle01_01_00_0d(PARAMS);  // reserved
		case 0x0e: return arcompact_handle01_01_00_0e(PARAMS);  // BBIT0 (reg-reg)
		case 0x0f: return arcompact_handle01_01_00_0f(PARAMS);  // BBIT1 (reg-reg)
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01(OPS_32) //  Branch on Compare/Bit Test Register-Immediate
{
	UINT8 subinstr3 = (op & 0x0000000f) >> 0;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle01_01_01_00(PARAMS);  // BREQ (reg-imm)
		case 0x01: return arcompact_handle01_01_01_01(PARAMS);  // BRNE (reg-imm)
		case 0x02: return arcompact_handle01_01_01_02(PARAMS);  // BRLT (reg-imm)
		case 0x03: return arcompact_handle01_01_01_03(PARAMS);  // BRGE (reg-imm)
		case 0x04: return arcompact_handle01_01_01_04(PARAMS);  // BRLO (reg-imm)
		case 0x05: return arcompact_handle01_01_01_05(PARAMS);  // BRHS (reg-imm)
		case 0x06: return arcompact_handle01_01_01_06(PARAMS);  // reserved
		case 0x07: return arcompact_handle01_01_01_07(PARAMS);  // reserved
		case 0x08: return arcompact_handle01_01_01_08(PARAMS);  // reserved
		case 0x09: return arcompact_handle01_01_01_09(PARAMS);  // reserved
		case 0x0a: return arcompact_handle01_01_01_0a(PARAMS);  // reserved
		case 0x0b: return arcompact_handle01_01_01_0b(PARAMS);  // reserved
		case 0x0c: return arcompact_handle01_01_01_0c(PARAMS);  // reserved
		case 0x0d: return arcompact_handle01_01_01_0d(PARAMS);  // reserved
		case 0x0e: return arcompact_handle01_01_01_0e(PARAMS);  // BBIT0 (reg-imm)
		case 0x0f: return arcompact_handle01_01_01_0f(PARAMS);  // BBIT1 (reg-imm)
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04(OPS_32)
{
	UINT8 subinstr = (op & 0x003f0000) >> 16;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle04_00(PARAMS);  // ADD
		case 0x01: return arcompact_handle04_01(PARAMS);  // ADC
		case 0x02: return arcompact_handle04_02(PARAMS);  // SUB
		case 0x03: return arcompact_handle04_03(PARAMS);  // SBC
		case 0x04: return arcompact_handle04_04(PARAMS);  // AND
		case 0x05: return arcompact_handle04_05(PARAMS);  // OR
		case 0x06: return arcompact_handle04_06(PARAMS);  // BIC
		case 0x07: return arcompact_handle04_07(PARAMS);  // XOR
		case 0x08: return arcompact_handle04_08(PARAMS);  // MAX
		case 0x09: return arcompact_handle04_09(PARAMS);  // MIN
		case 0x0a: return arcompact_handle04_0a(PARAMS);  // MOV
		case 0x0b: return arcompact_handle04_0b(PARAMS);  // TST
		case 0x0c: return arcompact_handle04_0c(PARAMS);  // CMP
		case 0x0d: return arcompact_handle04_0d(PARAMS);  // RCMP
		case 0x0e: return arcompact_handle04_0e(PARAMS);  // RSUB
		case 0x0f: return arcompact_handle04_0f(PARAMS);  // BSET
		case 0x10: return arcompact_handle04_10(PARAMS);  // BCLR
		case 0x11: return arcompact_handle04_11(PARAMS);  // BTST
		case 0x12: return arcompact_handle04_12(PARAMS);  // BXOR
		case 0x13: return arcompact_handle04_13(PARAMS);  // BMSK
		case 0x14: return arcompact_handle04_14(PARAMS);  // ADD1
		case 0x15: return arcompact_handle04_15(PARAMS);  // ADD2
		case 0x16: return arcompact_handle04_16(PARAMS);  // ADD3
		case 0x17: return arcompact_handle04_17(PARAMS);  // SUB1
		case 0x18: return arcompact_handle04_18(PARAMS);  // SUB2
		case 0x19: return arcompact_handle04_19(PARAMS);  // SUB3
		case 0x1a: return arcompact_handle04_1a(PARAMS);  // MPY *
		case 0x1b: return arcompact_handle04_1b(PARAMS);  // MPYH *
		case 0x1c: return arcompact_handle04_1c(PARAMS);  // MPYHU *
		case 0x1d: return arcompact_handle04_1d(PARAMS);  // MPYU *
		case 0x1e: return arcompact_handle04_1e(PARAMS);  // illegal
		case 0x1f: return arcompact_handle04_1f(PARAMS);  // illegal
		case 0x20: return arcompact_handle04_20(PARAMS);  // Jcc
		case 0x21: return arcompact_handle04_21(PARAMS);  // Jcc.D
		case 0x22: return arcompact_handle04_22(PARAMS);  // JLcc
		case 0x23: return arcompact_handle04_23(PARAMS);  // JLcc.D
		case 0x24: return arcompact_handle04_24(PARAMS);  // illegal
		case 0x25: return arcompact_handle04_25(PARAMS);  // illegal
		case 0x26: return arcompact_handle04_26(PARAMS);  // illegal
		case 0x27: return arcompact_handle04_27(PARAMS);  // illegal
		case 0x28: return arcompact_handle04_28(PARAMS);  // LPcc
		case 0x29: return arcompact_handle04_29(PARAMS);  // FLAG
		case 0x2a: return arcompact_handle04_2a(PARAMS);  // LR
		case 0x2b: return arcompact_handle04_2b(PARAMS);  // SR
		case 0x2c: return arcompact_handle04_2c(PARAMS);  // illegal
		case 0x2d: return arcompact_handle04_2d(PARAMS);  // illegal
		case 0x2e: return arcompact_handle04_2e(PARAMS);  // illegal
		case 0x2f: return arcompact_handle04_2f(PARAMS);  // Sub Opcode
		case 0x30: return arcompact_handle04_30(PARAMS);  // LD r-r
		case 0x31: return arcompact_handle04_31(PARAMS);  // LD r-r
		case 0x32: return arcompact_handle04_32(PARAMS);  // LD r-r
		case 0x33: return arcompact_handle04_33(PARAMS);  // LD r-r
		case 0x34: return arcompact_handle04_34(PARAMS);  // LD r-r
		case 0x35: return arcompact_handle04_35(PARAMS);  // LD r-r
		case 0x36: return arcompact_handle04_36(PARAMS);  // LD r-r
		case 0x37: return arcompact_handle04_37(PARAMS);  // LD r-r
		case 0x38: return arcompact_handle04_38(PARAMS);  // illegal
		case 0x39: return arcompact_handle04_39(PARAMS);  // illegal
		case 0x3a: return arcompact_handle04_3a(PARAMS);  // illegal
		case 0x3b: return arcompact_handle04_3b(PARAMS);  // illegal
		case 0x3c: return arcompact_handle04_3c(PARAMS);  // illegal
		case 0x3d: return arcompact_handle04_3d(PARAMS);  // illegal
		case 0x3e: return arcompact_handle04_3e(PARAMS);  // illegal
		case 0x3f: return arcompact_handle04_3f(PARAMS);  // illegal
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f(OPS_32)
{
	UINT8 subinstr2 = (op & 0x0000003f) >> 0;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle04_2f_00(PARAMS);  // ASL
		case 0x01: return arcompact_handle04_2f_01(PARAMS);  // ASR
		case 0x02: return arcompact_handle04_2f_02(PARAMS);  // LSR
		case 0x03: return arcompact_handle04_2f_03(PARAMS);  // ROR
		case 0x04: return arcompact_handle04_2f_04(PARAMS);  // RCC
		case 0x05: return arcompact_handle04_2f_05(PARAMS);  // SEXB
		case 0x06: return arcompact_handle04_2f_06(PARAMS);  // SEXW
		case 0x07: return arcompact_handle04_2f_07(PARAMS);  // EXTB
		case 0x08: return arcompact_handle04_2f_08(PARAMS);  // EXTW
		case 0x09: return arcompact_handle04_2f_09(PARAMS);  // ABS
		case 0x0a: return arcompact_handle04_2f_0a(PARAMS);  // NOT
		case 0x0b: return arcompact_handle04_2f_0b(PARAMS);  // RLC
		case 0x0c: return arcompact_handle04_2f_0c(PARAMS);  // EX
		case 0x0d: return arcompact_handle04_2f_0d(PARAMS);  // illegal
		case 0x0e: return arcompact_handle04_2f_0e(PARAMS);  // illegal
		case 0x0f: return arcompact_handle04_2f_0f(PARAMS);  // illegal
		case 0x10: return arcompact_handle04_2f_10(PARAMS);  // illegal
		case 0x11: return arcompact_handle04_2f_11(PARAMS);  // illegal
		case 0x12: return arcompact_handle04_2f_12(PARAMS);  // illegal
		case 0x13: return arcompact_handle04_2f_13(PARAMS);  // illegal
		case 0x14: return arcompact_handle04_2f_14(PARAMS);  // illegal
		case 0x15: return arcompact_handle04_2f_15(PARAMS);  // illegal
		case 0x16: return arcompact_handle04_2f_16(PARAMS);  // illegal
		case 0x17: return arcompact_handle04_2f_17(PARAMS);  // illegal
		case 0x18: return arcompact_handle04_2f_18(PARAMS);  // illegal
		case 0x19: return arcompact_handle04_2f_19(PARAMS);  // illegal
		case 0x1a: return arcompact_handle04_2f_1a(PARAMS);  // illegal
		case 0x1b: return arcompact_handle04_2f_1b(PARAMS);  // illegal
		case 0x1c: return arcompact_handle04_2f_1c(PARAMS);  // illegal
		case 0x1d: return arcompact_handle04_2f_1d(PARAMS);  // illegal
		case 0x1e: return arcompact_handle04_2f_1e(PARAMS);  // illegal
		case 0x1f: return arcompact_handle04_2f_1f(PARAMS);  // illegal
		case 0x20: return arcompact_handle04_2f_20(PARAMS);  // illegal
		case 0x21: return arcompact_handle04_2f_21(PARAMS);  // illegal
		case 0x22: return arcompact_handle04_2f_22(PARAMS);  // illegal
		case 0x23: return arcompact_handle04_2f_23(PARAMS);  // illegal
		case 0x24: return arcompact_handle04_2f_24(PARAMS);  // illegal
		case 0x25: return arcompact_handle04_2f_25(PARAMS);  // illegal
		case 0x26: return arcompact_handle04_2f_26(PARAMS);  // illegal
		case 0x27: return arcompact_handle04_2f_27(PARAMS);  // illegal
		case 0x28: return arcompact_handle04_2f_28(PARAMS);  // illegal
		case 0x29: return arcompact_handle04_2f_29(PARAMS);  // illegal
		case 0x2a: return arcompact_handle04_2f_2a(PARAMS);  // illegal
		case 0x2b: return arcompact_handle04_2f_2b(PARAMS);  // illegal
		case 0x2c: return arcompact_handle04_2f_2c(PARAMS);  // illegal
		case 0x2d: return arcompact_handle04_2f_2d(PARAMS);  // illegal
		case 0x2e: return arcompact_handle04_2f_2e(PARAMS);  // illegal
		case 0x2f: return arcompact_handle04_2f_2f(PARAMS);  // illegal
		case 0x30: return arcompact_handle04_2f_30(PARAMS);  // illegal
		case 0x31: return arcompact_handle04_2f_31(PARAMS);  // illegal
		case 0x32: return arcompact_handle04_2f_32(PARAMS);  // illegal
		case 0x33: return arcompact_handle04_2f_33(PARAMS);  // illegal
		case 0x34: return arcompact_handle04_2f_34(PARAMS);  // illegal
		case 0x35: return arcompact_handle04_2f_35(PARAMS);  // illegal
		case 0x36: return arcompact_handle04_2f_36(PARAMS);  // illegal
		case 0x37: return arcompact_handle04_2f_37(PARAMS);  // illegal
		case 0x38: return arcompact_handle04_2f_38(PARAMS);  // illegal
		case 0x39: return arcompact_handle04_2f_39(PARAMS);  // illegal
		case 0x3a: return arcompact_handle04_2f_3a(PARAMS);  // illegal
		case 0x3b: return arcompact_handle04_2f_3b(PARAMS);  // illegal
		case 0x3c: return arcompact_handle04_2f_3c(PARAMS);  // illegal
		case 0x3d: return arcompact_handle04_2f_3d(PARAMS);  // illegal
		case 0x3e: return arcompact_handle04_2f_3e(PARAMS);  // illegal
		case 0x3f: return arcompact_handle04_2f_3f(PARAMS);  // ZOPs (Zero Operand Opcodes)
	}

	return 0;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f(OPS_32)
{
	UINT8 subinstr2 = (op & 0x0000003f) >> 0;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle05_2f_00(PARAMS);  // SWAP
		case 0x01: return arcompact_handle05_2f_01(PARAMS);  // NORM
		case 0x02: return arcompact_handle05_2f_02(PARAMS);  // SAT16
		case 0x03: return arcompact_handle05_2f_03(PARAMS);  // RND16
		case 0x04: return arcompact_handle05_2f_04(PARAMS);  // ABSSW
		case 0x05: return arcompact_handle05_2f_05(PARAMS);  // ABSS
		case 0x06: return arcompact_handle05_2f_06(PARAMS);  // NEGSW
		case 0x07: return arcompact_handle05_2f_07(PARAMS);  // NEGS
		case 0x08: return arcompact_handle05_2f_08(PARAMS);  // NORMW
		case 0x09: return arcompact_handle05_2f_09(PARAMS);  // illegal
		case 0x0a: return arcompact_handle05_2f_0a(PARAMS);  // illegal
		case 0x0b: return arcompact_handle05_2f_0b(PARAMS);  // illegal
		case 0x0c: return arcompact_handle05_2f_0c(PARAMS);  // illegal
		case 0x0d: return arcompact_handle05_2f_0d(PARAMS);  // illegal
		case 0x0e: return arcompact_handle05_2f_0e(PARAMS);  // illegal
		case 0x0f: return arcompact_handle05_2f_0f(PARAMS);  // illegal
		case 0x10: return arcompact_handle05_2f_10(PARAMS);  // illegal
		case 0x11: return arcompact_handle05_2f_11(PARAMS);  // illegal
		case 0x12: return arcompact_handle05_2f_12(PARAMS);  // illegal
		case 0x13: return arcompact_handle05_2f_13(PARAMS);  // illegal
		case 0x14: return arcompact_handle05_2f_14(PARAMS);  // illegal
		case 0x15: return arcompact_handle05_2f_15(PARAMS);  // illegal
		case 0x16: return arcompact_handle05_2f_16(PARAMS);  // illegal
		case 0x17: return arcompact_handle05_2f_17(PARAMS);  // illegal
		case 0x18: return arcompact_handle05_2f_18(PARAMS);  // illegal
		case 0x19: return arcompact_handle05_2f_19(PARAMS);  // illegal
		case 0x1a: return arcompact_handle05_2f_1a(PARAMS);  // illegal
		case 0x1b: return arcompact_handle05_2f_1b(PARAMS);  // illegal
		case 0x1c: return arcompact_handle05_2f_1c(PARAMS);  // illegal
		case 0x1d: return arcompact_handle05_2f_1d(PARAMS);  // illegal
		case 0x1e: return arcompact_handle05_2f_1e(PARAMS);  // illegal
		case 0x1f: return arcompact_handle05_2f_1f(PARAMS);  // illegal
		case 0x20: return arcompact_handle05_2f_20(PARAMS);  // illegal
		case 0x21: return arcompact_handle05_2f_21(PARAMS);  // illegal
		case 0x22: return arcompact_handle05_2f_22(PARAMS);  // illegal
		case 0x23: return arcompact_handle05_2f_23(PARAMS);  // illegal
		case 0x24: return arcompact_handle05_2f_24(PARAMS);  // illegal
		case 0x25: return arcompact_handle05_2f_25(PARAMS);  // illegal
		case 0x26: return arcompact_handle05_2f_26(PARAMS);  // illegal
		case 0x27: return arcompact_handle05_2f_27(PARAMS);  // illegal
		case 0x28: return arcompact_handle05_2f_28(PARAMS);  // illegal
		case 0x29: return arcompact_handle05_2f_29(PARAMS);  // illegal
		case 0x2a: return arcompact_handle05_2f_2a(PARAMS);  // illegal
		case 0x2b: return arcompact_handle05_2f_2b(PARAMS);  // illegal
		case 0x2c: return arcompact_handle05_2f_2c(PARAMS);  // illegal
		case 0x2d: return arcompact_handle05_2f_2d(PARAMS);  // illegal
		case 0x2e: return arcompact_handle05_2f_2e(PARAMS);  // illegal
		case 0x2f: return arcompact_handle05_2f_2f(PARAMS);  // illegal
		case 0x30: return arcompact_handle05_2f_30(PARAMS);  // illegal
		case 0x31: return arcompact_handle05_2f_31(PARAMS);  // illegal
		case 0x32: return arcompact_handle05_2f_32(PARAMS);  // illegal
		case 0x33: return arcompact_handle05_2f_33(PARAMS);  // illegal
		case 0x34: return arcompact_handle05_2f_34(PARAMS);  // illegal
		case 0x35: return arcompact_handle05_2f_35(PARAMS);  // illegal
		case 0x36: return arcompact_handle05_2f_36(PARAMS);  // illegal
		case 0x37: return arcompact_handle05_2f_37(PARAMS);  // illegal
		case 0x38: return arcompact_handle05_2f_38(PARAMS);  // illegal
		case 0x39: return arcompact_handle05_2f_39(PARAMS);  // illegal
		case 0x3a: return arcompact_handle05_2f_3a(PARAMS);  // illegal
		case 0x3b: return arcompact_handle05_2f_3b(PARAMS);  // illegal
		case 0x3c: return arcompact_handle05_2f_3c(PARAMS);  // illegal
		case 0x3d: return arcompact_handle05_2f_3d(PARAMS);  // illegal
		case 0x3e: return arcompact_handle05_2f_3e(PARAMS);  // illegal
		case 0x3f: return arcompact_handle05_2f_3f(PARAMS);  // ZOPs (Zero Operand Opcodes)
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f(OPS_32)
{
	UINT8 subinstr3 = (op & 0x07000000) >> 24;
	subinstr3 |= ((op & 0x00007000) >> 12) << 3;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle04_2f_3f_00(PARAMS);  // illegal
		case 0x01: return arcompact_handle04_2f_3f_01(PARAMS);  // SLEEP
		case 0x02: return arcompact_handle04_2f_3f_02(PARAMS);  // SWI / TRAP9
		case 0x03: return arcompact_handle04_2f_3f_03(PARAMS);  // SYNC
		case 0x04: return arcompact_handle04_2f_3f_04(PARAMS);  // RTIE
		case 0x05: return arcompact_handle04_2f_3f_05(PARAMS);  // BRK
		case 0x06: return arcompact_handle04_2f_3f_06(PARAMS);  // illegal
		case 0x07: return arcompact_handle04_2f_3f_07(PARAMS);  // illegal
		case 0x08: return arcompact_handle04_2f_3f_08(PARAMS);  // illegal
		case 0x09: return arcompact_handle04_2f_3f_09(PARAMS);  // illegal
		case 0x0a: return arcompact_handle04_2f_3f_0a(PARAMS);  // illegal
		case 0x0b: return arcompact_handle04_2f_3f_0b(PARAMS);  // illegal
		case 0x0c: return arcompact_handle04_2f_3f_0c(PARAMS);  // illegal
		case 0x0d: return arcompact_handle04_2f_3f_0d(PARAMS);  // illegal
		case 0x0e: return arcompact_handle04_2f_3f_0e(PARAMS);  // illegal
		case 0x0f: return arcompact_handle04_2f_3f_0f(PARAMS);  // illegal
		case 0x10: return arcompact_handle04_2f_3f_10(PARAMS);  // illegal
		case 0x11: return arcompact_handle04_2f_3f_11(PARAMS);  // illegal
		case 0x12: return arcompact_handle04_2f_3f_12(PARAMS);  // illegal
		case 0x13: return arcompact_handle04_2f_3f_13(PARAMS);  // illegal
		case 0x14: return arcompact_handle04_2f_3f_14(PARAMS);  // illegal
		case 0x15: return arcompact_handle04_2f_3f_15(PARAMS);  // illegal
		case 0x16: return arcompact_handle04_2f_3f_16(PARAMS);  // illegal
		case 0x17: return arcompact_handle04_2f_3f_17(PARAMS);  // illegal
		case 0x18: return arcompact_handle04_2f_3f_18(PARAMS);  // illegal
		case 0x19: return arcompact_handle04_2f_3f_19(PARAMS);  // illegal
		case 0x1a: return arcompact_handle04_2f_3f_1a(PARAMS);  // illegal
		case 0x1b: return arcompact_handle04_2f_3f_1b(PARAMS);  // illegal
		case 0x1c: return arcompact_handle04_2f_3f_1c(PARAMS);  // illegal
		case 0x1d: return arcompact_handle04_2f_3f_1d(PARAMS);  // illegal
		case 0x1e: return arcompact_handle04_2f_3f_1e(PARAMS);  // illegal
		case 0x1f: return arcompact_handle04_2f_3f_1f(PARAMS);  // illegal
		case 0x20: return arcompact_handle04_2f_3f_20(PARAMS);  // illegal
		case 0x21: return arcompact_handle04_2f_3f_21(PARAMS);  // illegal
		case 0x22: return arcompact_handle04_2f_3f_22(PARAMS);  // illegal
		case 0x23: return arcompact_handle04_2f_3f_23(PARAMS);  // illegal
		case 0x24: return arcompact_handle04_2f_3f_24(PARAMS);  // illegal
		case 0x25: return arcompact_handle04_2f_3f_25(PARAMS);  // illegal
		case 0x26: return arcompact_handle04_2f_3f_26(PARAMS);  // illegal
		case 0x27: return arcompact_handle04_2f_3f_27(PARAMS);  // illegal
		case 0x28: return arcompact_handle04_2f_3f_28(PARAMS);  // illegal
		case 0x29: return arcompact_handle04_2f_3f_29(PARAMS);  // illegal
		case 0x2a: return arcompact_handle04_2f_3f_2a(PARAMS);  // illegal
		case 0x2b: return arcompact_handle04_2f_3f_2b(PARAMS);  // illegal
		case 0x2c: return arcompact_handle04_2f_3f_2c(PARAMS);  // illegal
		case 0x2d: return arcompact_handle04_2f_3f_2d(PARAMS);  // illegal
		case 0x2e: return arcompact_handle04_2f_3f_2e(PARAMS);  // illegal
		case 0x2f: return arcompact_handle04_2f_3f_2f(PARAMS);  // illegal
		case 0x30: return arcompact_handle04_2f_3f_30(PARAMS);  // illegal
		case 0x31: return arcompact_handle04_2f_3f_31(PARAMS);  // illegal
		case 0x32: return arcompact_handle04_2f_3f_32(PARAMS);  // illegal
		case 0x33: return arcompact_handle04_2f_3f_33(PARAMS);  // illegal
		case 0x34: return arcompact_handle04_2f_3f_34(PARAMS);  // illegal
		case 0x35: return arcompact_handle04_2f_3f_35(PARAMS);  // illegal
		case 0x36: return arcompact_handle04_2f_3f_36(PARAMS);  // illegal
		case 0x37: return arcompact_handle04_2f_3f_37(PARAMS);  // illegal
		case 0x38: return arcompact_handle04_2f_3f_38(PARAMS);  // illegal
		case 0x39: return arcompact_handle04_2f_3f_39(PARAMS);  // illegal
		case 0x3a: return arcompact_handle04_2f_3f_3a(PARAMS);  // illegal
		case 0x3b: return arcompact_handle04_2f_3f_3b(PARAMS);  // illegal
		case 0x3c: return arcompact_handle04_2f_3f_3c(PARAMS);  // illegal
		case 0x3d: return arcompact_handle04_2f_3f_3d(PARAMS);  // illegal
		case 0x3e: return arcompact_handle04_2f_3f_3e(PARAMS);  // illegal
		case 0x3f: return arcompact_handle04_2f_3f_3f(PARAMS);  // illegal
	}

	return 0;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f(OPS_32) // useless ZOP group, no actual opcodes
{
	UINT8 subinstr3 = (op & 0x07000000) >> 24;
	subinstr3 |= ((op & 0x00007000) >> 12) << 3;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle05_2f_3f_00(PARAMS);  // illegal
		case 0x01: return arcompact_handle05_2f_3f_01(PARAMS);  // illegal
		case 0x02: return arcompact_handle05_2f_3f_02(PARAMS);  // illegal
		case 0x03: return arcompact_handle05_2f_3f_03(PARAMS);  // illegal
		case 0x04: return arcompact_handle05_2f_3f_04(PARAMS);  // illegal
		case 0x05: return arcompact_handle05_2f_3f_05(PARAMS);  // illegal
		case 0x06: return arcompact_handle05_2f_3f_06(PARAMS);  // illegal
		case 0x07: return arcompact_handle05_2f_3f_07(PARAMS);  // illegal
		case 0x08: return arcompact_handle05_2f_3f_08(PARAMS);  // illegal
		case 0x09: return arcompact_handle05_2f_3f_09(PARAMS);  // illegal
		case 0x0a: return arcompact_handle05_2f_3f_0a(PARAMS);  // illegal
		case 0x0b: return arcompact_handle05_2f_3f_0b(PARAMS);  // illegal
		case 0x0c: return arcompact_handle05_2f_3f_0c(PARAMS);  // illegal
		case 0x0d: return arcompact_handle05_2f_3f_0d(PARAMS);  // illegal
		case 0x0e: return arcompact_handle05_2f_3f_0e(PARAMS);  // illegal
		case 0x0f: return arcompact_handle05_2f_3f_0f(PARAMS);  // illegal
		case 0x10: return arcompact_handle05_2f_3f_10(PARAMS);  // illegal
		case 0x11: return arcompact_handle05_2f_3f_11(PARAMS);  // illegal
		case 0x12: return arcompact_handle05_2f_3f_12(PARAMS);  // illegal
		case 0x13: return arcompact_handle05_2f_3f_13(PARAMS);  // illegal
		case 0x14: return arcompact_handle05_2f_3f_14(PARAMS);  // illegal
		case 0x15: return arcompact_handle05_2f_3f_15(PARAMS);  // illegal
		case 0x16: return arcompact_handle05_2f_3f_16(PARAMS);  // illegal
		case 0x17: return arcompact_handle05_2f_3f_17(PARAMS);  // illegal
		case 0x18: return arcompact_handle05_2f_3f_18(PARAMS);  // illegal
		case 0x19: return arcompact_handle05_2f_3f_19(PARAMS);  // illegal
		case 0x1a: return arcompact_handle05_2f_3f_1a(PARAMS);  // illegal
		case 0x1b: return arcompact_handle05_2f_3f_1b(PARAMS);  // illegal
		case 0x1c: return arcompact_handle05_2f_3f_1c(PARAMS);  // illegal
		case 0x1d: return arcompact_handle05_2f_3f_1d(PARAMS);  // illegal
		case 0x1e: return arcompact_handle05_2f_3f_1e(PARAMS);  // illegal
		case 0x1f: return arcompact_handle05_2f_3f_1f(PARAMS);  // illegal
		case 0x20: return arcompact_handle05_2f_3f_20(PARAMS);  // illegal
		case 0x21: return arcompact_handle05_2f_3f_21(PARAMS);  // illegal
		case 0x22: return arcompact_handle05_2f_3f_22(PARAMS);  // illegal
		case 0x23: return arcompact_handle05_2f_3f_23(PARAMS);  // illegal
		case 0x24: return arcompact_handle05_2f_3f_24(PARAMS);  // illegal
		case 0x25: return arcompact_handle05_2f_3f_25(PARAMS);  // illegal
		case 0x26: return arcompact_handle05_2f_3f_26(PARAMS);  // illegal
		case 0x27: return arcompact_handle05_2f_3f_27(PARAMS);  // illegal
		case 0x28: return arcompact_handle05_2f_3f_28(PARAMS);  // illegal
		case 0x29: return arcompact_handle05_2f_3f_29(PARAMS);  // illegal
		case 0x2a: return arcompact_handle05_2f_3f_2a(PARAMS);  // illegal
		case 0x2b: return arcompact_handle05_2f_3f_2b(PARAMS);  // illegal
		case 0x2c: return arcompact_handle05_2f_3f_2c(PARAMS);  // illegal
		case 0x2d: return arcompact_handle05_2f_3f_2d(PARAMS);  // illegal
		case 0x2e: return arcompact_handle05_2f_3f_2e(PARAMS);  // illegal
		case 0x2f: return arcompact_handle05_2f_3f_2f(PARAMS);  // illegal
		case 0x30: return arcompact_handle05_2f_3f_30(PARAMS);  // illegal
		case 0x31: return arcompact_handle05_2f_3f_31(PARAMS);  // illegal
		case 0x32: return arcompact_handle05_2f_3f_32(PARAMS);  // illegal
		case 0x33: return arcompact_handle05_2f_3f_33(PARAMS);  // illegal
		case 0x34: return arcompact_handle05_2f_3f_34(PARAMS);  // illegal
		case 0x35: return arcompact_handle05_2f_3f_35(PARAMS);  // illegal
		case 0x36: return arcompact_handle05_2f_3f_36(PARAMS);  // illegal
		case 0x37: return arcompact_handle05_2f_3f_37(PARAMS);  // illegal
		case 0x38: return arcompact_handle05_2f_3f_38(PARAMS);  // illegal
		case 0x39: return arcompact_handle05_2f_3f_39(PARAMS);  // illegal
		case 0x3a: return arcompact_handle05_2f_3f_3a(PARAMS);  // illegal
		case 0x3b: return arcompact_handle05_2f_3f_3b(PARAMS);  // illegal
		case 0x3c: return arcompact_handle05_2f_3f_3c(PARAMS);  // illegal
		case 0x3d: return arcompact_handle05_2f_3f_3d(PARAMS);  // illegal
		case 0x3e: return arcompact_handle05_2f_3f_3e(PARAMS);  // illegal
		case 0x3f: return arcompact_handle05_2f_3f_3f(PARAMS);  // illegal
	}

	return 0;
}


// this is an Extension ALU group, maybe optional on some CPUs?
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05(OPS_32)
{
	UINT8 subinstr = (op & 0x003f0000) >> 16;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle05_00(PARAMS);  // ASL
		case 0x01: return arcompact_handle05_01(PARAMS);  // LSR
		case 0x02: return arcompact_handle05_02(PARAMS);  // ASR
		case 0x03: return arcompact_handle05_03(PARAMS);  // ROR
		case 0x04: return arcompact_handle05_04(PARAMS);  // MUL64
		case 0x05: return arcompact_handle05_05(PARAMS);  // MULU64
		case 0x06: return arcompact_handle05_06(PARAMS);  // ADDS
		case 0x07: return arcompact_handle05_07(PARAMS);  // SUBS
		case 0x08: return arcompact_handle05_08(PARAMS);  // DIVAW
		case 0x09: return arcompact_handle05_09(PARAMS);  // illegal
		case 0x0a: return arcompact_handle05_0a(PARAMS);  // ASLS
		case 0x0b: return arcompact_handle05_0b(PARAMS);  // ASRS
		case 0x0c: return arcompact_handle05_0c(PARAMS);  // illegal
		case 0x0d: return arcompact_handle05_0d(PARAMS);  // illegal
		case 0x0e: return arcompact_handle05_0e(PARAMS);  // illegal
		case 0x0f: return arcompact_handle05_0f(PARAMS);  // illegal
		case 0x10: return arcompact_handle05_10(PARAMS);  // illegal
		case 0x11: return arcompact_handle05_11(PARAMS);  // illegal
		case 0x12: return arcompact_handle05_12(PARAMS);  // illegal
		case 0x13: return arcompact_handle05_13(PARAMS);  // illegal
		case 0x14: return arcompact_handle05_14(PARAMS);  // illegal
		case 0x15: return arcompact_handle05_15(PARAMS);  // illegal
		case 0x16: return arcompact_handle05_16(PARAMS);  // illegal
		case 0x17: return arcompact_handle05_17(PARAMS);  // illegal
		case 0x18: return arcompact_handle05_18(PARAMS);  // illegal
		case 0x19: return arcompact_handle05_19(PARAMS);  // illegal
		case 0x1a: return arcompact_handle05_1a(PARAMS);  // illegal
		case 0x1b: return arcompact_handle05_1b(PARAMS);  // illegal
		case 0x1c: return arcompact_handle05_1c(PARAMS);  // illegal
		case 0x1d: return arcompact_handle05_1d(PARAMS);  // illegal
		case 0x1e: return arcompact_handle05_1e(PARAMS);  // illegal
		case 0x1f: return arcompact_handle05_1f(PARAMS);  // illegal
		case 0x20: return arcompact_handle05_20(PARAMS);  // illegal
		case 0x21: return arcompact_handle05_21(PARAMS);  // illegal
		case 0x22: return arcompact_handle05_22(PARAMS);  // illegal
		case 0x23: return arcompact_handle05_23(PARAMS);  // illegal
		case 0x24: return arcompact_handle05_24(PARAMS);  // illegal
		case 0x25: return arcompact_handle05_25(PARAMS);  // illegal
		case 0x26: return arcompact_handle05_26(PARAMS);  // illegal
		case 0x27: return arcompact_handle05_27(PARAMS);  // illegal
		case 0x28: return arcompact_handle05_28(PARAMS);  // ADDSDW
		case 0x29: return arcompact_handle05_29(PARAMS);  // SUBSDW
		case 0x2a: return arcompact_handle05_2a(PARAMS);  // illegal
		case 0x2b: return arcompact_handle05_2b(PARAMS);  // illegal
		case 0x2c: return arcompact_handle05_2c(PARAMS);  // illegal
		case 0x2d: return arcompact_handle05_2d(PARAMS);  // illegal
		case 0x2e: return arcompact_handle05_2e(PARAMS);  // illegal
		case 0x2f: return arcompact_handle05_2f(PARAMS);  // SOPs
		case 0x30: return arcompact_handle05_30(PARAMS);  // illegal
		case 0x31: return arcompact_handle05_31(PARAMS);  // illegal
		case 0x32: return arcompact_handle05_32(PARAMS);  // illegal
		case 0x33: return arcompact_handle05_33(PARAMS);  // illegal
		case 0x34: return arcompact_handle05_34(PARAMS);  // illegal
		case 0x35: return arcompact_handle05_35(PARAMS);  // illegal
		case 0x36: return arcompact_handle05_36(PARAMS);  // illegal
		case 0x37: return arcompact_handle05_37(PARAMS);  // illegal
		case 0x38: return arcompact_handle05_38(PARAMS);  // illegal
		case 0x39: return arcompact_handle05_39(PARAMS);  // illegal
		case 0x3a: return arcompact_handle05_3a(PARAMS);  // illegal
		case 0x3b: return arcompact_handle05_3b(PARAMS);  // illegal
		case 0x3c: return arcompact_handle05_3c(PARAMS);  // illegal
		case 0x3d: return arcompact_handle05_3d(PARAMS);  // illegal
		case 0x3e: return arcompact_handle05_3e(PARAMS);  // illegal
		case 0x3f: return arcompact_handle05_3f(PARAMS);  // illegal
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c(OPS_16)
{
	UINT8 subinstr = (op & 0x0018) >> 3;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0c_00(PARAMS);  // LD_S
		case 0x01: return arcompact_handle0c_01(PARAMS);  // LDB_S
		case 0x02: return arcompact_handle0c_02(PARAMS);  // LDW_S
		case 0x03: return arcompact_handle0c_03(PARAMS);  // ADD_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0d(OPS_16)
{
	UINT8 subinstr = (op & 0x0018) >> 3;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0d_00(PARAMS);  // ADD_S
		case 0x01: return arcompact_handle0d_01(PARAMS);  // SUB_S
		case 0x02: return arcompact_handle0d_02(PARAMS);  // ASL_S
		case 0x03: return arcompact_handle0d_03(PARAMS);  // ASR_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e(OPS_16)
{
	UINT8 subinstr = (op & 0x0018) >> 3;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0e_00(PARAMS);  // ADD_S
		case 0x01: return arcompact_handle0e_01(PARAMS);  // MOV_S
		case 0x02: return arcompact_handle0e_02(PARAMS);  // CMP_S
		case 0x03: return arcompact_handle0e_03(PARAMS);  // MOV_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f(OPS_16)
{
	UINT8 subinstr = (op & 0x01f) >> 0;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0f_00(PARAMS);  // SOPs
		case 0x01: return arcompact_handle0f_01(PARAMS);  // 0x01 <illegal>
		case 0x02: return arcompact_handle0f_02(PARAMS);  // SUB_S
		case 0x03: return arcompact_handle0f_03(PARAMS);  // 0x03 <illegal>
		case 0x04: return arcompact_handle0f_04(PARAMS);  // AND_S
		case 0x05: return arcompact_handle0f_05(PARAMS);  // OR_S
		case 0x06: return arcompact_handle0f_06(PARAMS);  // BIC_S
		case 0x07: return arcompact_handle0f_07(PARAMS);  // XOR_S
		case 0x08: return arcompact_handle0f_08(PARAMS);  // 0x08 <illegal>
		case 0x09: return arcompact_handle0f_09(PARAMS);  // 0x09 <illegal>
		case 0x0a: return arcompact_handle0f_0a(PARAMS);  // 0x0a <illegal>
		case 0x0b: return arcompact_handle0f_0b(PARAMS);  // TST_S
		case 0x0c: return arcompact_handle0f_0c(PARAMS);  // MUL64_S
		case 0x0d: return arcompact_handle0f_0d(PARAMS);  // SEXB_S
		case 0x0e: return arcompact_handle0f_0e(PARAMS);  // SEXW_S
		case 0x0f: return arcompact_handle0f_0f(PARAMS);  // EXTB_S
		case 0x10: return arcompact_handle0f_10(PARAMS);  // EXTW_S
		case 0x11: return arcompact_handle0f_11(PARAMS);  // ABS_S
		case 0x12: return arcompact_handle0f_12(PARAMS);  // NOT_S
		case 0x13: return arcompact_handle0f_13(PARAMS);  // NEG_S
		case 0x14: return arcompact_handle0f_14(PARAMS);  // ADD1_S
		case 0x15: return arcompact_handle0f_15(PARAMS);  // ADD2_S
		case 0x16: return arcompact_handle0f_16(PARAMS);  // ADD3_S
		case 0x17: return arcompact_handle0f_17(PARAMS);  // 0x17 <illegal>
		case 0x18: return arcompact_handle0f_18(PARAMS);  // ASL_S (multiple)
		case 0x19: return arcompact_handle0f_19(PARAMS);  // LSR_S (multiple)
		case 0x1a: return arcompact_handle0f_1a(PARAMS);  // ASR_S (multiple)
		case 0x1b: return arcompact_handle0f_1b(PARAMS);  // ASL_S (single)
		case 0x1c: return arcompact_handle0f_1c(PARAMS);  // LSR_S (single)
		case 0x1d: return arcompact_handle0f_1d(PARAMS);  // ASR_S (single)
		case 0x1e: return arcompact_handle0f_1e(PARAMS);  // TRAP (not a5?)
		case 0x1f: return arcompact_handle0f_1f(PARAMS);  // BRK_S ( 0x7fff only? )

	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00(OPS_16)
{
	UINT8 subinstr = (op & 0x00e0) >> 5;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0f_00_00(PARAMS);  // J_S
		case 0x01: return arcompact_handle0f_00_01(PARAMS);  // J_S.D
		case 0x02: return arcompact_handle0f_00_02(PARAMS);  // JL_S
		case 0x03: return arcompact_handle0f_00_03(PARAMS);  // JL_S.D
		case 0x04: return arcompact_handle0f_00_04(PARAMS);  // 0x04 <illegal>
		case 0x05: return arcompact_handle0f_00_05(PARAMS);  // 0x05 <illegal>
		case 0x06: return arcompact_handle0f_00_06(PARAMS);  // SUB_S.NE
		case 0x07: return arcompact_handle0f_00_07(PARAMS);  // ZOPs

	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07(OPS_16)
{
	UINT8 subinstr3 = (op & 0x0700) >> 8;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle0f_00_07_00(PARAMS);  // NOP_S
		case 0x01: return arcompact_handle0f_00_07_01(PARAMS);  // UNIMP_S
		case 0x02: return arcompact_handle0f_00_07_02(PARAMS);  // 0x02 <illegal>
		case 0x03: return arcompact_handle0f_00_07_03(PARAMS);  // 0x03 <illegal>
		case 0x04: return arcompact_handle0f_00_07_04(PARAMS);  // JEQ_S [BLINK]
		case 0x05: return arcompact_handle0f_00_07_05(PARAMS);  // JNE_S [BLINK]
		case 0x06: return arcompact_handle0f_00_07_06(PARAMS);  // J_S [BLINK]
		case 0x07: return arcompact_handle0f_00_07_07(PARAMS);  // J_S.D [BLINK]

	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17(OPS_16)
{
	UINT8 subinstr = (op & 0x00e0) >> 5;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle17_00(PARAMS);  // ASL_S
		case 0x01: return arcompact_handle17_01(PARAMS);  // LSR_S
		case 0x02: return arcompact_handle17_02(PARAMS);  // ASR_S
		case 0x03: return arcompact_handle17_03(PARAMS);  // SUB_S
		case 0x04: return arcompact_handle17_04(PARAMS);  // BSET_S
		case 0x05: return arcompact_handle17_05(PARAMS);  // BCLR_S
		case 0x06: return arcompact_handle17_06(PARAMS);  // BMSK_S
		case 0x07: return arcompact_handle17_07(PARAMS);  // BTST_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18(OPS_16)
{
	UINT8 subinstr = (op & 0x00e0) >> 5;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle18_00(PARAMS);  // LD_S (SP)
		case 0x01: return arcompact_handle18_01(PARAMS);  // LDB_S (SP)
		case 0x02: return arcompact_handle18_02(PARAMS);  // ST_S (SP)
		case 0x03: return arcompact_handle18_03(PARAMS);  // STB_S (SP)
		case 0x04: return arcompact_handle18_04(PARAMS);  // ADD_S (SP)
		case 0x05: return arcompact_handle18_05(PARAMS);  // subtable 18_05
		case 0x06: return arcompact_handle18_06(PARAMS);  // subtable 18_06
		case 0x07: return arcompact_handle18_07(PARAMS);  // subtable 18_07
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05(OPS_16)
{
	UINT8 subinstr2 = (op & 0x0700) >> 8;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle18_05_00(PARAMS);  // ADD_S (SP)
		case 0x01: return arcompact_handle18_05_01(PARAMS);  // SUB_S (SP)
		case 0x02: return arcompact_handle18_05_02(PARAMS);  // <illegal 0x18_05_02>
		case 0x03: return arcompact_handle18_05_03(PARAMS);  // <illegal 0x18_05_03>
		case 0x04: return arcompact_handle18_05_04(PARAMS);  // <illegal 0x18_05_04>
		case 0x05: return arcompact_handle18_05_05(PARAMS);  // <illegal 0x18_05_05>
		case 0x06: return arcompact_handle18_05_06(PARAMS);  // <illegal 0x18_05_06>
		case 0x07: return arcompact_handle18_05_07(PARAMS);  // <illegal 0x18_05_07>
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06(OPS_16)
{
	UINT8 subinstr2 = (op & 0x001f) >> 0;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle18_06_00(PARAMS);  // <illegal 0x18_06_00>
		case 0x01: return arcompact_handle18_06_01(PARAMS);  // POP_S b
		case 0x02: return arcompact_handle18_06_02(PARAMS);  // <illegal 0x18_06_02>
		case 0x03: return arcompact_handle18_06_03(PARAMS);  // <illegal 0x18_06_03>
		case 0x04: return arcompact_handle18_06_04(PARAMS);  // <illegal 0x18_06_04>
		case 0x05: return arcompact_handle18_06_05(PARAMS);  // <illegal 0x18_06_05>
		case 0x06: return arcompact_handle18_06_06(PARAMS);  // <illegal 0x18_06_06>
		case 0x07: return arcompact_handle18_06_07(PARAMS);  // <illegal 0x18_06_07>
		case 0x08: return arcompact_handle18_06_08(PARAMS);  // <illegal 0x18_06_08>
		case 0x09: return arcompact_handle18_06_09(PARAMS);  // <illegal 0x18_06_09>
		case 0x0a: return arcompact_handle18_06_0a(PARAMS);  // <illegal 0x18_06_0a>
		case 0x0b: return arcompact_handle18_06_0b(PARAMS);  // <illegal 0x18_06_0b>
		case 0x0c: return arcompact_handle18_06_0c(PARAMS);  // <illegal 0x18_06_0c>
		case 0x0d: return arcompact_handle18_06_0d(PARAMS);  // <illegal 0x18_06_0d>
		case 0x0e: return arcompact_handle18_06_0e(PARAMS);  // <illegal 0x18_06_0e>
		case 0x0f: return arcompact_handle18_06_0f(PARAMS);  // <illegal 0x18_06_0f>
		case 0x10: return arcompact_handle18_06_10(PARAMS);  // <illegal 0x18_06_10>
		case 0x11: return arcompact_handle18_06_11(PARAMS);  // POP_S blink
		case 0x12: return arcompact_handle18_06_12(PARAMS);  // <illegal 0x18_06_12>
		case 0x13: return arcompact_handle18_06_13(PARAMS);  // <illegal 0x18_06_13>
		case 0x14: return arcompact_handle18_06_14(PARAMS);  // <illegal 0x18_06_14>
		case 0x15: return arcompact_handle18_06_15(PARAMS);  // <illegal 0x18_06_15>
		case 0x16: return arcompact_handle18_06_16(PARAMS);  // <illegal 0x18_06_16>
		case 0x17: return arcompact_handle18_06_17(PARAMS);  // <illegal 0x18_06_17>
		case 0x18: return arcompact_handle18_06_18(PARAMS);  // <illegal 0x18_06_18>
		case 0x19: return arcompact_handle18_06_19(PARAMS);  // <illegal 0x18_06_19>
		case 0x1a: return arcompact_handle18_06_1a(PARAMS);  // <illegal 0x18_06_1a>
		case 0x1b: return arcompact_handle18_06_1b(PARAMS);  // <illegal 0x18_06_1b>
		case 0x1c: return arcompact_handle18_06_1c(PARAMS);  // <illegal 0x18_06_1c>
		case 0x1d: return arcompact_handle18_06_1d(PARAMS);  // <illegal 0x18_06_1d>
		case 0x1e: return arcompact_handle18_06_1e(PARAMS);  // <illegal 0x18_06_1e>
		case 0x1f: return arcompact_handle18_06_1f(PARAMS);  // <illegal 0x18_06_1f>
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07(OPS_16)
{
	UINT8 subinstr2 = (op & 0x001f) >> 0;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle18_07_00(PARAMS);  // <illegal 0x18_07_00>
		case 0x01: return arcompact_handle18_07_01(PARAMS);  // PUSH_S b
		case 0x02: return arcompact_handle18_07_02(PARAMS);  // <illegal 0x18_07_02>
		case 0x03: return arcompact_handle18_07_03(PARAMS);  // <illegal 0x18_07_03>
		case 0x04: return arcompact_handle18_07_04(PARAMS);  // <illegal 0x18_07_04>
		case 0x05: return arcompact_handle18_07_05(PARAMS);  // <illegal 0x18_07_05>
		case 0x06: return arcompact_handle18_07_06(PARAMS);  // <illegal 0x18_07_06>
		case 0x07: return arcompact_handle18_07_07(PARAMS);  // <illegal 0x18_07_07>
		case 0x08: return arcompact_handle18_07_08(PARAMS);  // <illegal 0x18_07_08>
		case 0x09: return arcompact_handle18_07_09(PARAMS);  // <illegal 0x18_07_09>
		case 0x0a: return arcompact_handle18_07_0a(PARAMS);  // <illegal 0x18_07_0a>
		case 0x0b: return arcompact_handle18_07_0b(PARAMS);  // <illegal 0x18_07_0b>
		case 0x0c: return arcompact_handle18_07_0c(PARAMS);  // <illegal 0x18_07_0c>
		case 0x0d: return arcompact_handle18_07_0d(PARAMS);  // <illegal 0x18_07_0d>
		case 0x0e: return arcompact_handle18_07_0e(PARAMS);  // <illegal 0x18_07_0e>
		case 0x0f: return arcompact_handle18_07_0f(PARAMS);  // <illegal 0x18_07_0f>
		case 0x10: return arcompact_handle18_07_10(PARAMS);  // <illegal 0x18_07_10>
		case 0x11: return arcompact_handle18_07_11(PARAMS);  // PUSH_S blink
		case 0x12: return arcompact_handle18_07_12(PARAMS);  // <illegal 0x18_07_12>
		case 0x13: return arcompact_handle18_07_13(PARAMS);  // <illegal 0x18_07_13>
		case 0x14: return arcompact_handle18_07_14(PARAMS);  // <illegal 0x18_07_14>
		case 0x15: return arcompact_handle18_07_15(PARAMS);  // <illegal 0x18_07_15>
		case 0x16: return arcompact_handle18_07_16(PARAMS);  // <illegal 0x18_07_16>
		case 0x17: return arcompact_handle18_07_17(PARAMS);  // <illegal 0x18_07_17>
		case 0x18: return arcompact_handle18_07_18(PARAMS);  // <illegal 0x18_07_18>
		case 0x19: return arcompact_handle18_07_19(PARAMS);  // <illegal 0x18_07_19>
		case 0x1a: return arcompact_handle18_07_1a(PARAMS);  // <illegal 0x18_07_1a>
		case 0x1b: return arcompact_handle18_07_1b(PARAMS);  // <illegal 0x18_07_1b>
		case 0x1c: return arcompact_handle18_07_1c(PARAMS);  // <illegal 0x18_07_1c>
		case 0x1d: return arcompact_handle18_07_1d(PARAMS);  // <illegal 0x18_07_1d>
		case 0x1e: return arcompact_handle18_07_1e(PARAMS);  // <illegal 0x18_07_1e>
		case 0x1f: return arcompact_handle18_07_1f(PARAMS);  // <illegal 0x18_07_1f>
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19(OPS_16)
{
	UINT8 subinstr = (op & 0x0600) >> 9;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle19_00(PARAMS);  // LD_S (GP)
		case 0x01: return arcompact_handle19_01(PARAMS);  // LDB_S (GP)
		case 0x02: return arcompact_handle19_02(PARAMS);  // LDW_S (GP)
		case 0x03: return arcompact_handle19_03(PARAMS);  // ADD_S (GP)
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1c(OPS_16)
{
	UINT8 subinstr = (op & 0x0080) >> 7;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle1c_00(PARAMS);  // ADD_S
		case 0x01: return arcompact_handle1c_01(PARAMS);  // CMP_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1d(OPS_16)
{
	UINT8 subinstr = (op & 0x0080) >> 7;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle1d_00(PARAMS);  // BREQ_S
		case 0x01: return arcompact_handle1d_01(PARAMS);  // BRNE_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e(OPS_16)
{
	UINT8 subinstr = (op & 0x0600) >> 9;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle1e_00(PARAMS);  // B_S
		case 0x01: return arcompact_handle1e_01(PARAMS);  // BEQ_S
		case 0x02: return arcompact_handle1e_02(PARAMS);  // BNE_S
		case 0x03: return arcompact_handle1e_03(PARAMS);  // Bcc_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03(OPS_16)
{
	UINT8 subinstr2 = (op & 0x01c0) >> 6;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle1e_03_00(PARAMS);  // BGT_S
		case 0x01: return arcompact_handle1e_03_01(PARAMS);  // BGE_S
		case 0x02: return arcompact_handle1e_03_02(PARAMS);  // BLT_S
		case 0x03: return arcompact_handle1e_03_03(PARAMS);  // BLE_S
		case 0x04: return arcompact_handle1e_03_04(PARAMS);  // BHI_S
		case 0x05: return arcompact_handle1e_03_05(PARAMS);  // BHS_S
		case 0x06: return arcompact_handle1e_03_06(PARAMS);  // BLO_S
		case 0x07: return arcompact_handle1e_03_07(PARAMS);  // BLS_S
	}

	return 0;
}

// handlers

UINT32 arcompact_device::handle_jump_to_addr(int delay, int link, UINT32 address, UINT32 next_addr)
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

UINT32 arcompact_device::handle_jump_to_register(int delay, int link, UINT32 reg, UINT32 next_addr, int flag)
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
			UINT32 target = m_regs[reg];
			return handle_jump_to_addr(delay, link, target, next_addr);
		}
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle00_00(OPS_32)
{
	int size = 4;

	COMMON32_GET_CONDITION

	if (!check_condition(condition))
		return m_pc + (size>>0);

	// Branch Conditionally
	// 0000 0sss ssss sss0 SSSS SSSS SSNQ QQQQ
	INT32 address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x80000) address = -0x80000 + (address & 0x7ffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	UINT32 realaddress = PC_ALIGNED32 + (address * 2);

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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle00_01(OPS_32)
{
	int size = 4;
	// Branch Unconditionally Far
	INT32 address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address |= ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address & 0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
//  int res =  (op & 0x00000010) >> 4; op &= ~0x00000010; // should be set to 0

	UINT32 realaddress = PC_ALIGNED32 + (address * 2);

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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_00_00dasm(OPS_32)
{
	int size = 4;

	// Branch and Link Conditionally
	arcompact_log("unimplemented BLcc %08x", op);
	return m_pc + (size>>0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_00_01dasm(OPS_32)
{
	int size = 4;
	// Branch and Link Unconditionally Far
	// 00001 sssssssss 10  SSSSSSSSSS N R TTTT
	INT32 address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	address |=        ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
//  int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	UINT32 realaddress = PC_ALIGNED32 + (address * 2);

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



ARCOMPACT_RETTYPE arcompact_device::arcompact_01_01_00_helper(OPS_32, const char* optext)
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
		//UINT32 limm;
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
	UINT32 b,c; \
	if ((breg != LIMM_REG) && (creg != LIMM_REG)) \
	{ \
		b = m_regs[breg]; \
		c = m_regs[creg]; \
	} \
	else \
	{ \
		UINT32 limm; \
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
	UINT32 realaddress = PC_ALIGNED32 + (address * 2); \
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


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_00(OPS_32)  // register - register BREQ
{
	BR_REGREG_SETUP

	// BREQ
	if (b == c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_01(OPS_32) // register - register BRNE
{
	BR_REGREG_SETUP

	// BRNE
	if (b != c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_02(OPS_32) // regiter - register BRLT
{
	BR_REGREG_SETUP

	// BRLT  (signed operation)
	if ((INT32)b < (INT32)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);

}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_03(OPS_32) // register - register BRGE
{
	BR_REGREG_SETUP

	// BRGE  (signed operation)
	if ((INT32)b >= (INT32)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_04(OPS_32) // register - register BRLO
{
	BR_REGREG_SETUP

	// BRLO (unsigned operation)
	if (b < c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_05(OPS_32) // register - register BRHS
{
	BR_REGREG_SETUP

	// BRHS (unsigned operation)
	if (b >= c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0e(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BBIT0");}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0f(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BBIT1");}

ARCOMPACT_RETTYPE arcompact_device::arcompact_01_01_01_helper(OPS_32, const char* optext)
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
	UINT32 b,c; \
	c = u; \
	/* comparing a LIMM  to an immediate is pointless, is it a valid encoding? */ \
	if ((breg != LIMM_REG)) \
	{ \
		b = m_regs[breg]; \
	} \
	else \
	{ \
		UINT32 limm; \
		GET_LIMM_32; \
		size = 8; \
		b = limm; \
	}

// register -immediate cases
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_00(OPS_32) // BREQ reg-imm
{
	BR_REGIMM_SETUP

	// BREQ
	if (b == c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_01(OPS_32) // BRNE reg-imm
{
	BR_REGIMM_SETUP

	// BRNE
	if (b != c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_02(OPS_32) // BRLT reg-imm
{
	BR_REGIMM_SETUP

	// BRLT  (signed operation)
	if ((INT32)b < (INT32)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);

}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_03(OPS_32)
{
	BR_REGIMM_SETUP

	// BRGE  (signed operation)
	if ((INT32)b >= (INT32)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_04(OPS_32) //  register - immediate BRLO
{
	BR_REGIMM_SETUP

	// BRLO (unsigned operation)
	if (b < c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);

}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_05(OPS_32) // register - immediate BRHS
{
	BR_REGIMM_SETUP

	// BRHS (unsigned operation)
	if (b >= c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0e(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BBIT0"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0f(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BBIT1"); }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle02(OPS_32)
{
	int size = 4;
	UINT32 limm;

	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;
	if (S) s = -0x100 + s;

	COMMON32_GET_breg;
	COMMON32_GET_areg

	int X = (op & 0x00000040) >> 6;  //op &= ~0x00000040;
	int Z = (op & 0x00000180) >> 7;  //op &= ~0x00000180;
	int a = (op & 0x00000600) >> 9;  //op &= ~0x00000600;
//  int D = (op & 0x00000800) >> 11;// op &= ~0x00000800; // we don't use the data cache currently

	UINT32 address = m_regs[breg];

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

	UINT32 readdata = 0;

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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle03(OPS_32)
{
	int size = 4;
	UINT32 limm = 0;
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


	UINT32 address = m_regs[breg];

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;

		address = limm;
	}

	UINT32 writedata = m_regs[creg];

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





ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_helper(OPS_32, const char* optext, int ignore_dst, int b_reserved)
{
	int size;
	//UINT32 limm = 0;
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


#include "cpu/arcompact/arcompact.inc"

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_01(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x01], /*"ADC"*/ 0,0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_03(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x03], /*"SBC"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_08(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x08], /*"MAX"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_09(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x09], /*"MIN"*/ 0,0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_0b(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x0b], /*"TST"*/ 1,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_0c(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x0c], /*"CMP"*/ 1,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_0d(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x0d], /*"RCMP"*/ 1,0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_10(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x10], /*"BCLR"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_11(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x11], /*"BTST"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_12(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x12], /*"BXOR"*/ 0,0);
}






ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1a(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x1a], /*"MPY"*/ 0,0);
} // *

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1b(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x1b], /*"MPYH"*/ 0,0);
} // *

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1c(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x1c], /*"MPYHU"*/ 0,0);
} // *

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1d(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x1d], /*"MPYU"*/ 0,0);
} // *

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_20_p00(OPS_32)
{
	int size;
	UINT32 limm = 0;
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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_20_p01(OPS_32)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uu-- ----
	// J u6            0010 0RRR 0110 0000 0RRR uuuu uuRR RRRR
	int size = 4;
	arcompact_log("2 unimplemented J %08x", op);
	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_20_p10(OPS_32)
{
	// opcode          iiii i--- ppII IIII F--- ssss ssSS SSSS
	// J s12           0010 0RRR 1010 0000 0RRR ssss ssSS SSSS
	int size = 4;
	arcompact_log("3 unimplemented J %08x", op);
	return m_pc + (size>>0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_20_p11_m0(OPS_32) // Jcc   (no link, no delay)
{
	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_CONDITION;
	COMMON32_GET_F

	UINT32 c;

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
			arcompact_fatal ("fatal arcompact_handle04_20_p11_m0 J %08x (F not set but ILINK1 or ILINK2 used as dst)", op);
		}
		else
		{
			UINT32 realaddress = c;
			return realaddress;
		}
	}

	if (F)
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented arcompact_handle04_20_p11_m0 J %08x (F set)", op);
		}
		else
		{
			arcompact_fatal ("fatal arcompact_handle04_20_p11_m0 J %08x (F set but not ILINK1 or ILINK2 used as dst)", op);

		}
	}


	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_20_p11_m1(OPS_32)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uumq qqqq
	// Jcc u6          0010 0RRR 1110 0000 0RRR uuuu uu1Q QQQQ
	int size = 4;
	arcompact_log("unimplemented arcompact_handle04_20_p11_m1 J %08x (u6)", op);
	return m_pc + (size>>0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_21_p00(OPS_32)
{
	int size = 4;
	UINT32 limm = 0;
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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_21_p01(OPS_32)
{
	int size = 4;
	arcompact_log("unimplemented J.D (u6 type) %08x", op);
	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_21_p10(OPS_32)
{
	int size = 4;
	arcompact_log("unimplemented J.D (s12 type) %08x", op);
	return m_pc + (size>>0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_21_p11_m0(OPS_32) // Jcc.D   (no link, delay)
{
	int size = 4;
	UINT32 limm;
	int got_limm = 0;

	COMMON32_GET_creg
	COMMON32_GET_CONDITION;
	COMMON32_GET_F

	//UINT32 c = 0;

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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_21_p11_m1(OPS_32)
{
	int size = 4;
	arcompact_log("unimplemented arcompact_handle04_21_p11_m1 J.D %08x (u6)", op);
	return m_pc + (size>>0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_22(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x22], /*"JL"*/ 1,1);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_23(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x23], /*"JL.D"*/ 1,1);
}




ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_28(OPS_32) // LPcc (loop setup)
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
		//arcompact_fatal("Lp conditional %s not supported %d", conditions[condition], u);

		// if the loop condition fails then just jump to after the end of the loop, don't set any registers
		if (!check_condition(condition))
		{
			UINT32 realoffset = PC_ALIGNED32 + (u * 2);
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


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_29(OPS_32)
{
	// leapster bios uses formats for FLAG that are not defined, bug I guess work anyway (P modes 0 / 1)
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x29], /*"FLAG"*/ 1,1);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_helper(OPS_32, const char* optext)
{
	int size;

	COMMON32_GET_p;
	//COMMON32_GET_breg;

	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			//UINT32 limm;
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



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_00(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "ASL"); } // ASL
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_01(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "ASR"); } // ASR

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_04(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "RCC"); } // RCC
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_05(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "SEXB"); } // SEXB
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_06(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "SEXW"); } // SEXW


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_09(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "ABS"); } // ABS
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0a(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "NOT"); } // NOT
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0b(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "RCL"); } // RLC
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0c(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "EX"); } // EX


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_01(OPS_32)  { arcompact_log("SLEEP (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_02(OPS_32)  { arcompact_log("SWI / TRAP0 (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_03(OPS_32)  { arcompact_log("SYNC (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_04(OPS_32)  { arcompact_log("RTIE (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_05(OPS_32)  { arcompact_log("BRK (%08x)", op); return m_pc + (4 >> 0);}




ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3x_helper(OPS_32, int dsize, int extend)
{
	int size;
	//UINT32 limm=0;
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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_30(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,0,0); }
// ZZ value of 0x0 with X of 1 is illegal
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_31(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,0,1); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_32(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,1,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_33(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,1,1); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_34(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,2,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_35(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,2,1); }
// ZZ value of 0x3 is illegal
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_36(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,3,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_37(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,3,1); }








ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_02(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ASR", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_03(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ROR", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_04(OPS_32)  { return arcompact_handle04_helper(PARAMS, "MUL64", 2,0); } // special
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_05(OPS_32)  { return arcompact_handle04_helper(PARAMS, "MULU64", 2,0);} // special
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_06(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ADDS", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_07(OPS_32)  { return arcompact_handle04_helper(PARAMS, "SUBS", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_08(OPS_32)  { return arcompact_handle04_helper(PARAMS, "DIVAW", 0,0); }



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0a(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ASLS", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0b(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ASRS", 0,0); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_28(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ADDSDW", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_29(OPS_32)  { return arcompact_handle04_helper(PARAMS, "SUBSDW", 0,0); }



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0x_helper(OPS_32, const char* optext)
{
	int size;

	COMMON32_GET_p;
	//COMMON32_GET_breg;

	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			//UINT32 limm;
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


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_00(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "SWAP");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_01(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "NORM");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_02(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "SAT16"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_03(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "RND16"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_04(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "ABSSW"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_05(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "ABSS");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_06(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "NEGSW"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_07(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "NEGS");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_08(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "NORMW"); }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle06(OPS_32)
{
	arcompact_log("op a,b,c (06 ARC ext) (%08x)", op );
	return m_pc + (4 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle07(OPS_32)
{
	arcompact_log("op a,b,c (07 User ext) (%08x)", op );
	return m_pc + (4 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle08(OPS_32)
{
	arcompact_log("op a,b,c (08 User ext) (%08x)", op );
	return m_pc + (4 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle09(OPS_32)
{
	arcompact_log("op a,b,c (09 Market ext) (%08x)", op );
	return m_pc + (4 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0a(OPS_32)
{
	arcompact_log("op a,b,c (0a Market ext) (%08x)",  op );
	return m_pc + (4 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0b(OPS_32)
{
	arcompact_log("op a,b,c (0b Market ext) (%08x)",  op );
	return m_pc + (4 >> 0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x (0x0c group)", optext, op);
	return m_pc + (2 >> 0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_00(OPS_16)
{
	return arcompact_handle0c_helper(PARAMS, "LD_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_01(OPS_16)
{
	return arcompact_handle0c_helper(PARAMS, "LDB_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_02(OPS_16)
{
	return arcompact_handle0c_helper(PARAMS, "LDW_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_03(OPS_16) // ADD_S a <- b + c
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


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0d_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x (0x0d group)", optext, op);
	return m_pc + (2 >> 0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0d_03(OPS_16)
{
	return arcompact_handle0d_helper(PARAMS, "ASR_S");
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_0x_helper(OPS_16, const char* optext, int revop)
{
	int h;// , breg;
	int size;

	GROUP_0e_GET_h;

	if (h == LIMM_REG)
	{
		//UINT32 limm;
		//GET_LIMM;
		size = 6;
	}
	else
	{
	}

	arcompact_log("unimplemented %s %04x (0x0e_0x group)", optext, op);

	return m_pc+ (size>>0);

}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_00(OPS_16) // ADD_s b, b, h
{
	int h,breg;
	int size = 2;

	GROUP_0e_GET_h;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (h == LIMM_REG)
	{
		UINT32 limm;
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
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_01(OPS_16) // MOV_S b <- h
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

		UINT32 limm;
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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_02(OPS_16)
{
	return arcompact_handle0e_0x_helper(PARAMS, "CMP_S", 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_03(OPS_16) // MOV_S h <- b
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



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_0x_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_00(OPS_16)  { return arcompact_handle0f_00_0x_helper(PARAMS, "J_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_01(OPS_16)  { return arcompact_handle0f_00_0x_helper(PARAMS, "J_S.D"); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_02(OPS_16) // JL_S
{
	int breg;

	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_regs[REG_BLINK] = m_pc + (2 >> 0);

	return m_regs[breg];
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_03(OPS_16) // JL_S.D
{
	int breg;

	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_delayactive = 1;
	m_delayjump = m_regs[breg];
	m_delaylinks = 1;

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_06(OPS_16)  { return arcompact_handle0f_00_0x_helper(PARAMS, "SUB_S.NE"); }




// Zero parameters (ZOP)
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_00(OPS_16)  { /*arcompact_log("NOP_S");*/ return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_01(OPS_16)  { arcompact_log("UNIMP_S"); return m_pc + (2 >> 0);} // Unimplemented Instruction, same as illegal, but recommended to fill blank space
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_04(OPS_16)  { arcompact_log("JEQ_S [blink]"); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_05(OPS_16)  { arcompact_log("JNE_S [blink]"); return m_pc + (2 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_06(OPS_16) // J_S [blink]
{
	return m_regs[REG_BLINK];
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_07(OPS_16) // J_S.D [blink]
{
	m_delayactive = 1;
	m_delayjump = m_regs[REG_BLINK];
	m_delaylinks = 0;

	return m_pc + (2 >> 0);
}





ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0x_helper(OPS_16, const char* optext, int nodst)
{
	arcompact_log("unimplemented %s %04x (0xf_0x group)", optext, op);
	return m_pc + (2 >> 0);
}






ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_06(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "BIC_S",0);  }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0b(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "TST_S",1);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0c(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "MUL64_S",2);  } // actual destination is special multiply registers
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0d(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "SEXB_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0e(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "SEXW_S",0); }




ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_11(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ABS_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_12(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "NOT_S",0);  }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_18(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ASL_S",0);  }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1a(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ASR_S",0);  }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1c(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ASR1_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1d(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "LSR1_S",0); }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1e(OPS_16)  // special
{
	arcompact_log("unimplemented TRAP_S %04x",  op);
	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1f(OPS_16)  // special
{
	arcompact_log("unimplemented BRK_S %04x",  op);
	return m_pc + (2 >> 0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle_ld_helper(OPS_16, const char* optext, int shift, int swap)
{
	arcompact_log("unimplemented %s %04x (ld/st group %d %d)", optext, op, shift, swap);
	return m_pc + (2 >> 0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle10(OPS_16)
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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle11(OPS_16)
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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle12(OPS_16)
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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle13(OPS_16)
{
	return arcompact_handle_ld_helper(PARAMS, "LDW_S.X", 1, 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle14(OPS_16) // ST_S c, [b, u7]
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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle15(OPS_16) // STB_S c. [b, u6]
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

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle16(OPS_16) // STW_S c. [b, u6]
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


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle_l7_0x_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x (l7_0x group)", optext, op);
	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_05(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "BCLR_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_07(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "BTST_S");
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_0x_helper(OPS_16, const char* optext, int st)
{
	arcompact_log("unimplemented %s %04x (0x18_0x group)", optext, op);
	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_00(OPS_16)   // LD_S b, [SP, u7]
{
	int breg;
	UINT32 u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	UINT32 address = m_regs[REG_SP] + (u << 2);

	m_regs[breg] = READ32(address >> 2);

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_01(OPS_16)
{
	return arcompact_handle18_0x_helper(PARAMS, "LDB_S (SP)", 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_02(OPS_16)  // ST_S b, [SP, u7]
{
	int breg;
	UINT32 u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	UINT32 address = m_regs[REG_SP] + (u << 2);

	WRITE32(address >> 2, m_regs[breg]);

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_03(OPS_16)
{
	return arcompact_handle18_0x_helper(PARAMS, "STB_S (SP)", 1);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_04(OPS_16)  // ADD_S b, SP, u7
{
	int breg;
	UINT32 u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[REG_SP] + (u << 2);

	return m_pc + (2 >> 0);
}

// op bits remaining for 0x18_05_xx subgroups 0x001f
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_00(OPS_16)
{
	int u;
	COMMON16_GET_u5;

	m_regs[REG_SP] = m_regs[REG_SP] + (u << 2);

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_01(OPS_16)
{
	int u;
	COMMON16_GET_u5;

	m_regs[REG_SP] = m_regs[REG_SP] - (u << 2);

	return m_pc + (2 >> 0);
}

// op bits remaining for 0x18_06_xx subgroups 0x0700
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_01(OPS_16) // POP_S b
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_regs[breg] = READ32(m_regs[REG_SP] >> 2);
	m_regs[REG_SP] += 4;

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_11(OPS_16) // POP_S blink
{
	// breg bits are reserved
	m_regs[REG_BLINK] = READ32(m_regs[REG_SP] >> 2 );
	m_regs[REG_SP] += 4;

	return m_pc + (2 >> 0);
}

// op bits remaining for 0x18_07_xx subgroups 0x0700
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_01(OPS_16) // PUSH_S b
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	m_regs[REG_SP] -= 4;

	WRITE32(m_regs[REG_SP] >> 2, m_regs[breg]);

	return m_pc + (2 >> 0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_11(OPS_16) // PUSH_S [blink]
{
	// breg bits are reserved

	m_regs[REG_SP] -= 4;

	WRITE32(m_regs[REG_SP] >> 2, m_regs[REG_BLINK]);

	return m_pc + (2 >> 0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_0x_helper(OPS_16, const char* optext, int shift, int format)
{
	arcompact_log("unimplemented %s %04x (0x19_0x group)", optext, op);
	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_00(OPS_16)  { return arcompact_handle19_0x_helper(PARAMS, "LD_S", 2, 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_01(OPS_16)  { return arcompact_handle19_0x_helper(PARAMS, "LDB_S", 0, 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_02(OPS_16)  { return arcompact_handle19_0x_helper(PARAMS, "LDW_S", 1, 0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_03(OPS_16)  { return arcompact_handle19_0x_helper(PARAMS, "ADD_S", 2, 1); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1a(OPS_16)
{
	arcompact_log("unimplemented MOV_S x, [PCL, x] %04x",  op);
	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1b(OPS_16) // MOV_S b, u8
{
	int breg;
	UINT32 u;
	COMMON16_GET_breg;
	COMMON16_GET_u8;
	REG_16BIT_RANGE(breg);

	m_regs[breg] = u;

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1c_00(OPS_16) // ADD_S b, b, u7
{
	int breg;
	UINT32 u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	m_regs[breg] = m_regs[breg] + u;

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1c_01(OPS_16) // CMP b, u7
{
	int breg;
	UINT32 u;
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
	INT32 temp = (INT32)m_regs[breg] - (INT32)u;

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



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1d_00(OPS_16) // BREQ_S b,0,s8
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (!m_regs[breg])
	{
		int s = (op & 0x007f) >> 0; op &= ~0x007f;
		if (s & 0x40) s = -0x40 + (s & 0x3f);
		UINT32 realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1d_01(OPS_16) // BRNE_S b,0,s8
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	if (m_regs[breg])
	{
		int s = (op & 0x007f) >> 0; op &= ~0x007f;
		if (s & 0x40) s = -0x40 + (s & 0x3f);
		UINT32 realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_0x_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x (1e_0x type)", optext, op);
	return m_pc + (2 >> 0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_00(OPS_16) // B_S s10  (branch always)
{
	int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
	if (s & 0x100) s = -0x100 + (s & 0xff);
	UINT32 realaddress = PC_ALIGNED32 + (s * 2);
	//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
	return realaddress;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_01(OPS_16) // BEQ_S s10 (branch is zero bit is set)
{
	if (STATUS32_CHECK_Z)
	{
		int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
		if (s & 0x100) s = -0x100 + (s & 0xff);
		UINT32 realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_02(OPS_16) // BNE_S s10  (branch if zero bit isn't set)
{
	if (!STATUS32_CHECK_Z)
	{
		int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
		if (s & 0x100) s = -0x100 + (s & 0xff);
		UINT32 realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_0x_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_00(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BGT_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_01(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BGE_S"); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_02(OPS_16) // BLT_S
{
	if (CONDITION_LT)
	{
		int s = (op & 0x003f) >> 0; op &= ~0x003f;
		if (s & 0x020) s = -0x20 + (s & 0x1f);
		UINT32 realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_03(OPS_16) // BLE_S
{
	if (CONDITION_LE)
	{
		int s = (op & 0x003f) >> 0; op &= ~0x003f;
		if (s & 0x020) s = -0x20 + (s & 0x1f);
		UINT32 realaddress = PC_ALIGNED32 + (s * 2);
		//m_regs[REG_BLINK] = m_pc + (2 >> 0); // don't link
		return realaddress;
	}

	return m_pc + (2 >> 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_04(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BHI_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_05(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BHS_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_06(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BLO_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_07(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BLS_S"); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1f(OPS_16) // BL_S s13
{
	int s = (op & 0x07ff) >> 0; op &= ~0x07ff;
	if (s & 0x400) s = -0x400 + (s & 0x3ff);

	UINT32 realaddress = PC_ALIGNED32 + (s * 4);

	m_regs[REG_BLINK] = m_pc + (2 >> 0);
	return realaddress;
}

/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers (disassembly)                                                                                             *
*                                                                                                                                   *
************************************************************************************************************************************/

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_06(OPS_32)  { arcompact_fatal("<illegal 01_01_00_06> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_07(OPS_32)  { arcompact_fatal("<illegal 01_01_00_07> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_08(OPS_32)  { arcompact_fatal("<illegal 01_01_00_08> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_09(OPS_32)  { arcompact_fatal("<illegal 01_01_00_09> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0a(OPS_32)  { arcompact_fatal("<illegal 01_01_00_0a> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0b(OPS_32)  { arcompact_fatal("<illegal 01_01_00_0b> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0c(OPS_32)  { arcompact_fatal("<illegal 01_01_00_0c> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0d(OPS_32)  { arcompact_fatal("<illegal 01_01_00_0d> (%08x)", op); return m_pc + (4 >> 0); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_06(OPS_32)  { arcompact_fatal("<illegal 01_01_01_06> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_07(OPS_32)  { arcompact_fatal("<illegal 01_01_01_07> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_08(OPS_32)  { arcompact_fatal("<illegal 01_01_01_08> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_09(OPS_32)  { arcompact_fatal("<illegal 01_01_01_09> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0a(OPS_32)  { arcompact_fatal("<illegal 01_01_01_0a> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0b(OPS_32)  { arcompact_fatal("<illegal 01_01_01_0b> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0c(OPS_32)  { arcompact_fatal("<illegal 01_01_01_0c> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0d(OPS_32)  { arcompact_fatal("<illegal 01_01_01_0d> (%08x)", op); return m_pc + (4 >> 0); }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1e(OPS_32)  { arcompact_fatal("<illegal 0x04_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1f(OPS_32)  { arcompact_fatal("<illegal 0x04_1f> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_24(OPS_32)  { arcompact_fatal("<illegal 0x04_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_25(OPS_32)  { arcompact_fatal("<illegal 0x04_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_26(OPS_32)  { arcompact_fatal("<illegal 0x04_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_27(OPS_32)  { arcompact_fatal("<illegal 0x04_27> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2c(OPS_32)  { arcompact_fatal("<illegal 0x04_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2d(OPS_32)  { arcompact_fatal("<illegal 0x04_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2e(OPS_32)  { arcompact_fatal("<illegal 0x04_2e> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_10(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_11(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_12(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_13(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_14(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_15(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_16(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_17(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_18(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_19(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_20(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_21(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_22(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_23(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_24(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_25(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_26(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_27(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_27> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_28(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_28> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_29(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_29> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_30(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_31(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_32(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_33(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_34(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_35(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_36(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_37(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_38(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_39(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3e> (%08x)", op); return m_pc + (4 >> 0);}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_09(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_09> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_10(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_11(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_12(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_13(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_14(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_15(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_16(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_17(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_18(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_19(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_20(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_21(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_22(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_23(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_24(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_25(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_26(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_27(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_27> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_28(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_28> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_29(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_29> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_30(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_31(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_32(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_33(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_34(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_35(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_36(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_37(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_38(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_39(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3e> (%08x)", op); return m_pc + (4 >> 0);}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_00(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_00> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_06(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_06> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_07(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_07> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_08(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_08> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_09(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_09> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_10(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_11(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_12(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_13(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_14(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_15(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_16(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_17(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_18(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_19(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_20(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_21(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_22(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_23(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_24(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_25(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_26(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_27(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_27> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_28(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_28> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_29(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_29> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_30(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_31(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_32(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_33(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_34(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_35(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_36(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_37(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_38(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_39(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3f> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_00(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_00> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_01(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_01> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_02(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_02> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_03(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_03> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_04(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_04> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_05(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_05> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_06(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_06> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_07(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_07> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_08(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_08> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_09(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_09> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_10(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_11(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_12(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_13(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_14(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_15(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_16(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_17(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_18(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_19(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_20(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_21(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_22(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_23(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_24(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_25(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_26(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_27(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_27> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_28(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_28> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_29(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_29> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_30(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_31(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_32(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_33(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_34(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_35(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_36(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_37(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_38(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_39(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3f> (%08x)", op); return m_pc + (4 >> 0);}




ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_38(OPS_32)  { arcompact_fatal("<illegal 0x04_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_39(OPS_32)  { arcompact_fatal("<illegal 0x04_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3a(OPS_32)  { arcompact_fatal("<illegal 0x04_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3b(OPS_32)  { arcompact_fatal("<illegal 0x04_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3c(OPS_32)  { arcompact_fatal("<illegal 0x04_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3d(OPS_32)  { arcompact_fatal("<illegal 0x04_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3e(OPS_32)  { arcompact_fatal("<illegal 0x04_3e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3f(OPS_32)  { arcompact_fatal("<illegal 0x04_3f> (%08x)", op); return m_pc + (4 >> 0);}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_09(OPS_32)  { arcompact_fatal("<illegal 0x05_09> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0c(OPS_32)  { arcompact_fatal("<illegal 0x05_0c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0d(OPS_32)  { arcompact_fatal("<illegal 0x05_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0e(OPS_32)  { arcompact_fatal("<illegal 0x05_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0f(OPS_32)  { arcompact_fatal("<illegal 0x05_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_10(OPS_32)  { arcompact_fatal("<illegal 0x05_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_11(OPS_32)  { arcompact_fatal("<illegal 0x05_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_12(OPS_32)  { arcompact_fatal("<illegal 0x05_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_13(OPS_32)  { arcompact_fatal("<illegal 0x05_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_14(OPS_32)  { arcompact_fatal("<illegal 0x05_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_15(OPS_32)  { arcompact_fatal("<illegal 0x05_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_16(OPS_32)  { arcompact_fatal("<illegal 0x05_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_17(OPS_32)  { arcompact_fatal("<illegal 0x05_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_18(OPS_32)  { arcompact_fatal("<illegal 0x05_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_19(OPS_32)  { arcompact_fatal("<illegal 0x05_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1a(OPS_32)  { arcompact_fatal("<illegal 0x05_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1b(OPS_32)  { arcompact_fatal("<illegal 0x05_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1c(OPS_32)  { arcompact_fatal("<illegal 0x05_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1d(OPS_32)  { arcompact_fatal("<illegal 0x05_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1e(OPS_32)  { arcompact_fatal("<illegal 0x05_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1f(OPS_32)  { arcompact_fatal("<illegal 0x05_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_20(OPS_32)  { arcompact_fatal("<illegal 0x05_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_21(OPS_32)  { arcompact_fatal("<illegal 0x05_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_22(OPS_32)  { arcompact_fatal("<illegal 0x05_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_23(OPS_32)  { arcompact_fatal("<illegal 0x05_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_24(OPS_32)  { arcompact_fatal("<illegal 0x05_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_25(OPS_32)  { arcompact_fatal("<illegal 0x05_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_26(OPS_32)  { arcompact_fatal("<illegal 0x05_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_27(OPS_32)  { arcompact_fatal("<illegal 0x05_27> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2a(OPS_32)  { arcompact_fatal("<illegal 0x05_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2b(OPS_32)  { arcompact_fatal("<illegal 0x05_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2c(OPS_32)  { arcompact_fatal("<illegal 0x05_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2d(OPS_32)  { arcompact_fatal("<illegal 0x05_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2e(OPS_32)  { arcompact_fatal("<illegal 0x05_2e> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_30(OPS_32)  { arcompact_fatal("<illegal 0x05_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_31(OPS_32)  { arcompact_fatal("<illegal 0x05_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_32(OPS_32)  { arcompact_fatal("<illegal 0x05_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_33(OPS_32)  { arcompact_fatal("<illegal 0x05_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_34(OPS_32)  { arcompact_fatal("<illegal 0x05_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_35(OPS_32)  { arcompact_fatal("<illegal 0x05_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_36(OPS_32)  { arcompact_fatal("<illegal 0x05_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_37(OPS_32)  { arcompact_fatal("<illegal 0x05_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_38(OPS_32)  { arcompact_fatal("<illegal 0x05_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_39(OPS_32)  { arcompact_fatal("<illegal 0x05_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3a(OPS_32)  { arcompact_fatal("<illegal 0x05_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3b(OPS_32)  { arcompact_fatal("<illegal 0x05_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3c(OPS_32)  { arcompact_fatal("<illegal 0x05_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3d(OPS_32)  { arcompact_fatal("<illegal 0x05_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3e(OPS_32)  { arcompact_fatal("<illegal 0x05_3e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3f(OPS_32)  { arcompact_fatal("<illegal 0x05_3f> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_04(OPS_16)  { arcompact_fatal("<illegal 0x0f_00_00> (%08x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_05(OPS_16)  { arcompact_fatal("<illegal 0x0f_00_00> (%08x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_02(OPS_16)  { arcompact_fatal("<illegal 0x0f_00_07_02> (%08x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_03(OPS_16)  { arcompact_fatal("<illegal 0x0f_00_07_03> (%08x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_01(OPS_16)  { arcompact_fatal("<illegal 0x0f_01> (%08x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_03(OPS_16)  { arcompact_fatal("<illegal 0x0f_03> (%08x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_08(OPS_16)  { arcompact_fatal("<illegal 0x0f_08> (%08x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_09(OPS_16)  { arcompact_fatal("<illegal 0x0f_09> (%08x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0a(OPS_16)  { arcompact_fatal("<illegal 0x0f_0a> (%08x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_17(OPS_16)  { arcompact_fatal("<illegal 0x0f_17> (%08x)", op); return m_pc + (2 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_02(OPS_16)  { arcompact_fatal("<illegal 0x18_05_02> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_03(OPS_16)  { arcompact_fatal("<illegal 0x18_05_03> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_04(OPS_16)  { arcompact_fatal("<illegal 0x18_05_04> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_05(OPS_16)  { arcompact_fatal("<illegal 0x18_05_05> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_06(OPS_16)  { arcompact_fatal("<illegal 0x18_05_06> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_07(OPS_16)  { arcompact_fatal("<illegal 0x18_05_07> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_00(OPS_16)  { arcompact_fatal("<illegal 0x18_06_00> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_02(OPS_16)  { arcompact_fatal("<illegal 0x18_06_02> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_03(OPS_16)  { arcompact_fatal("<illegal 0x18_06_03> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_04(OPS_16)  { arcompact_fatal("<illegal 0x18_06_04> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_05(OPS_16)  { arcompact_fatal("<illegal 0x18_06_05> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_06(OPS_16)  { arcompact_fatal("<illegal 0x18_06_06> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_07(OPS_16)  { arcompact_fatal("<illegal 0x18_06_07> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_08(OPS_16)  { arcompact_fatal("<illegal 0x18_06_08> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_09(OPS_16)  { arcompact_fatal("<illegal 0x18_06_09> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0a(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0a> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0b(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0b> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0c(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0c> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0d(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0d> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0e(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0e> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0f(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0f> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_10(OPS_16)  { arcompact_fatal("<illegal 0x18_06_10> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_12(OPS_16)  { arcompact_fatal("<illegal 0x18_06_12> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_13(OPS_16)  { arcompact_fatal("<illegal 0x18_06_13> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_14(OPS_16)  { arcompact_fatal("<illegal 0x18_06_14> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_15(OPS_16)  { arcompact_fatal("<illegal 0x18_06_15> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_16(OPS_16)  { arcompact_fatal("<illegal 0x18_06_16> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_17(OPS_16)  { arcompact_fatal("<illegal 0x18_06_17> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_18(OPS_16)  { arcompact_fatal("<illegal 0x18_06_18> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_19(OPS_16)  { arcompact_fatal("<illegal 0x18_06_19> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1a(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1a> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1b(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1b> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1c(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1c> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1d(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1d> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1e(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1e> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1f(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1f> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_00(OPS_16)  { arcompact_fatal("<illegal 0x18_07_00> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_02(OPS_16)  { arcompact_fatal("<illegal 0x18_07_02> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_03(OPS_16)  { arcompact_fatal("<illegal 0x18_07_03> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_04(OPS_16)  { arcompact_fatal("<illegal 0x18_07_04> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_05(OPS_16)  { arcompact_fatal("<illegal 0x18_07_05> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_06(OPS_16)  { arcompact_fatal("<illegal 0x18_07_06> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_07(OPS_16)  { arcompact_fatal("<illegal 0x18_07_07> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_08(OPS_16)  { arcompact_fatal("<illegal 0x18_07_08> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_09(OPS_16)  { arcompact_fatal("<illegal 0x18_07_09> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0a(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0a> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0b(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0b> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0c(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0c> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0d(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0d> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0e(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0e> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0f(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0f> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_10(OPS_16)  { arcompact_fatal("<illegal 0x18_07_10> (%04x)", op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_12(OPS_16)  { arcompact_fatal("<illegal 0x18_07_12> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_13(OPS_16)  { arcompact_fatal("<illegal 0x18_07_13> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_14(OPS_16)  { arcompact_fatal("<illegal 0x18_07_14> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_15(OPS_16)  { arcompact_fatal("<illegal 0x18_07_15> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_16(OPS_16)  { arcompact_fatal("<illegal 0x18_07_16> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_17(OPS_16)  { arcompact_fatal("<illegal 0x18_07_17> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_18(OPS_16)  { arcompact_fatal("<illegal 0x18_07_18> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_19(OPS_16)  { arcompact_fatal("<illegal 0x18_07_19> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1a(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1a> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1b(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1b> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1c(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1c> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1d(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1d> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1e(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1e> (%04x)",  op); return m_pc + (2 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1f(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1f> (%04x)",  op); return m_pc + (2 >> 0);}
