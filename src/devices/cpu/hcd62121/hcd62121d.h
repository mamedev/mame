// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#ifndef MAME_CPU_HCD62121_HCD62121D_H
#define MAME_CPU_HCD62121_HCD62121D_H

#pragma once

class hcd62121_disassembler : public util::disasm_interface
{
public:
	hcd62121_disassembler() = default;
	virtual ~hcd62121_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct dasm
	{
		const char *str;
		u8       arg1;
		u8       arg2;
	};

	enum
	{
		ARG_NONE=0,    /* no argument or unknown */
		ARG_REG,       /* register */
		ARG_REGREG,    /* register1, register2, or register2, register1 or register1, imm byte */
		ARG_IRG,       /* register indirect */
		ARG_IRGREG,    /* 2 register indirect */
		ARG_A16,       /* 16bit address */
		ARG_A24,       /* seg:address */
		ARG_F,         /* flag register */
		ARG_CS,        /* cs register */
		ARG_DS,        /* ds register */
		ARG_SS,        /* ss register */
		ARG_PC,        /* program counter */
		ARG_SP,        /* stack pointer */
		ARG_I8,        /* immediate 8 bit value */
		ARG_I16,       /* immediate 16 bit value */
		ARG_I64,       /* immediate 64 bit value */
		ARG_I80,       /* immediate 80 bit value */
		ARG_ILR,       /* indirect last address register access */
		ARG_LAR,       /* last address register */
		ARG_DSZ,       /* dsize register? */
		ARG_OPT,       /* OPTx (output) pins */
		ARG_PORT,      /* PORTx (output) pins */
		ARG_TIME,      /* timing related register */
		ARG_KLO,       /* KO1 - KO8 output lines */
		ARG_KHI,       /* KO9 - KO14(?) output lines */
		ARG_KI,        /* K input lines */
		ARG_S1,        /* shift by 1 */
		ARG_S4,        /* shift by 4 */
		ARG_S8,        /* shift by 8 */
	};

	static const dasm ops[256];
};

#endif
