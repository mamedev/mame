// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi, Ville Linde
#ifndef MAME_CPU_MB86235_MB86235D_H
#define MAME_CPU_MB86235_MB86235D_H

#pragma once

class mb86235_disassembler : public util::disasm_interface
{
public:
	mb86235_disassembler() = default;
	virtual ~mb86235_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *regname[128];
	static const char *db_mnemonic[64];
	static const char *dbn_mnemonic[64];
	static const char *dc_mnemonic[64];
	static const char *dcn_mnemonic[64];
	static const char *mi1_field[16];
	static const char *mi2_field[32];
	static const char *mo_field[32];
	static const char *ai1_field[16];
	static const char *ai2_field[32];
	static const char *ai2f_field[32];

	void dasm_ea(std::ostream &stream, int md, int arx, int ary, int disp);
	void dasm_alu_mul(std::ostream &stream, uint64_t opcode, bool twoop);
	void dasm_control(std::ostream &stream, uint32_t pc, uint64_t opcode);
	void dasm_double_xfer1(std::ostream &stream, uint64_t opcode);
	void dasm_xfer1(std::ostream &stream, uint64_t opcode);
	void dasm_double_xfer2_field(std::ostream &stream, int sd, bool isbbus, uint32_t field);
	void dasm_double_xfer2(std::ostream &stream, uint64_t opcode);
	void dasm_xfer2(std::ostream &stream, uint64_t opcode);
	void dasm_xfer3(std::ostream &stream, uint64_t opcode);

};

#endif
