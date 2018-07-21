// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_CPU_TLCS870_TLCS870_H
#define MAME_CPU_TLCS870_TLCS870_H

#pragma once

class tlcs870_device : public cpu_device
{
protected:
	// construction/destruction
	tlcs870_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 2; }
	virtual uint32_t execute_max_cycles() const override { return 26; }
	virtual uint32_t execute_input_lines() const override { return 1; }
	virtual bool execute_input_edge_triggered(int inputnum) const override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void tmp87ph40an_mem(address_map &map);

	uint32_t m_debugger_temp;

private:

	enum _regs8 {
		REG_A,
		REG_W,
		REG_C,
		REG_B,
		REG_E,
		REG_D,
		REG_L,
		REG_H
	};

	enum _regs16 {
		REG_WA,
		REG_BC,
		REG_DE,
		REG_HL
	};

	enum _regs_debugger {
		DEBUGGER_REG_A,
		DEBUGGER_REG_W,
		DEBUGGER_REG_C,
		DEBUGGER_REG_B,
		DEBUGGER_REG_E,
		DEBUGGER_REG_D,
		DEBUGGER_REG_L,
		DEBUGGER_REG_H,
		DEBUGGER_REG_WA,
		DEBUGGER_REG_BC,
		DEBUGGER_REG_DE,
		DEBUGGER_REG_HL
	};

	enum _conditions {
		COND_EQ_Z,
		COND_NE_NZ,
		COND_LT_CS,
		COND_GE_CC,
		COND_LE,
		COND_GT,
		COND_T,
		COND_F
	};


	enum _srcdst_addressingmode
	{
		ADDR_IN_IMM_X,
		ADDR_IN_PC_PLUS_REG_A,
		ADDR_IN_DE,
		ADDR_IN_HL,
		ADDR_IN_HL_PLUS_IMM_D,
		ADDR_IN_HL_PLUS_REG_C,
		ADDR_IN_HLINC,
		ADDR_IN_DECHL
	};

	address_space_config m_program_config;
	address_space_config m_io_config;
	required_shared_ptr<uint8_t> m_intram;

	PAIR        m_prvpc, m_pc, m_sp;

	address_space *m_program;
	address_space *m_io;
	int     m_icount;

	// Work registers
	uint8_t m_cycles;
	uint16_t m_tmppc;
	uint32_t  m_addr;

	uint16_t m_F;

	/* CPU registers */
	uint8_t m_RBS; // register base (4-bits)

	inline uint8_t  RM8(uint32_t a) { return m_program->read_byte(a); }
	inline uint16_t RM16(uint32_t a) { return RM8(a) | (RM8((a + 1) & 0xffff) << 8); }

	inline void WM8(uint32_t a, uint8_t  v) { m_program->write_byte(a, v); }
	inline void WM16(uint32_t a, uint16_t v) { WM8(a, v);    WM8((a + 1) & 0xffff, v >> 8); }

	inline uint8_t  RX8(uint32_t a, uint32_t base) { return m_program->read_byte(base | a); }
	inline uint16_t RX16(uint32_t a, uint32_t base) { return RX8(a, base) | (RX8((a + 1) & 0xffff, base) << 8); }

	inline void WX8(uint32_t a, uint8_t  v, uint32_t base) { m_program->write_byte(base | a, v); }
	inline void WX16(uint32_t a, uint16_t v, uint32_t base) { WX8(a, v, base);   WX8((a + 1) & 0xffff, v >> 8, base); }

	inline uint8_t  READ8() { uint8_t b0 = RM8(m_addr++); m_addr &= 0xffff; return b0; }
	inline uint16_t READ16() { uint8_t b0 = READ8(); return b0 | (READ8() << 8); }

	uint16_t get_addr(uint16_t opbyte0, uint16_t val);

	uint8_t get_reg8(int reg);
	void set_reg8(int reg, uint8_t val);
	uint16_t get_reg16(int reg);
	void set_reg16(int reg, uint16_t val);

	static const int FLAG_J = 0x80;
	static const int FLAG_Z = 0x40;
	static const int FLAG_C = 0x20;
	static const int FLAG_H = 0x10;

	inline void clear_JF() { m_F &= ~FLAG_J; };
	inline void clear_ZF() { m_F &= ~FLAG_Z; };
	inline void clear_CF() { m_F &= ~FLAG_C; };
	inline void clear_HF() { m_F &= ~FLAG_H; };

	inline void set_JF() { m_F |= FLAG_J; };
	inline void set_ZF() { m_F |= FLAG_Z; };
	inline void set_CF() { m_F |= FLAG_C; };
	inline void set_HF() { m_F |= FLAG_H; };

	inline int is_JF() { return ((m_F & FLAG_J) ? 1 : 0); };
	inline int is_ZF() { return ((m_F & FLAG_Z) ? 1 : 0); };
	inline int is_CF() { return ((m_F & FLAG_C) ? 1 : 0); };
	inline int is_HF() { return ((m_F & FLAG_H) ? 1 : 0); };

	bool stream_arg(std::ostream &stream, uint32_t pc, const char *pre, const uint16_t mode, const uint16_t r, const uint16_t rb);

	// tlcs870_ops.cpp
	void decode();

	void do_illegal(uint8_t opbyte0);

	void do_NOP(uint8_t opbyte0);
	void do_SWAP_A(uint8_t opbyte0);
	void do_MUL_W_A(uint8_t opbyte0);
	void do_DIV_WA_C(uint8_t opbyte0);
	void do_RETI(uint8_t opbyte0);
	void do_RET(uint8_t opbyte0);
	void do_POP_PSW(uint8_t opbyte0);
	void do_PUSH_PSW(uint8_t opbyte0);

	void do_DAA_A(uint8_t opbyte0);
	void do_DAS_A(uint8_t opbyte0);
	void do_CLR_CF(uint8_t opbyte0);
	void do_SET_CF(uint8_t opbyte0);
	void do_CPL_CF(uint8_t opbyte0);
	void do_LD_RBS_n(uint8_t opbyte0);
	void do_INC_rr(uint8_t opbyte0);
	void do_LD_rr_mn(uint8_t opbyte0);
	void do_DEC_rr(uint8_t opbyte0);
	void do_1c_opcode(uint8_t opbyte0);
	void do_1d_opcode(uint8_t opbyte0);
	void do_1e_opcode(uint8_t opbyte0);
	void do_1f_opcode(uint8_t opbyte0);
	void do_INC_inx(uint8_t opbyte0);
	void do_INC_inHL(uint8_t opbyte0);
	void do_LD_A_inx(uint8_t opbyte0);
	void do_LD_A_inHL(uint8_t opbyte0);
	void do_LDW_inx_mn(uint8_t opbyte0);
	void do_LDW_inHL_mn(uint8_t opbyte0);
	void do_LD_inx_iny(uint8_t opbyte0);

	void do_DEC_inx(uint8_t opbyte0);
	void do_DEC_inHL(uint8_t opbyte0);
	void do_LD_inx_A(uint8_t opbyte0);
	void do_LD_inHL_A(uint8_t opbyte0);
	void do_LD_inx_n(uint8_t opbyte0);
	void do_LD_inHL_n(uint8_t opbyte0);
	void do_CLR_inx(uint8_t opbyte0);
	void do_CLR_inHL(uint8_t opbyte0);
	void do_LD_r_n(uint8_t opbyte0);

	void do_SET_inxbit(uint8_t opbyte0);
	void do_CLR_inxbit(uint8_t opbyte0);
	void do_LD_A_r(uint8_t opbyte0);
	void do_LD_r_A(uint8_t opbyte0);
	void do_INC_r(uint8_t opbyte0);
	void do_DEC_r(uint8_t opbyte0);
	void do_ALUOP_A_n(uint8_t opbyte0);
	void do_ALUOP_A_inx(uint8_t opbyte0);
	void do_JRS_T_a(uint8_t opbyte0);
	void do_JRS_F_a(uint8_t opbyte0);
	void do_CALLV_n(uint8_t opbyte0);
	void do_JR_cc_a(uint8_t opbyte0);
	void do_LD_CF_inxbit(uint8_t opbyte0);

	void do_LD_SP_mn(uint8_t opbyte0);
	void do_JR_a(uint8_t opbyte0);
	void do_CALL_mn(uint8_t opbyte0);
	void do_CALLP_n(uint8_t opbyte0);
	void do_JP_mn(uint8_t opbyte0);
	void do_ff_opcode(uint8_t opbyte0);

	// tlcs870_ops_src.cpp

	void do_e0_opcode(uint8_t opbyte0);
	void do_e1_to_e3_opcode(uint8_t opbyte0);
	void do_e4_opcode(uint8_t opbyte0);
	void do_e5_to_e7_opcode(uint8_t opbyte0);

	void do_e0_to_e7_opcode(uint8_t opbyte0, uint16_t srcaddr);

	void do_e0_to_e7_oprand_illegal(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);

	void do_ROLD_A_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_RORD_A_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_LD_rr_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_INC_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_LD_inx_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_LD_inHL_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_DEC_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_MCMP_insrc_n(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_SET_insrcbit(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_CLR_insrcbit(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_LD_r_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_ALUOP_insrc_inHL(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_ALUOP_insrc_n(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_ALUOP_A_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_XCH_r_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_CPL_insrcbit(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_LD_insrcbit_CF(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_XOR_CF_insrcbit(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_LD_CF_insrcbit(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_CALL_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);
	void do_JP_insrc(uint8_t opbyte0, uint8_t opbyte1, uint16_t srcaddr);

	uint16_t do_alu(int op, uint16_t param1, uint16_t param2);

	// tlcs870_ops_dst.cpp

	void do_f0_opcode(uint8_t opbyte0);
	void do_f2_to_f3_opcode(uint8_t opbyte0);
	void do_f4_opcode(uint8_t opbyte0);
	void do_f6_to_f7_opcode(uint8_t opbyte0);

	void do_f0_to_f7_opcode(uint8_t opbyte0, uint16_t dstaddr);

	void do_f0_to_f7_oprand_illegal_opcode(uint8_t opbyte0, uint8_t opbyte1, uint16_t dstaddr);

	void do_LD_indst_rr(uint8_t opbyte0, uint8_t opbyte1, uint16_t dstaddr);
	void do_LD_indst_n(uint8_t opbyte0, uint8_t opbyte1, uint16_t dstaddr);
	void do_LD_indst_r(uint8_t opbyte0, uint8_t opbyte1, uint16_t dstaddr);

	// tlcs870_ops_reg.cpp

	void do_regprefixtype_opcode(uint8_t opbyte0);

	void do_regprefixtype_oprand_illegal(uint8_t opbyte0, uint8_t opbyte1);

	void do_SWAP_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_MUL_gg(uint8_t opbyte0, uint8_t opbyte1);
	void do_DIV_gg_C(uint8_t opbyte0, uint8_t opbyte1);
	void do_RETN(uint8_t opbyte0, uint8_t opbyte1);
	void do_POP_gg(uint8_t opbyte0, uint8_t opbyte1);
	void do_PUSH_gg(uint8_t opbyte0, uint8_t opbyte1);
	void do_DAA_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_DAS_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_XCH_rr_gg(uint8_t opbyte0, uint8_t opbyte1);
	void do_LD_rr_gg(uint8_t opbyte0, uint8_t opbyte1);
	void do_SHLC_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_SHRC_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_ROLC_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_RORC_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_ALUOP_WA_gg(uint8_t opbyte0, uint8_t opbyte1);
	void do_ALUOP_gg_mn(uint8_t opbyte0, uint8_t opbyte1);
	void do_SET_gbit(uint8_t opbyte0, uint8_t opbyte1);
	void do_CLR_gbit(uint8_t opbyte0, uint8_t opbyte1);
	void do_LD_r_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_ALUOP_A_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_ALUOP_g_A(uint8_t opbyte0, uint8_t opbyte1);
	void do_ALUOP_g_n(uint8_t opbyte0, uint8_t opbyte1);
	void do_SET_inppbit(uint8_t opbyte0, uint8_t opbyte1);
	void do_CLR_inppbit(uint8_t opbyte0, uint8_t opbyte1);
	void do_CPL_inppbit(uint8_t opbyte0, uint8_t opbyte1);
	void do_LD_inppbit_CF(uint8_t opbyte0, uint8_t opbyte1);
	void do_LD_CF_inppbit(uint8_t opbyte0, uint8_t opbyte1);
	void do_XCH_r_g(uint8_t opbyte0, uint8_t opbyte1);
	void do_CPL_gbit(uint8_t opbyte0, uint8_t opbyte1);
	void do_LD_gbit_CF(uint8_t opbyte0, uint8_t opbyte1);
	void do_XOR_CF_gbit(uint8_t opbyte0, uint8_t opbyte1);
	void do_LD_CF_gbit(uint8_t opbyte0, uint8_t opbyte1);
	void do_LD_SP_gg(uint8_t opbyte0, uint8_t opbyte1);
	void do_LD_gg_SP(uint8_t opbyte0, uint8_t opbyte1);
	void do_CALL_gg(uint8_t opbyte0, uint8_t opbyte1);
	void do_JP_gg(uint8_t opbyte0, uint8_t opbyte1);

	void handle_div(int reg);
	void handle_mul(int reg);
	void handle_swap(int reg);

	uint8_t handle_SHLC(uint8_t val);
	uint8_t handle_SHRC(uint8_t val);
	uint8_t handle_DAS(uint8_t val);
	uint8_t handle_DAA(uint8_t val);
	uint8_t handle_ROLC(uint8_t val);
	uint8_t handle_RORC(uint8_t val);
	void handle_take_interrupt(int level);

	bool check_jump_condition(int param1);

	uint8_t get_PSW();
	void set_PSW(uint8_t data);
};


class tmp87ph40an_device : public tlcs870_device
{
public:
	// construction/destruction
	tmp87ph40an_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(TMP87PH40AN, tmp87ph40an_device)

#endif // MAME_CPU_TLCS870_TLCS870_H
