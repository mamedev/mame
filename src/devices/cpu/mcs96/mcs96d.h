// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/***************************************************************************

    mcs96.h

    MCS96

***************************************************************************/

#ifndef MAME_CPU_MCS96_MCS96D_H
#define MAME_CPU_MCS96_MCS96D_H

#pragma once

class mcs96_disassembler : public util::disasm_interface
{
public:
	struct disasm_entry {
		const char *opcode, *opcode_fe;
		int mode;
		offs_t flags;
	};

	mcs96_disassembler(const disasm_entry *entries);
	virtual ~mcs96_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	enum {
		DASM_none,              /* No parameters */
		DASM_nop_2,             /* One ignored parameter byte */
		DASM_rel8,              /* Relative, 8 bits */
		DASM_rel11,             /* Relative, 11 bits */
		DASM_rel16,             /* Relative, 16 bits */
		DASM_rrel8,             /* Register + relative, 8 bits */
		DASM_brrel8,            /* Bit test + register + relative, 8 bits */
		DASM_direct_1,          /* Register-direct references, 1 operator */
		DASM_direct_2,          /* Register-direct references, 2 operators */
		DASM_direct_3,          /* Register-direct references, 3 operators */
		DASM_immed_1b,          /* Immediate references to byte, 1 operator */
		DASM_immed_2b,          /* Immediate references to byte, 2 operators */
		DASM_immed_or_reg_2b,   /* Immediate references to byte or register, 2 operators */
		DASM_immed_3b,          /* Immediate references to byte, 3 operators */
		DASM_immed_1w,          /* Immediate references to word, 1 operator */
		DASM_immed_2w,          /* Immediate references to word, 2 operators */
		DASM_immed_3w,          /* Immediate references to word, 3 operators */
		DASM_indirect_1n,       /* Indirect normal, 1 operator */
		DASM_indirect_1,        /* Indirect, normal or auto-incrementing, 1 operator */
		DASM_indirect_2,        /* Indirect, normal or auto-incrementing, 2 operators */
		DASM_indirect_3,        /* Indirect, normal or auto-incrementing, 3 operators */
		DASM_indexed_1,         /* Indexed, short or long, 1 operator */
		DASM_indexed_2,         /* Indexed, short or long, 2 operators */
		DASM_indexed_3          /* Indexed, short or long, 3 operators */
	};

	const disasm_entry *m_entries;

	static std::string regname(uint8_t reg);
};

#endif
