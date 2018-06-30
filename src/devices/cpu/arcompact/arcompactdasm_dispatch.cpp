// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"
#include "arcompactdasm.h"

int arcompact_disassembler::handle00_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t subinstr = (op & 0x00010000) >> 16;
	op &= ~0x00010000;

	switch (subinstr)
	{
		case 0x00:size = handle00_00_dasm(stream, pc, op, opcodes); break; // Branch Conditionally
		case 0x01:size = handle00_01_dasm(stream, pc, op, opcodes); break; // Branch Unconditionally Far
	}

	return size;
}

int arcompact_disassembler::handle01_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t subinstr = (op & 0x00010000) >> 16;
	op &= ~0x00010000;

	switch (subinstr)
	{
		case 0x00:size = handle01_00_dasm(stream, pc, op, opcodes); break; // Branh & Link
		case 0x01:size = handle01_01_dasm(stream, pc, op, opcodes); break; // Branch on Compare
	}

	return size;
}

int arcompact_disassembler::handle01_00_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t subinstr2 = (op & 0x00020000) >> 17;
	op &= ~0x00020000;

	switch (subinstr2)
	{
		case 0x00:size = handle01_00_00dasm(stream, pc, op, opcodes); break; // Branch and Link Conditionally
		case 0x01:size = handle01_00_01dasm(stream, pc, op, opcodes); break; // Branch and Link Unconditional Far
	}

	return size;
}

int arcompact_disassembler::handle01_01_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;

	uint8_t subinstr2 = (op & 0x00000010) >> 4;
	op &= ~0x00000010;

	switch (subinstr2)
	{
		case 0x00:size = handle01_01_00_dasm(stream, pc, op, opcodes); break; // Branch on Compare Register-Register
		case 0x01:size = handle01_01_01_dasm(stream, pc, op, opcodes); break; // Branch on Compare/Bit Test Register-Immediate
	}

	return size;
}

int arcompact_disassembler::handle01_01_00_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t subinstr3 = (op & 0x0000000f) >> 0;
	op &= ~0x0000000f;

	switch (subinstr3)
	{
		case 0x00:size = handle01_01_00_00_dasm(stream, pc, op, opcodes); break; // BREQ (reg-reg)
		case 0x01:size = handle01_01_00_01_dasm(stream, pc, op, opcodes); break; // BRNE (reg-reg)
		case 0x02:size = handle01_01_00_02_dasm(stream, pc, op, opcodes); break; // BRLT (reg-reg)
		case 0x03:size = handle01_01_00_03_dasm(stream, pc, op, opcodes); break; // BRGE (reg-reg)
		case 0x04:size = handle01_01_00_04_dasm(stream, pc, op, opcodes); break; // BRLO (reg-reg)
		case 0x05:size = handle01_01_00_05_dasm(stream, pc, op, opcodes); break; // BRHS (reg-reg)
		case 0x06:size = handle01_01_00_06_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x07:size = handle01_01_00_07_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x08:size = handle01_01_00_08_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x09:size = handle01_01_00_09_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0a:size = handle01_01_00_0a_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0b:size = handle01_01_00_0b_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0c:size = handle01_01_00_0c_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0d:size = handle01_01_00_0d_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0e:size = handle01_01_00_0e_dasm(stream, pc, op, opcodes); break; // BBIT0 (reg-reg)
		case 0x0f:size = handle01_01_00_0f_dasm(stream, pc, op, opcodes); break; // BBIT1 (reg-reg)
	}

	return size;
}

int arcompact_disassembler::handle01_01_01_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes) //  Branch on Compare/Bit Test Register-Immediate
{
	int size = 4;
	uint8_t subinstr3 = (op & 0x0000000f) >> 0;
	op &= ~0x0000000f;

	switch (subinstr3)
	{
		case 0x00:size = handle01_01_01_00_dasm(stream, pc, op, opcodes); break; // BREQ (reg-imm)
		case 0x01:size = handle01_01_01_01_dasm(stream, pc, op, opcodes); break; // BRNE (reg-imm)
		case 0x02:size = handle01_01_01_02_dasm(stream, pc, op, opcodes); break; // BRLT (reg-imm)
		case 0x03:size = handle01_01_01_03_dasm(stream, pc, op, opcodes); break; // BRGE (reg-imm)
		case 0x04:size = handle01_01_01_04_dasm(stream, pc, op, opcodes); break; // BRLO (reg-imm)
		case 0x05:size = handle01_01_01_05_dasm(stream, pc, op, opcodes); break; // BRHS (reg-imm)
		case 0x06:size = handle01_01_01_06_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x07:size = handle01_01_01_07_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x08:size = handle01_01_01_08_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x09:size = handle01_01_01_09_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0a:size = handle01_01_01_0a_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0b:size = handle01_01_01_0b_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0c:size = handle01_01_01_0c_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0d:size = handle01_01_01_0d_dasm(stream, pc, op, opcodes); break; // reserved
		case 0x0e:size = handle01_01_01_0e_dasm(stream, pc, op, opcodes); break; // BBIT0 (reg-imm)
		case 0x0f:size = handle01_01_01_0f_dasm(stream, pc, op, opcodes); break; // BBIT1 (reg-imm)
	}

	return size;
}

int arcompact_disassembler::handle04_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
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
	uint8_t subinstr = (op & 0x003f0000) >> 16;
	op &= ~0x003f0000;

	switch (subinstr)
	{
		case 0x00:size = handle04_00_dasm(stream, pc, op, opcodes); break; // ADD
		case 0x01:size = handle04_01_dasm(stream, pc, op, opcodes); break; // ADC
		case 0x02:size = handle04_02_dasm(stream, pc, op, opcodes); break; // SUB
		case 0x03:size = handle04_03_dasm(stream, pc, op, opcodes); break; // SBC
		case 0x04:size = handle04_04_dasm(stream, pc, op, opcodes); break; // AND
		case 0x05:size = handle04_05_dasm(stream, pc, op, opcodes); break; // OR
		case 0x06:size = handle04_06_dasm(stream, pc, op, opcodes); break; // BIC
		case 0x07:size = handle04_07_dasm(stream, pc, op, opcodes); break; // XOR
		case 0x08:size = handle04_08_dasm(stream, pc, op, opcodes); break; // MAX
		case 0x09:size = handle04_09_dasm(stream, pc, op, opcodes); break; // MIN
		case 0x0a:size = handle04_0a_dasm(stream, pc, op, opcodes); break; // MOV
		case 0x0b:size = handle04_0b_dasm(stream, pc, op, opcodes); break; // TST
		case 0x0c:size = handle04_0c_dasm(stream, pc, op, opcodes); break; // CMP
		case 0x0d:size = handle04_0d_dasm(stream, pc, op, opcodes); break; // RCMP
		case 0x0e:size = handle04_0e_dasm(stream, pc, op, opcodes); break; // RSUB
		case 0x0f:size = handle04_0f_dasm(stream, pc, op, opcodes); break; // BSET
		case 0x10:size = handle04_10_dasm(stream, pc, op, opcodes); break; // BCLR
		case 0x11:size = handle04_11_dasm(stream, pc, op, opcodes); break; // BTST
		case 0x12:size = handle04_12_dasm(stream, pc, op, opcodes); break; // BXOR
		case 0x13:size = handle04_13_dasm(stream, pc, op, opcodes); break; // BMSK
		case 0x14:size = handle04_14_dasm(stream, pc, op, opcodes); break; // ADD1
		case 0x15:size = handle04_15_dasm(stream, pc, op, opcodes); break; // ADD2
		case 0x16:size = handle04_16_dasm(stream, pc, op, opcodes); break; // ADD3
		case 0x17:size = handle04_17_dasm(stream, pc, op, opcodes); break; // SUB1
		case 0x18:size = handle04_18_dasm(stream, pc, op, opcodes); break; // SUB2
		case 0x19:size = handle04_19_dasm(stream, pc, op, opcodes); break; // SUB3
		case 0x1a:size = handle04_1a_dasm(stream, pc, op, opcodes); break; // MPY *
		case 0x1b:size = handle04_1b_dasm(stream, pc, op, opcodes); break; // MPYH *
		case 0x1c:size = handle04_1c_dasm(stream, pc, op, opcodes); break; // MPYHU *
		case 0x1d:size = handle04_1d_dasm(stream, pc, op, opcodes); break; // MPYU *
		case 0x1e:size = handle04_1e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1f:size = handle04_1f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x20:size = handle04_20_dasm(stream, pc, op, opcodes); break; // Jcc
		case 0x21:size = handle04_21_dasm(stream, pc, op, opcodes); break; // Jcc.D
		case 0x22:size = handle04_22_dasm(stream, pc, op, opcodes); break; // JLcc
		case 0x23:size = handle04_23_dasm(stream, pc, op, opcodes); break; // JLcc.D
		case 0x24:size = handle04_24_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x25:size = handle04_25_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x26:size = handle04_26_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x27:size = handle04_27_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x28:size = handle04_28_dasm(stream, pc, op, opcodes); break; // LPcc
		case 0x29:size = handle04_29_dasm(stream, pc, op, opcodes); break; // FLAG
		case 0x2a:size = handle04_2a_dasm(stream, pc, op, opcodes); break; // LR
		case 0x2b:size = handle04_2b_dasm(stream, pc, op, opcodes); break; // SR
		case 0x2c:size = handle04_2c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2d:size = handle04_2d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2e:size = handle04_2e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2f:size = handle04_2f_dasm(stream, pc, op, opcodes); break; // Sub Opcode
		case 0x30:size = handle04_30_dasm(stream, pc, op, opcodes); break; // LD r-r
		case 0x31:size = handle04_31_dasm(stream, pc, op, opcodes); break; // LD r-r
		case 0x32:size = handle04_32_dasm(stream, pc, op, opcodes); break; // LD r-r
		case 0x33:size = handle04_33_dasm(stream, pc, op, opcodes); break; // LD r-r
		case 0x34:size = handle04_34_dasm(stream, pc, op, opcodes); break; // LD r-r
		case 0x35:size = handle04_35_dasm(stream, pc, op, opcodes); break; // LD r-r
		case 0x36:size = handle04_36_dasm(stream, pc, op, opcodes); break; // LD r-r
		case 0x37:size = handle04_37_dasm(stream, pc, op, opcodes); break; // LD r-r
		case 0x38:size = handle04_38_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x39:size = handle04_39_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3a:size = handle04_3a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3b:size = handle04_3b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3c:size = handle04_3c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3d:size = handle04_3d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3e:size = handle04_3e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3f:size = handle04_3f_dasm(stream, pc, op, opcodes); break; // illegal
	}

	return size;
}

int arcompact_disassembler::handle04_2f_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t subinstr2 = (op & 0x0000003f) >> 0;
	op &= ~0x0000003f;

	switch (subinstr2)
	{
		case 0x00:size = handle04_2f_00_dasm(stream, pc, op, opcodes); break; // ASL
		case 0x01:size = handle04_2f_01_dasm(stream, pc, op, opcodes); break; // ASR
		case 0x02:size = handle04_2f_02_dasm(stream, pc, op, opcodes); break; // LSR
		case 0x03:size = handle04_2f_03_dasm(stream, pc, op, opcodes); break; // ROR
		case 0x04:size = handle04_2f_04_dasm(stream, pc, op, opcodes); break; // RCC
		case 0x05:size = handle04_2f_05_dasm(stream, pc, op, opcodes); break; // SEXB
		case 0x06:size = handle04_2f_06_dasm(stream, pc, op, opcodes); break; // SEXW
		case 0x07:size = handle04_2f_07_dasm(stream, pc, op, opcodes); break; // EXTB
		case 0x08:size = handle04_2f_08_dasm(stream, pc, op, opcodes); break; // EXTW
		case 0x09:size = handle04_2f_09_dasm(stream, pc, op, opcodes); break; // ABS
		case 0x0a:size = handle04_2f_0a_dasm(stream, pc, op, opcodes); break; // NOT
		case 0x0b:size = handle04_2f_0b_dasm(stream, pc, op, opcodes); break; // RLC
		case 0x0c:size = handle04_2f_0c_dasm(stream, pc, op, opcodes); break; // EX
		case 0x0d:size = handle04_2f_0d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0e:size = handle04_2f_0e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0f:size = handle04_2f_0f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x10:size = handle04_2f_10_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x11:size = handle04_2f_11_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x12:size = handle04_2f_12_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x13:size = handle04_2f_13_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x14:size = handle04_2f_14_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x15:size = handle04_2f_15_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x16:size = handle04_2f_16_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x17:size = handle04_2f_17_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x18:size = handle04_2f_18_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x19:size = handle04_2f_19_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1a:size = handle04_2f_1a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1b:size = handle04_2f_1b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1c:size = handle04_2f_1c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1d:size = handle04_2f_1d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1e:size = handle04_2f_1e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1f:size = handle04_2f_1f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x20:size = handle04_2f_20_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x21:size = handle04_2f_21_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x22:size = handle04_2f_22_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x23:size = handle04_2f_23_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x24:size = handle04_2f_24_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x25:size = handle04_2f_25_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x26:size = handle04_2f_26_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x27:size = handle04_2f_27_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x28:size = handle04_2f_28_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x29:size = handle04_2f_29_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2a:size = handle04_2f_2a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2b:size = handle04_2f_2b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2c:size = handle04_2f_2c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2d:size = handle04_2f_2d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2e:size = handle04_2f_2e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2f:size = handle04_2f_2f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x30:size = handle04_2f_30_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x31:size = handle04_2f_31_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x32:size = handle04_2f_32_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x33:size = handle04_2f_33_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x34:size = handle04_2f_34_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x35:size = handle04_2f_35_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x36:size = handle04_2f_36_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x37:size = handle04_2f_37_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x38:size = handle04_2f_38_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x39:size = handle04_2f_39_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3a:size = handle04_2f_3a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3b:size = handle04_2f_3b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3c:size = handle04_2f_3c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3d:size = handle04_2f_3d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3e:size = handle04_2f_3e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3f:size = handle04_2f_3f_dasm(stream, pc, op, opcodes); break; // ZOPs (Zero Operand Opcodes)
	}

	return size;
}


int arcompact_disassembler::handle05_2f_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t subinstr2 = (op & 0x0000003f) >> 0;
	op &= ~0x0000003f;

	switch (subinstr2)
	{
		case 0x00:size = handle05_2f_00_dasm(stream, pc, op, opcodes); break; // SWAP
		case 0x01:size = handle05_2f_01_dasm(stream, pc, op, opcodes); break; // NORM
		case 0x02:size = handle05_2f_02_dasm(stream, pc, op, opcodes); break; // SAT16
		case 0x03:size = handle05_2f_03_dasm(stream, pc, op, opcodes); break; // RND16
		case 0x04:size = handle05_2f_04_dasm(stream, pc, op, opcodes); break; // ABSSW
		case 0x05:size = handle05_2f_05_dasm(stream, pc, op, opcodes); break; // ABSS
		case 0x06:size = handle05_2f_06_dasm(stream, pc, op, opcodes); break; // NEGSW
		case 0x07:size = handle05_2f_07_dasm(stream, pc, op, opcodes); break; // NEGS
		case 0x08:size = handle05_2f_08_dasm(stream, pc, op, opcodes); break; // NORMW
		case 0x09:size = handle05_2f_09_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0a:size = handle05_2f_0a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0b:size = handle05_2f_0b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0c:size = handle05_2f_0c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0d:size = handle05_2f_0d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0e:size = handle05_2f_0e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0f:size = handle05_2f_0f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x10:size = handle05_2f_10_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x11:size = handle05_2f_11_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x12:size = handle05_2f_12_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x13:size = handle05_2f_13_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x14:size = handle05_2f_14_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x15:size = handle05_2f_15_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x16:size = handle05_2f_16_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x17:size = handle05_2f_17_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x18:size = handle05_2f_18_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x19:size = handle05_2f_19_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1a:size = handle05_2f_1a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1b:size = handle05_2f_1b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1c:size = handle05_2f_1c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1d:size = handle05_2f_1d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1e:size = handle05_2f_1e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1f:size = handle05_2f_1f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x20:size = handle05_2f_20_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x21:size = handle05_2f_21_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x22:size = handle05_2f_22_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x23:size = handle05_2f_23_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x24:size = handle05_2f_24_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x25:size = handle05_2f_25_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x26:size = handle05_2f_26_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x27:size = handle05_2f_27_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x28:size = handle05_2f_28_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x29:size = handle05_2f_29_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2a:size = handle05_2f_2a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2b:size = handle05_2f_2b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2c:size = handle05_2f_2c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2d:size = handle05_2f_2d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2e:size = handle05_2f_2e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2f:size = handle05_2f_2f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x30:size = handle05_2f_30_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x31:size = handle05_2f_31_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x32:size = handle05_2f_32_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x33:size = handle05_2f_33_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x34:size = handle05_2f_34_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x35:size = handle05_2f_35_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x36:size = handle05_2f_36_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x37:size = handle05_2f_37_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x38:size = handle05_2f_38_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x39:size = handle05_2f_39_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3a:size = handle05_2f_3a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3b:size = handle05_2f_3b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3c:size = handle05_2f_3c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3d:size = handle05_2f_3d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3e:size = handle05_2f_3e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3f:size = handle05_2f_3f_dasm(stream, pc, op, opcodes); break; // ZOPs (Zero Operand Opcodes)
	}

	return size;
}

int arcompact_disassembler::handle04_2f_3f_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t subinstr3 = (op & 0x07000000) >> 24;
	subinstr3 |= ((op & 0x00007000) >> 12) << 3;

	op &= ~0x07007000;

	switch (subinstr3)
	{
		case 0x00:size = handle04_2f_3f_00_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x01:size = handle04_2f_3f_01_dasm(stream, pc, op, opcodes); break; // SLEEP
		case 0x02:size = handle04_2f_3f_02_dasm(stream, pc, op, opcodes); break; // SWI / TRAP9
		case 0x03:size = handle04_2f_3f_03_dasm(stream, pc, op, opcodes); break; // SYNC
		case 0x04:size = handle04_2f_3f_04_dasm(stream, pc, op, opcodes); break; // RTIE
		case 0x05:size = handle04_2f_3f_05_dasm(stream, pc, op, opcodes); break; // BRK
		case 0x06:size = handle04_2f_3f_06_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x07:size = handle04_2f_3f_07_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x08:size = handle04_2f_3f_08_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x09:size = handle04_2f_3f_09_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0a:size = handle04_2f_3f_0a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0b:size = handle04_2f_3f_0b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0c:size = handle04_2f_3f_0c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0d:size = handle04_2f_3f_0d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0e:size = handle04_2f_3f_0e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0f:size = handle04_2f_3f_0f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x10:size = handle04_2f_3f_10_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x11:size = handle04_2f_3f_11_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x12:size = handle04_2f_3f_12_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x13:size = handle04_2f_3f_13_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x14:size = handle04_2f_3f_14_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x15:size = handle04_2f_3f_15_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x16:size = handle04_2f_3f_16_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x17:size = handle04_2f_3f_17_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x18:size = handle04_2f_3f_18_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x19:size = handle04_2f_3f_19_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1a:size = handle04_2f_3f_1a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1b:size = handle04_2f_3f_1b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1c:size = handle04_2f_3f_1c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1d:size = handle04_2f_3f_1d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1e:size = handle04_2f_3f_1e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1f:size = handle04_2f_3f_1f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x20:size = handle04_2f_3f_20_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x21:size = handle04_2f_3f_21_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x22:size = handle04_2f_3f_22_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x23:size = handle04_2f_3f_23_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x24:size = handle04_2f_3f_24_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x25:size = handle04_2f_3f_25_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x26:size = handle04_2f_3f_26_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x27:size = handle04_2f_3f_27_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x28:size = handle04_2f_3f_28_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x29:size = handle04_2f_3f_29_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2a:size = handle04_2f_3f_2a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2b:size = handle04_2f_3f_2b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2c:size = handle04_2f_3f_2c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2d:size = handle04_2f_3f_2d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2e:size = handle04_2f_3f_2e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2f:size = handle04_2f_3f_2f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x30:size = handle04_2f_3f_30_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x31:size = handle04_2f_3f_31_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x32:size = handle04_2f_3f_32_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x33:size = handle04_2f_3f_33_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x34:size = handle04_2f_3f_34_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x35:size = handle04_2f_3f_35_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x36:size = handle04_2f_3f_36_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x37:size = handle04_2f_3f_37_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x38:size = handle04_2f_3f_38_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x39:size = handle04_2f_3f_39_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3a:size = handle04_2f_3f_3a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3b:size = handle04_2f_3f_3b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3c:size = handle04_2f_3f_3c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3d:size = handle04_2f_3f_3d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3e:size = handle04_2f_3f_3e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3f:size = handle04_2f_3f_3f_dasm(stream, pc, op, opcodes); break; // illegal
	}

	return size;
}


int arcompact_disassembler::handle05_2f_3f_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes) // useless ZOP group, no actual opcodes
{
	int size = 4;
	uint8_t subinstr3 = (op & 0x07000000) >> 24;
	subinstr3 |= ((op & 0x00007000) >> 12) << 3;

	op &= ~0x07007000;

	switch (subinstr3)
	{
		case 0x00:size = handle05_2f_3f_00_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x01:size = handle05_2f_3f_01_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x02:size = handle05_2f_3f_02_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x03:size = handle05_2f_3f_03_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x04:size = handle05_2f_3f_04_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x05:size = handle05_2f_3f_05_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x06:size = handle05_2f_3f_06_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x07:size = handle05_2f_3f_07_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x08:size = handle05_2f_3f_08_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x09:size = handle05_2f_3f_09_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0a:size = handle05_2f_3f_0a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0b:size = handle05_2f_3f_0b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0c:size = handle05_2f_3f_0c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0d:size = handle05_2f_3f_0d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0e:size = handle05_2f_3f_0e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0f:size = handle05_2f_3f_0f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x10:size = handle05_2f_3f_10_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x11:size = handle05_2f_3f_11_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x12:size = handle05_2f_3f_12_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x13:size = handle05_2f_3f_13_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x14:size = handle05_2f_3f_14_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x15:size = handle05_2f_3f_15_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x16:size = handle05_2f_3f_16_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x17:size = handle05_2f_3f_17_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x18:size = handle05_2f_3f_18_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x19:size = handle05_2f_3f_19_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1a:size = handle05_2f_3f_1a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1b:size = handle05_2f_3f_1b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1c:size = handle05_2f_3f_1c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1d:size = handle05_2f_3f_1d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1e:size = handle05_2f_3f_1e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1f:size = handle05_2f_3f_1f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x20:size = handle05_2f_3f_20_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x21:size = handle05_2f_3f_21_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x22:size = handle05_2f_3f_22_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x23:size = handle05_2f_3f_23_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x24:size = handle05_2f_3f_24_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x25:size = handle05_2f_3f_25_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x26:size = handle05_2f_3f_26_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x27:size = handle05_2f_3f_27_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x28:size = handle05_2f_3f_28_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x29:size = handle05_2f_3f_29_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2a:size = handle05_2f_3f_2a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2b:size = handle05_2f_3f_2b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2c:size = handle05_2f_3f_2c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2d:size = handle05_2f_3f_2d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2e:size = handle05_2f_3f_2e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2f:size = handle05_2f_3f_2f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x30:size = handle05_2f_3f_30_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x31:size = handle05_2f_3f_31_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x32:size = handle05_2f_3f_32_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x33:size = handle05_2f_3f_33_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x34:size = handle05_2f_3f_34_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x35:size = handle05_2f_3f_35_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x36:size = handle05_2f_3f_36_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x37:size = handle05_2f_3f_37_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x38:size = handle05_2f_3f_38_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x39:size = handle05_2f_3f_39_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3a:size = handle05_2f_3f_3a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3b:size = handle05_2f_3f_3b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3c:size = handle05_2f_3f_3c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3d:size = handle05_2f_3f_3d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3e:size = handle05_2f_3f_3e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3f:size = handle05_2f_3f_3f_dasm(stream, pc, op, opcodes); break; // illegal
	}

	return size;
}


// this is an Extension ALU group, maybe optional on some CPUs?
int arcompact_disassembler::handle05_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	uint8_t subinstr = (op & 0x003f0000) >> 16;
	op &= ~0x003f0000;

	switch (subinstr)
	{
		case 0x00:size = handle05_00_dasm(stream, pc, op, opcodes); break; // ASL
		case 0x01:size = handle05_01_dasm(stream, pc, op, opcodes); break; // LSR
		case 0x02:size = handle05_02_dasm(stream, pc, op, opcodes); break; // ASR
		case 0x03:size = handle05_03_dasm(stream, pc, op, opcodes); break; // ROR
		case 0x04:size = handle05_04_dasm(stream, pc, op, opcodes); break; // MUL64
		case 0x05:size = handle05_05_dasm(stream, pc, op, opcodes); break; // MULU64
		case 0x06:size = handle05_06_dasm(stream, pc, op, opcodes); break; // ADDS
		case 0x07:size = handle05_07_dasm(stream, pc, op, opcodes); break; // SUBS
		case 0x08:size = handle05_08_dasm(stream, pc, op, opcodes); break; // DIVAW
		case 0x09:size = handle05_09_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0a:size = handle05_0a_dasm(stream, pc, op, opcodes); break; // ASLS
		case 0x0b:size = handle05_0b_dasm(stream, pc, op, opcodes); break; // ASRS
		case 0x0c:size = handle05_0c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0d:size = handle05_0d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0e:size = handle05_0e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x0f:size = handle05_0f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x10:size = handle05_10_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x11:size = handle05_11_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x12:size = handle05_12_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x13:size = handle05_13_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x14:size = handle05_14_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x15:size = handle05_15_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x16:size = handle05_16_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x17:size = handle05_17_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x18:size = handle05_18_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x19:size = handle05_19_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1a:size = handle05_1a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1b:size = handle05_1b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1c:size = handle05_1c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1d:size = handle05_1d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1e:size = handle05_1e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x1f:size = handle05_1f_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x20:size = handle05_20_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x21:size = handle05_21_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x22:size = handle05_22_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x23:size = handle05_23_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x24:size = handle05_24_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x25:size = handle05_25_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x26:size = handle05_26_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x27:size = handle05_27_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x28:size = handle05_28_dasm(stream, pc, op, opcodes); break; // ADDSDW
		case 0x29:size = handle05_29_dasm(stream, pc, op, opcodes); break; // SUBSDW
		case 0x2a:size = handle05_2a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2b:size = handle05_2b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2c:size = handle05_2c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2d:size = handle05_2d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2e:size = handle05_2e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x2f:size = handle05_2f_dasm(stream, pc, op, opcodes); break; // SOPs
		case 0x30:size = handle05_30_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x31:size = handle05_31_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x32:size = handle05_32_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x33:size = handle05_33_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x34:size = handle05_34_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x35:size = handle05_35_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x36:size = handle05_36_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x37:size = handle05_37_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x38:size = handle05_38_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x39:size = handle05_39_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3a:size = handle05_3a_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3b:size = handle05_3b_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3c:size = handle05_3c_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3d:size = handle05_3d_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3e:size = handle05_3e_dasm(stream, pc, op, opcodes); break; // illegal
		case 0x3f:size = handle05_3f_dasm(stream, pc, op, opcodes); break; // illegal
	}

	return size;
}

int arcompact_disassembler::handle0c_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr = (op & 0x0018) >> 3;
	op &= ~0x0018;

	switch (subinstr)
	{
		case 0x00:size = handle0c_00_dasm(stream, pc, op, opcodes); break; // LD_S
		case 0x01:size = handle0c_01_dasm(stream, pc, op, opcodes); break; // LDB_S
		case 0x02:size = handle0c_02_dasm(stream, pc, op, opcodes); break; // LDW_S
		case 0x03:size = handle0c_03_dasm(stream, pc, op, opcodes); break; // ADD_S
	}
	return size;
}

int arcompact_disassembler::handle0d_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr = (op & 0x0018) >> 3;
	op &= ~0x0018;

	switch (subinstr)
	{
		case 0x00:size = handle0d_00_dasm(stream, pc, op, opcodes); break; // ADD_S
		case 0x01:size = handle0d_01_dasm(stream, pc, op, opcodes); break; // SUB_S
		case 0x02:size = handle0d_02_dasm(stream, pc, op, opcodes); break; // ASL_S
		case 0x03:size = handle0d_03_dasm(stream, pc, op, opcodes); break; // ASR_S
	}
	return size;
}

int arcompact_disassembler::handle0e_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr = (op & 0x0018) >> 3;
	op &= ~0x0018;

	switch (subinstr)
	{
		case 0x00:size = handle0e_00_dasm(stream, pc, op, opcodes); break; // ADD_S
		case 0x01:size = handle0e_01_dasm(stream, pc, op, opcodes); break; // MOV_S
		case 0x02:size = handle0e_02_dasm(stream, pc, op, opcodes); break; // CMP_S
		case 0x03:size = handle0e_03_dasm(stream, pc, op, opcodes); break; // MOV_S
	}
	return size;
}

int arcompact_disassembler::handle0f_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	// General Register Instructions (16-bit)
	// 0111 1bbb ccci iiii
	uint8_t subinstr = (op & 0x01f) >> 0;
	op &= ~0x001f;

	switch (subinstr)
	{
		case 0x00:size = handle0f_00_dasm(stream, pc, op, opcodes); break; // SOPs
		case 0x01:size = handle0f_01_dasm(stream, pc, op, opcodes); break; // 0x01 <illegal>
		case 0x02:size = handle0f_02_dasm(stream, pc, op, opcodes); break; // SUB_S
		case 0x03:size = handle0f_03_dasm(stream, pc, op, opcodes); break; // 0x03 <illegal>
		case 0x04:size = handle0f_04_dasm(stream, pc, op, opcodes); break; // AND_S
		case 0x05:size = handle0f_05_dasm(stream, pc, op, opcodes); break; // OR_S
		case 0x06:size = handle0f_06_dasm(stream, pc, op, opcodes); break; // BIC_S
		case 0x07:size = handle0f_07_dasm(stream, pc, op, opcodes); break; // XOR_S
		case 0x08:size = handle0f_08_dasm(stream, pc, op, opcodes); break; // 0x08 <illegal>
		case 0x09:size = handle0f_09_dasm(stream, pc, op, opcodes); break; // 0x09 <illegal>
		case 0x0a:size = handle0f_0a_dasm(stream, pc, op, opcodes); break; // 0x0a <illegal>
		case 0x0b:size = handle0f_0b_dasm(stream, pc, op, opcodes); break; // TST_S
		case 0x0c:size = handle0f_0c_dasm(stream, pc, op, opcodes); break; // MUL64_S
		case 0x0d:size = handle0f_0d_dasm(stream, pc, op, opcodes); break; // SEXB_S
		case 0x0e:size = handle0f_0e_dasm(stream, pc, op, opcodes); break; // SEXW_S
		case 0x0f:size = handle0f_0f_dasm(stream, pc, op, opcodes); break; // EXTB_S
		case 0x10:size = handle0f_10_dasm(stream, pc, op, opcodes); break; // EXTW_S
		case 0x11:size = handle0f_11_dasm(stream, pc, op, opcodes); break; // ABS_S
		case 0x12:size = handle0f_12_dasm(stream, pc, op, opcodes); break; // NOT_S
		case 0x13:size = handle0f_13_dasm(stream, pc, op, opcodes); break; // NEG_S
		case 0x14:size = handle0f_14_dasm(stream, pc, op, opcodes); break; // ADD1_S
		case 0x15:size = handle0f_15_dasm(stream, pc, op, opcodes); break; // ADD2_S
		case 0x16:size = handle0f_16_dasm(stream, pc, op, opcodes); break; // ADD3_S
		case 0x17:size = handle0f_17_dasm(stream, pc, op, opcodes); break; // 0x17 <illegal>
		case 0x18:size = handle0f_18_dasm(stream, pc, op, opcodes); break; // ASL_S (multiple)
		case 0x19:size = handle0f_19_dasm(stream, pc, op, opcodes); break; // LSR_S (multiple)
		case 0x1a:size = handle0f_1a_dasm(stream, pc, op, opcodes); break; // ASR_S (multiple)
		case 0x1b:size = handle0f_1b_dasm(stream, pc, op, opcodes); break; // ASL_S (single)
		case 0x1c:size = handle0f_1c_dasm(stream, pc, op, opcodes); break; // LSR_S (single)
		case 0x1d:size = handle0f_1d_dasm(stream, pc, op, opcodes); break; // ASR_S (single)
		case 0x1e:size = handle0f_1e_dasm(stream, pc, op, opcodes); break; // TRAP (not a5?)
		case 0x1f:size = handle0f_1f_dasm(stream, pc, op, opcodes); break; // BRK_S ( 0x7fff only? )

	}
	return size;
}

int arcompact_disassembler::handle0f_00_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr = (op & 0x00e0) >> 5;
	op &= ~0x00e0;

	switch (subinstr)
	{
		case 0x00:size = handle0f_00_00_dasm(stream, pc, op, opcodes); break; // J_S
		case 0x01:size = handle0f_00_01_dasm(stream, pc, op, opcodes); break; // J_S.D
		case 0x02:size = handle0f_00_02_dasm(stream, pc, op, opcodes); break; // JL_S
		case 0x03:size = handle0f_00_03_dasm(stream, pc, op, opcodes); break; // JL_S.D
		case 0x04:size = handle0f_00_04_dasm(stream, pc, op, opcodes); break; // 0x04 <illegal>
		case 0x05:size = handle0f_00_05_dasm(stream, pc, op, opcodes); break; // 0x05 <illegal>
		case 0x06:size = handle0f_00_06_dasm(stream, pc, op, opcodes); break; // SUB_S.NE
		case 0x07:size = handle0f_00_07_dasm(stream, pc, op, opcodes); break; // ZOPs

	}

	return size;
}

int arcompact_disassembler::handle0f_00_07_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	// General Operations w/o Register
	// 01111 iii 111 00000
	uint8_t subinstr3 = (op & 0x0700) >> 8;
	op &= ~0x0700;

	switch (subinstr3)
	{
		case 0x00:size = handle0f_00_07_00_dasm(stream, pc, op, opcodes); break; // NOP_S
		case 0x01:size = handle0f_00_07_01_dasm(stream, pc, op, opcodes); break; // UNIMP_S
		case 0x02:size = handle0f_00_07_02_dasm(stream, pc, op, opcodes); break; // 0x02 <illegal>
		case 0x03:size = handle0f_00_07_03_dasm(stream, pc, op, opcodes); break; // 0x03 <illegal>
		case 0x04:size = handle0f_00_07_04_dasm(stream, pc, op, opcodes); break; // JEQ_S [BLINK]
		case 0x05:size = handle0f_00_07_05_dasm(stream, pc, op, opcodes); break; // JNE_S [BLINK]
		case 0x06:size = handle0f_00_07_06_dasm(stream, pc, op, opcodes); break; // J_S [BLINK]
		case 0x07:size = handle0f_00_07_07_dasm(stream, pc, op, opcodes); break; // J_S.D [BLINK]

	}
	return size;
}

int arcompact_disassembler::handle17_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr = (op & 0x00e0) >> 5;
	op &= ~0x00e0;

	switch (subinstr)
	{
		case 0x00:size = handle17_00_dasm(stream, pc, op, opcodes); break; // ASL_S
		case 0x01:size = handle17_01_dasm(stream, pc, op, opcodes); break; // LSR_S
		case 0x02:size = handle17_02_dasm(stream, pc, op, opcodes); break; // ASR_S
		case 0x03:size = handle17_03_dasm(stream, pc, op, opcodes); break; // SUB_S
		case 0x04:size = handle17_04_dasm(stream, pc, op, opcodes); break; // BSET_S
		case 0x05:size = handle17_05_dasm(stream, pc, op, opcodes); break; // BCLR_S
		case 0x06:size = handle17_06_dasm(stream, pc, op, opcodes); break; // BMSK_S
		case 0x07:size = handle17_07_dasm(stream, pc, op, opcodes); break; // BTST_S
	}

	return size;
}

int arcompact_disassembler::handle18_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	// Stack Pointer Based Instructions (16-bit)
	// 11000 bbb iii uuuuu
	uint8_t subinstr = (op & 0x00e0) >> 5;
	op &= ~0x00e0;

	switch (subinstr)
	{
		case 0x00:size = handle18_00_dasm(stream, pc, op, opcodes); break; // LD_S (SP)
		case 0x01:size = handle18_01_dasm(stream, pc, op, opcodes); break; // LDB_S (SP)
		case 0x02:size = handle18_02_dasm(stream, pc, op, opcodes); break; // ST_S (SP)
		case 0x03:size = handle18_03_dasm(stream, pc, op, opcodes); break; // STB_S (SP)
		case 0x04:size = handle18_04_dasm(stream, pc, op, opcodes); break; // ADD_S (SP)
		case 0x05:size = handle18_05_dasm(stream, pc, op, opcodes); break; // subtable 18_05
		case 0x06:size = handle18_06_dasm(stream, pc, op, opcodes); break; // subtable 18_06
		case 0x07:size = handle18_07_dasm(stream, pc, op, opcodes); break; // subtable 18_07
	}

	return size;
}

int arcompact_disassembler::handle18_05_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr2 = (op & 0x0700) >> 8;
	op &= ~0x0700;

	switch (subinstr2)
	{
		case 0x00:size = handle18_05_00_dasm(stream, pc, op, opcodes); break; // ADD_S (SP)
		case 0x01:size = handle18_05_01_dasm(stream, pc, op, opcodes); break; // SUB_S (SP)
		case 0x02:size = handle18_05_02_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_05_02>
		case 0x03:size = handle18_05_03_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_05_03>
		case 0x04:size = handle18_05_04_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_05_04>
		case 0x05:size = handle18_05_05_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_05_05>
		case 0x06:size = handle18_05_06_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_05_06>
		case 0x07:size = handle18_05_07_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_05_07>
	}

	return size;
}

int arcompact_disassembler::handle18_06_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr2 = (op & 0x001f) >> 0;
	op &= ~0x001f;

	switch (subinstr2)
	{
		case 0x00:size = handle18_06_00_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_00>
		case 0x01:size = handle18_06_01_dasm(stream, pc, op, opcodes); break; // POP_S b
		case 0x02:size = handle18_06_02_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_02>
		case 0x03:size = handle18_06_03_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_03>
		case 0x04:size = handle18_06_04_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_04>
		case 0x05:size = handle18_06_05_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_05>
		case 0x06:size = handle18_06_06_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_06>
		case 0x07:size = handle18_06_07_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_07>
		case 0x08:size = handle18_06_08_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_08>
		case 0x09:size = handle18_06_09_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_09>
		case 0x0a:size = handle18_06_0a_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_0a>
		case 0x0b:size = handle18_06_0b_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_0b>
		case 0x0c:size = handle18_06_0c_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_0c>
		case 0x0d:size = handle18_06_0d_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_0d>
		case 0x0e:size = handle18_06_0e_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_0e>
		case 0x0f:size = handle18_06_0f_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_0f>
		case 0x10:size = handle18_06_10_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_10>
		case 0x11:size = handle18_06_11_dasm(stream, pc, op, opcodes); break; // POP_S blink
		case 0x12:size = handle18_06_12_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_12>
		case 0x13:size = handle18_06_13_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_13>
		case 0x14:size = handle18_06_14_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_14>
		case 0x15:size = handle18_06_15_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_15>
		case 0x16:size = handle18_06_16_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_16>
		case 0x17:size = handle18_06_17_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_17>
		case 0x18:size = handle18_06_18_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_18>
		case 0x19:size = handle18_06_19_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_19>
		case 0x1a:size = handle18_06_1a_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_1a>
		case 0x1b:size = handle18_06_1b_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_1b>
		case 0x1c:size = handle18_06_1c_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_1c>
		case 0x1d:size = handle18_06_1d_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_1d>
		case 0x1e:size = handle18_06_1e_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_1e>
		case 0x1f:size = handle18_06_1f_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_06_1f>
	}

	return size;
}

int arcompact_disassembler::handle18_07_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr2 = (op & 0x001f) >> 0;
	op &= ~0x001f;

	switch (subinstr2)
	{
		case 0x00:size = handle18_07_00_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_00>
		case 0x01:size = handle18_07_01_dasm(stream, pc, op, opcodes); break; // PUSH_S b
		case 0x02:size = handle18_07_02_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_02>
		case 0x03:size = handle18_07_03_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_03>
		case 0x04:size = handle18_07_04_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_04>
		case 0x05:size = handle18_07_05_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_05>
		case 0x06:size = handle18_07_06_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_06>
		case 0x07:size = handle18_07_07_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_07>
		case 0x08:size = handle18_07_08_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_08>
		case 0x09:size = handle18_07_09_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_09>
		case 0x0a:size = handle18_07_0a_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_0a>
		case 0x0b:size = handle18_07_0b_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_0b>
		case 0x0c:size = handle18_07_0c_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_0c>
		case 0x0d:size = handle18_07_0d_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_0d>
		case 0x0e:size = handle18_07_0e_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_0e>
		case 0x0f:size = handle18_07_0f_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_0f>
		case 0x10:size = handle18_07_10_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_10>
		case 0x11:size = handle18_07_11_dasm(stream, pc, op, opcodes); break; // PUSH_S blink
		case 0x12:size = handle18_07_12_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_12>
		case 0x13:size = handle18_07_13_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_13>
		case 0x14:size = handle18_07_14_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_14>
		case 0x15:size = handle18_07_15_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_15>
		case 0x16:size = handle18_07_16_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_16>
		case 0x17:size = handle18_07_17_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_17>
		case 0x18:size = handle18_07_18_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_18>
		case 0x19:size = handle18_07_19_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_19>
		case 0x1a:size = handle18_07_1a_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_1a>
		case 0x1b:size = handle18_07_1b_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_1b>
		case 0x1c:size = handle18_07_1c_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_1c>
		case 0x1d:size = handle18_07_1d_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_1d>
		case 0x1e:size = handle18_07_1e_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_1e>
		case 0x1f:size = handle18_07_1f_dasm(stream, pc, op, opcodes); break; // <illegal 0x18_07_1f>
	}

	return size;
}

int arcompact_disassembler::handle19_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr = (op & 0x0600) >> 9;
	op &= ~0x0600;

	switch (subinstr)
	{
		case 0x00:size = handle19_00_dasm(stream, pc, op, opcodes); break; // LD_S (GP)
		case 0x01:size = handle19_01_dasm(stream, pc, op, opcodes); break; // LDB_S (GP)
		case 0x02:size = handle19_02_dasm(stream, pc, op, opcodes); break; // LDW_S (GP)
		case 0x03:size = handle19_03_dasm(stream, pc, op, opcodes); break; // ADD_S (GP)
	}
	return size;
}

int arcompact_disassembler::handle1c_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr = (op & 0x0080) >> 7;
	op &= ~0x0080;

	switch (subinstr)
	{
		case 0x00:size = handle1c_00_dasm(stream, pc, op, opcodes); break; // ADD_S
		case 0x01:size = handle1c_01_dasm(stream, pc, op, opcodes); break; // CMP_S
	}
	return size;
}

int arcompact_disassembler::handle1d_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr = (op & 0x0080) >> 7;
	op &= ~0x0080;

	switch (subinstr)
	{
		case 0x00:size = handle1d_00_dasm(stream, pc, op, opcodes); break; // BREQ_S
		case 0x01:size = handle1d_01_dasm(stream, pc, op, opcodes); break; // BRNE_S
	}
	return size;
}

int arcompact_disassembler::handle1e_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr = (op & 0x0600) >> 9;
	op &= ~0x0600;

	switch (subinstr)
	{
		case 0x00:size = handle1e_00_dasm(stream, pc, op, opcodes); break; // B_S
		case 0x01:size = handle1e_01_dasm(stream, pc, op, opcodes); break; // BEQ_S
		case 0x02:size = handle1e_02_dasm(stream, pc, op, opcodes); break; // BNE_S
		case 0x03:size = handle1e_03_dasm(stream, pc, op, opcodes); break; // Bcc_S
	}
	return size;
}

int arcompact_disassembler::handle1e_03_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int size = 2;
	uint8_t subinstr2 = (op & 0x01c0) >> 6;
	op &= ~0x01c0;

	switch (subinstr2)
	{
		case 0x00:size = handle1e_03_00_dasm(stream, pc, op, opcodes); break; // BGT_S
		case 0x01:size = handle1e_03_01_dasm(stream, pc, op, opcodes); break; // BGE_S
		case 0x02:size = handle1e_03_02_dasm(stream, pc, op, opcodes); break; // BLT_S
		case 0x03:size = handle1e_03_03_dasm(stream, pc, op, opcodes); break; // BLE_S
		case 0x04:size = handle1e_03_04_dasm(stream, pc, op, opcodes); break; // BHI_S
		case 0x05:size = handle1e_03_05_dasm(stream, pc, op, opcodes); break; // BHS_S
		case 0x06:size = handle1e_03_06_dasm(stream, pc, op, opcodes); break; // BLO_S
		case 0x07:size = handle1e_03_07_dasm(stream, pc, op, opcodes); break; // BLS_S
	}
	return size;

}
