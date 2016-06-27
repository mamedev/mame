// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Vas Crabb
/*
    SPARC disassembler
*/

#ifndef MAME_DEVICES_CPU_SPARC_SPARC_DASM_H
#define MAME_DEVICES_CPU_SPARC_SPARC_DASM_H

#pragma once

#include <map>


class sparc_debug_state
{
public:
	virtual UINT64 get_reg_r(unsigned index) const = 0;
	virtual UINT64 get_translated_pc() const = 0;
	virtual UINT8 get_icc() const = 0;
	virtual UINT8 get_xcc() const = 0;
	virtual UINT8 get_fcc(unsigned index) const = 0; // ?><=

protected:
	~sparc_debug_state() { }
};


class sparc_disassembler
{
public:
	enum vis_level { vis_none, vis_1, vis_2, vis_2p, vis_3, vis_3b };

	struct asi_desc
	{
		asi_desc() { }
		asi_desc(const char *name_, const char *desc_) : name(name_), desc(desc_) { }
		const char *name = nullptr;
		const char *desc = nullptr;
	};
	typedef std::map<UINT8, asi_desc> asi_desc_map;

	struct state_reg_desc
	{
		state_reg_desc() { }
		state_reg_desc(bool reserved_, const char *read_name_, const char *write_name_) : reserved(reserved_), read_name(read_name_), write_name(write_name_) { }
		bool        reserved = false;
		const char  *read_name = nullptr;
		const char  *write_name = nullptr;
	};
	typedef std::map<UINT8, state_reg_desc> state_reg_desc_map;

	struct prftch_desc
	{
		prftch_desc() { }
		prftch_desc(const char *name_) : name(name_) { }
		const char *name = nullptr;
	};
	typedef std::map<UINT8, prftch_desc> prftch_desc_map;

	sparc_disassembler(const sparc_debug_state *state, unsigned version);
	sparc_disassembler(const sparc_debug_state *state, unsigned version, vis_level vis);

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
		const char *    (*get_comment)(const sparc_debug_state *state, bool use_cc, offs_t pc, UINT32 op);
		int             disp_width;
		bool            use_pred, use_cc;
		const char      *reg_cc[4];
		const char      *mnemonic[16];
	};

	struct int_op_desc
	{
		bool            hex_imm;
		const char      *mnemonic;
	};
	typedef std::map<UINT8, int_op_desc> int_op_desc_map;

	struct fpop1_desc
	{
		fpop1_desc() { }
		fpop1_desc(bool three_op_, bool rs1_shift_, bool rs2_shift_, bool rd_shift_, const char *mnemonic_) : three_op(three_op_), rs1_shift(rs1_shift_), rs2_shift(rs2_shift_), rd_shift(rd_shift_), mnemonic(mnemonic_) { }
		bool        three_op = true;
		bool        rs1_shift = false;
		bool        rs2_shift = false;
		bool        rd_shift = false;
		const char  *mnemonic = nullptr;
	};
	typedef std::map<UINT16, fpop1_desc> fpop1_desc_map;

	struct fpop2_desc
	{
		fpop2_desc() { }
		fpop2_desc(bool int_rs1_, bool shift_, const char *mnemonic_) : int_rs1(int_rs1_), shift(shift_), mnemonic(mnemonic_) { }
		bool        int_rs1 = false;
		bool        shift = false;
		const char  *mnemonic = nullptr;
	};
	typedef std::map<UINT16, fpop2_desc> fpop2_desc_map;

	struct ldst_desc
	{
		ldst_desc() { }
		ldst_desc(bool rd_first_, bool alternate_, char rd_alt_reg_, bool rd_shift_, const char *mnemonic_, const char *g0_synth_) : rd_first(rd_first_), alternate(alternate_), rd_alt_reg(rd_alt_reg_), rd_shift(rd_shift_), mnemonic(mnemonic_), g0_synth(g0_synth_) { }
		bool        rd_first = false;
		bool        alternate = false;
		char        rd_alt_reg = '\0';
		bool        rd_shift = false;
		const char  *mnemonic = nullptr;
		const char  *g0_synth = nullptr;
	};
	typedef std::map<UINT8, ldst_desc> ldst_desc_map;

	struct vis_op_desc
	{
		enum arg { X, R, Fs, Fd };
		vis_op_desc() { }
		vis_op_desc(arg rs1_, arg rs2_, arg rd_, bool collapse_, const char *mnemonic_) : rs1(rs1_), rs2(rs2_), rd(rd_), collapse(collapse_), mnemonic(mnemonic_) { }
		arg         rs1 = X;
		arg         rs2 = X;
		arg         rd = X;
		bool        collapse = false;
		const char  *mnemonic = nullptr;
	};
	typedef std::map<UINT16, vis_op_desc> vis_op_desc_map;

	offs_t dasm_invalid(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_branch(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_shift(char *buf, offs_t pc, UINT32 op, const char *mnemonic, const char *mnemonicx, const char *mnemonicx0) const;
	offs_t dasm_read_state_reg(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_write_state_reg(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_move_cond(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_move_reg_cond(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_fpop1(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_fpop2(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_impdep1(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_jmpl(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_return(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_tcc(char *buf, offs_t pc, UINT32 op) const;
	offs_t dasm_ldst(char *buf, offs_t pc, UINT32 op) const;

	void dasm_address(char *&output, UINT32 op) const;
	void dasm_asi(char *&output, UINT32 op) const;
	void dasm_asi_comment(char *&output, UINT32 op) const;
	void dasm_vis_arg(char *&output, bool &args, vis_op_desc::arg fmt, UINT32 reg) const;

	UINT32 freg(UINT32 val, bool shift) const;

	template <typename T> void add_int_op_desc(const T &desc);
	template <typename T> void add_fpop1_desc(const T &desc);
	template <typename T> void add_fpop2_desc(const T &desc);
	template <typename T> void add_ldst_desc(const T &desc);
	template <typename T> void add_vis_op_desc(const T &desc);

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
	static const int_op_desc_map::value_type    V7_INT_OP_DESC[];
	static const int_op_desc_map::value_type    V8_INT_OP_DESC[];
	static const int_op_desc_map::value_type    V9_INT_OP_DESC[];
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
	static const vis_op_desc_map::value_type    VIS1_OP_DESC[];
	static const state_reg_desc_map::value_type VIS1_STATE_REG_DESC[];
	static const asi_desc_map::value_type       VIS1_ASI_DESC[];
	static const vis_op_desc_map::value_type    VIS2_OP_DESC[];
	static const asi_desc_map::value_type       VIS2P_ASI_DESC[];
	static const fpop1_desc_map::value_type     VIS3_FPOP1_DESC[];
	static const vis_op_desc_map::value_type    VIS3_OP_DESC[];
	static const vis_op_desc_map::value_type    VIS3B_OP_DESC[];

	const sparc_debug_state *m_state;
	unsigned                m_version;
	vis_level               m_vis_level;
	int                     m_op_field_width;
	branch_desc             m_branch_desc[8];
	int_op_desc_map         m_int_op_desc;
	state_reg_desc_map      m_state_reg_desc;
	fpop1_desc_map          m_fpop1_desc;
	fpop2_desc_map          m_fpop2_desc;
	ldst_desc_map           m_ldst_desc;
	asi_desc_map            m_asi_desc;
	prftch_desc_map         m_prftch_desc;
	vis_op_desc_map         m_vis_op_desc;
};

#endif // MAME_DEVICES_CPU_SPARC_SPARC_DASM_H
