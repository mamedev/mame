// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_VAX_VAXDASM_H
#define MAME_CPU_VAX_VAXDASM_H

#pragma once

class vax_disassembler : public util::disasm_interface
{
public:
	enum class mode : u8
	{
		none,
		rb, srb, urb, cntb, rw, srw, urw, mskw, rl, srl, url, posl, prl, rq, srq, urq, ro, uro,
		mb, mw, ml, mq, mo,
		wb, ww, wl, wq, wo,
		ab, aw, al, aq, ao,
		bb, bw,
		vb
	};

	struct opdef
	{
		std::string_view mnemonic;
		mode operand[6];
		u32 flags;
	};

	// construction/destruction
	vax_disassembler();

	static bool is_read_mode(mode m);
	static const mode *get_operands(u8 op);

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// internal helpers
	void format_immediate(std::ostream &stream, u32 i) const;
	void format_signed(std::ostream &stream, s32 value) const;
	void format_relative(std::ostream &stream, u32 pc, s32 displ) const;
	void format_register_mask(std::ostream &stream, u16 mask) const;
	offs_t disassemble_inst(std::ostream &stream, const opdef &inst, offs_t &pc, offs_t ppc, const data_buffer &opcodes) const;
};

#endif // MAME_CPU_VAX_VAXDASM_H
