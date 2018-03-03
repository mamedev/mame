// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8d.h

    H8-300 base cpu emulation, disassembler

***************************************************************************/

#ifndef MAME_CPU_H8_H8D_H
#define MAME_CPU_H8_H8D_H

#pragma once

class h8_disassembler : public util::disasm_interface
{
protected:
	struct disasm_entry {
		int slot;
		u32 val, mask;
		u16 val0, mask0;
		const char *opcode;
		int am1, am2;
		offs_t flags;
	};

public:
	h8_disassembler(const disasm_entry *_table);
	h8_disassembler();

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	enum {
		DASM_none,     /* no additional arguments */

		DASM_r8l,      /* 8-bits register in bits 0-3 */
		DASM_r8h,      /* 8-bits register in bits 4-7 */
		DASM_r8u,      /* 8-bits register in bits 8-15 */
		DASM_r16l,     /* 16-bits register in bits 0-3 */
		DASM_r16h,     /* 16-bits register in bits 4-7 */
		DASM_r32l,     /* 32-bits register in bits 0-3 */
		DASM_r32h,     /* 32-bits register in bits 4-7 */

		DASM_r16ih,    /* indexed through 16-bits register in bits 4-6 */
		DASM_r16ihh,   /* indexed through 16-bits register in bits 4-6 in 4-bytes instruction */
		DASM_pr16h,    /* indexed through predecremented 16-bits register in bits 4-6 */
		DASM_r16ph,    /* indexed through postincremented 16-bits register in bits 4-6 */
		DASM_r16d16h,  /* indexed through 16-bits register in bits 4-6 with 16-bits displacement at end of instruction */

		DASM_r32ih,    /* indexed through 32-bits register in bits 4-6 */
		DASM_r32ihh,   /* indexed through 32-bits register in bits 4-6 in 4-bytes instruction */
		DASM_pr32h,    /* indexed through predecremented 32-bits register in bits 4-6 */
		DASM_r32pl,    /* indexed through postincremented 32-bits register in bits 0-2 */
		DASM_r32ph,    /* indexed through postincremented 32-bits register in bits 4-6 */
		DASM_r32d16h,  /* indexed through 32-bits register in bits 4-6 with 16-bits displacement at end of instruction */
		DASM_r32d32hh, /* indexed through 32-bits register in bits 20-22 with 32-bits displacement at end of instruction */

		DASM_psp,      /* indexed through predecremented stack pointer */
		DASM_spp,      /* indexed through postincremented stack pointer */

		DASM_r32n2l,   /* Block of 2 registers */
		DASM_r32n3l,   /* Block of 3 registers */
		DASM_r32n4l,   /* Block of 4 registers */

		DASM_abs8,     /* 8-bit address present at +1 */
		DASM_abs16,    /* 16-bit address present at end of instruction */
		DASM_abs32,    /* 32-bit address present at end of instruction */
		DASM_abs8i,    /* 8-bit indirect jump address present at +1 */
		DASM_abs16e,   /* 16-bit jump address present at +2 */
		DASM_abs24e,   /* 24-bit jump address present at +1 */

		DASM_rel8,     /* 8-bit pc-relative jump address at +1, offset=2 */
		DASM_rel16,    /* 16-bit pc-relative jump address at +2, offset=4 */

		DASM_one,      /* immediate value 1 */
		DASM_two,      /* immediate value 2 */
		DASM_four,     /* immediate value 4 */

		DASM_imm2,     /* 2-bit immediate in bits 4-5 (trapa) */
		DASM_imm3,     /* 3-bit immediate in bits 4-6 (bit selection */
		DASM_imm8,     /* 8-bit immediate at +1 */
		DASM_imm16,    /* 16-bit immediate at +2 */
		DASM_imm32,    /* 32-bit immediate at +2 */

		DASM_ccr,      /* internal register ccr */
		DASM_exr,      /* internal register exr */
		DASM_macl,     /* internal register macl */
		DASM_mach      /* internal register mach */
	};

	void disassemble_am(std::ostream &stream, int am, offs_t pc, const data_buffer &opcodes, u32 opcode, int slot, int offset);

	const disasm_entry *table;

	static const disasm_entry disasm_entries[];
};

#endif
