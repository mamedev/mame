// license:BSD-3-Clause
// copyright-holders:Ville Linde, Peter Ferrie

#ifndef MAME_CPU_I386_I386DASM_H
#define MAME_CPU_I386_I386DASM_H

#pragma once

class i386_disassembler : public util::disasm_interface
{
public:
	class config {
	public:
		virtual ~config() = default;
		virtual int get_mode() const = 0;
	};

	i386_disassembler(config *conf);

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum
	{
		PARAM_REG = 1,      /* 16 or 32-bit register */
		PARAM_REG8,         /* 8-bit register */
		PARAM_REG16,        /* 16-bit register */
		PARAM_REG32,        /* 32-bit register */
		PARAM_REG3264,      /* 32-bit or 64-bit register */
		PARAM_REG2_32,      /* 32-bit register */
		PARAM_MMX,          /* MMX register */
		PARAM_MMX2,         /* MMX register in modrm */
		PARAM_XMM,          /* XMM register */
		PARAM_RM,           /* 16 or 32-bit memory or register */
		PARAM_RM8,          /* 8-bit memory or register */
		PARAM_RM16,         /* 16-bit memory or register */
		PARAM_RM32,         /* 32-bit memory or register */
		PARAM_RMPTR,        /* 16 or 32-bit memory or register */
		PARAM_RMPTR8,       /* 8-bit memory or register */
		PARAM_RMPTR16,      /* 16-bit memory or register */
		PARAM_RMPTR32,      /* 32-bit memory or register */
		PARAM_RMXMM,        /* 32 or 64-bit memory or register */
		PARAM_REGORXMM,     /* 32 or 64-bit register or XMM register */
		PARAM_M64,          /* 64-bit memory */
		PARAM_M64PTR,       /* 64-bit memory */
		PARAM_MMXM,         /* 64-bit memory or MMX register */
		PARAM_XMMM,         /* 128-bit memory or XMM register */
		PARAM_I4,           /* 4-bit signed immediate */
		PARAM_I8,           /* 8-bit signed immediate */
		PARAM_I16,          /* 16-bit signed immediate */
		PARAM_UI8,          /* 8-bit unsigned immediate */
		PARAM_UI16,         /* 16-bit unsigned immediate */
		PARAM_IMM,          /* 16 or 32-bit immediate */
		PARAM_IMM64,        /* 16, 32 or 64-bit immediate */
		PARAM_ADDR,         /* 16:16 or 16:32 address */
		PARAM_REL,          /* 16 or 32-bit PC-relative displacement */
		PARAM_REL8,         /* 8-bit PC-relative displacement */
		PARAM_MEM_OFFS,     /* 16 or 32-bit mem offset */
		PARAM_PREIMP,       /* prefix with implicit register */
		PARAM_SREG,         /* segment register */
		PARAM_CREG,         /* control register */
		PARAM_DREG,         /* debug register */
		PARAM_TREG,         /* test register */
		PARAM_1,            /* used by shift/rotate instructions */
		PARAM_AL,
		PARAM_CL,
		PARAM_DL,
		PARAM_BL,
		PARAM_AH,
		PARAM_CH,
		PARAM_DH,
		PARAM_BH,
		PARAM_DX,
		PARAM_EAX,          /* EAX or AX */
		PARAM_ECX,          /* ECX or CX */
		PARAM_EDX,          /* EDX or DX */
		PARAM_EBX,          /* EBX or BX */
		PARAM_ESP,          /* ESP or SP */
		PARAM_EBP,          /* EBP or BP */
		PARAM_ESI,          /* ESI or SI */
		PARAM_EDI,          /* EDI or DI */
		PARAM_XMM0,
		PARAM_XMM64,            /* 64-bit memory or XMM register */
		PARAM_XMM32,            /* 32-bit memory or XMM register */
		PARAM_XMM16             /* 16-bit memory or XMM register */
	};

	enum
	{
		MODRM = 1,
		GROUP,
		FPU,
		OP_SIZE,
		ADDR_SIZE,
		TWO_BYTE,
		PREFIX,
		SEG_CS,
		SEG_DS,
		SEG_ES,
		SEG_FS,
		SEG_GS,
		SEG_SS,
		ISREX,
		THREE_BYTE          /* [prefix] 0f op1 op2 and then mod/rm */
	};

	enum {
		FLAGS_MASK =         0x0ff,
		VAR_NAME   =         0x100,
		VAR_NAME4  =         0x200,
		ALWAYS64   =         0x400,
		SPECIAL64  =         0x800,
		GROUP_MOD  =        0x1000
	};

	struct I386_OPCODE {
		const char *mnemonic;
		u32 flags;
		u32 param1;
		u32 param2;
		u32 param3;
		offs_t dasm_flags;
	};

	struct GROUP_OP {
		char mnemonic[32];
		const I386_OPCODE *opcode;
	};

	static constexpr u32 SPECIAL64_ENT(u32 x) {
		return SPECIAL64 | (x << 24);
	}

	static const I386_OPCODE i386_opcode_table1[256];
	static const I386_OPCODE x64_opcode_alt[];
	static const I386_OPCODE i386_opcode_table2[256];
	static const I386_OPCODE i386_opcode_table0F38[256];
	static const I386_OPCODE i386_opcode_table0F3A[256];
	static const I386_OPCODE group80_table[8];
	static const I386_OPCODE group81_table[8];
	static const I386_OPCODE group83_table[8];
	static const I386_OPCODE groupC0_table[8];
	static const I386_OPCODE groupC1_table[8];
	static const I386_OPCODE groupD0_table[8];
	static const I386_OPCODE groupD1_table[8];
	static const I386_OPCODE groupD2_table[8];
	static const I386_OPCODE groupD3_table[8];
	static const I386_OPCODE groupF6_table[8];
	static const I386_OPCODE groupF7_table[8];
	static const I386_OPCODE groupFE_table[8];
	static const I386_OPCODE groupFF_table[8];
	static const I386_OPCODE group0F00_table[8];
	static const I386_OPCODE group0F01_table[8];
	static const I386_OPCODE group0F0D_table[8];
	static const I386_OPCODE group0F12_table[4];
	static const I386_OPCODE group0F16_table[4];
	static const I386_OPCODE group0F18_table[8];
	static const I386_OPCODE group0F71_table[8];
	static const I386_OPCODE group0F72_table[8];
	static const I386_OPCODE group0F73_table[8];
	static const I386_OPCODE group0FAE_table[8];
	static const I386_OPCODE group0FBA_table[8];
	static const I386_OPCODE group0FC7_table[8];
	static const GROUP_OP group_op_table[];
	static const char *const i386_reg[3][16];
	static const char *const i386_reg8[8];
	static const char *const i386_reg8rex[16];
	static const char *const i386_sreg[8];

	config *m_config;

	int address_size;
	int operand_size;
	int address_prefix;
	int operand_prefix;
	int max_length;
	uint8_t modrm;
	uint32_t segment;
	offs_t dasm_flags;
	std::string modrm_string;
	uint8_t rex, regex, sibex, rmex;
	uint8_t pre0f;

	inline u8 MODRM_REG1() const {
		return (modrm >> 3) & 0x7;
	}

	inline u8 MODRM_REG2() const {
		return modrm & 0x7;
	}

	inline u8 MODRM_MOD() const {
		return (modrm >> 6) & 0x7;
	}

	inline uint8_t FETCH(offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	inline uint16_t FETCH16(offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	inline uint32_t FETCH32(offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	inline uint8_t FETCHD(offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	inline uint16_t FETCHD16(offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	inline uint32_t FETCHD32(offs_t base_pc, offs_t &pc, const data_buffer &opcodes);

	static char *hexstring(uint32_t value, int digits);
	static char *hexstring64(uint32_t lo, uint32_t hi);
	std::string hexstringpc(uint64_t pc);
	static std::string shexstring(uint32_t value, int digits, bool always);
	void handle_sib_byte(std::ostream &stream, uint8_t mod, offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	void handle_modrm(std::ostream &stream, offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	void handle_modrm(std::string &buffer, offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	void handle_param(std::ostream &stream, uint32_t param, offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	void handle_fpu(std::ostream &stream, uint8_t op1, uint8_t op2, offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
	void decode_opcode(std::ostream &stream, const I386_OPCODE *op, uint8_t op1, offs_t base_pc, offs_t &pc, const data_buffer &opcodes);
};

#endif
