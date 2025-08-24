// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/************************************************************

  Nintendo Minx CPU disassembly


************************************************************/

#ifndef MAME_CPU_MINX_MINXD_H
#define MAME_CPU_MINX_MINXD_H

#pragma once

class minx_disassembler : public util::disasm_interface
{
public:
	minx_disassembler() = default;
	virtual ~minx_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonic {
		zADD=0, zADDC, zAND, zBCDD, zBCDE, zBCDX, zCALL, zCALLC, zCALLG, zCALLGE, zCALLL,
		zCALLLE, zCALLN, zCALLNC, zCALLNO, zCALLNZ, zCALLO, zCALLP, zCALLNX0,
		zCALLNX1, zCALLNX2, zCALLNX3, zCALLX0, zCALLX1, zCALLX2, zCALLX3, zCALLZ,
		zCMP, zCMPN, zDEC, zDIV, zEXT, zHALT, zINC, zINT,
		zJC, zJDBNZ, zJG, zJGE, zJINT, zJL, zJLE, zJMP,
		zJN, zJNX0, zJNX1, zJNX2, zJNX3, zJNC, zJNO, zJNZ,
		zJO, zJP, zJX0, zJX1, zJX2, zJX3, zJZ, zMOV,
		zMUL, zNEG, zNOP, zNOT, zOR, zPOP, zPOPA, zPOPAX,
		zPOPX, zPUSH, zPUSHA, zPUSHAX, zPUSHX, zRET, zRETI, zRETSKIP,
		zROL, zROLC, zROR, zRORC, zSAL, zSAR, zSHL, zSHR, zSUB,
		zSUBC, zTEST, zXCHG, zXOR, zDB
	};

	enum e_operand {
		R_A=1,      /* A */
		R_B,        /* B */
		R_L,        /* L */
		R_H,        /* H */
		R_N,        /* N */
		R_F,        /* F */
		R_SP,       /* SP */
		R_BA,       /* BA */
		R_HL,       /* HL */
		R_X,        /* X */
		R_Y,        /* Y */
		R_U,        /* U */
		R_V,        /* V */
		R_I,        /* I */
		R_XI,       /* XI */
		R_YI,       /* YI */
		R_PC,       /* PC */
		I_8,        /* 8 bit immediate */
		I_16,       /* 16 bit immediate */
		D_8,        /* PC + 8 bit displacement (signed) */
		D_16,       /* PC + 16 bit displacement */
		S_8,        /* SP + 8 bit displacement (signed) */
		M_IHL,      /* [I+HL] */
		M_N8,       /* [I+N+ofs8] */
		M_I16,      /* [I+ofs16] */
		M_X,        /* [X] */
		M_Y,        /* [Y] */
		M_X8,       /* [X + 8 bit displacement (signed)] */
		M_Y8,       /* [Y + 8 bit displacement (signed)] */
		M_XL,       /* [X + L (signed)] */
		M_YL,       /* [Y + L (signed)] */
		M_16,       /* [16bit] */
		M_HL,       /* [HL] */
		OP, OP1
	};

	struct minxdasm {
		uint8_t   mnemonic;
		uint8_t   argument1;
		uint8_t   argument2;
	};

	static const char *const s_mnemonic[];
	static const uint32_t s_flags[];
	static const minxdasm mnemonic[256];
	static const minxdasm mnemonic_ce[256];
	static const minxdasm mnemonic_cf[256];
};

#endif // MAME_CPU_MINX_MINXD_H
