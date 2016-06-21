// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Vas Crabb
/*
    SPARC disassembler
*/

#ifndef MAME_DEVICES_CPU_SPARC_SPARC_DASM_H
#define MAME_DEVICES_CPU_SPARC_SPARC_DASM_H

#pragma once

#include <map>


class sparc_disassembler
{
public:
	struct asi_desc
	{
		const char *name = nullptr;
		const char *desc = nullptr;
	};
	typedef std::map<UINT8, asi_desc> asi_desc_map;

	struct state_reg_desc
	{
		bool reserved = false;
		const char *read_name = nullptr;
		const char *write_name = nullptr;
	};
	typedef std::map<UINT8, state_reg_desc> state_reg_desc_map;

	struct prftch_desc
	{
		const char *name = nullptr;
	};
	typedef std::map<UINT8, prftch_desc> prftch_desc_map;

	sparc_disassembler(unsigned version);

	template <typename T> void add_state_reg_desc(const T &desc)
	{
		for (const auto &it : desc)
		{
			auto ins = m_state_reg_desc.insert(it);
			if (!ins.second)
			{
				ins.first->second.reserved = it.second.reserved;
				if (it.second.read_name)
					ins.first->second.read_name = it.second.read_name;
				if (it.second.write_name)
					ins.first->second.write_name = it.second.write_name;
			}
		}
	}

	template <typename T> void add_asi_desc(const T &desc)
	{
		// TODO: support ranges
		for (const auto &it : desc)
		{
			auto ins = m_asi_desc.insert(it);
			if (!ins.second)
			{
				if (it.second.name)
					ins.first->second.name = it.second.name;
				if (it.second.desc)
					ins.first->second.desc = it.second.desc;
			}
		}
	}

	template <typename T> void add_prftch_desc(const T &desc)
	{
		for (const auto &it : desc)
		{
			auto ins = m_prftch_desc.insert(it);
			if (!ins.second)
			{
				if (it.second.name)
					ins.first->second.name = it.second.name;
			}
		}
	}

	offs_t dasm(char *buf, offs_t pc, UINT32 op) const;

private:
	struct branch_desc
	{
		INT32           (*get_disp)(UINT32 op);
		int             disp_width;
		bool            use_pred, use_cc;
		const char      *reg_cc[4];
		const char      *mnemonic[16];
	};

	struct int_op_desc
	{
		unsigned        min_version;
		bool            hex_imm;
		const char      *mnemonic;
	};
	typedef std::map<UINT8, int_op_desc> int_op_desc_map;

	struct fpop1_desc
	{
		bool        three_op = true;
		bool        rs1_shift = false;
		bool        rs2_shift = false;
		bool        rd_shift = false;
		const char  *mnemonic = nullptr;
	};
	typedef std::map<UINT16, fpop1_desc> fpop1_desc_map;

	struct fpop2_desc
	{
		bool        int_rs1 = false;
		bool        shift = false;
		const char  *mnemonic = nullptr;
	};
	typedef std::map<UINT16, fpop2_desc> fpop2_desc_map;

	struct ldst_desc
	{
		bool        rd_first = false;
		bool        alternate = false;
		char        rd_alt_reg = '\0';
		bool        rd_shift = false;
		const char  *mnemonic = nullptr;
		const char  *g0_synth = nullptr;
	};
	typedef std::map<UINT8, ldst_desc> ldst_desc_map;

	offs_t dasm_invalid(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_branch(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_shift(char *buf, offs_t pc, UINT32 op, const char *mnemonic, const char *mnemonicx, const char *mnemonicx0) const;
	offs_t dasm_read_state_reg(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_write_state_reg(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_move_cond(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_move_reg_cond(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_fpop1(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_fpop2(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_jmpl(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_return(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_tcc(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_ldst(char *buf, offs_t pc, UINT32 op) const;

	void dasm_address(char *&output, UINT32 op) const;
	void dasm_asi(char *&output, UINT32 op) const;
	void dasm_asi_comment(char *&output, UINT32 op) const;

	UINT32 freg(UINT32 val, bool shift) const;

	template <typename T> void add_fpop1_desc(const T &desc);
	template <typename T> void add_fpop2_desc(const T &desc);
	template <typename T> void add_ldst_desc(const T &desc);

	void pad_op_field(char *buf, char *&output) const;
	ATTR_PRINTF(2, 3) static void print(char *&output, const char *fmt, ...);

	static const char * const                   REG_NAMES[32];
	static const branch_desc                    EMPTY_BRANCH_DESC;
	static const branch_desc                    BPCC_DESC;
	static const branch_desc                    BICC_DESC;
	static const branch_desc                    BPR_DESC;
	static const branch_desc                    FBPFCC_DESC;
	static const branch_desc                    FBFCC_DESC;
	static const branch_desc                    CBCCC_DESC;
	static const int_op_desc_map::value_type    SIMPLE_INT_OP_DESC[];
	static const state_reg_desc_map::value_type V9_STATE_REG_DESC[];
	static const char * const                   MOVCC_CC_NAMES[8];
	static const char * const                   MOVCC_COND_NAMES[32];
	static const char * const                   MOVE_INT_COND_MNEMONICS[8];
	static const char * const                   V9_PRIV_REG_NAMES[32];
	static const fpop1_desc_map::value_type     V7_FPOP1_DESC[];
	static const fpop1_desc_map::value_type     V9_FPOP1_DESC[];
	static const fpop2_desc_map::value_type     V7_FPOP2_DESC[];
	static const fpop2_desc_map::value_type     V9_FPOP2_DESC[];
	static const ldst_desc_map::value_type      V7_LDST_DESC[];
	static const ldst_desc_map::value_type      V9_LDST_DESC[];
	static const asi_desc_map::value_type       V9_ASI_DESC[];
	static const prftch_desc_map::value_type    V9_PRFTCH_DESC[];

	unsigned            m_version;
	int                 m_op_field_width;
	branch_desc         m_branch_desc[8];
	int_op_desc_map     m_simple_int_op_desc;
	state_reg_desc_map  m_state_reg_desc;
	fpop1_desc_map      m_fpop1_desc;
	fpop2_desc_map      m_fpop2_desc;
	ldst_desc_map       m_ldst_desc;
	asi_desc_map        m_asi_desc;
	prftch_desc_map     m_prftch_desc;
};

CPU_DISASSEMBLE( sparcv7 );
CPU_DISASSEMBLE( sparcv8 );
CPU_DISASSEMBLE( sparcv9 );

#endif // MAME_DEVICES_CPU_SPARC_SPARC_DASM_H
