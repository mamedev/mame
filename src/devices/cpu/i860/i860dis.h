// license:BSD-3-Clause
// copyright-holders:Jason Eckhardt
/***************************************************************************

    i860dis.c

    Disassembler for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)

***************************************************************************/

#ifndef MAME_CPU_I860_I860DIS_H
#define MAME_CPU_I860_I860DIS_H

#pragma once

class i860_disassembler : public util::disasm_interface
{
public:
	i860_disassembler() = default;
	virtual ~i860_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	/* Flags for the decode table.  */
	enum
	{
		DEC_MORE    = 1,    /* More decoding necessary.  */
		DEC_DECODED = 2     /* Fully decoded, go.  */
	};


	struct decode_tbl_t
	{
		/* Disassembly function for this opcode.
		   Call with buffer, mnemonic, pc, insn.  */
		void (i860_disassembler::*insn_dis)(std::ostream &, char *, uint32_t, uint32_t);

		/* Flags for this opcode.  */
		char flags;

		/* Mnemonic of this opcode (sometimes partial when more decode is
		   done in disassembly routines-- e.g., loads and stores).  */
		const char *mnemonic;
	};

	static const char *const cr2str[];
	static const decode_tbl_t decode_tbl[64];
	static const decode_tbl_t core_esc_decode_tbl[8];
	static const decode_tbl_t fp_decode_tbl[128];

	int32_t sign_ext(uint32_t x, int n);
	void int_12d(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_i2d(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_1d(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_cd(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_1c(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_1(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_0(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void flop_12d(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void flop_1d(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void flop_2d(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void flop_fxfr(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_12S(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_i2S(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_L(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_ldx(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_stx(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_fldst(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void int_flush(std::ostream &stream, char *mnemonic, uint32_t pc, uint32_t insn);
	void i860_dasm_tab_replacer(std::ostream &stream, const std::string &buf, int tab_size);
};

#endif
