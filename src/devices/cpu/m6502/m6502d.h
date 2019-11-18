// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502d.h

    MOS Technology 6502, original NMOS variant, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_M6502D_H
#define MAME_CPU_M6502_M6502D_H

#pragma once

class m6502_base_disassembler : public util::disasm_interface
{
public:
	struct disasm_entry {
		const char *opcode;
		int mode;
		offs_t flags;
	};

	m6502_base_disassembler(const disasm_entry *table);
	virtual ~m6502_base_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	enum {
		DASM_non,    /* no additional arguments */
		DASM_aba,    /* absolute */
		DASM_abx,    /* absolute + X */
		DASM_aby,    /* absolute + Y */
		DASM_acc,    /* accumulator */
		DASM_adr,    /* absolute address (jmp,jsr) */
		DASM_bzp,    /* zero page with bit selection */
		DASM_iax,    /* indirect + X (65c02 jmp) */
		DASM_idx,    /* zero page pre indexed */
		DASM_idy,    /* zero page post indexed */
		DASM_idz,    /* zero page post indexed (65ce02) */
		DASM_imm,    /* immediate */
		DASM_imp,    /* implicit */
		DASM_ind,    /* indirect (jmp) */
		DASM_isy,    /* zero page pre indexed sp and post indexed Y (65ce02) */
		DASM_iw2,    /* immediate word (65ce02) */
		DASM_iw3,    /* augment (65ce02) */
		DASM_rel,    /* relative */
		DASM_rw2,    /* relative word (65cs02, 65ce02) */
		DASM_zpb,    /* zero page and branch (65c02 bbr, bbs) */
		DASM_zpg,    /* zero page */
		DASM_zpi,    /* zero page indirect (65c02) */
		DASM_zpx,    /* zero page + X */
		DASM_zpy,    /* zero page + Y */
		DASM_imz,    /* load immediate byte, store to zero page address (M740) */
		DASM_spg,    /* "special page": implied FF00 OR immediate value (M740)*/
		DASM_biz,    /* bit, zero page (M740) */
		DASM_bzr,    /* bit, zero page, relative offset (M740) */
		DASM_bar,    /* bit, accumulator, relative offset (M740) */
		DASM_bac,    /* bit, accumulator (M740) */
		DASM_xa3     /* unknown XaviX opcode, 24-bit ROM pointer? */
	};

	virtual u32 get_instruction_bank() const;

private:
	const disasm_entry *table;
};

class m6502_disassembler : public m6502_base_disassembler
{
public:
	m6502_disassembler();
	virtual ~m6502_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
