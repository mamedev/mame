// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_F2MC16_F2MC16D_H
#define MAME_CPU_F2MC16_F2MC16D_H

#pragma once

class f2mc16_disassembler : public util::disasm_interface
{
public:
	f2mc16_disassembler();

protected:
	// disasm_interface overrides
	virtual offs_t opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// mnemonic tables
	static const std::string_view s_segment_prefixes[4];
	static const std::string_view s_segment_registers[6];
	static const std::string_view s_bit_ops[8];
	static const std::string_view s_movs_ops[4];
	static const std::string_view s_sceq_ops[4];
	static const std::string_view s_shift_ops[3][4];
	static const std::string_view s_movp_ops[4];
	static const std::string_view s_bcc_ops[16];

	// formatting helpers
	void format_imm4(std::ostream &stream, u8 x);
	void format_imm8(std::ostream &stream, u8 x);
	void format_imm16(std::ostream &stream, u16 x);
	void format_imm32(std::ostream &stream, u32 x);
	void format_imm_signed(std::ostream &stream, s32 x);
	void format_dir(std::ostream &stream, u8 segm, u8 addr);
	void format_io(std::ostream &stream, u8 addr);
	void format_addr16(std::ostream &stream, u8 segm, u16 addr);
	void format_dst16(std::ostream &stream, u16 addr);
	void format_addr24(std::ostream &stream, u32 addr);
	void format_rel(std::ostream &stream, offs_t pc, s8 disp);
	void format_rwdisp8(std::ostream &stream, u8 r, u8 segm, s8 disp);
	void format_rwdisp16(std::ostream &stream, u8 r, u8 segm, u16 disp);
	void format_rldisp8(std::ostream &stream, u8 r, s8 disp);
	void format_spdisp8(std::ostream &stream, s8 disp);
	void format_pcdisp16(std::ostream &stream, s16 disp);
	void format_rlist(std::ostream &stream, u8 rlst);

	// disassembly helpers
	u32 dasm_ea8(std::ostream &stream, offs_t pc, u8 op2, u8 segm, const data_buffer &opcodes);
	u32 dasm_ea16(std::ostream &stream, offs_t pc, u8 op2, u8 segm, const data_buffer &opcodes);
	u32 dasm_ea32(std::ostream &stream, offs_t pc, u8 op2, u8 segm, const data_buffer &opcodes);
	offs_t dasm_bitop(std::ostream &stream, offs_t pc, u32 bytes, u8 segm, const data_buffer &opcodes);
	offs_t dasm_movm(std::ostream &stream, offs_t pc, u32 bytes, u8 segm, const data_buffer &opcodes);
	offs_t dasm_cstrop(std::ostream &stream, offs_t pc, u32 bytes, u8 segm, const data_buffer &opcodes);
	offs_t dasm_op6f(std::ostream &stream, offs_t pc, u32 bytes, u8 segm, const data_buffer &opcodes);
	offs_t dasm_eainst(std::ostream &stream, offs_t pc, u32 bytes, u8 op1, u8 segm, const data_buffer &opcodes);
};

#endif // MAME_CPU_F2MC16_F2MC16D_H
