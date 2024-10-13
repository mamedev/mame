// license:BSD-3-Clause
// copyright-holders:Jason Eckhardt
/***************************************************************************

    i860.h

    Interface file for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)

***************************************************************************/

#ifndef MAME_CPU_I860_I860_H
#define MAME_CPU_I860_I860_H

#pragma once


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

class i860_cpu_device : public cpu_device
{
public:
	// construction/destruction
	i860_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	/* This is the external interface for asserting an external interrupt
	   to the i860.  */
	void i860_gen_interrupt();

	/* This is the external interface for asserting/deasserting a pin on
	   the i860.  */
	void i860_set_pin(int, int);

	/* Hard or soft reset.  */
	void reset_i860();

protected:
	enum
	{
		I860_PC = 1,

		I860_FIR,
		I860_PSR,
		I860_DIRBASE,
		I860_DB,
		I860_FSR,
		I860_EPSR,

		I860_R0,  I860_R1,  I860_R2,  I860_R3,  I860_R4,  I860_R5,  I860_R6,  I860_R7,  I860_R8,  I860_R9,
		I860_R10, I860_R11, I860_R12, I860_R13, I860_R14, I860_R15, I860_R16, I860_R17, I860_R18, I860_R19,
		I860_R20, I860_R21, I860_R22, I860_R23, I860_R24, I860_R25, I860_R26, I860_R27, I860_R28, I860_R29,
		I860_R30, I860_R31,

		I860_F0,  I860_F1,  I860_F2,  I860_F3,  I860_F4,  I860_F5,  I860_F6,  I860_F7,  I860_F8,  I860_F9,
		I860_F10, I860_F11, I860_F12, I860_F13, I860_F14, I860_F15, I860_F16, I860_F17, I860_F18, I860_F19,
		I860_F20, I860_F21, I860_F22, I860_F23, I860_F24, I860_F25, I860_F26, I860_F27, I860_F28, I860_F29,
		I860_F30, I860_F31
	};


	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 8; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_import(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	/* Integer registers (32 x 32-bits).  */
	uint32_t m_iregs[32];

	/* Floating point registers (32 x 32-bits, 16 x 64 bits, or 8 x 128 bits).
	   When referenced as pairs or quads, the higher numbered registers
	   are the upper bits. E.g., double precision f0 is f1:f0.  */
	uint8_t m_frg[32 * 4];

	/* Control registers (6 x 32-bits).  */
	uint32_t m_cregs[6];

	/* Program counter (1 x 32-bits).  Reset starts at pc=0xffffff00.  */
	uint32_t m_pc;

	/* Special registers (4 x 64-bits).  */
	union
	{
		float s;
		double d;
	} m_KR, m_KI, m_T;
	uint64_t m_merge;

	/* The adder pipeline, always 3 stages.  */
	struct
	{
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Adder result precision (1 = dbl, 0 = sgl).  */
			char arp;
		} stat;
	} m_A[3];

	/* The multiplier pipeline. 3 stages for single precision, 2 stages
	   for double precision, and confusing for mixed precision.  */
	struct {
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Multiplier result precision (1 = dbl, 0 = sgl).  */
			char mrp;
		} stat;
	} m_M[3];

	/* The load pipeline, always 3 stages.  */
	struct {
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Load result precision (1 = dbl, 0 = sgl).  */
			char lrp;
		} stat;
	} m_L[3];

	/* The graphics/integer pipeline, always 1 stage.  */
	struct {
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Integer/graphics result precision (1 = dbl, 0 = sgl).  */
			char irp;
		} stat;
	} m_G;

	/* Pins.  */
	int m_pin_bus_hold;
	int m_pin_reset;

	/*
	 * Other emulator state.
	 */
	int m_exiting_readmem;
	int m_exiting_ifetch;

	/* Indicate a control-flow instruction, so we know the PC is updated.  */
	int m_pc_updated;

	/* Indicate an instruction just generated a trap, so we know the PC
	   needs to go to the trap address.  */
	int m_pending_trap;

	/* This is 1 if the next fir load gets the trap address, otherwise
	   it is 0 to get the ld.c address.  This is set to 1 only when a
	   non-reset trap occurs.  */
	int m_fir_gets_trap_addr;

	/* Single stepping flag for internal use.  */
	int m_single_stepping;

	/*
	 * MAME-specific stuff.
	 */
	address_space *m_program;
	int m_icount;
	// For debugger
	uint32_t m_freg[32];

	void writememi_emu (uint32_t addr, int size, uint32_t data);
	void fp_readmem_emu (uint32_t addr, int size, uint8_t *dest);
	void fp_writemem_emu (uint32_t addr, int size, uint8_t *data, uint32_t wmask);
	void unrecog_opcode (uint32_t pc, uint32_t insn);
	void insn_ld_ctrl (uint32_t insn);
	void insn_st_ctrl (uint32_t insn);
	void insn_ldx (uint32_t insn);
	void insn_stx (uint32_t insn);
	void insn_fsty (uint32_t insn);
	void insn_fldy (uint32_t insn);
	void insn_pstd (uint32_t insn);
	void insn_ixfr (uint32_t insn);
	void insn_addu (uint32_t insn);
	void insn_addu_imm (uint32_t insn);
	void insn_adds (uint32_t insn);
	void insn_adds_imm (uint32_t insn);
	void insn_subu (uint32_t insn);
	void insn_subu_imm (uint32_t insn);
	void insn_subs (uint32_t insn);
	void insn_subs_imm (uint32_t insn);
	void insn_shl (uint32_t insn);
	void insn_shl_imm (uint32_t insn);
	void insn_shr (uint32_t insn);
	void insn_shr_imm (uint32_t insn);
	void insn_shra (uint32_t insn);
	void insn_shra_imm (uint32_t insn);
	void insn_shrd (uint32_t insn);
	void insn_and (uint32_t insn);
	void insn_and_imm (uint32_t insn);
	void insn_andh_imm (uint32_t insn);
	void insn_andnot (uint32_t insn);
	void insn_andnot_imm (uint32_t insn);
	void insn_andnoth_imm (uint32_t insn);
	void insn_or (uint32_t insn);
	void insn_or_imm (uint32_t insn);
	void insn_orh_imm (uint32_t insn);
	void insn_xor (uint32_t insn);
	void insn_xor_imm (uint32_t insn);
	void insn_xorh_imm (uint32_t insn);
	void insn_trap (uint32_t insn);
	void insn_intovr (uint32_t insn);
	void insn_bte (uint32_t insn);
	void insn_bte_imm (uint32_t insn);
	void insn_btne (uint32_t insn);
	void insn_btne_imm (uint32_t insn);
	void insn_bc (uint32_t insn);
	void insn_bnc (uint32_t insn);
	void insn_bct (uint32_t insn);
	void insn_bnct (uint32_t insn);
	void insn_call (uint32_t insn);
	void insn_br (uint32_t insn);
	void insn_bri (uint32_t insn);
	void insn_calli (uint32_t insn);
	void insn_bla (uint32_t insn);
	void insn_flush (uint32_t insn);
	void insn_fmul (uint32_t insn);
	void insn_fmlow (uint32_t insn);
	void insn_fadd_sub (uint32_t insn);
	void insn_dualop (uint32_t insn);
	void insn_frcp (uint32_t insn);
	void insn_frsqr (uint32_t insn);
	void insn_fxfr (uint32_t insn);
	void insn_ftrunc (uint32_t insn);
	void insn_famov (uint32_t insn);
	void insn_fiadd_sub (uint32_t insn);
	void insn_fcmp (uint32_t insn);
	void insn_fzchk (uint32_t insn);
	void insn_form (uint32_t insn);
	void insn_faddp (uint32_t insn);
	void insn_faddz (uint32_t insn);
	void decode_exec (uint32_t insn, uint32_t non_shadow);
	float get_fregval_s (int fr);
	double get_fregval_d (int fr);
	void set_fregval_s (int fr, float s);
	void set_fregval_d (int fr, double d);
	int has_delay_slot(uint32_t insn);
	uint32_t ifetch (uint32_t pc);
	uint32_t get_address_translation (uint32_t vaddr, int is_dataref, int is_write);
	uint32_t readmemi_emu (uint32_t addr, int size);
	float get_fval_from_optype_s (uint32_t insn, int optype);
	double get_fval_from_optype_d (uint32_t insn, int optype);

	typedef void (i860_cpu_device::*insn_func)(uint32_t);
	struct decode_tbl_t
	{
		/* Execute function for this opcode.  */
		insn_func insn_exec;
		/* Flags for this opcode.  */
		char flags;
	};
	static const decode_tbl_t decode_tbl[64];
	static const decode_tbl_t core_esc_decode_tbl[8];
	static const decode_tbl_t fp_decode_tbl[128];
};


/* i860 pins.  */
enum {
	DEC_PIN_BUS_HOLD,       /* Bus HOLD pin.      */
	DEC_PIN_RESET           /* System reset pin.  */
};


DECLARE_DEVICE_TYPE(I860, i860_cpu_device)

#endif // MAME_CPU_I860_I860_H
