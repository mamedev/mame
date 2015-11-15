// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"
#include <stdarg.h>

#include "arcompactdasm_dispatch.h"
#include "arcompactdasm_ops.h"

int arcompact_handle00_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT8 subinstr = (op & 0x00010000) >> 16;
	op &= ~0x00010000;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle00_00_dasm(DASM_PARAMS); break; // Branch Conditionally
		case 0x01: size = arcompact_handle00_01_dasm(DASM_PARAMS); break; // Branch Unconditionally Far
	}

	return size;
}

int arcompact_handle01_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT8 subinstr = (op & 0x00010000) >> 16;
	op &= ~0x00010000;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle01_00_dasm(DASM_PARAMS); break; // Branh & Link
		case 0x01: size = arcompact_handle01_01_dasm(DASM_PARAMS); break; // Branch on Compare
	}

	return size;
}

int arcompact_handle01_00_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT8 subinstr2 = (op & 0x00020000) >> 17;
	op &= ~0x00020000;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle01_00_00dasm(DASM_PARAMS); break; // Branch and Link Conditionally
		case 0x01: size = arcompact_handle01_00_01dasm(DASM_PARAMS); break; // Branch and Link Unconditional Far
	}

	return size;
}

int arcompact_handle01_01_dasm(DASM_OPS_32)
{
	int size = 4;

	UINT8 subinstr2 = (op & 0x00000010) >> 4;
	op &= ~0x00000010;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle01_01_00_dasm(DASM_PARAMS); break; // Branch on Compare Register-Register
		case 0x01: size = arcompact_handle01_01_01_dasm(DASM_PARAMS); break; // Branch on Compare/Bit Test Register-Immediate
	}

	return size;
}

int arcompact_handle01_01_00_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT8 subinstr3 = (op & 0x0000000f) >> 0;
	op &= ~0x0000000f;

	switch (subinstr3)
	{
		case 0x00: size = arcompact_handle01_01_00_00_dasm(DASM_PARAMS); break; // BREQ (reg-reg)
		case 0x01: size = arcompact_handle01_01_00_01_dasm(DASM_PARAMS); break; // BRNE (reg-reg)
		case 0x02: size = arcompact_handle01_01_00_02_dasm(DASM_PARAMS); break; // BRLT (reg-reg)
		case 0x03: size = arcompact_handle01_01_00_03_dasm(DASM_PARAMS); break; // BRGE (reg-reg)
		case 0x04: size = arcompact_handle01_01_00_04_dasm(DASM_PARAMS); break; // BRLO (reg-reg)
		case 0x05: size = arcompact_handle01_01_00_05_dasm(DASM_PARAMS); break; // BRHS (reg-reg)
		case 0x06: size = arcompact_handle01_01_00_06_dasm(DASM_PARAMS); break; // reserved
		case 0x07: size = arcompact_handle01_01_00_07_dasm(DASM_PARAMS); break; // reserved
		case 0x08: size = arcompact_handle01_01_00_08_dasm(DASM_PARAMS); break; // reserved
		case 0x09: size = arcompact_handle01_01_00_09_dasm(DASM_PARAMS); break; // reserved
		case 0x0a: size = arcompact_handle01_01_00_0a_dasm(DASM_PARAMS); break; // reserved
		case 0x0b: size = arcompact_handle01_01_00_0b_dasm(DASM_PARAMS); break; // reserved
		case 0x0c: size = arcompact_handle01_01_00_0c_dasm(DASM_PARAMS); break; // reserved
		case 0x0d: size = arcompact_handle01_01_00_0d_dasm(DASM_PARAMS); break; // reserved
		case 0x0e: size = arcompact_handle01_01_00_0e_dasm(DASM_PARAMS); break; // BBIT0 (reg-reg)
		case 0x0f: size = arcompact_handle01_01_00_0f_dasm(DASM_PARAMS); break; // BBIT1 (reg-reg)
	}

	return size;
}

int arcompact_handle01_01_01_dasm(DASM_OPS_32) //  Branch on Compare/Bit Test Register-Immediate
{
	int size = 4;
	UINT8 subinstr3 = (op & 0x0000000f) >> 0;
	op &= ~0x0000000f;

	switch (subinstr3)
	{
		case 0x00: size = arcompact_handle01_01_01_00_dasm(DASM_PARAMS); break; // BREQ (reg-imm)
		case 0x01: size = arcompact_handle01_01_01_01_dasm(DASM_PARAMS); break; // BRNE (reg-imm)
		case 0x02: size = arcompact_handle01_01_01_02_dasm(DASM_PARAMS); break; // BRLT (reg-imm)
		case 0x03: size = arcompact_handle01_01_01_03_dasm(DASM_PARAMS); break; // BRGE (reg-imm)
		case 0x04: size = arcompact_handle01_01_01_04_dasm(DASM_PARAMS); break; // BRLO (reg-imm)
		case 0x05: size = arcompact_handle01_01_01_05_dasm(DASM_PARAMS); break; // BRHS (reg-imm)
		case 0x06: size = arcompact_handle01_01_01_06_dasm(DASM_PARAMS); break; // reserved
		case 0x07: size = arcompact_handle01_01_01_07_dasm(DASM_PARAMS); break; // reserved
		case 0x08: size = arcompact_handle01_01_01_08_dasm(DASM_PARAMS); break; // reserved
		case 0x09: size = arcompact_handle01_01_01_09_dasm(DASM_PARAMS); break; // reserved
		case 0x0a: size = arcompact_handle01_01_01_0a_dasm(DASM_PARAMS); break; // reserved
		case 0x0b: size = arcompact_handle01_01_01_0b_dasm(DASM_PARAMS); break; // reserved
		case 0x0c: size = arcompact_handle01_01_01_0c_dasm(DASM_PARAMS); break; // reserved
		case 0x0d: size = arcompact_handle01_01_01_0d_dasm(DASM_PARAMS); break; // reserved
		case 0x0e: size = arcompact_handle01_01_01_0e_dasm(DASM_PARAMS); break; // BBIT0 (reg-imm)
		case 0x0f: size = arcompact_handle01_01_01_0f_dasm(DASM_PARAMS); break; // BBIT1 (reg-imm)
	}

	return size;
}

int arcompact_handle04_dasm(DASM_OPS_32)
{
	int size = 4;
	// General Operations

	// bitpos
	// 11111 111 11 111111 0 000 000000 0 00000
	// fedcb a98 76 543210 f edc ba9876 5 43210
	//
	// 00100 bbb 00 iiiiii F BBB CCCCCC A AAAAA   General Operations *UN*Conditional Register to Register
	// 00100 bbb 01 iiiiii F BBB UUUUUU A AAAAA   General Operations *UN*Conditional Register (Unsigned 6-bit IMM)
	// 00100 bbb 10 iiiiii F BBB ssssss S SSSSS   General Operations *UN*Conditional Register (Signed 12-bit IMM)

	// 00100 bbb 11 iiiiii F BBB CCCCCC 0 QQQQQ   General Operations Conditional Register
	// 00100 bbb 11 iiiiii F BBB UUUUUU 1 QQQQQ   General Operations Conditional Register (Unsigned 6-bit IMM)
	UINT8 subinstr = (op & 0x003f0000) >> 16;
	op &= ~0x003f0000;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle04_00_dasm(DASM_PARAMS); break; // ADD
		case 0x01: size = arcompact_handle04_01_dasm(DASM_PARAMS); break; // ADC
		case 0x02: size = arcompact_handle04_02_dasm(DASM_PARAMS); break; // SUB
		case 0x03: size = arcompact_handle04_03_dasm(DASM_PARAMS); break; // SBC
		case 0x04: size = arcompact_handle04_04_dasm(DASM_PARAMS); break; // AND
		case 0x05: size = arcompact_handle04_05_dasm(DASM_PARAMS); break; // OR
		case 0x06: size = arcompact_handle04_06_dasm(DASM_PARAMS); break; // BIC
		case 0x07: size = arcompact_handle04_07_dasm(DASM_PARAMS); break; // XOR
		case 0x08: size = arcompact_handle04_08_dasm(DASM_PARAMS); break; // MAX
		case 0x09: size = arcompact_handle04_09_dasm(DASM_PARAMS); break; // MIN
		case 0x0a: size = arcompact_handle04_0a_dasm(DASM_PARAMS); break; // MOV
		case 0x0b: size = arcompact_handle04_0b_dasm(DASM_PARAMS); break; // TST
		case 0x0c: size = arcompact_handle04_0c_dasm(DASM_PARAMS); break; // CMP
		case 0x0d: size = arcompact_handle04_0d_dasm(DASM_PARAMS); break; // RCMP
		case 0x0e: size = arcompact_handle04_0e_dasm(DASM_PARAMS); break; // RSUB
		case 0x0f: size = arcompact_handle04_0f_dasm(DASM_PARAMS); break; // BSET
		case 0x10: size = arcompact_handle04_10_dasm(DASM_PARAMS); break; // BCLR
		case 0x11: size = arcompact_handle04_11_dasm(DASM_PARAMS); break; // BTST
		case 0x12: size = arcompact_handle04_12_dasm(DASM_PARAMS); break; // BXOR
		case 0x13: size = arcompact_handle04_13_dasm(DASM_PARAMS); break; // BMSK
		case 0x14: size = arcompact_handle04_14_dasm(DASM_PARAMS); break; // ADD1
		case 0x15: size = arcompact_handle04_15_dasm(DASM_PARAMS); break; // ADD2
		case 0x16: size = arcompact_handle04_16_dasm(DASM_PARAMS); break; // ADD3
		case 0x17: size = arcompact_handle04_17_dasm(DASM_PARAMS); break; // SUB1
		case 0x18: size = arcompact_handle04_18_dasm(DASM_PARAMS); break; // SUB2
		case 0x19: size = arcompact_handle04_19_dasm(DASM_PARAMS); break; // SUB3
		case 0x1a: size = arcompact_handle04_1a_dasm(DASM_PARAMS); break; // MPY *
		case 0x1b: size = arcompact_handle04_1b_dasm(DASM_PARAMS); break; // MPYH *
		case 0x1c: size = arcompact_handle04_1c_dasm(DASM_PARAMS); break; // MPYHU *
		case 0x1d: size = arcompact_handle04_1d_dasm(DASM_PARAMS); break; // MPYU *
		case 0x1e: size = arcompact_handle04_1e_dasm(DASM_PARAMS); break; // illegal
		case 0x1f: size = arcompact_handle04_1f_dasm(DASM_PARAMS); break; // illegal
		case 0x20: size = arcompact_handle04_20_dasm(DASM_PARAMS); break; // Jcc
		case 0x21: size = arcompact_handle04_21_dasm(DASM_PARAMS); break; // Jcc.D
		case 0x22: size = arcompact_handle04_22_dasm(DASM_PARAMS); break; // JLcc
		case 0x23: size = arcompact_handle04_23_dasm(DASM_PARAMS); break; // JLcc.D
		case 0x24: size = arcompact_handle04_24_dasm(DASM_PARAMS); break; // illegal
		case 0x25: size = arcompact_handle04_25_dasm(DASM_PARAMS); break; // illegal
		case 0x26: size = arcompact_handle04_26_dasm(DASM_PARAMS); break; // illegal
		case 0x27: size = arcompact_handle04_27_dasm(DASM_PARAMS); break; // illegal
		case 0x28: size = arcompact_handle04_28_dasm(DASM_PARAMS); break; // LPcc
		case 0x29: size = arcompact_handle04_29_dasm(DASM_PARAMS); break; // FLAG
		case 0x2a: size = arcompact_handle04_2a_dasm(DASM_PARAMS); break; // LR
		case 0x2b: size = arcompact_handle04_2b_dasm(DASM_PARAMS); break; // SR
		case 0x2c: size = arcompact_handle04_2c_dasm(DASM_PARAMS); break; // illegal
		case 0x2d: size = arcompact_handle04_2d_dasm(DASM_PARAMS); break; // illegal
		case 0x2e: size = arcompact_handle04_2e_dasm(DASM_PARAMS); break; // illegal
		case 0x2f: size = arcompact_handle04_2f_dasm(DASM_PARAMS); break; // Sub Opcode
		case 0x30: size = arcompact_handle04_30_dasm(DASM_PARAMS); break; // LD r-r
		case 0x31: size = arcompact_handle04_31_dasm(DASM_PARAMS); break; // LD r-r
		case 0x32: size = arcompact_handle04_32_dasm(DASM_PARAMS); break; // LD r-r
		case 0x33: size = arcompact_handle04_33_dasm(DASM_PARAMS); break; // LD r-r
		case 0x34: size = arcompact_handle04_34_dasm(DASM_PARAMS); break; // LD r-r
		case 0x35: size = arcompact_handle04_35_dasm(DASM_PARAMS); break; // LD r-r
		case 0x36: size = arcompact_handle04_36_dasm(DASM_PARAMS); break; // LD r-r
		case 0x37: size = arcompact_handle04_37_dasm(DASM_PARAMS); break; // LD r-r
		case 0x38: size = arcompact_handle04_38_dasm(DASM_PARAMS); break; // illegal
		case 0x39: size = arcompact_handle04_39_dasm(DASM_PARAMS); break; // illegal
		case 0x3a: size = arcompact_handle04_3a_dasm(DASM_PARAMS); break; // illegal
		case 0x3b: size = arcompact_handle04_3b_dasm(DASM_PARAMS); break; // illegal
		case 0x3c: size = arcompact_handle04_3c_dasm(DASM_PARAMS); break; // illegal
		case 0x3d: size = arcompact_handle04_3d_dasm(DASM_PARAMS); break; // illegal
		case 0x3e: size = arcompact_handle04_3e_dasm(DASM_PARAMS); break; // illegal
		case 0x3f: size = arcompact_handle04_3f_dasm(DASM_PARAMS); break; // illegal
	}

	return size;
}

int arcompact_handle04_2f_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT8 subinstr2 = (op & 0x0000003f) >> 0;
	op &= ~0x0000003f;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle04_2f_00_dasm(DASM_PARAMS); break; // ASL
		case 0x01: size = arcompact_handle04_2f_01_dasm(DASM_PARAMS); break; // ASR
		case 0x02: size = arcompact_handle04_2f_02_dasm(DASM_PARAMS); break; // LSR
		case 0x03: size = arcompact_handle04_2f_03_dasm(DASM_PARAMS); break; // ROR
		case 0x04: size = arcompact_handle04_2f_04_dasm(DASM_PARAMS); break; // RCC
		case 0x05: size = arcompact_handle04_2f_05_dasm(DASM_PARAMS); break; // SEXB
		case 0x06: size = arcompact_handle04_2f_06_dasm(DASM_PARAMS); break; // SEXW
		case 0x07: size = arcompact_handle04_2f_07_dasm(DASM_PARAMS); break; // EXTB
		case 0x08: size = arcompact_handle04_2f_08_dasm(DASM_PARAMS); break; // EXTW
		case 0x09: size = arcompact_handle04_2f_09_dasm(DASM_PARAMS); break; // ABS
		case 0x0a: size = arcompact_handle04_2f_0a_dasm(DASM_PARAMS); break; // NOT
		case 0x0b: size = arcompact_handle04_2f_0b_dasm(DASM_PARAMS); break; // RLC
		case 0x0c: size = arcompact_handle04_2f_0c_dasm(DASM_PARAMS); break; // EX
		case 0x0d: size = arcompact_handle04_2f_0d_dasm(DASM_PARAMS); break; // illegal
		case 0x0e: size = arcompact_handle04_2f_0e_dasm(DASM_PARAMS); break; // illegal
		case 0x0f: size = arcompact_handle04_2f_0f_dasm(DASM_PARAMS); break; // illegal
		case 0x10: size = arcompact_handle04_2f_10_dasm(DASM_PARAMS); break; // illegal
		case 0x11: size = arcompact_handle04_2f_11_dasm(DASM_PARAMS); break; // illegal
		case 0x12: size = arcompact_handle04_2f_12_dasm(DASM_PARAMS); break; // illegal
		case 0x13: size = arcompact_handle04_2f_13_dasm(DASM_PARAMS); break; // illegal
		case 0x14: size = arcompact_handle04_2f_14_dasm(DASM_PARAMS); break; // illegal
		case 0x15: size = arcompact_handle04_2f_15_dasm(DASM_PARAMS); break; // illegal
		case 0x16: size = arcompact_handle04_2f_16_dasm(DASM_PARAMS); break; // illegal
		case 0x17: size = arcompact_handle04_2f_17_dasm(DASM_PARAMS); break; // illegal
		case 0x18: size = arcompact_handle04_2f_18_dasm(DASM_PARAMS); break; // illegal
		case 0x19: size = arcompact_handle04_2f_19_dasm(DASM_PARAMS); break; // illegal
		case 0x1a: size = arcompact_handle04_2f_1a_dasm(DASM_PARAMS); break; // illegal
		case 0x1b: size = arcompact_handle04_2f_1b_dasm(DASM_PARAMS); break; // illegal
		case 0x1c: size = arcompact_handle04_2f_1c_dasm(DASM_PARAMS); break; // illegal
		case 0x1d: size = arcompact_handle04_2f_1d_dasm(DASM_PARAMS); break; // illegal
		case 0x1e: size = arcompact_handle04_2f_1e_dasm(DASM_PARAMS); break; // illegal
		case 0x1f: size = arcompact_handle04_2f_1f_dasm(DASM_PARAMS); break; // illegal
		case 0x20: size = arcompact_handle04_2f_20_dasm(DASM_PARAMS); break; // illegal
		case 0x21: size = arcompact_handle04_2f_21_dasm(DASM_PARAMS); break; // illegal
		case 0x22: size = arcompact_handle04_2f_22_dasm(DASM_PARAMS); break; // illegal
		case 0x23: size = arcompact_handle04_2f_23_dasm(DASM_PARAMS); break; // illegal
		case 0x24: size = arcompact_handle04_2f_24_dasm(DASM_PARAMS); break; // illegal
		case 0x25: size = arcompact_handle04_2f_25_dasm(DASM_PARAMS); break; // illegal
		case 0x26: size = arcompact_handle04_2f_26_dasm(DASM_PARAMS); break; // illegal
		case 0x27: size = arcompact_handle04_2f_27_dasm(DASM_PARAMS); break; // illegal
		case 0x28: size = arcompact_handle04_2f_28_dasm(DASM_PARAMS); break; // illegal
		case 0x29: size = arcompact_handle04_2f_29_dasm(DASM_PARAMS); break; // illegal
		case 0x2a: size = arcompact_handle04_2f_2a_dasm(DASM_PARAMS); break; // illegal
		case 0x2b: size = arcompact_handle04_2f_2b_dasm(DASM_PARAMS); break; // illegal
		case 0x2c: size = arcompact_handle04_2f_2c_dasm(DASM_PARAMS); break; // illegal
		case 0x2d: size = arcompact_handle04_2f_2d_dasm(DASM_PARAMS); break; // illegal
		case 0x2e: size = arcompact_handle04_2f_2e_dasm(DASM_PARAMS); break; // illegal
		case 0x2f: size = arcompact_handle04_2f_2f_dasm(DASM_PARAMS); break; // illegal
		case 0x30: size = arcompact_handle04_2f_30_dasm(DASM_PARAMS); break; // illegal
		case 0x31: size = arcompact_handle04_2f_31_dasm(DASM_PARAMS); break; // illegal
		case 0x32: size = arcompact_handle04_2f_32_dasm(DASM_PARAMS); break; // illegal
		case 0x33: size = arcompact_handle04_2f_33_dasm(DASM_PARAMS); break; // illegal
		case 0x34: size = arcompact_handle04_2f_34_dasm(DASM_PARAMS); break; // illegal
		case 0x35: size = arcompact_handle04_2f_35_dasm(DASM_PARAMS); break; // illegal
		case 0x36: size = arcompact_handle04_2f_36_dasm(DASM_PARAMS); break; // illegal
		case 0x37: size = arcompact_handle04_2f_37_dasm(DASM_PARAMS); break; // illegal
		case 0x38: size = arcompact_handle04_2f_38_dasm(DASM_PARAMS); break; // illegal
		case 0x39: size = arcompact_handle04_2f_39_dasm(DASM_PARAMS); break; // illegal
		case 0x3a: size = arcompact_handle04_2f_3a_dasm(DASM_PARAMS); break; // illegal
		case 0x3b: size = arcompact_handle04_2f_3b_dasm(DASM_PARAMS); break; // illegal
		case 0x3c: size = arcompact_handle04_2f_3c_dasm(DASM_PARAMS); break; // illegal
		case 0x3d: size = arcompact_handle04_2f_3d_dasm(DASM_PARAMS); break; // illegal
		case 0x3e: size = arcompact_handle04_2f_3e_dasm(DASM_PARAMS); break; // illegal
		case 0x3f: size = arcompact_handle04_2f_3f_dasm(DASM_PARAMS); break; // ZOPs (Zero Operand Opcodes)
	}

	return size;
}


int arcompact_handle05_2f_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT8 subinstr2 = (op & 0x0000003f) >> 0;
	op &= ~0x0000003f;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle05_2f_00_dasm(DASM_PARAMS); break; // SWAP
		case 0x01: size = arcompact_handle05_2f_01_dasm(DASM_PARAMS); break; // NORM
		case 0x02: size = arcompact_handle05_2f_02_dasm(DASM_PARAMS); break; // SAT16
		case 0x03: size = arcompact_handle05_2f_03_dasm(DASM_PARAMS); break; // RND16
		case 0x04: size = arcompact_handle05_2f_04_dasm(DASM_PARAMS); break; // ABSSW
		case 0x05: size = arcompact_handle05_2f_05_dasm(DASM_PARAMS); break; // ABSS
		case 0x06: size = arcompact_handle05_2f_06_dasm(DASM_PARAMS); break; // NEGSW
		case 0x07: size = arcompact_handle05_2f_07_dasm(DASM_PARAMS); break; // NEGS
		case 0x08: size = arcompact_handle05_2f_08_dasm(DASM_PARAMS); break; // NORMW
		case 0x09: size = arcompact_handle05_2f_09_dasm(DASM_PARAMS); break; // illegal
		case 0x0a: size = arcompact_handle05_2f_0a_dasm(DASM_PARAMS); break; // illegal
		case 0x0b: size = arcompact_handle05_2f_0b_dasm(DASM_PARAMS); break; // illegal
		case 0x0c: size = arcompact_handle05_2f_0c_dasm(DASM_PARAMS); break; // illegal
		case 0x0d: size = arcompact_handle05_2f_0d_dasm(DASM_PARAMS); break; // illegal
		case 0x0e: size = arcompact_handle05_2f_0e_dasm(DASM_PARAMS); break; // illegal
		case 0x0f: size = arcompact_handle05_2f_0f_dasm(DASM_PARAMS); break; // illegal
		case 0x10: size = arcompact_handle05_2f_10_dasm(DASM_PARAMS); break; // illegal
		case 0x11: size = arcompact_handle05_2f_11_dasm(DASM_PARAMS); break; // illegal
		case 0x12: size = arcompact_handle05_2f_12_dasm(DASM_PARAMS); break; // illegal
		case 0x13: size = arcompact_handle05_2f_13_dasm(DASM_PARAMS); break; // illegal
		case 0x14: size = arcompact_handle05_2f_14_dasm(DASM_PARAMS); break; // illegal
		case 0x15: size = arcompact_handle05_2f_15_dasm(DASM_PARAMS); break; // illegal
		case 0x16: size = arcompact_handle05_2f_16_dasm(DASM_PARAMS); break; // illegal
		case 0x17: size = arcompact_handle05_2f_17_dasm(DASM_PARAMS); break; // illegal
		case 0x18: size = arcompact_handle05_2f_18_dasm(DASM_PARAMS); break; // illegal
		case 0x19: size = arcompact_handle05_2f_19_dasm(DASM_PARAMS); break; // illegal
		case 0x1a: size = arcompact_handle05_2f_1a_dasm(DASM_PARAMS); break; // illegal
		case 0x1b: size = arcompact_handle05_2f_1b_dasm(DASM_PARAMS); break; // illegal
		case 0x1c: size = arcompact_handle05_2f_1c_dasm(DASM_PARAMS); break; // illegal
		case 0x1d: size = arcompact_handle05_2f_1d_dasm(DASM_PARAMS); break; // illegal
		case 0x1e: size = arcompact_handle05_2f_1e_dasm(DASM_PARAMS); break; // illegal
		case 0x1f: size = arcompact_handle05_2f_1f_dasm(DASM_PARAMS); break; // illegal
		case 0x20: size = arcompact_handle05_2f_20_dasm(DASM_PARAMS); break; // illegal
		case 0x21: size = arcompact_handle05_2f_21_dasm(DASM_PARAMS); break; // illegal
		case 0x22: size = arcompact_handle05_2f_22_dasm(DASM_PARAMS); break; // illegal
		case 0x23: size = arcompact_handle05_2f_23_dasm(DASM_PARAMS); break; // illegal
		case 0x24: size = arcompact_handle05_2f_24_dasm(DASM_PARAMS); break; // illegal
		case 0x25: size = arcompact_handle05_2f_25_dasm(DASM_PARAMS); break; // illegal
		case 0x26: size = arcompact_handle05_2f_26_dasm(DASM_PARAMS); break; // illegal
		case 0x27: size = arcompact_handle05_2f_27_dasm(DASM_PARAMS); break; // illegal
		case 0x28: size = arcompact_handle05_2f_28_dasm(DASM_PARAMS); break; // illegal
		case 0x29: size = arcompact_handle05_2f_29_dasm(DASM_PARAMS); break; // illegal
		case 0x2a: size = arcompact_handle05_2f_2a_dasm(DASM_PARAMS); break; // illegal
		case 0x2b: size = arcompact_handle05_2f_2b_dasm(DASM_PARAMS); break; // illegal
		case 0x2c: size = arcompact_handle05_2f_2c_dasm(DASM_PARAMS); break; // illegal
		case 0x2d: size = arcompact_handle05_2f_2d_dasm(DASM_PARAMS); break; // illegal
		case 0x2e: size = arcompact_handle05_2f_2e_dasm(DASM_PARAMS); break; // illegal
		case 0x2f: size = arcompact_handle05_2f_2f_dasm(DASM_PARAMS); break; // illegal
		case 0x30: size = arcompact_handle05_2f_30_dasm(DASM_PARAMS); break; // illegal
		case 0x31: size = arcompact_handle05_2f_31_dasm(DASM_PARAMS); break; // illegal
		case 0x32: size = arcompact_handle05_2f_32_dasm(DASM_PARAMS); break; // illegal
		case 0x33: size = arcompact_handle05_2f_33_dasm(DASM_PARAMS); break; // illegal
		case 0x34: size = arcompact_handle05_2f_34_dasm(DASM_PARAMS); break; // illegal
		case 0x35: size = arcompact_handle05_2f_35_dasm(DASM_PARAMS); break; // illegal
		case 0x36: size = arcompact_handle05_2f_36_dasm(DASM_PARAMS); break; // illegal
		case 0x37: size = arcompact_handle05_2f_37_dasm(DASM_PARAMS); break; // illegal
		case 0x38: size = arcompact_handle05_2f_38_dasm(DASM_PARAMS); break; // illegal
		case 0x39: size = arcompact_handle05_2f_39_dasm(DASM_PARAMS); break; // illegal
		case 0x3a: size = arcompact_handle05_2f_3a_dasm(DASM_PARAMS); break; // illegal
		case 0x3b: size = arcompact_handle05_2f_3b_dasm(DASM_PARAMS); break; // illegal
		case 0x3c: size = arcompact_handle05_2f_3c_dasm(DASM_PARAMS); break; // illegal
		case 0x3d: size = arcompact_handle05_2f_3d_dasm(DASM_PARAMS); break; // illegal
		case 0x3e: size = arcompact_handle05_2f_3e_dasm(DASM_PARAMS); break; // illegal
		case 0x3f: size = arcompact_handle05_2f_3f_dasm(DASM_PARAMS); break; // ZOPs (Zero Operand Opcodes)
	}

	return size;
}

int arcompact_handle04_2f_3f_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT8 subinstr3 = (op & 0x07000000) >> 24;
	subinstr3 |= ((op & 0x00007000) >> 12) << 3;

	op &= ~0x07007000;

	switch (subinstr3)
	{
		case 0x00: size = arcompact_handle04_2f_3f_00_dasm(DASM_PARAMS); break; // illegal
		case 0x01: size = arcompact_handle04_2f_3f_01_dasm(DASM_PARAMS); break; // SLEEP
		case 0x02: size = arcompact_handle04_2f_3f_02_dasm(DASM_PARAMS); break; // SWI / TRAP9
		case 0x03: size = arcompact_handle04_2f_3f_03_dasm(DASM_PARAMS); break; // SYNC
		case 0x04: size = arcompact_handle04_2f_3f_04_dasm(DASM_PARAMS); break; // RTIE
		case 0x05: size = arcompact_handle04_2f_3f_05_dasm(DASM_PARAMS); break; // BRK
		case 0x06: size = arcompact_handle04_2f_3f_06_dasm(DASM_PARAMS); break; // illegal
		case 0x07: size = arcompact_handle04_2f_3f_07_dasm(DASM_PARAMS); break; // illegal
		case 0x08: size = arcompact_handle04_2f_3f_08_dasm(DASM_PARAMS); break; // illegal
		case 0x09: size = arcompact_handle04_2f_3f_09_dasm(DASM_PARAMS); break; // illegal
		case 0x0a: size = arcompact_handle04_2f_3f_0a_dasm(DASM_PARAMS); break; // illegal
		case 0x0b: size = arcompact_handle04_2f_3f_0b_dasm(DASM_PARAMS); break; // illegal
		case 0x0c: size = arcompact_handle04_2f_3f_0c_dasm(DASM_PARAMS); break; // illegal
		case 0x0d: size = arcompact_handle04_2f_3f_0d_dasm(DASM_PARAMS); break; // illegal
		case 0x0e: size = arcompact_handle04_2f_3f_0e_dasm(DASM_PARAMS); break; // illegal
		case 0x0f: size = arcompact_handle04_2f_3f_0f_dasm(DASM_PARAMS); break; // illegal
		case 0x10: size = arcompact_handle04_2f_3f_10_dasm(DASM_PARAMS); break; // illegal
		case 0x11: size = arcompact_handle04_2f_3f_11_dasm(DASM_PARAMS); break; // illegal
		case 0x12: size = arcompact_handle04_2f_3f_12_dasm(DASM_PARAMS); break; // illegal
		case 0x13: size = arcompact_handle04_2f_3f_13_dasm(DASM_PARAMS); break; // illegal
		case 0x14: size = arcompact_handle04_2f_3f_14_dasm(DASM_PARAMS); break; // illegal
		case 0x15: size = arcompact_handle04_2f_3f_15_dasm(DASM_PARAMS); break; // illegal
		case 0x16: size = arcompact_handle04_2f_3f_16_dasm(DASM_PARAMS); break; // illegal
		case 0x17: size = arcompact_handle04_2f_3f_17_dasm(DASM_PARAMS); break; // illegal
		case 0x18: size = arcompact_handle04_2f_3f_18_dasm(DASM_PARAMS); break; // illegal
		case 0x19: size = arcompact_handle04_2f_3f_19_dasm(DASM_PARAMS); break; // illegal
		case 0x1a: size = arcompact_handle04_2f_3f_1a_dasm(DASM_PARAMS); break; // illegal
		case 0x1b: size = arcompact_handle04_2f_3f_1b_dasm(DASM_PARAMS); break; // illegal
		case 0x1c: size = arcompact_handle04_2f_3f_1c_dasm(DASM_PARAMS); break; // illegal
		case 0x1d: size = arcompact_handle04_2f_3f_1d_dasm(DASM_PARAMS); break; // illegal
		case 0x1e: size = arcompact_handle04_2f_3f_1e_dasm(DASM_PARAMS); break; // illegal
		case 0x1f: size = arcompact_handle04_2f_3f_1f_dasm(DASM_PARAMS); break; // illegal
		case 0x20: size = arcompact_handle04_2f_3f_20_dasm(DASM_PARAMS); break; // illegal
		case 0x21: size = arcompact_handle04_2f_3f_21_dasm(DASM_PARAMS); break; // illegal
		case 0x22: size = arcompact_handle04_2f_3f_22_dasm(DASM_PARAMS); break; // illegal
		case 0x23: size = arcompact_handle04_2f_3f_23_dasm(DASM_PARAMS); break; // illegal
		case 0x24: size = arcompact_handle04_2f_3f_24_dasm(DASM_PARAMS); break; // illegal
		case 0x25: size = arcompact_handle04_2f_3f_25_dasm(DASM_PARAMS); break; // illegal
		case 0x26: size = arcompact_handle04_2f_3f_26_dasm(DASM_PARAMS); break; // illegal
		case 0x27: size = arcompact_handle04_2f_3f_27_dasm(DASM_PARAMS); break; // illegal
		case 0x28: size = arcompact_handle04_2f_3f_28_dasm(DASM_PARAMS); break; // illegal
		case 0x29: size = arcompact_handle04_2f_3f_29_dasm(DASM_PARAMS); break; // illegal
		case 0x2a: size = arcompact_handle04_2f_3f_2a_dasm(DASM_PARAMS); break; // illegal
		case 0x2b: size = arcompact_handle04_2f_3f_2b_dasm(DASM_PARAMS); break; // illegal
		case 0x2c: size = arcompact_handle04_2f_3f_2c_dasm(DASM_PARAMS); break; // illegal
		case 0x2d: size = arcompact_handle04_2f_3f_2d_dasm(DASM_PARAMS); break; // illegal
		case 0x2e: size = arcompact_handle04_2f_3f_2e_dasm(DASM_PARAMS); break; // illegal
		case 0x2f: size = arcompact_handle04_2f_3f_2f_dasm(DASM_PARAMS); break; // illegal
		case 0x30: size = arcompact_handle04_2f_3f_30_dasm(DASM_PARAMS); break; // illegal
		case 0x31: size = arcompact_handle04_2f_3f_31_dasm(DASM_PARAMS); break; // illegal
		case 0x32: size = arcompact_handle04_2f_3f_32_dasm(DASM_PARAMS); break; // illegal
		case 0x33: size = arcompact_handle04_2f_3f_33_dasm(DASM_PARAMS); break; // illegal
		case 0x34: size = arcompact_handle04_2f_3f_34_dasm(DASM_PARAMS); break; // illegal
		case 0x35: size = arcompact_handle04_2f_3f_35_dasm(DASM_PARAMS); break; // illegal
		case 0x36: size = arcompact_handle04_2f_3f_36_dasm(DASM_PARAMS); break; // illegal
		case 0x37: size = arcompact_handle04_2f_3f_37_dasm(DASM_PARAMS); break; // illegal
		case 0x38: size = arcompact_handle04_2f_3f_38_dasm(DASM_PARAMS); break; // illegal
		case 0x39: size = arcompact_handle04_2f_3f_39_dasm(DASM_PARAMS); break; // illegal
		case 0x3a: size = arcompact_handle04_2f_3f_3a_dasm(DASM_PARAMS); break; // illegal
		case 0x3b: size = arcompact_handle04_2f_3f_3b_dasm(DASM_PARAMS); break; // illegal
		case 0x3c: size = arcompact_handle04_2f_3f_3c_dasm(DASM_PARAMS); break; // illegal
		case 0x3d: size = arcompact_handle04_2f_3f_3d_dasm(DASM_PARAMS); break; // illegal
		case 0x3e: size = arcompact_handle04_2f_3f_3e_dasm(DASM_PARAMS); break; // illegal
		case 0x3f: size = arcompact_handle04_2f_3f_3f_dasm(DASM_PARAMS); break; // illegal
	}

	return size;
}


int arcompact_handle05_2f_3f_dasm(DASM_OPS_32) // useless ZOP group, no actual opcodes
{
	int size = 4;
	UINT8 subinstr3 = (op & 0x07000000) >> 24;
	subinstr3 |= ((op & 0x00007000) >> 12) << 3;

	op &= ~0x07007000;

	switch (subinstr3)
	{
		case 0x00: size = arcompact_handle05_2f_3f_00_dasm(DASM_PARAMS); break; // illegal
		case 0x01: size = arcompact_handle05_2f_3f_01_dasm(DASM_PARAMS); break; // illegal
		case 0x02: size = arcompact_handle05_2f_3f_02_dasm(DASM_PARAMS); break; // illegal
		case 0x03: size = arcompact_handle05_2f_3f_03_dasm(DASM_PARAMS); break; // illegal
		case 0x04: size = arcompact_handle05_2f_3f_04_dasm(DASM_PARAMS); break; // illegal
		case 0x05: size = arcompact_handle05_2f_3f_05_dasm(DASM_PARAMS); break; // illegal
		case 0x06: size = arcompact_handle05_2f_3f_06_dasm(DASM_PARAMS); break; // illegal
		case 0x07: size = arcompact_handle05_2f_3f_07_dasm(DASM_PARAMS); break; // illegal
		case 0x08: size = arcompact_handle05_2f_3f_08_dasm(DASM_PARAMS); break; // illegal
		case 0x09: size = arcompact_handle05_2f_3f_09_dasm(DASM_PARAMS); break; // illegal
		case 0x0a: size = arcompact_handle05_2f_3f_0a_dasm(DASM_PARAMS); break; // illegal
		case 0x0b: size = arcompact_handle05_2f_3f_0b_dasm(DASM_PARAMS); break; // illegal
		case 0x0c: size = arcompact_handle05_2f_3f_0c_dasm(DASM_PARAMS); break; // illegal
		case 0x0d: size = arcompact_handle05_2f_3f_0d_dasm(DASM_PARAMS); break; // illegal
		case 0x0e: size = arcompact_handle05_2f_3f_0e_dasm(DASM_PARAMS); break; // illegal
		case 0x0f: size = arcompact_handle05_2f_3f_0f_dasm(DASM_PARAMS); break; // illegal
		case 0x10: size = arcompact_handle05_2f_3f_10_dasm(DASM_PARAMS); break; // illegal
		case 0x11: size = arcompact_handle05_2f_3f_11_dasm(DASM_PARAMS); break; // illegal
		case 0x12: size = arcompact_handle05_2f_3f_12_dasm(DASM_PARAMS); break; // illegal
		case 0x13: size = arcompact_handle05_2f_3f_13_dasm(DASM_PARAMS); break; // illegal
		case 0x14: size = arcompact_handle05_2f_3f_14_dasm(DASM_PARAMS); break; // illegal
		case 0x15: size = arcompact_handle05_2f_3f_15_dasm(DASM_PARAMS); break; // illegal
		case 0x16: size = arcompact_handle05_2f_3f_16_dasm(DASM_PARAMS); break; // illegal
		case 0x17: size = arcompact_handle05_2f_3f_17_dasm(DASM_PARAMS); break; // illegal
		case 0x18: size = arcompact_handle05_2f_3f_18_dasm(DASM_PARAMS); break; // illegal
		case 0x19: size = arcompact_handle05_2f_3f_19_dasm(DASM_PARAMS); break; // illegal
		case 0x1a: size = arcompact_handle05_2f_3f_1a_dasm(DASM_PARAMS); break; // illegal
		case 0x1b: size = arcompact_handle05_2f_3f_1b_dasm(DASM_PARAMS); break; // illegal
		case 0x1c: size = arcompact_handle05_2f_3f_1c_dasm(DASM_PARAMS); break; // illegal
		case 0x1d: size = arcompact_handle05_2f_3f_1d_dasm(DASM_PARAMS); break; // illegal
		case 0x1e: size = arcompact_handle05_2f_3f_1e_dasm(DASM_PARAMS); break; // illegal
		case 0x1f: size = arcompact_handle05_2f_3f_1f_dasm(DASM_PARAMS); break; // illegal
		case 0x20: size = arcompact_handle05_2f_3f_20_dasm(DASM_PARAMS); break; // illegal
		case 0x21: size = arcompact_handle05_2f_3f_21_dasm(DASM_PARAMS); break; // illegal
		case 0x22: size = arcompact_handle05_2f_3f_22_dasm(DASM_PARAMS); break; // illegal
		case 0x23: size = arcompact_handle05_2f_3f_23_dasm(DASM_PARAMS); break; // illegal
		case 0x24: size = arcompact_handle05_2f_3f_24_dasm(DASM_PARAMS); break; // illegal
		case 0x25: size = arcompact_handle05_2f_3f_25_dasm(DASM_PARAMS); break; // illegal
		case 0x26: size = arcompact_handle05_2f_3f_26_dasm(DASM_PARAMS); break; // illegal
		case 0x27: size = arcompact_handle05_2f_3f_27_dasm(DASM_PARAMS); break; // illegal
		case 0x28: size = arcompact_handle05_2f_3f_28_dasm(DASM_PARAMS); break; // illegal
		case 0x29: size = arcompact_handle05_2f_3f_29_dasm(DASM_PARAMS); break; // illegal
		case 0x2a: size = arcompact_handle05_2f_3f_2a_dasm(DASM_PARAMS); break; // illegal
		case 0x2b: size = arcompact_handle05_2f_3f_2b_dasm(DASM_PARAMS); break; // illegal
		case 0x2c: size = arcompact_handle05_2f_3f_2c_dasm(DASM_PARAMS); break; // illegal
		case 0x2d: size = arcompact_handle05_2f_3f_2d_dasm(DASM_PARAMS); break; // illegal
		case 0x2e: size = arcompact_handle05_2f_3f_2e_dasm(DASM_PARAMS); break; // illegal
		case 0x2f: size = arcompact_handle05_2f_3f_2f_dasm(DASM_PARAMS); break; // illegal
		case 0x30: size = arcompact_handle05_2f_3f_30_dasm(DASM_PARAMS); break; // illegal
		case 0x31: size = arcompact_handle05_2f_3f_31_dasm(DASM_PARAMS); break; // illegal
		case 0x32: size = arcompact_handle05_2f_3f_32_dasm(DASM_PARAMS); break; // illegal
		case 0x33: size = arcompact_handle05_2f_3f_33_dasm(DASM_PARAMS); break; // illegal
		case 0x34: size = arcompact_handle05_2f_3f_34_dasm(DASM_PARAMS); break; // illegal
		case 0x35: size = arcompact_handle05_2f_3f_35_dasm(DASM_PARAMS); break; // illegal
		case 0x36: size = arcompact_handle05_2f_3f_36_dasm(DASM_PARAMS); break; // illegal
		case 0x37: size = arcompact_handle05_2f_3f_37_dasm(DASM_PARAMS); break; // illegal
		case 0x38: size = arcompact_handle05_2f_3f_38_dasm(DASM_PARAMS); break; // illegal
		case 0x39: size = arcompact_handle05_2f_3f_39_dasm(DASM_PARAMS); break; // illegal
		case 0x3a: size = arcompact_handle05_2f_3f_3a_dasm(DASM_PARAMS); break; // illegal
		case 0x3b: size = arcompact_handle05_2f_3f_3b_dasm(DASM_PARAMS); break; // illegal
		case 0x3c: size = arcompact_handle05_2f_3f_3c_dasm(DASM_PARAMS); break; // illegal
		case 0x3d: size = arcompact_handle05_2f_3f_3d_dasm(DASM_PARAMS); break; // illegal
		case 0x3e: size = arcompact_handle05_2f_3f_3e_dasm(DASM_PARAMS); break; // illegal
		case 0x3f: size = arcompact_handle05_2f_3f_3f_dasm(DASM_PARAMS); break; // illegal
	}

	return size;
}


// this is an Extension ALU group, maybe optional on some CPUs?
int arcompact_handle05_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT8 subinstr = (op & 0x003f0000) >> 16;
	op &= ~0x003f0000;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle05_00_dasm(DASM_PARAMS); break; // ASL
		case 0x01: size = arcompact_handle05_01_dasm(DASM_PARAMS); break; // LSR
		case 0x02: size = arcompact_handle05_02_dasm(DASM_PARAMS); break; // ASR
		case 0x03: size = arcompact_handle05_03_dasm(DASM_PARAMS); break; // ROR
		case 0x04: size = arcompact_handle05_04_dasm(DASM_PARAMS); break; // MUL64
		case 0x05: size = arcompact_handle05_05_dasm(DASM_PARAMS); break; // MULU64
		case 0x06: size = arcompact_handle05_06_dasm(DASM_PARAMS); break; // ADDS
		case 0x07: size = arcompact_handle05_07_dasm(DASM_PARAMS); break; // SUBS
		case 0x08: size = arcompact_handle05_08_dasm(DASM_PARAMS); break; // DIVAW
		case 0x09: size = arcompact_handle05_09_dasm(DASM_PARAMS); break; // illegal
		case 0x0a: size = arcompact_handle05_0a_dasm(DASM_PARAMS); break; // ASLS
		case 0x0b: size = arcompact_handle05_0b_dasm(DASM_PARAMS); break; // ASRS
		case 0x0c: size = arcompact_handle05_0c_dasm(DASM_PARAMS); break; // illegal
		case 0x0d: size = arcompact_handle05_0d_dasm(DASM_PARAMS); break; // illegal
		case 0x0e: size = arcompact_handle05_0e_dasm(DASM_PARAMS); break; // illegal
		case 0x0f: size = arcompact_handle05_0f_dasm(DASM_PARAMS); break; // illegal
		case 0x10: size = arcompact_handle05_10_dasm(DASM_PARAMS); break; // illegal
		case 0x11: size = arcompact_handle05_11_dasm(DASM_PARAMS); break; // illegal
		case 0x12: size = arcompact_handle05_12_dasm(DASM_PARAMS); break; // illegal
		case 0x13: size = arcompact_handle05_13_dasm(DASM_PARAMS); break; // illegal
		case 0x14: size = arcompact_handle05_14_dasm(DASM_PARAMS); break; // illegal
		case 0x15: size = arcompact_handle05_15_dasm(DASM_PARAMS); break; // illegal
		case 0x16: size = arcompact_handle05_16_dasm(DASM_PARAMS); break; // illegal
		case 0x17: size = arcompact_handle05_17_dasm(DASM_PARAMS); break; // illegal
		case 0x18: size = arcompact_handle05_18_dasm(DASM_PARAMS); break; // illegal
		case 0x19: size = arcompact_handle05_19_dasm(DASM_PARAMS); break; // illegal
		case 0x1a: size = arcompact_handle05_1a_dasm(DASM_PARAMS); break; // illegal
		case 0x1b: size = arcompact_handle05_1b_dasm(DASM_PARAMS); break; // illegal
		case 0x1c: size = arcompact_handle05_1c_dasm(DASM_PARAMS); break; // illegal
		case 0x1d: size = arcompact_handle05_1d_dasm(DASM_PARAMS); break; // illegal
		case 0x1e: size = arcompact_handle05_1e_dasm(DASM_PARAMS); break; // illegal
		case 0x1f: size = arcompact_handle05_1f_dasm(DASM_PARAMS); break; // illegal
		case 0x20: size = arcompact_handle05_20_dasm(DASM_PARAMS); break; // illegal
		case 0x21: size = arcompact_handle05_21_dasm(DASM_PARAMS); break; // illegal
		case 0x22: size = arcompact_handle05_22_dasm(DASM_PARAMS); break; // illegal
		case 0x23: size = arcompact_handle05_23_dasm(DASM_PARAMS); break; // illegal
		case 0x24: size = arcompact_handle05_24_dasm(DASM_PARAMS); break; // illegal
		case 0x25: size = arcompact_handle05_25_dasm(DASM_PARAMS); break; // illegal
		case 0x26: size = arcompact_handle05_26_dasm(DASM_PARAMS); break; // illegal
		case 0x27: size = arcompact_handle05_27_dasm(DASM_PARAMS); break; // illegal
		case 0x28: size = arcompact_handle05_28_dasm(DASM_PARAMS); break; // ADDSDW
		case 0x29: size = arcompact_handle05_29_dasm(DASM_PARAMS); break; // SUBSDW
		case 0x2a: size = arcompact_handle05_2a_dasm(DASM_PARAMS); break; // illegal
		case 0x2b: size = arcompact_handle05_2b_dasm(DASM_PARAMS); break; // illegal
		case 0x2c: size = arcompact_handle05_2c_dasm(DASM_PARAMS); break; // illegal
		case 0x2d: size = arcompact_handle05_2d_dasm(DASM_PARAMS); break; // illegal
		case 0x2e: size = arcompact_handle05_2e_dasm(DASM_PARAMS); break; // illegal
		case 0x2f: size = arcompact_handle05_2f_dasm(DASM_PARAMS); break; // SOPs
		case 0x30: size = arcompact_handle05_30_dasm(DASM_PARAMS); break; // illegal
		case 0x31: size = arcompact_handle05_31_dasm(DASM_PARAMS); break; // illegal
		case 0x32: size = arcompact_handle05_32_dasm(DASM_PARAMS); break; // illegal
		case 0x33: size = arcompact_handle05_33_dasm(DASM_PARAMS); break; // illegal
		case 0x34: size = arcompact_handle05_34_dasm(DASM_PARAMS); break; // illegal
		case 0x35: size = arcompact_handle05_35_dasm(DASM_PARAMS); break; // illegal
		case 0x36: size = arcompact_handle05_36_dasm(DASM_PARAMS); break; // illegal
		case 0x37: size = arcompact_handle05_37_dasm(DASM_PARAMS); break; // illegal
		case 0x38: size = arcompact_handle05_38_dasm(DASM_PARAMS); break; // illegal
		case 0x39: size = arcompact_handle05_39_dasm(DASM_PARAMS); break; // illegal
		case 0x3a: size = arcompact_handle05_3a_dasm(DASM_PARAMS); break; // illegal
		case 0x3b: size = arcompact_handle05_3b_dasm(DASM_PARAMS); break; // illegal
		case 0x3c: size = arcompact_handle05_3c_dasm(DASM_PARAMS); break; // illegal
		case 0x3d: size = arcompact_handle05_3d_dasm(DASM_PARAMS); break; // illegal
		case 0x3e: size = arcompact_handle05_3e_dasm(DASM_PARAMS); break; // illegal
		case 0x3f: size = arcompact_handle05_3f_dasm(DASM_PARAMS); break; // illegal
	}

	return size;
}

int arcompact_handle0c_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr = (op & 0x0018) >> 3;
	op &= ~0x0018;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle0c_00_dasm(DASM_PARAMS); break; // LD_S
		case 0x01: size = arcompact_handle0c_01_dasm(DASM_PARAMS); break; // LDB_S
		case 0x02: size = arcompact_handle0c_02_dasm(DASM_PARAMS); break; // LDW_S
		case 0x03: size = arcompact_handle0c_03_dasm(DASM_PARAMS); break; // ADD_S
	}
	return size;
}

int arcompact_handle0d_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr = (op & 0x0018) >> 3;
	op &= ~0x0018;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle0d_00_dasm(DASM_PARAMS); break; // ADD_S
		case 0x01: size = arcompact_handle0d_01_dasm(DASM_PARAMS); break; // SUB_S
		case 0x02: size = arcompact_handle0d_02_dasm(DASM_PARAMS); break; // ASL_S
		case 0x03: size = arcompact_handle0d_03_dasm(DASM_PARAMS); break; // ASR_S
	}
	return size;
}

int arcompact_handle0e_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr = (op & 0x0018) >> 3;
	op &= ~0x0018;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle0e_00_dasm(DASM_PARAMS); break; // ADD_S
		case 0x01: size = arcompact_handle0e_01_dasm(DASM_PARAMS); break; // MOV_S
		case 0x02: size = arcompact_handle0e_02_dasm(DASM_PARAMS); break; // CMP_S
		case 0x03: size = arcompact_handle0e_03_dasm(DASM_PARAMS); break; // MOV_S
	}
	return size;
}

int arcompact_handle0f_dasm(DASM_OPS_16)
{
	int size = 2;
	// General Register Instructions (16-bit)
	// 0111 1bbb ccci iiii
	UINT8 subinstr = (op & 0x01f) >> 0;
	op &= ~0x001f;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle0f_00_dasm(DASM_PARAMS); break; // SOPs
		case 0x01: size = arcompact_handle0f_01_dasm(DASM_PARAMS); break; // 0x01 <illegal>
		case 0x02: size = arcompact_handle0f_02_dasm(DASM_PARAMS); break; // SUB_S
		case 0x03: size = arcompact_handle0f_03_dasm(DASM_PARAMS); break; // 0x03 <illegal>
		case 0x04: size = arcompact_handle0f_04_dasm(DASM_PARAMS); break; // AND_S
		case 0x05: size = arcompact_handle0f_05_dasm(DASM_PARAMS); break; // OR_S
		case 0x06: size = arcompact_handle0f_06_dasm(DASM_PARAMS); break; // BIC_S
		case 0x07: size = arcompact_handle0f_07_dasm(DASM_PARAMS); break; // XOR_S
		case 0x08: size = arcompact_handle0f_08_dasm(DASM_PARAMS); break; // 0x08 <illegal>
		case 0x09: size = arcompact_handle0f_09_dasm(DASM_PARAMS); break; // 0x09 <illegal>
		case 0x0a: size = arcompact_handle0f_0a_dasm(DASM_PARAMS); break; // 0x0a <illegal>
		case 0x0b: size = arcompact_handle0f_0b_dasm(DASM_PARAMS); break; // TST_S
		case 0x0c: size = arcompact_handle0f_0c_dasm(DASM_PARAMS); break; // MUL64_S
		case 0x0d: size = arcompact_handle0f_0d_dasm(DASM_PARAMS); break; // SEXB_S
		case 0x0e: size = arcompact_handle0f_0e_dasm(DASM_PARAMS); break; // SEXW_S
		case 0x0f: size = arcompact_handle0f_0f_dasm(DASM_PARAMS); break; // EXTB_S
		case 0x10: size = arcompact_handle0f_10_dasm(DASM_PARAMS); break; // EXTW_S
		case 0x11: size = arcompact_handle0f_11_dasm(DASM_PARAMS); break; // ABS_S
		case 0x12: size = arcompact_handle0f_12_dasm(DASM_PARAMS); break; // NOT_S
		case 0x13: size = arcompact_handle0f_13_dasm(DASM_PARAMS); break; // NEG_S
		case 0x14: size = arcompact_handle0f_14_dasm(DASM_PARAMS); break; // ADD1_S
		case 0x15: size = arcompact_handle0f_15_dasm(DASM_PARAMS); break; // ADD2_S
		case 0x16: size = arcompact_handle0f_16_dasm(DASM_PARAMS); break; // ADD3_S
		case 0x17: size = arcompact_handle0f_17_dasm(DASM_PARAMS); break; // 0x17 <illegal>
		case 0x18: size = arcompact_handle0f_18_dasm(DASM_PARAMS); break; // ASL_S (multiple)
		case 0x19: size = arcompact_handle0f_19_dasm(DASM_PARAMS); break; // LSR_S (multiple)
		case 0x1a: size = arcompact_handle0f_1a_dasm(DASM_PARAMS); break; // ASR_S (multiple)
		case 0x1b: size = arcompact_handle0f_1b_dasm(DASM_PARAMS); break; // ASL_S (single)
		case 0x1c: size = arcompact_handle0f_1c_dasm(DASM_PARAMS); break; // LSR_S (single)
		case 0x1d: size = arcompact_handle0f_1d_dasm(DASM_PARAMS); break; // ASR_S (single)
		case 0x1e: size = arcompact_handle0f_1e_dasm(DASM_PARAMS); break; // TRAP (not a5?)
		case 0x1f: size = arcompact_handle0f_1f_dasm(DASM_PARAMS); break; // BRK_S ( 0x7fff only? )

	}
	return size;
}

int arcompact_handle0f_00_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr = (op & 0x00e0) >> 5;
	op &= ~0x00e0;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle0f_00_00_dasm(DASM_PARAMS); break; // J_S
		case 0x01: size = arcompact_handle0f_00_01_dasm(DASM_PARAMS); break; // J_S.D
		case 0x02: size = arcompact_handle0f_00_02_dasm(DASM_PARAMS); break; // JL_S
		case 0x03: size = arcompact_handle0f_00_03_dasm(DASM_PARAMS); break; // JL_S.D
		case 0x04: size = arcompact_handle0f_00_04_dasm(DASM_PARAMS); break; // 0x04 <illegal>
		case 0x05: size = arcompact_handle0f_00_05_dasm(DASM_PARAMS); break; // 0x05 <illegal>
		case 0x06: size = arcompact_handle0f_00_06_dasm(DASM_PARAMS); break; // SUB_S.NE
		case 0x07: size = arcompact_handle0f_00_07_dasm(DASM_PARAMS); break; // ZOPs

	}

	return size;
}

int arcompact_handle0f_00_07_dasm(DASM_OPS_16)
{
	int size = 2;
	// General Operations w/o Register
	// 01111 iii 111 00000
	UINT8 subinstr3 = (op & 0x0700) >> 8;
	op &= ~0x0700;

	switch (subinstr3)
	{
		case 0x00: size = arcompact_handle0f_00_07_00_dasm(DASM_PARAMS); break; // NOP_S
		case 0x01: size = arcompact_handle0f_00_07_01_dasm(DASM_PARAMS); break; // UNIMP_S
		case 0x02: size = arcompact_handle0f_00_07_02_dasm(DASM_PARAMS); break; // 0x02 <illegal>
		case 0x03: size = arcompact_handle0f_00_07_03_dasm(DASM_PARAMS); break; // 0x03 <illegal>
		case 0x04: size = arcompact_handle0f_00_07_04_dasm(DASM_PARAMS); break; // JEQ_S [BLINK]
		case 0x05: size = arcompact_handle0f_00_07_05_dasm(DASM_PARAMS); break; // JNE_S [BLINK]
		case 0x06: size = arcompact_handle0f_00_07_06_dasm(DASM_PARAMS); break; // J_S [BLINK]
		case 0x07: size = arcompact_handle0f_00_07_07_dasm(DASM_PARAMS); break; // J_S.D [BLINK]

	}
	return size;
}

int arcompact_handle17_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr = (op & 0x00e0) >> 5;
	op &= ~0x00e0;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle17_00_dasm(DASM_PARAMS); break; // ASL_S
		case 0x01: size = arcompact_handle17_01_dasm(DASM_PARAMS); break; // LSR_S
		case 0x02: size = arcompact_handle17_02_dasm(DASM_PARAMS); break; // ASR_S
		case 0x03: size = arcompact_handle17_03_dasm(DASM_PARAMS); break; // SUB_S
		case 0x04: size = arcompact_handle17_04_dasm(DASM_PARAMS); break; // BSET_S
		case 0x05: size = arcompact_handle17_05_dasm(DASM_PARAMS); break; // BCLR_S
		case 0x06: size = arcompact_handle17_06_dasm(DASM_PARAMS); break; // BMSK_S
		case 0x07: size = arcompact_handle17_07_dasm(DASM_PARAMS); break; // BTST_S
	}

	return size;
}

int arcompact_handle18_dasm(DASM_OPS_16)
{
	int size = 2;
	// Stack Pointer Based Instructions (16-bit)
	// 11000 bbb iii uuuuu
	UINT8 subinstr = (op & 0x00e0) >> 5;
	op &= ~0x00e0;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle18_00_dasm(DASM_PARAMS); break; // LD_S (SP)
		case 0x01: size = arcompact_handle18_01_dasm(DASM_PARAMS); break; // LDB_S (SP)
		case 0x02: size = arcompact_handle18_02_dasm(DASM_PARAMS); break; // ST_S (SP)
		case 0x03: size = arcompact_handle18_03_dasm(DASM_PARAMS); break; // STB_S (SP)
		case 0x04: size = arcompact_handle18_04_dasm(DASM_PARAMS); break; // ADD_S (SP)
		case 0x05: size = arcompact_handle18_05_dasm(DASM_PARAMS); break; // subtable 18_05
		case 0x06: size = arcompact_handle18_06_dasm(DASM_PARAMS); break; // subtable 18_06
		case 0x07: size = arcompact_handle18_07_dasm(DASM_PARAMS); break; // subtable 18_07
	}

	return size;
}

int arcompact_handle18_05_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr2 = (op & 0x0700) >> 8;
	op &= ~0x0700;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle18_05_00_dasm(DASM_PARAMS); break; // ADD_S (SP)
		case 0x01: size = arcompact_handle18_05_01_dasm(DASM_PARAMS); break; // SUB_S (SP)
		case 0x02: size = arcompact_handle18_05_02_dasm(DASM_PARAMS); break; // <illegal 0x18_05_02>
		case 0x03: size = arcompact_handle18_05_03_dasm(DASM_PARAMS); break; // <illegal 0x18_05_03>
		case 0x04: size = arcompact_handle18_05_04_dasm(DASM_PARAMS); break; // <illegal 0x18_05_04>
		case 0x05: size = arcompact_handle18_05_05_dasm(DASM_PARAMS); break; // <illegal 0x18_05_05>
		case 0x06: size = arcompact_handle18_05_06_dasm(DASM_PARAMS); break; // <illegal 0x18_05_06>
		case 0x07: size = arcompact_handle18_05_07_dasm(DASM_PARAMS); break; // <illegal 0x18_05_07>
	}

	return size;
}

int arcompact_handle18_06_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr2 = (op & 0x001f) >> 0;
	op &= ~0x001f;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle18_06_00_dasm(DASM_PARAMS); break; // <illegal 0x18_06_00>
		case 0x01: size = arcompact_handle18_06_01_dasm(DASM_PARAMS); break; // POP_S b
		case 0x02: size = arcompact_handle18_06_02_dasm(DASM_PARAMS); break; // <illegal 0x18_06_02>
		case 0x03: size = arcompact_handle18_06_03_dasm(DASM_PARAMS); break; // <illegal 0x18_06_03>
		case 0x04: size = arcompact_handle18_06_04_dasm(DASM_PARAMS); break; // <illegal 0x18_06_04>
		case 0x05: size = arcompact_handle18_06_05_dasm(DASM_PARAMS); break; // <illegal 0x18_06_05>
		case 0x06: size = arcompact_handle18_06_06_dasm(DASM_PARAMS); break; // <illegal 0x18_06_06>
		case 0x07: size = arcompact_handle18_06_07_dasm(DASM_PARAMS); break; // <illegal 0x18_06_07>
		case 0x08: size = arcompact_handle18_06_08_dasm(DASM_PARAMS); break; // <illegal 0x18_06_08>
		case 0x09: size = arcompact_handle18_06_09_dasm(DASM_PARAMS); break; // <illegal 0x18_06_09>
		case 0x0a: size = arcompact_handle18_06_0a_dasm(DASM_PARAMS); break; // <illegal 0x18_06_0a>
		case 0x0b: size = arcompact_handle18_06_0b_dasm(DASM_PARAMS); break; // <illegal 0x18_06_0b>
		case 0x0c: size = arcompact_handle18_06_0c_dasm(DASM_PARAMS); break; // <illegal 0x18_06_0c>
		case 0x0d: size = arcompact_handle18_06_0d_dasm(DASM_PARAMS); break; // <illegal 0x18_06_0d>
		case 0x0e: size = arcompact_handle18_06_0e_dasm(DASM_PARAMS); break; // <illegal 0x18_06_0e>
		case 0x0f: size = arcompact_handle18_06_0f_dasm(DASM_PARAMS); break; // <illegal 0x18_06_0f>
		case 0x10: size = arcompact_handle18_06_10_dasm(DASM_PARAMS); break; // <illegal 0x18_06_10>
		case 0x11: size = arcompact_handle18_06_11_dasm(DASM_PARAMS); break; // POP_S blink
		case 0x12: size = arcompact_handle18_06_12_dasm(DASM_PARAMS); break; // <illegal 0x18_06_12>
		case 0x13: size = arcompact_handle18_06_13_dasm(DASM_PARAMS); break; // <illegal 0x18_06_13>
		case 0x14: size = arcompact_handle18_06_14_dasm(DASM_PARAMS); break; // <illegal 0x18_06_14>
		case 0x15: size = arcompact_handle18_06_15_dasm(DASM_PARAMS); break; // <illegal 0x18_06_15>
		case 0x16: size = arcompact_handle18_06_16_dasm(DASM_PARAMS); break; // <illegal 0x18_06_16>
		case 0x17: size = arcompact_handle18_06_17_dasm(DASM_PARAMS); break; // <illegal 0x18_06_17>
		case 0x18: size = arcompact_handle18_06_18_dasm(DASM_PARAMS); break; // <illegal 0x18_06_18>
		case 0x19: size = arcompact_handle18_06_19_dasm(DASM_PARAMS); break; // <illegal 0x18_06_19>
		case 0x1a: size = arcompact_handle18_06_1a_dasm(DASM_PARAMS); break; // <illegal 0x18_06_1a>
		case 0x1b: size = arcompact_handle18_06_1b_dasm(DASM_PARAMS); break; // <illegal 0x18_06_1b>
		case 0x1c: size = arcompact_handle18_06_1c_dasm(DASM_PARAMS); break; // <illegal 0x18_06_1c>
		case 0x1d: size = arcompact_handle18_06_1d_dasm(DASM_PARAMS); break; // <illegal 0x18_06_1d>
		case 0x1e: size = arcompact_handle18_06_1e_dasm(DASM_PARAMS); break; // <illegal 0x18_06_1e>
		case 0x1f: size = arcompact_handle18_06_1f_dasm(DASM_PARAMS); break; // <illegal 0x18_06_1f>
	}

	return size;
}

int arcompact_handle18_07_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr2 = (op & 0x001f) >> 0;
	op &= ~0x001f;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle18_07_00_dasm(DASM_PARAMS); break; // <illegal 0x18_07_00>
		case 0x01: size = arcompact_handle18_07_01_dasm(DASM_PARAMS); break; // PUSH_S b
		case 0x02: size = arcompact_handle18_07_02_dasm(DASM_PARAMS); break; // <illegal 0x18_07_02>
		case 0x03: size = arcompact_handle18_07_03_dasm(DASM_PARAMS); break; // <illegal 0x18_07_03>
		case 0x04: size = arcompact_handle18_07_04_dasm(DASM_PARAMS); break; // <illegal 0x18_07_04>
		case 0x05: size = arcompact_handle18_07_05_dasm(DASM_PARAMS); break; // <illegal 0x18_07_05>
		case 0x06: size = arcompact_handle18_07_06_dasm(DASM_PARAMS); break; // <illegal 0x18_07_06>
		case 0x07: size = arcompact_handle18_07_07_dasm(DASM_PARAMS); break; // <illegal 0x18_07_07>
		case 0x08: size = arcompact_handle18_07_08_dasm(DASM_PARAMS); break; // <illegal 0x18_07_08>
		case 0x09: size = arcompact_handle18_07_09_dasm(DASM_PARAMS); break; // <illegal 0x18_07_09>
		case 0x0a: size = arcompact_handle18_07_0a_dasm(DASM_PARAMS); break; // <illegal 0x18_07_0a>
		case 0x0b: size = arcompact_handle18_07_0b_dasm(DASM_PARAMS); break; // <illegal 0x18_07_0b>
		case 0x0c: size = arcompact_handle18_07_0c_dasm(DASM_PARAMS); break; // <illegal 0x18_07_0c>
		case 0x0d: size = arcompact_handle18_07_0d_dasm(DASM_PARAMS); break; // <illegal 0x18_07_0d>
		case 0x0e: size = arcompact_handle18_07_0e_dasm(DASM_PARAMS); break; // <illegal 0x18_07_0e>
		case 0x0f: size = arcompact_handle18_07_0f_dasm(DASM_PARAMS); break; // <illegal 0x18_07_0f>
		case 0x10: size = arcompact_handle18_07_10_dasm(DASM_PARAMS); break; // <illegal 0x18_07_10>
		case 0x11: size = arcompact_handle18_07_11_dasm(DASM_PARAMS); break; // PUSH_S blink
		case 0x12: size = arcompact_handle18_07_12_dasm(DASM_PARAMS); break; // <illegal 0x18_07_12>
		case 0x13: size = arcompact_handle18_07_13_dasm(DASM_PARAMS); break; // <illegal 0x18_07_13>
		case 0x14: size = arcompact_handle18_07_14_dasm(DASM_PARAMS); break; // <illegal 0x18_07_14>
		case 0x15: size = arcompact_handle18_07_15_dasm(DASM_PARAMS); break; // <illegal 0x18_07_15>
		case 0x16: size = arcompact_handle18_07_16_dasm(DASM_PARAMS); break; // <illegal 0x18_07_16>
		case 0x17: size = arcompact_handle18_07_17_dasm(DASM_PARAMS); break; // <illegal 0x18_07_17>
		case 0x18: size = arcompact_handle18_07_18_dasm(DASM_PARAMS); break; // <illegal 0x18_07_18>
		case 0x19: size = arcompact_handle18_07_19_dasm(DASM_PARAMS); break; // <illegal 0x18_07_19>
		case 0x1a: size = arcompact_handle18_07_1a_dasm(DASM_PARAMS); break; // <illegal 0x18_07_1a>
		case 0x1b: size = arcompact_handle18_07_1b_dasm(DASM_PARAMS); break; // <illegal 0x18_07_1b>
		case 0x1c: size = arcompact_handle18_07_1c_dasm(DASM_PARAMS); break; // <illegal 0x18_07_1c>
		case 0x1d: size = arcompact_handle18_07_1d_dasm(DASM_PARAMS); break; // <illegal 0x18_07_1d>
		case 0x1e: size = arcompact_handle18_07_1e_dasm(DASM_PARAMS); break; // <illegal 0x18_07_1e>
		case 0x1f: size = arcompact_handle18_07_1f_dasm(DASM_PARAMS); break; // <illegal 0x18_07_1f>
	}

	return size;
}

int arcompact_handle19_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr = (op & 0x0600) >> 9;
	op &= ~0x0600;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle19_00_dasm(DASM_PARAMS); break; // LD_S (GP)
		case 0x01: size = arcompact_handle19_01_dasm(DASM_PARAMS); break; // LDB_S (GP)
		case 0x02: size = arcompact_handle19_02_dasm(DASM_PARAMS); break; // LDW_S (GP)
		case 0x03: size = arcompact_handle19_03_dasm(DASM_PARAMS); break; // ADD_S (GP)
	}
	return size;
}

int arcompact_handle1c_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr = (op & 0x0080) >> 7;
	op &= ~0x0080;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle1c_00_dasm(DASM_PARAMS); break; // ADD_S
		case 0x01: size = arcompact_handle1c_01_dasm(DASM_PARAMS); break; // CMP_S
	}
	return size;
}

int arcompact_handle1d_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr = (op & 0x0080) >> 7;
	op &= ~0x0080;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle1d_00_dasm(DASM_PARAMS); break; // BREQ_S
		case 0x01: size = arcompact_handle1d_01_dasm(DASM_PARAMS); break; // BRNE_S
	}
	return size;
}

int arcompact_handle1e_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr = (op & 0x0600) >> 9;
	op &= ~0x0600;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle1e_00_dasm(DASM_PARAMS); break; // B_S
		case 0x01: size = arcompact_handle1e_01_dasm(DASM_PARAMS); break; // BEQ_S
		case 0x02: size = arcompact_handle1e_02_dasm(DASM_PARAMS); break; // BNE_S
		case 0x03: size = arcompact_handle1e_03_dasm(DASM_PARAMS); break; // Bcc_S
	}
	return size;
}

int arcompact_handle1e_03_dasm(DASM_OPS_16)
{
	int size = 2;
	UINT8 subinstr2 = (op & 0x01c0) >> 6;
	op &= ~0x01c0;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle1e_03_00_dasm(DASM_PARAMS); break; // BGT_S
		case 0x01: size = arcompact_handle1e_03_01_dasm(DASM_PARAMS); break; // BGE_S
		case 0x02: size = arcompact_handle1e_03_02_dasm(DASM_PARAMS); break; // BLT_S
		case 0x03: size = arcompact_handle1e_03_03_dasm(DASM_PARAMS); break; // BLE_S
		case 0x04: size = arcompact_handle1e_03_04_dasm(DASM_PARAMS); break; // BHI_S
		case 0x05: size = arcompact_handle1e_03_05_dasm(DASM_PARAMS); break; // BHS_S
		case 0x06: size = arcompact_handle1e_03_06_dasm(DASM_PARAMS); break; // BLO_S
		case 0x07: size = arcompact_handle1e_03_07_dasm(DASM_PARAMS); break; // BLS_S
	}
	return size;

}
