// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

Toshiba TLCS-900/H disassembly

*******************************************************************/

#ifndef MAME_CPU_TLCS900_DASM900_H
#define MAME_CPU_TLCS900_DASM900_H

#pragma once

class tlcs900_disassembler : public util::disasm_interface
{
protected:
	tlcs900_disassembler(uint16_t num_sfr, const char *const sfr_names[]);
	virtual void decode_control_register_8(std::ostream &stream, uint8_t imm);
	virtual void decode_control_register_16(std::ostream &stream, uint8_t imm);
	virtual void decode_control_register_32(std::ostream &stream, uint8_t imm);

public:
	virtual ~tlcs900_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics
	{
		M_ADC, M_ADD, M_AND, M_ANDCF, M_BIT, M_BS1B,
		M_BS1F, M_CALL, M_CALR, M_CCF, M_CHG, M_CP,
		M_CPD, M_CPDW, M_CPDR, M_CPDRW, M_CPI, M_CPIR,
		M_CPIRW, M_CPIW, M_CPL, M_DAA, M_DB, M_DEC,
		M_DECF, M_DECW, M_DIV, M_DIVS, M_DJNZ, M_EI,
		M_EX, M_EXTS, M_EXTZ, M_HALT, M_INC, M_INCF,
		M_INCW, M_JP, M_JR, M_JRL, M_LD, M_LDA,
		M_LDC, M_LDCF, M_LDD, M_LDDR, M_LDDRW, M_LDDW,
		M_LDF, M_LDI, M_LDIR, M_LDIRW, M_LDIW, M_LDW,
		M_LDX, M_LINK, M_MAX, M_MDEC1, M_MDEC2, M_MDEC4,
		M_MINC1, M_MINC2, M_MINC4, M_MIRR, M_MUL, M_MULA,
		M_MULS, M_NEG, M_NOP, M_NORMAL, M_OR, M_ORCF,
		M_PAA, M_POP, M_POPW, M_PUSH, M_PUSHW, M_RCF,
		M_RES, M_RET, M_RETD, M_RETI, M_RL, M_RLC,
		M_RLCW, M_RLD, M_RLW, M_RR, M_RRC, M_RRCW,
		M_RRD, M_RRW, M_SBC, M_SCC, M_SCF, M_SET,
		M_SLA, M_SLAW, M_SLL, M_SLLW, M_SRA, M_SRAW,
		M_SRL, M_SRLW, M_STCF, M_SUB, M_SWI, M_TSET,
		M_UNLK, M_XOR, M_XORCF, M_ZCF,
		M_80, M_88, M_90, M_98, M_A0, M_A8, M_B0, M_B8,
		M_C0, oC8, M_D0, oD8, M_E0, M_E8, M_F0
	};

	enum e_operand
	{
		O_NONE,
		O_A,        /* current register set register A */
		O_C8,       /* current register set byte */
		O_C16,      /* current register set word */
		O_C32,      /* current register set long word */
		O_MC16,     /* current register set mul/div register word */
		O_CC,       /* condition */
		O_CR8,      /* byte control register */
		O_CR16,     /* word control register */
		O_CR32,     /* long word control register */
		O_D8,       /* byte displacement */
		O_D16,      /* word displacement */
		O_F,            /* F register */
		O_I3,       /* immediate 3 bit (part of last byte) */
		O_I8,       /* immediate byte */
		O_I16,      /* immediate word */
		O_I24,      /* immediate 3 byte address */
		O_I32,      /* immediate long word */
		O_M,            /* memory location (defined by extension) */
		O_M8,       /* (8) */
		O_M16,      /* (i16) */
		O_R,            /* register */
		O_SR        /* status register */
	};

	struct tlcs900inst
	{
		e_mnemonics mnemonic;
		e_operand   operand1;
		e_operand   operand2;
	};

	static const char *const s_mnemonic[];
	static const tlcs900inst mnemonic_80[256];
	static const tlcs900inst mnemonic_88[256];
	static const tlcs900inst mnemonic_90[256];
	static const tlcs900inst mnemonic_98[256];
	static const tlcs900inst mnemonic_a0[256];
	static const tlcs900inst mnemonic_b0[256];
	static const tlcs900inst mnemonic_b8[256];
	static const tlcs900inst mnemonic_c0[256];
	static const tlcs900inst mnemonic_c8[256];
	static const tlcs900inst mnemonic_d0[256];
	static const tlcs900inst mnemonic_d8[256];
	static const tlcs900inst mnemonic_e0[256];
	static const tlcs900inst mnemonic_e8[256];
	static const tlcs900inst mnemonic_f0[256];
	static const tlcs900inst mnemonic[256];

	static const char *const s_reg8[8];
	static const char *const s_reg16[8];
	static const char *const s_reg32[8];
	static const char *const s_mulreg16[8];
	static const char *const s_allreg8[256];
	static const char *const s_allreg16[256];
	static const char *const s_allreg32[256];
	static const char *const s_cond[16];

	const uint16_t m_num_sfr;
	const char *const *m_sfr_names;
};


class tmp94c241_disassembler : public tlcs900_disassembler
{
public:
	tmp94c241_disassembler();
	void decode_control_register_8(std::ostream &stream, uint8_t imm) override;
	void decode_control_register_16(std::ostream &stream, uint8_t imm) override;
	void decode_control_register_32(std::ostream &stream, uint8_t imm) override;

private:
	static const char *const s_sfr_names[];
};


class tmp95c061_disassembler : public tlcs900_disassembler
{
public:
	tmp95c061_disassembler();

private:
	static const char *const s_sfr_names[];
};


class tmp95c063_disassembler : public tlcs900_disassembler
{
public:
	tmp95c063_disassembler();

private:
	static const char *const s_sfr_names[];
};


class tmp96c141_disassembler : public tlcs900_disassembler
{
public:
	tmp96c141_disassembler();

private:
	static const char *const s_sfr_names[];
};

#endif
