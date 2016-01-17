// license:BSD-3-Clause
// copyright-holders:Jason Eckhardt
/***************************************************************************

    i860.h

    Interface file for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)

***************************************************************************/

#pragma once

#ifndef __I860_H__
#define __I860_H__


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

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


class i860_cpu_device : public cpu_device
{
public:
	// construction/destruction
	i860_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	/* This is the external interface for asserting an external interrupt
	   to the i860.  */
	void i860_gen_interrupt();

	/* This is the external interface for asserting/deasserting a pin on
	   the i860.  */
	void i860_set_pin(int, int);

	/* Hard or soft reset.  */
	void reset_i860();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 8; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	void state_export(const device_state_entry &entry) override;
	void state_import(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	/* Integer registers (32 x 32-bits).  */
	UINT32 m_iregs[32];

	/* Floating point registers (32 x 32-bits, 16 x 64 bits, or 8 x 128 bits).
	   When referenced as pairs or quads, the higher numbered registers
	   are the upper bits. E.g., double precision f0 is f1:f0.  */
	UINT8 m_frg[32 * 4];

	/* Control registers (6 x 32-bits).  */
	UINT32 m_cregs[6];

	/* Program counter (1 x 32-bits).  Reset starts at pc=0xffffff00.  */
	UINT32 m_pc;

	/* Special registers (4 x 64-bits).  */
	union
	{
		float s;
		double d;
	} m_KR, m_KI, m_T;
	UINT64 m_merge;

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
	UINT32 m_freg[32];

	void writememi_emu (UINT32 addr, int size, UINT32 data);
	void fp_readmem_emu (UINT32 addr, int size, UINT8 *dest);
	void fp_writemem_emu (UINT32 addr, int size, UINT8 *data, UINT32 wmask);
	void unrecog_opcode (UINT32 pc, UINT32 insn);
	void insn_ld_ctrl (UINT32 insn);
	void insn_st_ctrl (UINT32 insn);
	void insn_ldx (UINT32 insn);
	void insn_stx (UINT32 insn);
	void insn_fsty (UINT32 insn);
	void insn_fldy (UINT32 insn);
	void insn_pstd (UINT32 insn);
	void insn_ixfr (UINT32 insn);
	void insn_addu (UINT32 insn);
	void insn_addu_imm (UINT32 insn);
	void insn_adds (UINT32 insn);
	void insn_adds_imm (UINT32 insn);
	void insn_subu (UINT32 insn);
	void insn_subu_imm (UINT32 insn);
	void insn_subs (UINT32 insn);
	void insn_subs_imm (UINT32 insn);
	void insn_shl (UINT32 insn);
	void insn_shl_imm (UINT32 insn);
	void insn_shr (UINT32 insn);
	void insn_shr_imm (UINT32 insn);
	void insn_shra (UINT32 insn);
	void insn_shra_imm (UINT32 insn);
	void insn_shrd (UINT32 insn);
	void insn_and (UINT32 insn);
	void insn_and_imm (UINT32 insn);
	void insn_andh_imm (UINT32 insn);
	void insn_andnot (UINT32 insn);
	void insn_andnot_imm (UINT32 insn);
	void insn_andnoth_imm (UINT32 insn);
	void insn_or (UINT32 insn);
	void insn_or_imm (UINT32 insn);
	void insn_orh_imm (UINT32 insn);
	void insn_xor (UINT32 insn);
	void insn_xor_imm (UINT32 insn);
	void insn_xorh_imm (UINT32 insn);
	void insn_trap (UINT32 insn);
	void insn_intovr (UINT32 insn);
	void insn_bte (UINT32 insn);
	void insn_bte_imm (UINT32 insn);
	void insn_btne (UINT32 insn);
	void insn_btne_imm (UINT32 insn);
	void insn_bc (UINT32 insn);
	void insn_bnc (UINT32 insn);
	void insn_bct (UINT32 insn);
	void insn_bnct (UINT32 insn);
	void insn_call (UINT32 insn);
	void insn_br (UINT32 insn);
	void insn_bri (UINT32 insn);
	void insn_calli (UINT32 insn);
	void insn_bla (UINT32 insn);
	void insn_flush (UINT32 insn);
	void insn_fmul (UINT32 insn);
	void insn_fmlow (UINT32 insn);
	void insn_fadd_sub (UINT32 insn);
	void insn_dualop (UINT32 insn);
	void insn_frcp (UINT32 insn);
	void insn_frsqr (UINT32 insn);
	void insn_fxfr (UINT32 insn);
	void insn_ftrunc (UINT32 insn);
	void insn_famov (UINT32 insn);
	void insn_fiadd_sub (UINT32 insn);
	void insn_fcmp (UINT32 insn);
	void insn_fzchk (UINT32 insn);
	void insn_form (UINT32 insn);
	void insn_faddp (UINT32 insn);
	void insn_faddz (UINT32 insn);
	void decode_exec (UINT32 insn, UINT32 non_shadow);
	float get_fregval_s (int fr);
	double get_fregval_d (int fr);
	void set_fregval_s (int fr, float s);
	void set_fregval_d (int fr, double d);
	int has_delay_slot(UINT32 insn);
	UINT32 ifetch (UINT32 pc);
	UINT32 get_address_translation (UINT32 vaddr, int is_dataref, int is_write);
	UINT32 readmemi_emu (UINT32 addr, int size);
	float get_fval_from_optype_s (UINT32 insn, int optype);
	double get_fval_from_optype_d (UINT32 insn, int optype);

	typedef void (i860_cpu_device::*insn_func)(UINT32);
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


extern const device_type I860;


#endif /* __I860_H__ */
