// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller,Ernesto Corvi
/*****************************************************************************
 *
 *   8000dasm.c
 *   Portable Z8000(2) emulator
 *
 *****************************************************************************/

#ifndef MAME_CPU_Z8000_8000DASM_H
#define MAME_CPU_Z8000_8000DASM_H

#pragma once

class z8000_disassembler : public util::disasm_interface
{
public:
	struct config {
		virtual ~config() = default;
		virtual bool get_segmented_mode() const = 0;
	};

	z8000_disassembler(config *conf);
	virtual ~z8000_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct opcode {
		u16     beg, end, step;
		u16     size;
		const char  *dasm;
		offs_t dasmflags;
	};

	static const opcode table[];
	static const char *const cc[16];
	static const char *const flg[16];
	static const char *const ints[4];

	config *m_config;

	u16 oplist[0x10000];

	void get_op(const data_buffer &opcodes, int i, offs_t &new_pc, u16 *w, u8 *b, u8 *n);

};

#endif
