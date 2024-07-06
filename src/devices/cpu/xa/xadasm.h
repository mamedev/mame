// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_XA_XADASM_H
#define MAME_CPU_XA_XADASM_H

#pragma once

#define XA_DASM_PARAMS uint8_t op, std::ostream& stream, offs_t pc, const data_buffer& opcodes, const data_buffer& params
#define XA_CALL_PARAMS op, stream, pc, opcodes, params

class xa_dasm : public util::disasm_interface
{
public:
	xa_dasm()
	{
		add_names(default_names);
	};

	virtual ~xa_dasm() = default;

private:

	struct mem_info {
		int addr;
		const char *name;
	};

	static const mem_info default_names[];
	void add_names(const mem_info *info);

	std::string get_data_address(u16 arg) const;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	typedef int (xa_dasm::*op_func) (XA_DASM_PARAMS);
	static const op_func s_instruction[256];

	const char* m_regnames16[16] = { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "illegal", "illegal", "illegal", "illegal", "illegal", "illegal", "illegal", "illegal" };
	const char* m_regnames8[16] = { "R0L", "R0H", "R1L", "R1H", "R2L", "R2H", "R3L", "R3H", "R4L", "R4H", "R5L", "R5H", "R6L", "R6H", "R7L", "R7H"};
	const char* m_aluops[16] = { "ADD", "ADDC", "SUB", "SUBC", "CMP", "AND", "OR", "XOR", "MOV", "illegal ALU 0x09", "illegal ALU 0x0a", "illegal ALU 0x0b", "illegal ALU 0x0c", "illegal ALU 0x0d", "illegal ALU 0x0e", "illegal ALU 0x0f"};
	const char* m_branches[15] = { "BCC", "BCS", "BNE", "BEQ", "BNV", "BOV", "BPL", "BMI", "BG", "BL", "BGE", "BLT", "BGT", "BLE", "BR" };
	const char* m_addsmovs[2] = { "ADDS", "MOVS" };
	const char* m_pushpull[4] = { "PUSH", "PUSHU", "POP", "POPU" };
	const char* m_shifts[3] = { "ASL", "ASR", "LSR" };
	const char* m_dwparamsizes[4] = { ".b", "invalid", ".w", ".dw" };

	std::string get_bittext(int bit);
	std::string get_directtext(int bit);
	std::string show_expanded_data4(u16 data4, int size);

	int handle_alu_type0(XA_DASM_PARAMS, int alu_op);
	int handle_alu_type1(XA_DASM_PARAMS, uint8_t op2);
	int handle_pushpop_rlist(XA_DASM_PARAMS, int type);
	int handle_adds_movs(XA_DASM_PARAMS, int which);
	int handle_shift(XA_DASM_PARAMS, int shift_type);

	int d_illegal(XA_DASM_PARAMS);

	int d_nop(XA_DASM_PARAMS);
	int d_bitgroup(XA_DASM_PARAMS);
	int d_add(XA_DASM_PARAMS);
	int d_push_rlist(XA_DASM_PARAMS);
	int d_addc(XA_DASM_PARAMS);
	int d_pushu_rlist(XA_DASM_PARAMS);
	int d_sub(XA_DASM_PARAMS);
	int d_pop_rlist(XA_DASM_PARAMS);
	int d_subb(XA_DASM_PARAMS);
	int d_popu_rlist(XA_DASM_PARAMS);
	int d_lea_offset8(XA_DASM_PARAMS);
	int d_lea_offset16(XA_DASM_PARAMS);
	int d_cmp(XA_DASM_PARAMS);
	int d_xch_type1(XA_DASM_PARAMS);
	int d_and(XA_DASM_PARAMS);
	int d_xch_type2(XA_DASM_PARAMS);
	int d_or(XA_DASM_PARAMS);
	int d_xor(XA_DASM_PARAMS);
	int d_movc_rd_rsinc(XA_DASM_PARAMS);
	int d_mov(XA_DASM_PARAMS);
	int d_pushpop_djnz_subgroup(XA_DASM_PARAMS);
	int d_g9_subgroup(XA_DASM_PARAMS);
	int d_alu(XA_DASM_PARAMS);
	int d_jb_mov_subgroup(XA_DASM_PARAMS);
	int d_movdir(XA_DASM_PARAMS);
	int d_adds(XA_DASM_PARAMS);
	int d_movx_subgroup(XA_DASM_PARAMS);
	int d_rr(XA_DASM_PARAMS);
	int d_movs(XA_DASM_PARAMS);
	int d_rrc(XA_DASM_PARAMS);
	int d_lsr_fc(XA_DASM_PARAMS);
	int d_asl_c(XA_DASM_PARAMS);
	int d_asr_c(XA_DASM_PARAMS);
	int d_norm(XA_DASM_PARAMS);
	int d_lsr_fj(XA_DASM_PARAMS);
	int d_asl_j(XA_DASM_PARAMS);
	int d_asr_j(XA_DASM_PARAMS);
	int d_rl(XA_DASM_PARAMS);
	int d_rlc(XA_DASM_PARAMS);
	int d_djnz_cjne(XA_DASM_PARAMS);
	int d_mulu_b(XA_DASM_PARAMS);
	int d_divu_b(XA_DASM_PARAMS);
	int d_mulu_w(XA_DASM_PARAMS);
	int d_divu_w(XA_DASM_PARAMS);
	int d_mul_w(XA_DASM_PARAMS);
	int d_div_w(XA_DASM_PARAMS);
	int d_div_data8(XA_DASM_PARAMS);
	int d_div_d16(XA_DASM_PARAMS);
	int d_divu_d(XA_DASM_PARAMS);
	int d_div_d(XA_DASM_PARAMS);
	int d_cjne_d8(XA_DASM_PARAMS);
	int d_cjne_d16(XA_DASM_PARAMS);
	int d_jz_rel8(XA_DASM_PARAMS);
	int d_jnz_rel8(XA_DASM_PARAMS);
	int d_branch(XA_DASM_PARAMS);
	int d_bkpt(XA_DASM_PARAMS);

	std::unordered_map<offs_t, const char *> m_names;
};

#endif // MAME_CPU_XA_XADASM_H
