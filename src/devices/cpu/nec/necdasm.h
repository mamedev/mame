// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// NEC disassembler interface

#ifndef MAME_CPU_NEC_NECDASM_H
#define MAME_CPU_NEC_NECDASM_H

#pragma once

class nec_disassembler : public util::disasm_interface
{
public:
	nec_disassembler(const u8 *decryption_table = nullptr);
	virtual ~nec_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum
	{
		PARAM_REG8 = 1,     /* 8-bit register */
		PARAM_REG16,        /* 16-bit register */
		PARAM_REG2_8,       /* 8-bit register */
		PARAM_REG2_16,      /* 16-bit register */
		PARAM_RM8,          /* 8-bit memory or register */
		PARAM_RM16,         /* 16-bit memory or register */
		PARAM_RMPTR8,       /* 8-bit memory or register */
		PARAM_RMPTR16,      /* 16-bit memory or register */
		PARAM_I3,           /* 3-bit immediate */
		PARAM_I4,           /* 4-bit immediate */
		PARAM_I8,           /* 8-bit signed immediate */
		PARAM_I16,          /* 16-bit signed immediate */
		PARAM_UI8,          /* 8-bit unsigned immediate */
		PARAM_IMM,          /* 16-bit immediate */
		PARAM_ADDR,         /* 16:16 address */
		PARAM_REL8,         /* 8-bit PC-relative displacement */
		PARAM_REL16,        /* 16-bit PC-relative displacement */
		PARAM_MEM_OFFS,     /* 16-bit mem offset */
		PARAM_SREG,         /* segment register */
		PARAM_SFREG,        /* V25/V35 special function register */
		PARAM_1,            /* used by shift/rotate instructions */
		PARAM_AL,
		PARAM_CL,
		PARAM_DL,
		PARAM_BL,
		PARAM_AH,
		PARAM_CH,
		PARAM_DH,
		PARAM_BH,
		PARAM_AW,
		PARAM_CW,
		PARAM_DW,
		PARAM_BW,
		PARAM_SP,
		PARAM_BP,
		PARAM_IX,
		PARAM_IY
	};

	enum
	{
		MODRM = 1,
		GROUP,
		FPU,
		TWO_BYTE,
		PREFIX,
		SEG_PS,
		SEG_DS0,
		SEG_DS1,
		SEG_SS
	};

	struct NEC_I386_OPCODE {
		char mnemonic[32];
		uint32_t flags;
		uint32_t param1;
		uint32_t param2;
		uint32_t param3;
		offs_t dasm_flags;
	};

	struct NEC_GROUP_OP {
		char mnemonic[32];
		const NEC_I386_OPCODE *opcode;
	};

	static const NEC_I386_OPCODE necv_opcode_table1[256];
	static const NEC_I386_OPCODE necv_opcode_table2[256];
	static const NEC_I386_OPCODE immb_table[8];
	static const NEC_I386_OPCODE immw_table[8];
	static const NEC_I386_OPCODE immws_table[8];
	static const NEC_I386_OPCODE shiftbi_table[8];
	static const NEC_I386_OPCODE shiftwi_table[8];
	static const NEC_I386_OPCODE shiftb_table[8];
	static const NEC_I386_OPCODE shiftw_table[8];
	static const NEC_I386_OPCODE shiftbv_table[8];
	static const NEC_I386_OPCODE shiftwv_table[8];
	static const NEC_I386_OPCODE group1b_table[8];
	static const NEC_I386_OPCODE group1w_table[8];
	static const NEC_I386_OPCODE group2b_table[8];
	static const NEC_I386_OPCODE group2w_table[8];
	static const NEC_GROUP_OP group_op_table[];
	static const char *const nec_reg[8];
	static const char *const nec_reg8[8];
	static const char *const nec_sreg[8];
	static const char *const nec_sfreg[256];

	const u8 *m_decryption_table;

	u8 m_modrm;
	u32 m_segment;
	offs_t m_dasm_flags;
	std::string m_modrm_string;

	inline u8 FETCH(offs_t pc_base, offs_t &pc, const data_buffer &opcodes);
	inline u16 FETCH16(offs_t pc_base, offs_t &pc, const data_buffer &opcodes);
	std::string hexstring(uint32_t value, int digits);
	std::string shexstring(uint32_t value, int digits, bool always);
	void handle_modrm(offs_t pc_base, offs_t &pc, const data_buffer &params);
	void handle_param(std::ostream &stream, uint32_t param, offs_t pc_base, offs_t &pc, const data_buffer &params);
	void handle_fpu(std::ostream &stream, uint8_t op1, uint8_t op2, offs_t pc_base, offs_t &pc, const data_buffer &params);

	void decode_opcode(std::ostream &stream, const NEC_I386_OPCODE *op, uint8_t op1, offs_t pc_base, offs_t &pc, const data_buffer &opcodes, const data_buffer &params);
};


#endif
