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
		DASM_rrel8,             /* Register (8-bit) + relative, 8 bits */
		DASM_brrel8,            /* Bit test + register + relative, 8 bits */
		DASM_wrrel8,            /* Register (16-bit) + relative, 8 bits */
		DASM_direct_1b,         /* Register-direct references, 1 operator, 8 bits */
		DASM_direct_2b,         /* Register-direct references, 2 operators, 8 bits */
		DASM_direct_2e,         /* Register-direct references, 2 operators, 8 bits extended to 16 */
		DASM_direct_3b,         /* Register-direct references, 3 operators, 8 bits */
		DASM_direct_3e,         /* Register-direct references, 3 operators, 8 bits extended to 16 */
		DASM_direct_1w,         /* Register-direct references, 1 operator, 16 bits */
		DASM_direct_2w,         /* Register-direct references, 2 operators, 16 bits */
		DASM_direct_3w,         /* Register-direct references, 3 operators, 16 bits */
		DASM_immed_1b,          /* Immediate references to byte, 1 operator, 8 bits */
		DASM_immed_2b,          /* Immediate references to byte, 2 operators, 8 bits */
		DASM_immed_2e,          /* Immediate references to byte, 2 operators, 8 bits extended to 16 */
		DASM_immed_or_reg_2b,   /* Immediate references to byte or register, 2 operators, 8 bits */
		DASM_immed_3b,          /* Immediate references to byte, 3 operators, 8 bits */
		DASM_immed_3e,          /* Immediate references to byte, 3 operators, 8 bits extended to 16 */
		DASM_immed_1w,          /* Immediate references to word, 1 operator, 16 bits */
		DASM_immed_2w,          /* Immediate references to word, 2 operators, 16 bits */
		DASM_immed_or_reg_2w,   /* Immediate references to byte or register, 2 operators, 16 bits */
		DASM_immed_3w,          /* Immediate references to word, 3 operators, 16 bits */
		DASM_indirect_1n,       /* Indirect normal, 1 operator */
		DASM_indirect_1w,       /* Indirect, normal or auto-incrementing, 1 operator, 16 bits */
		DASM_indirect_2b,       /* Indirect, normal or auto-incrementing, 2 operators, 8 bits */
		DASM_indirect_2w,       /* Indirect, normal or auto-incrementing, 2 operators, 16 bits */
		DASM_indirect_3b,       /* Indirect, normal or auto-incrementing, 3 operators, 8 bits */
		DASM_indirect_3e,       /* Indirect, normal or auto-incrementing, 3 operators, 8 bits extended to 16 */
		DASM_indirect_3w,       /* Indirect, normal or auto-incrementing, 3 operators, 16 bits */
		DASM_indexed_1w,        /* Indexed, short or long, 1 operator, 16 bits */
		DASM_indexed_2b,        /* Indexed, short or long, 2 operators, 8 bits */
		DASM_indexed_2w,        /* Indexed, short or long, 2 operators, 16 bits */
		DASM_indexed_3b,        /* Indexed, short or long, 3 operators, 8 bits */
		DASM_indexed_3e,        /* Indexed, short or long, 3 operators, 8 bits extended to 16 */
		DASM_indexed_3w         /* Indexed, short or long, 3 operators, 16 bits */
	};

	const disasm_entry *m_entries;

	virtual std::string regname8(uint8_t reg, bool is_dest) const;
	virtual std::string regname16(uint8_t reg, bool is_dest) const;
	std::string regname_indirect(uint8_t reg) const;
	std::string regname_indexed(uint8_t reg, int8_t delta) const;
};

#endif
