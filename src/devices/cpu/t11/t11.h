// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** T-11: Portable DEC T-11 emulator ******************************************/

#ifndef MAME_CPU_T11_T11_H
#define MAME_CPU_T11_T11_H

#pragma once

#include "machine/z80daisy.h"


enum
{
	T11_R0=1, T11_R1, T11_R2, T11_R3, T11_R4, T11_R5, T11_SP, T11_PC, T11_PSW
};


class t11_device :  public cpu_device
{
public:
	// T11 input lines
	enum
	{
		CP0_LINE = 0,           // -AI4 (at PI time)
		CP1_LINE = 1,           // -AI3 (at PI time)
		CP2_LINE = 2,           // -AI2 (at PI time)
		CP3_LINE = 3,           // -AI1 (at PI time)
		VEC_LINE = 4,           // -AI5 (at PI time)
		PF_LINE = 5,            // -AI6 (at PI time)
		HLT_LINE = 6            // -AI7 (at PI time)
	};

	// generic hardware traps
	static constexpr uint8_t POWER_FAIL = PF_LINE;
	static constexpr uint8_t BUS_ERROR = 8;

	// construction/destruction
	t11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_initial_mode(const uint16_t mode) { c_initial_mode = mode; }
	auto out_reset() { return m_out_reset_func.bind(); }
	auto in_iack() { return m_in_iack_func.bind(); }

protected:
	enum
	{
		T11_RESERVED    = 000,  // Reserved vector
		T11_TIMEOUT     = 004,  // Time-out/system error vector
		T11_ILLINST     = 010,  // Illegal and reserved instruction vector
		T11_BPT         = 014,  // BPT instruction vector
		T11_IOT         = 020,  // IOT instruction vector
		T11_PWRFAIL     = 024,  // Power fail vector
		T11_EMT         = 030,  // EMT instruction vector
		T11_TRAP        = 034,  // TRAP instruction vector
		// K1801 vectors
		VM1_EVNT        = 0100, // EVNT pin vector
		VM1_IRQ3        = 0270, // IRQ3 pin vector
		VM1_HALT        = 0160002, // HALT instruction vector
		VM2_HALT        = 0170  // HALT instruction vector
	};
	// K1801 microcode constants
	enum
	{
		VM1_STACK       = 0177674,  // start of HALT mode save area
		VM1_SEL1        = 0177716,  // DIP switch register (read) and HALT mode selector (write)
		SEL1_HALT       = 010,
		MCIR_ILL        = -2,
		MCIR_SET        = -1,
		MCIR_WAIT       = 0,
		MCIR_NONE       = 1,
		MCIR_HALT       = 2,
		MCIR_IRQ        = 3
	};
	enum
	{
		// DEC command set extensions
		IS_LEIS     = 1 << 0,   // MARK, RTT, SOB, SXT, XOR
		IS_EIS      = 1 << 1,   // same plus ASH, ASHC, MUL, DIV
		IS_MFPT     = 1 << 2,   // MFPT
		IS_MXPS     = 1 << 3,   // MFPS, MTPS
		IS_T11      = 1 << 4,   // LEIS without MARK
		// K1801 command set extensions
		IS_VM1      = 1 << 5,   // START, STEP
		IS_VM2      = 1 << 6,   // same plus RSEL, MxUS, RCPx, WCPx
	};

	t11_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 12; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 114; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == PF_LINE || inputnum == HLT_LINE || inputnum == BUS_ERROR; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;

	uint16_t c_initial_mode;
	uint16_t c_insn_set;

	PAIR                m_ppc;    /* previous program counter */
	PAIR                m_reg[8];
	PAIR                m_psw;
	uint16_t            m_initial_pc;
	uint8_t             m_wait_state;
	int8_t              m_mcir;
	uint16_t            m_vsel;
	uint8_t             m_cp_state;
	bool                m_vec_active;
	bool                m_pf_active;
	bool                m_berr_active;
	bool                m_hlt_active;
	bool                m_power_fail;
	bool                m_bus_error;
	bool                m_ext_halt;
	bool                m_check_irqs;
	bool                m_trace_trap;
	int                 m_icount;
	memory_access<16, 1, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<16, 1, 0, ENDIANNESS_LITTLE>::specific m_program;

	devcb_write_line    m_out_reset_func;
	devcb_read8         m_in_iack_func;

	inline int ROPCODE();
	inline int RBYTE(int addr);
	inline void WBYTE(int addr, int data);
	inline int RWORD(int addr);
	inline void WWORD(int addr, int data);
	inline void PUSH(int val);
	inline int POP();

	virtual void t11_check_irqs();
	void take_interrupt(uint8_t vector);
	void trap_to(uint16_t vector);

	typedef void ( t11_device::*opcode_func )(uint16_t op);
	static const opcode_func s_opcode_table[65536 >> 3];

	void op_0000(uint16_t op);
	void op_0001(uint16_t op);
	void halt(uint16_t op);
	void illegal(uint16_t op);
	void illegal4(uint16_t op);
	void jmp_rgd(uint16_t op);
	void jmp_in(uint16_t op);
	void jmp_ind(uint16_t op);
	void jmp_de(uint16_t op);
	void jmp_ded(uint16_t op);
	void jmp_ix(uint16_t op);
	void jmp_ixd(uint16_t op);
	void rts(uint16_t op);
	void ccc(uint16_t op);
	void scc(uint16_t op);
	void swab_rg(uint16_t op);
	void swab_rgd(uint16_t op);
	void swab_in(uint16_t op);
	void swab_ind(uint16_t op);
	void swab_de(uint16_t op);
	void swab_ded(uint16_t op);
	void swab_ix(uint16_t op);
	void swab_ixd(uint16_t op);
	void br(uint16_t op);
	void bne(uint16_t op);
	void beq(uint16_t op);
	void bge(uint16_t op);
	void blt(uint16_t op);
	void bgt(uint16_t op);
	void ble(uint16_t op);
	void jsr_rgd(uint16_t op);
	void jsr_in(uint16_t op);
	void jsr_ind(uint16_t op);
	void jsr_de(uint16_t op);
	void jsr_ded(uint16_t op);
	void jsr_ix(uint16_t op);
	void jsr_ixd(uint16_t op);
	void clr_rg(uint16_t op);
	void clr_rgd(uint16_t op);
	void clr_in(uint16_t op);
	void clr_ind(uint16_t op);
	void clr_de(uint16_t op);
	void clr_ded(uint16_t op);
	void clr_ix(uint16_t op);
	void clr_ixd(uint16_t op);
	void com_rg(uint16_t op);
	void com_rgd(uint16_t op);
	void com_in(uint16_t op);
	void com_ind(uint16_t op);
	void com_de(uint16_t op);
	void com_ded(uint16_t op);
	void com_ix(uint16_t op);
	void com_ixd(uint16_t op);
	void inc_rg(uint16_t op);
	void inc_rgd(uint16_t op);
	void inc_in(uint16_t op);
	void inc_ind(uint16_t op);
	void inc_de(uint16_t op);
	void inc_ded(uint16_t op);
	void inc_ix(uint16_t op);
	void inc_ixd(uint16_t op);
	void dec_rg(uint16_t op);
	void dec_rgd(uint16_t op);
	void dec_in(uint16_t op);
	void dec_ind(uint16_t op);
	void dec_de(uint16_t op);
	void dec_ded(uint16_t op);
	void dec_ix(uint16_t op);
	void dec_ixd(uint16_t op);
	void neg_rg(uint16_t op);
	void neg_rgd(uint16_t op);
	void neg_in(uint16_t op);
	void neg_ind(uint16_t op);
	void neg_de(uint16_t op);
	void neg_ded(uint16_t op);
	void neg_ix(uint16_t op);
	void neg_ixd(uint16_t op);
	void adc_rg(uint16_t op);
	void adc_rgd(uint16_t op);
	void adc_in(uint16_t op);
	void adc_ind(uint16_t op);
	void adc_de(uint16_t op);
	void adc_ded(uint16_t op);
	void adc_ix(uint16_t op);
	void adc_ixd(uint16_t op);
	void sbc_rg(uint16_t op);
	void sbc_rgd(uint16_t op);
	void sbc_in(uint16_t op);
	void sbc_ind(uint16_t op);
	void sbc_de(uint16_t op);
	void sbc_ded(uint16_t op);
	void sbc_ix(uint16_t op);
	void sbc_ixd(uint16_t op);
	void tst_rg(uint16_t op);
	void tst_rgd(uint16_t op);
	void tst_in(uint16_t op);
	void tst_ind(uint16_t op);
	void tst_de(uint16_t op);
	void tst_ded(uint16_t op);
	void tst_ix(uint16_t op);
	void tst_ixd(uint16_t op);
	void ror_rg(uint16_t op);
	void ror_rgd(uint16_t op);
	void ror_in(uint16_t op);
	void ror_ind(uint16_t op);
	void ror_de(uint16_t op);
	void ror_ded(uint16_t op);
	void ror_ix(uint16_t op);
	void ror_ixd(uint16_t op);
	void rol_rg(uint16_t op);
	void rol_rgd(uint16_t op);
	void rol_in(uint16_t op);
	void rol_ind(uint16_t op);
	void rol_de(uint16_t op);
	void rol_ded(uint16_t op);
	void rol_ix(uint16_t op);
	void rol_ixd(uint16_t op);
	void asr_rg(uint16_t op);
	void asr_rgd(uint16_t op);
	void asr_in(uint16_t op);
	void asr_ind(uint16_t op);
	void asr_de(uint16_t op);
	void asr_ded(uint16_t op);
	void asr_ix(uint16_t op);
	void asr_ixd(uint16_t op);
	void asl_rg(uint16_t op);
	void asl_rgd(uint16_t op);
	void asl_in(uint16_t op);
	void asl_ind(uint16_t op);
	void asl_de(uint16_t op);
	void asl_ded(uint16_t op);
	void asl_ix(uint16_t op);
	void asl_ixd(uint16_t op);
	void mark(uint16_t op);
	void sxt_rg(uint16_t op);
	void sxt_rgd(uint16_t op);
	void sxt_in(uint16_t op);
	void sxt_ind(uint16_t op);
	void sxt_de(uint16_t op);
	void sxt_ded(uint16_t op);
	void sxt_ix(uint16_t op);
	void sxt_ixd(uint16_t op);
	void mov_rg_rg(uint16_t op);
	void mov_rg_rgd(uint16_t op);
	void mov_rg_in(uint16_t op);
	void mov_rg_ind(uint16_t op);
	void mov_rg_de(uint16_t op);
	void mov_rg_ded(uint16_t op);
	void mov_rg_ix(uint16_t op);
	void mov_rg_ixd(uint16_t op);
	void mov_rgd_rg(uint16_t op);
	void mov_rgd_rgd(uint16_t op);
	void mov_rgd_in(uint16_t op);
	void mov_rgd_ind(uint16_t op);
	void mov_rgd_de(uint16_t op);
	void mov_rgd_ded(uint16_t op);
	void mov_rgd_ix(uint16_t op);
	void mov_rgd_ixd(uint16_t op);
	void mov_in_rg(uint16_t op);
	void mov_in_rgd(uint16_t op);
	void mov_in_in(uint16_t op);
	void mov_in_ind(uint16_t op);
	void mov_in_de(uint16_t op);
	void mov_in_ded(uint16_t op);
	void mov_in_ix(uint16_t op);
	void mov_in_ixd(uint16_t op);
	void mov_ind_rg(uint16_t op);
	void mov_ind_rgd(uint16_t op);
	void mov_ind_in(uint16_t op);
	void mov_ind_ind(uint16_t op);
	void mov_ind_de(uint16_t op);
	void mov_ind_ded(uint16_t op);
	void mov_ind_ix(uint16_t op);
	void mov_ind_ixd(uint16_t op);
	void mov_de_rg(uint16_t op);
	void mov_de_rgd(uint16_t op);
	void mov_de_in(uint16_t op);
	void mov_de_ind(uint16_t op);
	void mov_de_de(uint16_t op);
	void mov_de_ded(uint16_t op);
	void mov_de_ix(uint16_t op);
	void mov_de_ixd(uint16_t op);
	void mov_ded_rg(uint16_t op);
	void mov_ded_rgd(uint16_t op);
	void mov_ded_in(uint16_t op);
	void mov_ded_ind(uint16_t op);
	void mov_ded_de(uint16_t op);
	void mov_ded_ded(uint16_t op);
	void mov_ded_ix(uint16_t op);
	void mov_ded_ixd(uint16_t op);
	void mov_ix_rg(uint16_t op);
	void mov_ix_rgd(uint16_t op);
	void mov_ix_in(uint16_t op);
	void mov_ix_ind(uint16_t op);
	void mov_ix_de(uint16_t op);
	void mov_ix_ded(uint16_t op);
	void mov_ix_ix(uint16_t op);
	void mov_ix_ixd(uint16_t op);
	void mov_ixd_rg(uint16_t op);
	void mov_ixd_rgd(uint16_t op);
	void mov_ixd_in(uint16_t op);
	void mov_ixd_ind(uint16_t op);
	void mov_ixd_de(uint16_t op);
	void mov_ixd_ded(uint16_t op);
	void mov_ixd_ix(uint16_t op);
	void mov_ixd_ixd(uint16_t op);
	void cmp_rg_rg(uint16_t op);
	void cmp_rg_rgd(uint16_t op);
	void cmp_rg_in(uint16_t op);
	void cmp_rg_ind(uint16_t op);
	void cmp_rg_de(uint16_t op);
	void cmp_rg_ded(uint16_t op);
	void cmp_rg_ix(uint16_t op);
	void cmp_rg_ixd(uint16_t op);
	void cmp_rgd_rg(uint16_t op);
	void cmp_rgd_rgd(uint16_t op);
	void cmp_rgd_in(uint16_t op);
	void cmp_rgd_ind(uint16_t op);
	void cmp_rgd_de(uint16_t op);
	void cmp_rgd_ded(uint16_t op);
	void cmp_rgd_ix(uint16_t op);
	void cmp_rgd_ixd(uint16_t op);
	void cmp_in_rg(uint16_t op);
	void cmp_in_rgd(uint16_t op);
	void cmp_in_in(uint16_t op);
	void cmp_in_ind(uint16_t op);
	void cmp_in_de(uint16_t op);
	void cmp_in_ded(uint16_t op);
	void cmp_in_ix(uint16_t op);
	void cmp_in_ixd(uint16_t op);
	void cmp_ind_rg(uint16_t op);
	void cmp_ind_rgd(uint16_t op);
	void cmp_ind_in(uint16_t op);
	void cmp_ind_ind(uint16_t op);
	void cmp_ind_de(uint16_t op);
	void cmp_ind_ded(uint16_t op);
	void cmp_ind_ix(uint16_t op);
	void cmp_ind_ixd(uint16_t op);
	void cmp_de_rg(uint16_t op);
	void cmp_de_rgd(uint16_t op);
	void cmp_de_in(uint16_t op);
	void cmp_de_ind(uint16_t op);
	void cmp_de_de(uint16_t op);
	void cmp_de_ded(uint16_t op);
	void cmp_de_ix(uint16_t op);
	void cmp_de_ixd(uint16_t op);
	void cmp_ded_rg(uint16_t op);
	void cmp_ded_rgd(uint16_t op);
	void cmp_ded_in(uint16_t op);
	void cmp_ded_ind(uint16_t op);
	void cmp_ded_de(uint16_t op);
	void cmp_ded_ded(uint16_t op);
	void cmp_ded_ix(uint16_t op);
	void cmp_ded_ixd(uint16_t op);
	void cmp_ix_rg(uint16_t op);
	void cmp_ix_rgd(uint16_t op);
	void cmp_ix_in(uint16_t op);
	void cmp_ix_ind(uint16_t op);
	void cmp_ix_de(uint16_t op);
	void cmp_ix_ded(uint16_t op);
	void cmp_ix_ix(uint16_t op);
	void cmp_ix_ixd(uint16_t op);
	void cmp_ixd_rg(uint16_t op);
	void cmp_ixd_rgd(uint16_t op);
	void cmp_ixd_in(uint16_t op);
	void cmp_ixd_ind(uint16_t op);
	void cmp_ixd_de(uint16_t op);
	void cmp_ixd_ded(uint16_t op);
	void cmp_ixd_ix(uint16_t op);
	void cmp_ixd_ixd(uint16_t op);
	void bit_rg_rg(uint16_t op);
	void bit_rg_rgd(uint16_t op);
	void bit_rg_in(uint16_t op);
	void bit_rg_ind(uint16_t op);
	void bit_rg_de(uint16_t op);
	void bit_rg_ded(uint16_t op);
	void bit_rg_ix(uint16_t op);
	void bit_rg_ixd(uint16_t op);
	void bit_rgd_rg(uint16_t op);
	void bit_rgd_rgd(uint16_t op);
	void bit_rgd_in(uint16_t op);
	void bit_rgd_ind(uint16_t op);
	void bit_rgd_de(uint16_t op);
	void bit_rgd_ded(uint16_t op);
	void bit_rgd_ix(uint16_t op);
	void bit_rgd_ixd(uint16_t op);
	void bit_in_rg(uint16_t op);
	void bit_in_rgd(uint16_t op);
	void bit_in_in(uint16_t op);
	void bit_in_ind(uint16_t op);
	void bit_in_de(uint16_t op);
	void bit_in_ded(uint16_t op);
	void bit_in_ix(uint16_t op);
	void bit_in_ixd(uint16_t op);
	void bit_ind_rg(uint16_t op);
	void bit_ind_rgd(uint16_t op);
	void bit_ind_in(uint16_t op);
	void bit_ind_ind(uint16_t op);
	void bit_ind_de(uint16_t op);
	void bit_ind_ded(uint16_t op);
	void bit_ind_ix(uint16_t op);
	void bit_ind_ixd(uint16_t op);
	void bit_de_rg(uint16_t op);
	void bit_de_rgd(uint16_t op);
	void bit_de_in(uint16_t op);
	void bit_de_ind(uint16_t op);
	void bit_de_de(uint16_t op);
	void bit_de_ded(uint16_t op);
	void bit_de_ix(uint16_t op);
	void bit_de_ixd(uint16_t op);
	void bit_ded_rg(uint16_t op);
	void bit_ded_rgd(uint16_t op);
	void bit_ded_in(uint16_t op);
	void bit_ded_ind(uint16_t op);
	void bit_ded_de(uint16_t op);
	void bit_ded_ded(uint16_t op);
	void bit_ded_ix(uint16_t op);
	void bit_ded_ixd(uint16_t op);
	void bit_ix_rg(uint16_t op);
	void bit_ix_rgd(uint16_t op);
	void bit_ix_in(uint16_t op);
	void bit_ix_ind(uint16_t op);
	void bit_ix_de(uint16_t op);
	void bit_ix_ded(uint16_t op);
	void bit_ix_ix(uint16_t op);
	void bit_ix_ixd(uint16_t op);
	void bit_ixd_rg(uint16_t op);
	void bit_ixd_rgd(uint16_t op);
	void bit_ixd_in(uint16_t op);
	void bit_ixd_ind(uint16_t op);
	void bit_ixd_de(uint16_t op);
	void bit_ixd_ded(uint16_t op);
	void bit_ixd_ix(uint16_t op);
	void bit_ixd_ixd(uint16_t op);
	void bic_rg_rg(uint16_t op);
	void bic_rg_rgd(uint16_t op);
	void bic_rg_in(uint16_t op);
	void bic_rg_ind(uint16_t op);
	void bic_rg_de(uint16_t op);
	void bic_rg_ded(uint16_t op);
	void bic_rg_ix(uint16_t op);
	void bic_rg_ixd(uint16_t op);
	void bic_rgd_rg(uint16_t op);
	void bic_rgd_rgd(uint16_t op);
	void bic_rgd_in(uint16_t op);
	void bic_rgd_ind(uint16_t op);
	void bic_rgd_de(uint16_t op);
	void bic_rgd_ded(uint16_t op);
	void bic_rgd_ix(uint16_t op);
	void bic_rgd_ixd(uint16_t op);
	void bic_in_rg(uint16_t op);
	void bic_in_rgd(uint16_t op);
	void bic_in_in(uint16_t op);
	void bic_in_ind(uint16_t op);
	void bic_in_de(uint16_t op);
	void bic_in_ded(uint16_t op);
	void bic_in_ix(uint16_t op);
	void bic_in_ixd(uint16_t op);
	void bic_ind_rg(uint16_t op);
	void bic_ind_rgd(uint16_t op);
	void bic_ind_in(uint16_t op);
	void bic_ind_ind(uint16_t op);
	void bic_ind_de(uint16_t op);
	void bic_ind_ded(uint16_t op);
	void bic_ind_ix(uint16_t op);
	void bic_ind_ixd(uint16_t op);
	void bic_de_rg(uint16_t op);
	void bic_de_rgd(uint16_t op);
	void bic_de_in(uint16_t op);
	void bic_de_ind(uint16_t op);
	void bic_de_de(uint16_t op);
	void bic_de_ded(uint16_t op);
	void bic_de_ix(uint16_t op);
	void bic_de_ixd(uint16_t op);
	void bic_ded_rg(uint16_t op);
	void bic_ded_rgd(uint16_t op);
	void bic_ded_in(uint16_t op);
	void bic_ded_ind(uint16_t op);
	void bic_ded_de(uint16_t op);
	void bic_ded_ded(uint16_t op);
	void bic_ded_ix(uint16_t op);
	void bic_ded_ixd(uint16_t op);
	void bic_ix_rg(uint16_t op);
	void bic_ix_rgd(uint16_t op);
	void bic_ix_in(uint16_t op);
	void bic_ix_ind(uint16_t op);
	void bic_ix_de(uint16_t op);
	void bic_ix_ded(uint16_t op);
	void bic_ix_ix(uint16_t op);
	void bic_ix_ixd(uint16_t op);
	void bic_ixd_rg(uint16_t op);
	void bic_ixd_rgd(uint16_t op);
	void bic_ixd_in(uint16_t op);
	void bic_ixd_ind(uint16_t op);
	void bic_ixd_de(uint16_t op);
	void bic_ixd_ded(uint16_t op);
	void bic_ixd_ix(uint16_t op);
	void bic_ixd_ixd(uint16_t op);
	void bis_rg_rg(uint16_t op);
	void bis_rg_rgd(uint16_t op);
	void bis_rg_in(uint16_t op);
	void bis_rg_ind(uint16_t op);
	void bis_rg_de(uint16_t op);
	void bis_rg_ded(uint16_t op);
	void bis_rg_ix(uint16_t op);
	void bis_rg_ixd(uint16_t op);
	void bis_rgd_rg(uint16_t op);
	void bis_rgd_rgd(uint16_t op);
	void bis_rgd_in(uint16_t op);
	void bis_rgd_ind(uint16_t op);
	void bis_rgd_de(uint16_t op);
	void bis_rgd_ded(uint16_t op);
	void bis_rgd_ix(uint16_t op);
	void bis_rgd_ixd(uint16_t op);
	void bis_in_rg(uint16_t op);
	void bis_in_rgd(uint16_t op);
	void bis_in_in(uint16_t op);
	void bis_in_ind(uint16_t op);
	void bis_in_de(uint16_t op);
	void bis_in_ded(uint16_t op);
	void bis_in_ix(uint16_t op);
	void bis_in_ixd(uint16_t op);
	void bis_ind_rg(uint16_t op);
	void bis_ind_rgd(uint16_t op);
	void bis_ind_in(uint16_t op);
	void bis_ind_ind(uint16_t op);
	void bis_ind_de(uint16_t op);
	void bis_ind_ded(uint16_t op);
	void bis_ind_ix(uint16_t op);
	void bis_ind_ixd(uint16_t op);
	void bis_de_rg(uint16_t op);
	void bis_de_rgd(uint16_t op);
	void bis_de_in(uint16_t op);
	void bis_de_ind(uint16_t op);
	void bis_de_de(uint16_t op);
	void bis_de_ded(uint16_t op);
	void bis_de_ix(uint16_t op);
	void bis_de_ixd(uint16_t op);
	void bis_ded_rg(uint16_t op);
	void bis_ded_rgd(uint16_t op);
	void bis_ded_in(uint16_t op);
	void bis_ded_ind(uint16_t op);
	void bis_ded_de(uint16_t op);
	void bis_ded_ded(uint16_t op);
	void bis_ded_ix(uint16_t op);
	void bis_ded_ixd(uint16_t op);
	void bis_ix_rg(uint16_t op);
	void bis_ix_rgd(uint16_t op);
	void bis_ix_in(uint16_t op);
	void bis_ix_ind(uint16_t op);
	void bis_ix_de(uint16_t op);
	void bis_ix_ded(uint16_t op);
	void bis_ix_ix(uint16_t op);
	void bis_ix_ixd(uint16_t op);
	void bis_ixd_rg(uint16_t op);
	void bis_ixd_rgd(uint16_t op);
	void bis_ixd_in(uint16_t op);
	void bis_ixd_ind(uint16_t op);
	void bis_ixd_de(uint16_t op);
	void bis_ixd_ded(uint16_t op);
	void bis_ixd_ix(uint16_t op);
	void bis_ixd_ixd(uint16_t op);
	void add_rg_rg(uint16_t op);
	void add_rg_rgd(uint16_t op);
	void add_rg_in(uint16_t op);
	void add_rg_ind(uint16_t op);
	void add_rg_de(uint16_t op);
	void add_rg_ded(uint16_t op);
	void add_rg_ix(uint16_t op);
	void add_rg_ixd(uint16_t op);
	void add_rgd_rg(uint16_t op);
	void add_rgd_rgd(uint16_t op);
	void add_rgd_in(uint16_t op);
	void add_rgd_ind(uint16_t op);
	void add_rgd_de(uint16_t op);
	void add_rgd_ded(uint16_t op);
	void add_rgd_ix(uint16_t op);
	void add_rgd_ixd(uint16_t op);
	void add_in_rg(uint16_t op);
	void add_in_rgd(uint16_t op);
	void add_in_in(uint16_t op);
	void add_in_ind(uint16_t op);
	void add_in_de(uint16_t op);
	void add_in_ded(uint16_t op);
	void add_in_ix(uint16_t op);
	void add_in_ixd(uint16_t op);
	void add_ind_rg(uint16_t op);
	void add_ind_rgd(uint16_t op);
	void add_ind_in(uint16_t op);
	void add_ind_ind(uint16_t op);
	void add_ind_de(uint16_t op);
	void add_ind_ded(uint16_t op);
	void add_ind_ix(uint16_t op);
	void add_ind_ixd(uint16_t op);
	void add_de_rg(uint16_t op);
	void add_de_rgd(uint16_t op);
	void add_de_in(uint16_t op);
	void add_de_ind(uint16_t op);
	void add_de_de(uint16_t op);
	void add_de_ded(uint16_t op);
	void add_de_ix(uint16_t op);
	void add_de_ixd(uint16_t op);
	void add_ded_rg(uint16_t op);
	void add_ded_rgd(uint16_t op);
	void add_ded_in(uint16_t op);
	void add_ded_ind(uint16_t op);
	void add_ded_de(uint16_t op);
	void add_ded_ded(uint16_t op);
	void add_ded_ix(uint16_t op);
	void add_ded_ixd(uint16_t op);
	void add_ix_rg(uint16_t op);
	void add_ix_rgd(uint16_t op);
	void add_ix_in(uint16_t op);
	void add_ix_ind(uint16_t op);
	void add_ix_de(uint16_t op);
	void add_ix_ded(uint16_t op);
	void add_ix_ix(uint16_t op);
	void add_ix_ixd(uint16_t op);
	void add_ixd_rg(uint16_t op);
	void add_ixd_rgd(uint16_t op);
	void add_ixd_in(uint16_t op);
	void add_ixd_ind(uint16_t op);
	void add_ixd_de(uint16_t op);
	void add_ixd_ded(uint16_t op);
	void add_ixd_ix(uint16_t op);
	void add_ixd_ixd(uint16_t op);
	void xor_rg(uint16_t op);
	void xor_rgd(uint16_t op);
	void xor_in(uint16_t op);
	void xor_ind(uint16_t op);
	void xor_de(uint16_t op);
	void xor_ded(uint16_t op);
	void xor_ix(uint16_t op);
	void xor_ixd(uint16_t op);
	void sob(uint16_t op);
	void bpl(uint16_t op);
	void bmi(uint16_t op);
	void bhi(uint16_t op);
	void blos(uint16_t op);
	void bvc(uint16_t op);
	void bvs(uint16_t op);
	void bcc(uint16_t op);
	void bcs(uint16_t op);
	void emt(uint16_t op);
	void trap(uint16_t op);
	void clrb_rg(uint16_t op);
	void clrb_rgd(uint16_t op);
	void clrb_in(uint16_t op);
	void clrb_ind(uint16_t op);
	void clrb_de(uint16_t op);
	void clrb_ded(uint16_t op);
	void clrb_ix(uint16_t op);
	void clrb_ixd(uint16_t op);
	void comb_rg(uint16_t op);
	void comb_rgd(uint16_t op);
	void comb_in(uint16_t op);
	void comb_ind(uint16_t op);
	void comb_de(uint16_t op);
	void comb_ded(uint16_t op);
	void comb_ix(uint16_t op);
	void comb_ixd(uint16_t op);
	void incb_rg(uint16_t op);
	void incb_rgd(uint16_t op);
	void incb_in(uint16_t op);
	void incb_ind(uint16_t op);
	void incb_de(uint16_t op);
	void incb_ded(uint16_t op);
	void incb_ix(uint16_t op);
	void incb_ixd(uint16_t op);
	void decb_rg(uint16_t op);
	void decb_rgd(uint16_t op);
	void decb_in(uint16_t op);
	void decb_ind(uint16_t op);
	void decb_de(uint16_t op);
	void decb_ded(uint16_t op);
	void decb_ix(uint16_t op);
	void decb_ixd(uint16_t op);
	void negb_rg(uint16_t op);
	void negb_rgd(uint16_t op);
	void negb_in(uint16_t op);
	void negb_ind(uint16_t op);
	void negb_de(uint16_t op);
	void negb_ded(uint16_t op);
	void negb_ix(uint16_t op);
	void negb_ixd(uint16_t op);
	void adcb_rg(uint16_t op);
	void adcb_rgd(uint16_t op);
	void adcb_in(uint16_t op);
	void adcb_ind(uint16_t op);
	void adcb_de(uint16_t op);
	void adcb_ded(uint16_t op);
	void adcb_ix(uint16_t op);
	void adcb_ixd(uint16_t op);
	void sbcb_rg(uint16_t op);
	void sbcb_rgd(uint16_t op);
	void sbcb_in(uint16_t op);
	void sbcb_ind(uint16_t op);
	void sbcb_de(uint16_t op);
	void sbcb_ded(uint16_t op);
	void sbcb_ix(uint16_t op);
	void sbcb_ixd(uint16_t op);
	void tstb_rg(uint16_t op);
	void tstb_rgd(uint16_t op);
	void tstb_in(uint16_t op);
	void tstb_ind(uint16_t op);
	void tstb_de(uint16_t op);
	void tstb_ded(uint16_t op);
	void tstb_ix(uint16_t op);
	void tstb_ixd(uint16_t op);
	void rorb_rg(uint16_t op);
	void rorb_rgd(uint16_t op);
	void rorb_in(uint16_t op);
	void rorb_ind(uint16_t op);
	void rorb_de(uint16_t op);
	void rorb_ded(uint16_t op);
	void rorb_ix(uint16_t op);
	void rorb_ixd(uint16_t op);
	void rolb_rg(uint16_t op);
	void rolb_rgd(uint16_t op);
	void rolb_in(uint16_t op);
	void rolb_ind(uint16_t op);
	void rolb_de(uint16_t op);
	void rolb_ded(uint16_t op);
	void rolb_ix(uint16_t op);
	void rolb_ixd(uint16_t op);
	void asrb_rg(uint16_t op);
	void asrb_rgd(uint16_t op);
	void asrb_in(uint16_t op);
	void asrb_ind(uint16_t op);
	void asrb_de(uint16_t op);
	void asrb_ded(uint16_t op);
	void asrb_ix(uint16_t op);
	void asrb_ixd(uint16_t op);
	void aslb_rg(uint16_t op);
	void aslb_rgd(uint16_t op);
	void aslb_in(uint16_t op);
	void aslb_ind(uint16_t op);
	void aslb_de(uint16_t op);
	void aslb_ded(uint16_t op);
	void aslb_ix(uint16_t op);
	void aslb_ixd(uint16_t op);
	void mtps_rg(uint16_t op);
	void mtps_rgd(uint16_t op);
	void mtps_in(uint16_t op);
	void mtps_ind(uint16_t op);
	void mtps_de(uint16_t op);
	void mtps_ded(uint16_t op);
	void mtps_ix(uint16_t op);
	void mtps_ixd(uint16_t op);
	void mfps_rg(uint16_t op);
	void mfps_rgd(uint16_t op);
	void mfps_in(uint16_t op);
	void mfps_ind(uint16_t op);
	void mfps_de(uint16_t op);
	void mfps_ded(uint16_t op);
	void mfps_ix(uint16_t op);
	void mfps_ixd(uint16_t op);
	void movb_rg_rg(uint16_t op);
	void movb_rg_rgd(uint16_t op);
	void movb_rg_in(uint16_t op);
	void movb_rg_ind(uint16_t op);
	void movb_rg_de(uint16_t op);
	void movb_rg_ded(uint16_t op);
	void movb_rg_ix(uint16_t op);
	void movb_rg_ixd(uint16_t op);
	void movb_rgd_rg(uint16_t op);
	void movb_rgd_rgd(uint16_t op);
	void movb_rgd_in(uint16_t op);
	void movb_rgd_ind(uint16_t op);
	void movb_rgd_de(uint16_t op);
	void movb_rgd_ded(uint16_t op);
	void movb_rgd_ix(uint16_t op);
	void movb_rgd_ixd(uint16_t op);
	void movb_in_rg(uint16_t op);
	void movb_in_rgd(uint16_t op);
	void movb_in_in(uint16_t op);
	void movb_in_ind(uint16_t op);
	void movb_in_de(uint16_t op);
	void movb_in_ded(uint16_t op);
	void movb_in_ix(uint16_t op);
	void movb_in_ixd(uint16_t op);
	void movb_ind_rg(uint16_t op);
	void movb_ind_rgd(uint16_t op);
	void movb_ind_in(uint16_t op);
	void movb_ind_ind(uint16_t op);
	void movb_ind_de(uint16_t op);
	void movb_ind_ded(uint16_t op);
	void movb_ind_ix(uint16_t op);
	void movb_ind_ixd(uint16_t op);
	void movb_de_rg(uint16_t op);
	void movb_de_rgd(uint16_t op);
	void movb_de_in(uint16_t op);
	void movb_de_ind(uint16_t op);
	void movb_de_de(uint16_t op);
	void movb_de_ded(uint16_t op);
	void movb_de_ix(uint16_t op);
	void movb_de_ixd(uint16_t op);
	void movb_ded_rg(uint16_t op);
	void movb_ded_rgd(uint16_t op);
	void movb_ded_in(uint16_t op);
	void movb_ded_ind(uint16_t op);
	void movb_ded_de(uint16_t op);
	void movb_ded_ded(uint16_t op);
	void movb_ded_ix(uint16_t op);
	void movb_ded_ixd(uint16_t op);
	void movb_ix_rg(uint16_t op);
	void movb_ix_rgd(uint16_t op);
	void movb_ix_in(uint16_t op);
	void movb_ix_ind(uint16_t op);
	void movb_ix_de(uint16_t op);
	void movb_ix_ded(uint16_t op);
	void movb_ix_ix(uint16_t op);
	void movb_ix_ixd(uint16_t op);
	void movb_ixd_rg(uint16_t op);
	void movb_ixd_rgd(uint16_t op);
	void movb_ixd_in(uint16_t op);
	void movb_ixd_ind(uint16_t op);
	void movb_ixd_de(uint16_t op);
	void movb_ixd_ded(uint16_t op);
	void movb_ixd_ix(uint16_t op);
	void movb_ixd_ixd(uint16_t op);
	void cmpb_rg_rg(uint16_t op);
	void cmpb_rg_rgd(uint16_t op);
	void cmpb_rg_in(uint16_t op);
	void cmpb_rg_ind(uint16_t op);
	void cmpb_rg_de(uint16_t op);
	void cmpb_rg_ded(uint16_t op);
	void cmpb_rg_ix(uint16_t op);
	void cmpb_rg_ixd(uint16_t op);
	void cmpb_rgd_rg(uint16_t op);
	void cmpb_rgd_rgd(uint16_t op);
	void cmpb_rgd_in(uint16_t op);
	void cmpb_rgd_ind(uint16_t op);
	void cmpb_rgd_de(uint16_t op);
	void cmpb_rgd_ded(uint16_t op);
	void cmpb_rgd_ix(uint16_t op);
	void cmpb_rgd_ixd(uint16_t op);
	void cmpb_in_rg(uint16_t op);
	void cmpb_in_rgd(uint16_t op);
	void cmpb_in_in(uint16_t op);
	void cmpb_in_ind(uint16_t op);
	void cmpb_in_de(uint16_t op);
	void cmpb_in_ded(uint16_t op);
	void cmpb_in_ix(uint16_t op);
	void cmpb_in_ixd(uint16_t op);
	void cmpb_ind_rg(uint16_t op);
	void cmpb_ind_rgd(uint16_t op);
	void cmpb_ind_in(uint16_t op);
	void cmpb_ind_ind(uint16_t op);
	void cmpb_ind_de(uint16_t op);
	void cmpb_ind_ded(uint16_t op);
	void cmpb_ind_ix(uint16_t op);
	void cmpb_ind_ixd(uint16_t op);
	void cmpb_de_rg(uint16_t op);
	void cmpb_de_rgd(uint16_t op);
	void cmpb_de_in(uint16_t op);
	void cmpb_de_ind(uint16_t op);
	void cmpb_de_de(uint16_t op);
	void cmpb_de_ded(uint16_t op);
	void cmpb_de_ix(uint16_t op);
	void cmpb_de_ixd(uint16_t op);
	void cmpb_ded_rg(uint16_t op);
	void cmpb_ded_rgd(uint16_t op);
	void cmpb_ded_in(uint16_t op);
	void cmpb_ded_ind(uint16_t op);
	void cmpb_ded_de(uint16_t op);
	void cmpb_ded_ded(uint16_t op);
	void cmpb_ded_ix(uint16_t op);
	void cmpb_ded_ixd(uint16_t op);
	void cmpb_ix_rg(uint16_t op);
	void cmpb_ix_rgd(uint16_t op);
	void cmpb_ix_in(uint16_t op);
	void cmpb_ix_ind(uint16_t op);
	void cmpb_ix_de(uint16_t op);
	void cmpb_ix_ded(uint16_t op);
	void cmpb_ix_ix(uint16_t op);
	void cmpb_ix_ixd(uint16_t op);
	void cmpb_ixd_rg(uint16_t op);
	void cmpb_ixd_rgd(uint16_t op);
	void cmpb_ixd_in(uint16_t op);
	void cmpb_ixd_ind(uint16_t op);
	void cmpb_ixd_de(uint16_t op);
	void cmpb_ixd_ded(uint16_t op);
	void cmpb_ixd_ix(uint16_t op);
	void cmpb_ixd_ixd(uint16_t op);
	void bitb_rg_rg(uint16_t op);
	void bitb_rg_rgd(uint16_t op);
	void bitb_rg_in(uint16_t op);
	void bitb_rg_ind(uint16_t op);
	void bitb_rg_de(uint16_t op);
	void bitb_rg_ded(uint16_t op);
	void bitb_rg_ix(uint16_t op);
	void bitb_rg_ixd(uint16_t op);
	void bitb_rgd_rg(uint16_t op);
	void bitb_rgd_rgd(uint16_t op);
	void bitb_rgd_in(uint16_t op);
	void bitb_rgd_ind(uint16_t op);
	void bitb_rgd_de(uint16_t op);
	void bitb_rgd_ded(uint16_t op);
	void bitb_rgd_ix(uint16_t op);
	void bitb_rgd_ixd(uint16_t op);
	void bitb_in_rg(uint16_t op);
	void bitb_in_rgd(uint16_t op);
	void bitb_in_in(uint16_t op);
	void bitb_in_ind(uint16_t op);
	void bitb_in_de(uint16_t op);
	void bitb_in_ded(uint16_t op);
	void bitb_in_ix(uint16_t op);
	void bitb_in_ixd(uint16_t op);
	void bitb_ind_rg(uint16_t op);
	void bitb_ind_rgd(uint16_t op);
	void bitb_ind_in(uint16_t op);
	void bitb_ind_ind(uint16_t op);
	void bitb_ind_de(uint16_t op);
	void bitb_ind_ded(uint16_t op);
	void bitb_ind_ix(uint16_t op);
	void bitb_ind_ixd(uint16_t op);
	void bitb_de_rg(uint16_t op);
	void bitb_de_rgd(uint16_t op);
	void bitb_de_in(uint16_t op);
	void bitb_de_ind(uint16_t op);
	void bitb_de_de(uint16_t op);
	void bitb_de_ded(uint16_t op);
	void bitb_de_ix(uint16_t op);
	void bitb_de_ixd(uint16_t op);
	void bitb_ded_rg(uint16_t op);
	void bitb_ded_rgd(uint16_t op);
	void bitb_ded_in(uint16_t op);
	void bitb_ded_ind(uint16_t op);
	void bitb_ded_de(uint16_t op);
	void bitb_ded_ded(uint16_t op);
	void bitb_ded_ix(uint16_t op);
	void bitb_ded_ixd(uint16_t op);
	void bitb_ix_rg(uint16_t op);
	void bitb_ix_rgd(uint16_t op);
	void bitb_ix_in(uint16_t op);
	void bitb_ix_ind(uint16_t op);
	void bitb_ix_de(uint16_t op);
	void bitb_ix_ded(uint16_t op);
	void bitb_ix_ix(uint16_t op);
	void bitb_ix_ixd(uint16_t op);
	void bitb_ixd_rg(uint16_t op);
	void bitb_ixd_rgd(uint16_t op);
	void bitb_ixd_in(uint16_t op);
	void bitb_ixd_ind(uint16_t op);
	void bitb_ixd_de(uint16_t op);
	void bitb_ixd_ded(uint16_t op);
	void bitb_ixd_ix(uint16_t op);
	void bitb_ixd_ixd(uint16_t op);
	void bicb_rg_rg(uint16_t op);
	void bicb_rg_rgd(uint16_t op);
	void bicb_rg_in(uint16_t op);
	void bicb_rg_ind(uint16_t op);
	void bicb_rg_de(uint16_t op);
	void bicb_rg_ded(uint16_t op);
	void bicb_rg_ix(uint16_t op);
	void bicb_rg_ixd(uint16_t op);
	void bicb_rgd_rg(uint16_t op);
	void bicb_rgd_rgd(uint16_t op);
	void bicb_rgd_in(uint16_t op);
	void bicb_rgd_ind(uint16_t op);
	void bicb_rgd_de(uint16_t op);
	void bicb_rgd_ded(uint16_t op);
	void bicb_rgd_ix(uint16_t op);
	void bicb_rgd_ixd(uint16_t op);
	void bicb_in_rg(uint16_t op);
	void bicb_in_rgd(uint16_t op);
	void bicb_in_in(uint16_t op);
	void bicb_in_ind(uint16_t op);
	void bicb_in_de(uint16_t op);
	void bicb_in_ded(uint16_t op);
	void bicb_in_ix(uint16_t op);
	void bicb_in_ixd(uint16_t op);
	void bicb_ind_rg(uint16_t op);
	void bicb_ind_rgd(uint16_t op);
	void bicb_ind_in(uint16_t op);
	void bicb_ind_ind(uint16_t op);
	void bicb_ind_de(uint16_t op);
	void bicb_ind_ded(uint16_t op);
	void bicb_ind_ix(uint16_t op);
	void bicb_ind_ixd(uint16_t op);
	void bicb_de_rg(uint16_t op);
	void bicb_de_rgd(uint16_t op);
	void bicb_de_in(uint16_t op);
	void bicb_de_ind(uint16_t op);
	void bicb_de_de(uint16_t op);
	void bicb_de_ded(uint16_t op);
	void bicb_de_ix(uint16_t op);
	void bicb_de_ixd(uint16_t op);
	void bicb_ded_rg(uint16_t op);
	void bicb_ded_rgd(uint16_t op);
	void bicb_ded_in(uint16_t op);
	void bicb_ded_ind(uint16_t op);
	void bicb_ded_de(uint16_t op);
	void bicb_ded_ded(uint16_t op);
	void bicb_ded_ix(uint16_t op);
	void bicb_ded_ixd(uint16_t op);
	void bicb_ix_rg(uint16_t op);
	void bicb_ix_rgd(uint16_t op);
	void bicb_ix_in(uint16_t op);
	void bicb_ix_ind(uint16_t op);
	void bicb_ix_de(uint16_t op);
	void bicb_ix_ded(uint16_t op);
	void bicb_ix_ix(uint16_t op);
	void bicb_ix_ixd(uint16_t op);
	void bicb_ixd_rg(uint16_t op);
	void bicb_ixd_rgd(uint16_t op);
	void bicb_ixd_in(uint16_t op);
	void bicb_ixd_ind(uint16_t op);
	void bicb_ixd_de(uint16_t op);
	void bicb_ixd_ded(uint16_t op);
	void bicb_ixd_ix(uint16_t op);
	void bicb_ixd_ixd(uint16_t op);
	void bisb_rg_rg(uint16_t op);
	void bisb_rg_rgd(uint16_t op);
	void bisb_rg_in(uint16_t op);
	void bisb_rg_ind(uint16_t op);
	void bisb_rg_de(uint16_t op);
	void bisb_rg_ded(uint16_t op);
	void bisb_rg_ix(uint16_t op);
	void bisb_rg_ixd(uint16_t op);
	void bisb_rgd_rg(uint16_t op);
	void bisb_rgd_rgd(uint16_t op);
	void bisb_rgd_in(uint16_t op);
	void bisb_rgd_ind(uint16_t op);
	void bisb_rgd_de(uint16_t op);
	void bisb_rgd_ded(uint16_t op);
	void bisb_rgd_ix(uint16_t op);
	void bisb_rgd_ixd(uint16_t op);
	void bisb_in_rg(uint16_t op);
	void bisb_in_rgd(uint16_t op);
	void bisb_in_in(uint16_t op);
	void bisb_in_ind(uint16_t op);
	void bisb_in_de(uint16_t op);
	void bisb_in_ded(uint16_t op);
	void bisb_in_ix(uint16_t op);
	void bisb_in_ixd(uint16_t op);
	void bisb_ind_rg(uint16_t op);
	void bisb_ind_rgd(uint16_t op);
	void bisb_ind_in(uint16_t op);
	void bisb_ind_ind(uint16_t op);
	void bisb_ind_de(uint16_t op);
	void bisb_ind_ded(uint16_t op);
	void bisb_ind_ix(uint16_t op);
	void bisb_ind_ixd(uint16_t op);
	void bisb_de_rg(uint16_t op);
	void bisb_de_rgd(uint16_t op);
	void bisb_de_in(uint16_t op);
	void bisb_de_ind(uint16_t op);
	void bisb_de_de(uint16_t op);
	void bisb_de_ded(uint16_t op);
	void bisb_de_ix(uint16_t op);
	void bisb_de_ixd(uint16_t op);
	void bisb_ded_rg(uint16_t op);
	void bisb_ded_rgd(uint16_t op);
	void bisb_ded_in(uint16_t op);
	void bisb_ded_ind(uint16_t op);
	void bisb_ded_de(uint16_t op);
	void bisb_ded_ded(uint16_t op);
	void bisb_ded_ix(uint16_t op);
	void bisb_ded_ixd(uint16_t op);
	void bisb_ix_rg(uint16_t op);
	void bisb_ix_rgd(uint16_t op);
	void bisb_ix_in(uint16_t op);
	void bisb_ix_ind(uint16_t op);
	void bisb_ix_de(uint16_t op);
	void bisb_ix_ded(uint16_t op);
	void bisb_ix_ix(uint16_t op);
	void bisb_ix_ixd(uint16_t op);
	void bisb_ixd_rg(uint16_t op);
	void bisb_ixd_rgd(uint16_t op);
	void bisb_ixd_in(uint16_t op);
	void bisb_ixd_ind(uint16_t op);
	void bisb_ixd_de(uint16_t op);
	void bisb_ixd_ded(uint16_t op);
	void bisb_ixd_ix(uint16_t op);
	void bisb_ixd_ixd(uint16_t op);
	void sub_rg_rg(uint16_t op);
	void sub_rg_rgd(uint16_t op);
	void sub_rg_in(uint16_t op);
	void sub_rg_ind(uint16_t op);
	void sub_rg_de(uint16_t op);
	void sub_rg_ded(uint16_t op);
	void sub_rg_ix(uint16_t op);
	void sub_rg_ixd(uint16_t op);
	void sub_rgd_rg(uint16_t op);
	void sub_rgd_rgd(uint16_t op);
	void sub_rgd_in(uint16_t op);
	void sub_rgd_ind(uint16_t op);
	void sub_rgd_de(uint16_t op);
	void sub_rgd_ded(uint16_t op);
	void sub_rgd_ix(uint16_t op);
	void sub_rgd_ixd(uint16_t op);
	void sub_in_rg(uint16_t op);
	void sub_in_rgd(uint16_t op);
	void sub_in_in(uint16_t op);
	void sub_in_ind(uint16_t op);
	void sub_in_de(uint16_t op);
	void sub_in_ded(uint16_t op);
	void sub_in_ix(uint16_t op);
	void sub_in_ixd(uint16_t op);
	void sub_ind_rg(uint16_t op);
	void sub_ind_rgd(uint16_t op);
	void sub_ind_in(uint16_t op);
	void sub_ind_ind(uint16_t op);
	void sub_ind_de(uint16_t op);
	void sub_ind_ded(uint16_t op);
	void sub_ind_ix(uint16_t op);
	void sub_ind_ixd(uint16_t op);
	void sub_de_rg(uint16_t op);
	void sub_de_rgd(uint16_t op);
	void sub_de_in(uint16_t op);
	void sub_de_ind(uint16_t op);
	void sub_de_de(uint16_t op);
	void sub_de_ded(uint16_t op);
	void sub_de_ix(uint16_t op);
	void sub_de_ixd(uint16_t op);
	void sub_ded_rg(uint16_t op);
	void sub_ded_rgd(uint16_t op);
	void sub_ded_in(uint16_t op);
	void sub_ded_ind(uint16_t op);
	void sub_ded_de(uint16_t op);
	void sub_ded_ded(uint16_t op);
	void sub_ded_ix(uint16_t op);
	void sub_ded_ixd(uint16_t op);
	void sub_ix_rg(uint16_t op);
	void sub_ix_rgd(uint16_t op);
	void sub_ix_in(uint16_t op);
	void sub_ix_ind(uint16_t op);
	void sub_ix_de(uint16_t op);
	void sub_ix_ded(uint16_t op);
	void sub_ix_ix(uint16_t op);
	void sub_ix_ixd(uint16_t op);
	void sub_ixd_rg(uint16_t op);
	void sub_ixd_rgd(uint16_t op);
	void sub_ixd_in(uint16_t op);
	void sub_ixd_ind(uint16_t op);
	void sub_ixd_de(uint16_t op);
	void sub_ixd_ded(uint16_t op);
	void sub_ixd_ix(uint16_t op);
	void sub_ixd_ixd(uint16_t op);
};

class k1801vm1_device : public t11_device, public z80_daisy_chain_interface
{
public:
	// construction/destruction
	k1801vm1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual void t11_check_irqs() override;
	void take_interrupt_halt(uint16_t vector);
};


class k1801vm2_device : public t11_device, public z80_daisy_chain_interface
{
public:
	// construction/destruction
	k1801vm2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
};


DECLARE_DEVICE_TYPE(T11,      t11_device)
DECLARE_DEVICE_TYPE(K1801VM1, k1801vm1_device)
DECLARE_DEVICE_TYPE(K1801VM2, k1801vm2_device)

#endif // MAME_CPU_T11_T11_H
