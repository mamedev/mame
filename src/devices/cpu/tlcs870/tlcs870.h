// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_CPU_TLCS870_TLCS870_H
#define MAME_CPU_TLCS870_TLCS870_H

#pragma once

class tlcs870_device : public cpu_device
{
public:

	auto p0_in_cb() { return m_port_in_cb[0].bind(); }
	auto p1_in_cb() { return m_port_in_cb[1].bind(); }
	auto p2_in_cb() { return m_port_in_cb[2].bind(); }
	auto p3_in_cb() { return m_port_in_cb[3].bind(); }
	auto p4_in_cb() { return m_port_in_cb[4].bind(); }
	auto p5_in_cb() { return m_port_in_cb[5].bind(); }
	auto p6_in_cb() { return m_port_in_cb[6].bind(); }
	auto p7_in_cb() { return m_port_in_cb[7].bind(); }

	auto p0_out_cb() { return m_port_out_cb[0].bind(); }
	auto p1_out_cb() { return m_port_out_cb[1].bind(); }
	auto p2_out_cb() { return m_port_out_cb[2].bind(); }
	auto p3_out_cb() { return m_port_out_cb[3].bind(); }
	auto p4_out_cb() { return m_port_out_cb[4].bind(); }
	auto p5_out_cb() { return m_port_out_cb[5].bind(); }
	auto p6_out_cb() { return m_port_out_cb[6].bind(); }
	auto p7_out_cb() { return m_port_out_cb[7].bind(); }

	auto an0_in_cb() { return m_port_analog_in_cb[0].bind(); }
	auto an1_in_cb() { return m_port_analog_in_cb[1].bind(); }
	auto an2_in_cb() { return m_port_analog_in_cb[2].bind(); }
	auto an3_in_cb() { return m_port_analog_in_cb[3].bind(); }
	auto an4_in_cb() { return m_port_analog_in_cb[4].bind(); }
	auto an5_in_cb() { return m_port_analog_in_cb[5].bind(); }
	auto an6_in_cb() { return m_port_analog_in_cb[6].bind(); }
	auto an7_in_cb() { return m_port_analog_in_cb[7].bind(); }

	auto serial0_out_cb() { return m_serial_out_cb[0].bind(); }
	auto serial1_out_cb() { return m_serial_out_cb[1].bind(); }


protected:
	// construction/destruction
	tlcs870_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 2; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 26; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
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

	void tmp87ph40an_mem(address_map &map) ATTR_COLD;

	uint32_t m_debugger_temp;

	enum _tlcs870interrupts
	{
		TLCS870_IRQ_INT5, // FFE0 INT5    (External Interrupt 5) - priority 15 (lowest)
		TLCS870_IRQ_INTTC2,       // FFE2 INTTC2  (16-bit TC2 Interrupt)
		TLCS870_IRQ_INTSIO2,      // FFE4 INTSIO2 (Serial Interface 2 Interrupt)
		TLCS870_IRQ_INT4,         // FFE6 INT4    (External Interrupt 4)
		TLCS870_IRQ_INT3,         // FFE8 INT3    (External Interrupt 3)
		TLCS870_IRQ_INTTC4,       // FFEA INTTC4  (8-bit TC4 Interrupt)
		TLCS870_IRQ_INTSIO1,      // FFEC INTSIO1 (Serial Interface 1 Interrupt)
		TLCS870_IRQ_INTTC3,       // FFEE INTTC3  (8-bit TC3 Interrupt)
		TLCS870_IRQ_INT2,         // FFF0 INT2    (External Interrupt 2)
		TLCS870_IRQ_INTTBT,       // FFF2 INTTBT  (Time Base Timer Interrupt)
		TLCS870_IRQ_INT1,         // FFF4 INT1    (External Interrupt 1)
		TLCS870_IRQ_INTTC1,       // FFF6 INTTC1  (16-bit TC1 Interrupt)
		TLCS870_IRQ_INT0,         // FFF8 INT0    (External Interrupt 0)
		TLCS870_IRQ_INTWDT,       // FFFA INTWDT  (Watchdog Timer Interrupt)
		TLCS870_IRQ_INTSW,        // FFFC INTSW   (Software Interrupt)
		TLCS870_IRQ_RESET,        // FFFE RESET   (Reset Interrupt) - priority 0 (highest)
	};

private:

	enum _regs8
	{
		REG_A,
		REG_W,
		REG_C,
		REG_B,
		REG_E,
		REG_D,
		REG_L,
		REG_H
	};

	enum _regs16
	{
		REG_WA,
		REG_BC,
		REG_DE,
		REG_HL
	};

	enum _regs_debugger
	{
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
		DEBUGGER_REG_HL,
		DEBUGGER_REG_RB,
		DEBUGGER_REG_SP
	};

	enum _conditions
	{
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
	required_shared_ptr<uint8_t> m_dbr;

	PAIR        m_prvpc, m_pc, m_sp;

	address_space *m_program;
	address_space *m_io;
	int     m_icount;

	devcb_read8::array<8>   m_port_in_cb;
	devcb_write8::array<8>  m_port_out_cb;
	devcb_read8::array<8>   m_port_analog_in_cb;
	devcb_write_line::array<2> m_serial_out_cb;


	uint8_t m_port_out_latch[8];
	int m_read_input_port;
	uint8_t m_port0_cr, m_port1_cr, m_port6_cr, m_port7_cr;

	uint8_t port0_r();
	uint8_t port1_r();
	uint8_t port2_r();
	uint8_t port3_r();
	uint8_t port4_r();
	uint8_t port5_r();
	uint8_t port6_r();
	uint8_t port7_r();

	void port0_w(uint8_t data);
	void port1_w(uint8_t data);
	void port2_w(uint8_t data);
	void port3_w(uint8_t data);
	void port4_w(uint8_t data);
	void port5_w(uint8_t data);
	void port6_w(uint8_t data);
	void port7_w(uint8_t data);

	void p0cr_w(uint8_t data);
	void p1cr_w(uint8_t data);
	void p6cr_w(uint8_t data);
	void p7cr_w(uint8_t data);

	uint8_t eir_l_r();
	uint8_t eir_h_r();
	uint8_t il_l_r();
	uint8_t il_h_r();

	void eir_l_w(uint8_t data);
	void eir_h_w(uint8_t data);
	void il_l_w(uint8_t data);
	void il_h_w(uint8_t data);

	uint8_t eintcr_r();

	void eintcr_w(uint8_t data);

	void treg1a_l_w(uint8_t data);
	void treg1a_h_w(uint8_t data);
	void treg1b_l_w(uint8_t data);
	void treg1b_h_w(uint8_t data);
	void tc1cr_w(uint8_t data);
	void tc2cr_w(uint8_t data);
	void treg2_l_w(uint8_t data);
	void treg2_h_w(uint8_t data);
	void treg3a_w(uint8_t data);
	void tc3cr_w(uint8_t data);
	void tc4cr_w(uint8_t data);

	uint8_t treg1b_l_r();
	uint8_t treg1b_h_r();
	uint8_t treg3a_r();
	uint8_t treg3b_r();
	void treg4_w(uint8_t data);


	void sio1cr1_w(uint8_t data);
	void sio1cr2_w(uint8_t data);
	void sio2cr1_w(uint8_t data);
	void sio2cr2_w(uint8_t data);
	uint8_t sio2sr_r();
	uint8_t sio1sr_r();

	void wdtcr1_w(uint8_t data);
	void wdtcr2_w(uint8_t data);

	void tbtcr_w(uint8_t data);

	uint8_t tbtcr_r();

	void syscr1_w(uint8_t data);
	void syscr2_w(uint8_t data);
	uint8_t syscr1_r();
	uint8_t syscr2_r();
	void rbs_w(uint8_t data);
	uint8_t psw_r();

	uint8_t adccr_r();
	uint8_t adcdr_r();

	void adccr_w(uint8_t data);

	// Work registers
	uint8_t m_cycles;
	uint16_t m_tmppc;
	uint32_t  m_addr;

	uint16_t m_F;

	/* CPU registers */
	uint8_t m_RBS; // register base (4-bits)

	uint16_t m_IL; // 3D / 3C
	uint16_t m_EIR; // 3B / 3A
	uint8_t m_EINTCR;
	uint8_t m_ADCCR;
	uint8_t m_ADCDR;
	uint8_t m_SYSCR1;
	uint8_t m_SYSCR2;
	uint8_t m_TBTCR;
	uint16_t m_TREG1A;
	uint16_t m_TREG1B;
	uint8_t m_TC1CR;
	uint16_t m_TREG2;
	uint8_t m_TC2CR;
	uint8_t m_TREG3A;
	uint8_t m_TREG3B;
	uint8_t m_TC3CR;
	uint8_t m_TREG4;
	uint8_t m_TC4CR;
	uint8_t m_SIOCR1[2];
	uint8_t m_SIOCR2[2];
	uint8_t m_WDTCR1;

	uint16_t m_irqstate;

	// serial stuff
	TIMER_CALLBACK_MEMBER(sio0_transmit_cb);
	TIMER_CALLBACK_MEMBER(sio1_transmit_cb);
	uint8_t m_transfer_numbytes[2];
	uint8_t m_transfer_pos[2];
	uint8_t m_transfer_shiftreg[2];
	uint8_t m_transfer_shiftpos[2];
	uint8_t m_transfer_mode[2];
	int m_transmit_bits[2];
	int m_receive_bits[2];

	emu_timer *m_serial_transmit_timer[2];

	TIMER_CALLBACK_MEMBER(tc1_cb);
	TIMER_CALLBACK_MEMBER(tc2_cb);
	TIMER_CALLBACK_MEMBER(tc3_cb);
	TIMER_CALLBACK_MEMBER(tc4_cb);

	void tc2_reload();
	void tc2_cancel();

	emu_timer *m_tcx_timer[4];


	void set_irq_line(int irqline, int state);
	void check_interrupts();
	void take_interrupt(int priority);

	uint8_t  RM8(const uint32_t a) { return m_program->read_byte(a); }
	uint16_t RM16(const uint32_t a) { return RM8(a) | (RM8((a + 1) & 0xffff) << 8); }

	void WM8(const uint32_t a, const uint8_t  v) { m_program->write_byte(a, v); }
	void WM16(const uint32_t a, const uint16_t v) { WM8(a, v);    WM8((a + 1) & 0xffff, v >> 8); }

	uint8_t  RX8(const uint32_t a, const uint32_t base) { return m_program->read_byte(base | a); }
	uint16_t RX16(const uint32_t a, const uint32_t base) { return RX8(a, base) | (RX8((a + 1) & 0xffff, base) << 8); }

	void WX8(const uint32_t a, const uint8_t  v, const uint32_t base) { m_program->write_byte(base | a, v); }
	void WX16(const uint32_t a, const uint16_t v, const uint32_t base) { WX8(a, v, base);   WX8((a + 1) & 0xffff, v >> 8, base); }

	uint8_t  READ8() { const uint8_t b0 = RM8(m_addr++); m_addr &= 0xffff; return b0; }
	uint16_t READ16() { const uint8_t b0 = READ8(); return b0 | (READ8() << 8); }

	uint16_t get_addr(uint16_t opbyte0, uint16_t val);

	const uint8_t get_reg8(const int reg);
	void set_reg8(const int reg, uint8_t val);
	const uint16_t get_reg16(const int reg);
	void set_reg16(const int reg, uint16_t val);

	static constexpr int FLAG_J = 0x80;
	static constexpr int FLAG_Z = 0x40;
	static constexpr int FLAG_C = 0x20;
	static constexpr int FLAG_H = 0x10;

	void clear_JF() { m_F &= ~FLAG_J; }
	void clear_ZF() { m_F &= ~FLAG_Z; }
	void clear_CF() { m_F &= ~FLAG_C; }
	void clear_HF() { m_F &= ~FLAG_H; }

	void set_JF() { m_F |= FLAG_J; }
	void set_ZF() { m_F |= FLAG_Z; }
	void set_CF() { m_F |= FLAG_C; }
	void set_HF() { m_F |= FLAG_H; }

	int is_JF() const { return ((m_F & FLAG_J) ? 1 : 0); }
	int is_ZF() const { return ((m_F & FLAG_Z) ? 1 : 0); }
	int is_CF() const { return ((m_F & FLAG_C) ? 1 : 0); }
	int is_HF() const { return ((m_F & FLAG_H) ? 1 : 0); }

	int get_base_srcdst_cycles(int i) const { const int SRC_DST_CYCLES[8] = { 1, 2, 0, 0, 2, 2, 1, 1 }; return SRC_DST_CYCLES[i]; }

	bool stream_arg(std::ostream &stream, uint32_t pc, const char *pre, const uint16_t mode, const uint16_t r, const uint16_t rb);

	// tlcs870_ops.cpp
	void decode();

	void do_illegal(const uint8_t opbyte0);

	void do_NOP(const uint8_t opbyte0);
	void do_SWAP_A(const uint8_t opbyte0);
	void do_MUL_W_A(const uint8_t opbyte0);
	void do_DIV_WA_C(const uint8_t opbyte0);
	void do_RETI(const uint8_t opbyte0);
	void do_RET(const uint8_t opbyte0);
	void do_POP_PSW(const uint8_t opbyte0);
	void do_PUSH_PSW(const uint8_t opbyte0);

	void do_DAA_A(const uint8_t opbyte0);
	void do_DAS_A(const uint8_t opbyte0);
	void do_CLR_CF(const uint8_t opbyte0);
	void do_SET_CF(const uint8_t opbyte0);
	void do_CPL_CF(const uint8_t opbyte0);
	void do_LD_RBS_n(const uint8_t opbyte0);
	void do_INC_rr(const uint8_t opbyte0);
	void do_LD_rr_mn(const uint8_t opbyte0);
	void do_DEC_rr(const uint8_t opbyte0);
	void do_SHLC_A(const uint8_t opbyte0);
	void do_SHRC_A(const uint8_t opbyte0);
	void do_ROLC_A(const uint8_t opbyte0);
	void do_RORC_A(const uint8_t opbyte0);
	void do_INC_inx(const uint8_t opbyte0);
	void do_INC_inHL(const uint8_t opbyte0);
	void do_LD_A_inx(const uint8_t opbyte0);
	void do_LD_A_inHL(const uint8_t opbyte0);
	void do_LDW_inx_mn(const uint8_t opbyte0);
	void do_LDW_inHL_mn(const uint8_t opbyte0);
	void do_LD_inx_iny(const uint8_t opbyte0);

	void do_DEC_inx(const uint8_t opbyte0);
	void do_DEC_inHL(const uint8_t opbyte0);
	void do_LD_inx_A(const uint8_t opbyte0);
	void do_LD_inHL_A(const uint8_t opbyte0);
	void do_LD_inx_n(const uint8_t opbyte0);
	void do_LD_inHL_n(const uint8_t opbyte0);
	void do_CLR_inx(const uint8_t opbyte0);
	void do_CLR_inHL(const uint8_t opbyte0);
	void do_LD_r_n(const uint8_t opbyte0);

	void do_SET_inxbit(const uint8_t opbyte0);
	void do_CLR_inxbit(const uint8_t opbyte0);
	void do_LD_A_r(const uint8_t opbyte0);
	void do_LD_r_A(const uint8_t opbyte0);
	void do_INC_r(const uint8_t opbyte0);
	void do_DEC_r(const uint8_t opbyte0);
	void do_ALUOP_A_n(const uint8_t opbyte0);
	void do_ALUOP_A_inx(const uint8_t opbyte0);
	void do_JRS_T_a(const uint8_t opbyte0);
	void do_JRS_F_a(const uint8_t opbyte0);
	void do_CALLV_n(const uint8_t opbyte0);
	void do_JR_cc_a(const uint8_t opbyte0);
	void do_LD_CF_inxbit(const uint8_t opbyte0);

	void do_LD_SP_mn(const uint8_t opbyte0);
	void do_JR_a(const uint8_t opbyte0);
	void do_CALL_mn(const uint8_t opbyte0);
	void do_CALLP_n(const uint8_t opbyte0);
	void do_JP_mn(const uint8_t opbyte0);
	void do_ff_opcode(const uint8_t opbyte0);

	// tlcs870_ops_src.cpp

	void do_srcprefixtype_opcode(const uint8_t opbyte0);

	void do_e0_to_e7_oprand_illegal(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);

	void do_ROLD_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_RORD_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_rr_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_INC_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_inx_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_inHL_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_DEC_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_MCMP_insrc_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_SET_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_CLR_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_r_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_ALUOP_insrc_inHL(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_ALUOP_insrc_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_ALUOP_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_XCH_r_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_CPL_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_insrcbit_CF(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_XOR_CF_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_LD_CF_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_CALL_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);
	void do_JP_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr);

	// tlcs870_ops_dst.cpp
	void do_dstprefixtype_opcode(const uint8_t opbyte0);

	void do_f0_to_f7_oprand_illegal_opcode(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr);

	void do_LD_indst_rr(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr);
	void do_LD_indst_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr);
	void do_LD_indst_r(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr);

	// tlcs870_ops_reg.cpp

	void do_regprefixtype_opcode(const uint8_t opbyte0);

	void do_regprefixtype_oprand_illegal(const uint8_t opbyte0, const uint8_t opbyte1);

	void do_SWAP_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_MUL_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_DIV_gg_C(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_RETN(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_POP_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_PUSH_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_DAA_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_DAS_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_XCH_rr_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_rr_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_SHLC_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_SHRC_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ROLC_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_RORC_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_WA_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_gg_mn(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_SET_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CLR_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_r_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_A_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_g_A(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_ALUOP_g_n(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_SET_inppbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CLR_inppbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CPL_inpp_indirectbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_inpp_indirectbit_CF(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_CF_inpp_indirectbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_XCH_r_g(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CPL_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_gbit_CF(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_XOR_CF_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_CF_gbit(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_SP_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_LD_gg_SP(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_CALL_gg(const uint8_t opbyte0, const uint8_t opbyte1);
	void do_JP_gg(const uint8_t opbyte0, const uint8_t opbyte1);


	// ALU related
	uint16_t do_or(uint16_t param1, uint16_t param2);
	uint16_t do_xor(uint16_t param1, uint16_t param2);
	uint16_t do_and(uint16_t param1, uint16_t param2);

	uint8_t do_add_8bit(uint16_t param1, uint16_t param2);
	uint8_t do_sub_8bit(uint16_t param1, uint16_t param2);
	void do_cmp_8bit(uint16_t param1, uint16_t param2);
	uint8_t do_alu_8bit(int op, uint16_t param1, uint16_t param2);

	uint16_t do_add_16bit(uint32_t param1, uint32_t param2);
	uint16_t do_sub_16bit(uint32_t param1, uint32_t param2);
	void do_cmp_16bit(uint32_t param1, uint32_t param2);
	uint16_t do_alu_16bit(int op, uint32_t param1, uint32_t param2);

	// Generic opcode helpers
	void handle_div(const int reg);
	void handle_mul(const int reg);
	void handle_swap(const int reg);
	uint8_t handle_SHLC(uint8_t val);
	uint8_t handle_SHRC(uint8_t val);
	uint8_t handle_DAS(uint8_t val);
	uint8_t handle_DAA(uint8_t val);
	uint8_t handle_ROLC(uint8_t val);
	uint8_t handle_RORC(uint8_t val);

	const bool check_jump_condition(int param1);

	const uint8_t get_PSW();
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
