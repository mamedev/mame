// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    SPARC v7 emulator
*/

#ifndef __SPARC_H__
#define __SPARC_H__

#pragma once

#include "sparcdasm.h"

#define SPARCV8			(0)
#define LOG_FCODES		(0)

#if LOG_FCODES
#include <map>
#endif

#define SPARC_NO_TRAP						256
#define SPARC_RESET							0
#define SPARC_INSTRUCTION_ACCESS_EXCEPTION	1
#define SPARC_ILLEGAL_INSTRUCTION			2
#define SPARC_PRIVILEGED_INSTRUCTION		3
#define SPARC_FP_DISABLED					4
#define SPARC_WINDOW_OVERFLOW				5
#define SPARC_WINDOW_UNDERFLOW				6
#define SPARC_MEM_ADDRESS_NOT_ALIGNED		7
#define SPARC_FLOATING_POINT_EXCEPTION		8
#define SPARC_DATA_ACCESS_EXCEPTION			9
#define SPARC_TAG_OVERFLOW					10
#define SPARC_INT1							17
#define SPARC_INT2							18
#define SPARC_INT3							19
#define SPARC_INT4							20
#define SPARC_INT5							21
#define SPARC_INT6							22
#define SPARC_INT7							23
#define SPARC_INT8							24
#define SPARC_INT9							25
#define SPARC_INT10							26
#define SPARC_INT11							27
#define SPARC_INT12							28
#define SPARC_INT13							29
#define SPARC_INT14							30
#define SPARC_INT15							31
#define SPARC_TRAP_INSTRUCTION				128

#define SPARC_FPU_SEQUENCE_ERROR
// TODO: when there are more SPARC CPUs, move setter to a base class
#define MCFG_SPARC_ADD_ASI_DESC(desc) \
	mb86901_device::add_asi_desc(*device, desc);

class mb86901_device : public cpu_device, protected sparc_debug_state
{
public:
	mb86901_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	UINT8 get_asi() { return m_asi; }
	UINT32 pc() { return m_pc; }

	template<typename T> static void add_asi_desc(device_t &device, const T &desc) { return downcast<mb86901_device &>(device).add_asi_desc(desc); }

protected:
	template<typename T> void add_asi_desc(const T &desc) { m_dasm.add_asi_desc(desc); }

	void update_gpr_pointers();

	void execute_add(UINT32 op);
	void execute_taddcc(UINT32 op);
	void execute_sub(UINT32 op);
	void execute_tsubcc(UINT32 op);
	void execute_logical(UINT32 op);
	void execute_shift(UINT32 op);
	void execute_mulscc(UINT32 op);
	void execute_rdsr(UINT32 op);
	void execute_wrsr(UINT32 op);
	void execute_rett(UINT32 op);
	void execute_saverestore(UINT32 op);
	void execute_jmpl(UINT32 op);
#if SPARCV8
	void execute_mul(UINT32 op);
	void execute_div(UINT32 op);
#endif
	void execute_group2(UINT32 op);

	void execute_load(UINT32 op);
	void execute_store(UINT32 op);
	void execute_ldstub(UINT32 op);
#if SPARCV8
	void execute_swap(UINT32 op);
#endif
	void execute_group3(UINT32 op);

	bool evaluate_condition(UINT32 op);
	void execute_bicc(UINT32 op);
	void execute_ticc(UINT32 op);
	void select_trap();
	void execute_trap();

	void complete_instruction_execution(UINT32 op);
	void dispatch_instruction(UINT32 op);
	void complete_fp_execution(UINT32 /*op*/);
	void execute_step();

	void reset_step();
	void error_step();

#if LOG_FCODES
	void indent();
	void disassemble_ss1_fcode(UINT32 r5, UINT32 opcode, UINT32 handler_base, UINT32 entry_point, UINT32 stack);
#endif

	// address spaces
	const address_space_config m_program_config;

	// memory access
	UINT32 read_sized_word(UINT8 asi, UINT32 address, int size);
	void write_sized_word(UINT8 asi, UINT32 address, UINT32 data, int size);

	// helpers for the disassembler
	virtual UINT64 get_reg_r(unsigned index) const override;
	virtual UINT64 get_translated_pc() const override;
	virtual UINT8 get_icc() const override;
	virtual UINT8 get_xcc() const override;
	virtual UINT8 get_fcc(unsigned index) const override;

	// general-purpose registers
	UINT32 m_r[120];

	// FPU registers
	UINT32 m_fpr[32];
	UINT32 m_fsr;
	UINT8 m_ftt;

	// control/status registers
	UINT32 m_pc;
	UINT32 m_npc;
	UINT32 m_psr;
	UINT32 m_wim;
	UINT32 m_tbr;
	UINT32 m_y;

	bool m_bp_reset_in;
	UINT8 m_bp_irl;
	bool m_bp_fpu_present;
	bool m_bp_cp_present;
	bool m_pb_error;
	bool m_pb_block_ldst_byte;
	bool m_pb_block_ldst_word;
	UINT16 m_irq_state;

	// trap and error registers
	bool m_trap;
	UINT8 m_tt;
	UINT8 m_ticc_trap_type;
	UINT8 m_interrupt_level;
	bool m_privileged_instruction;
	bool m_illegal_instruction;
	bool m_mem_address_not_aligned;
	bool m_fp_disabled;
	bool m_fp_exception;
	bool m_cp_disabled; // SPARCv8
	bool m_cp_exception; // SPARCv8
	bool m_unimplemented_FLUSH; // SPARCv8
	bool m_r_register_access_error; // SPARCv8
	bool m_instruction_access_error; // SPARCv8
	bool m_instruction_access_exception;
	bool m_data_access_error; // SPARCv8
	bool m_data_store_error; // SPARCv8
	bool m_data_access_exception;
	bool m_division_by_zero; // SPARCv8
	bool m_trap_instruction;
	bool m_window_underflow;
	bool m_window_overflow;
	bool m_tag_overflow;
	bool m_reset_mode;
	bool m_reset_trap;
	bool m_execute_mode;
	bool m_error_mode;
	UINT8 m_fpu_sequence_err;
	UINT8 m_cp_sequence_err;

	// fields separated out from PSR (Processor State Register)
	UINT8 m_impl;   // implementation (always 0 in MB86901)
	UINT8 m_ver;    // version (always 0 in MB86901)
	UINT8 m_icc;    // integer condition codes
	bool m_ec;      // enable coprocessor
	bool m_ef;      // enable FPU
	UINT8 m_pil;    // processor interrupt level
	bool m_s;       // supervisor mode
	bool m_ps;      // prior S state
	bool m_et;      // enable traps
	UINT8 m_cwp;    // current window pointer

	bool m_alu_op3_assigned[64];
	bool m_ldst_op3_assigned[64];

	// register windowing helpers
	UINT32* m_regs[32];

	// addressing helpers
	UINT8 m_asi;

	// other internal states
	bool m_privileged_asr[32];
	bool m_illegal_instruction_asr[32];
	bool m_mae;
	bool m_annul;
	bool m_hold_bus;
	int m_icount;

	// debugger helpers
	UINT32 m_dbgregs[24];
	sparc_disassembler m_dasm;

	// address spaces
	address_space *m_program;

#if LOG_FCODES
	UINT32 m_ss1_next_pc;
	UINT32 m_ss1_next_opcode;
	UINT32 m_ss1_next_handler_base;
	UINT32 m_ss1_next_entry_point;
	UINT32 m_ss1_next_stack;
	std::map<UINT16, std::string> m_ss1_fcode_table;
#endif

	// processor configuration
	static const int NWINDOWS;
};

// device type definition
extern const device_type MB86901;

enum
{
	SPARC_PC = 1,
	SPARC_NPC,
	SPARC_PSR,
	SPARC_WIM,
	SPARC_TBR,
	SPARC_Y,

	SPARC_ANNUL,
	SPARC_ICC,
	SPARC_CWP,

	SPARC_G0,   SPARC_G1,   SPARC_G2,   SPARC_G3,   SPARC_G4,   SPARC_G5,   SPARC_G6,   SPARC_G7,
	SPARC_O0,   SPARC_O1,   SPARC_O2,   SPARC_O3,   SPARC_O4,   SPARC_O5,   SPARC_O6,   SPARC_O7,
	SPARC_L0,   SPARC_L1,   SPARC_L2,   SPARC_L3,   SPARC_L4,   SPARC_L5,   SPARC_L6,   SPARC_L7,
	SPARC_I0,   SPARC_I1,   SPARC_I2,   SPARC_I3,   SPARC_I4,   SPARC_I5,   SPARC_I6,   SPARC_I7,

	SPARC_EC,
	SPARC_EF,
	SPARC_ET,
	SPARC_PIL,
	SPARC_S,
	SPARC_PS,

	SPARC_R0,   SPARC_R1,   SPARC_R2,   SPARC_R3,   SPARC_R4,   SPARC_R5,   SPARC_R6,   SPARC_R7,   SPARC_R8,   SPARC_R9,   SPARC_R10,  SPARC_R11,  SPARC_R12,  SPARC_R13,  SPARC_R14,  SPARC_R15,
	SPARC_R16,  SPARC_R17,  SPARC_R18,  SPARC_R19,  SPARC_R20,  SPARC_R21,  SPARC_R22,  SPARC_R23,  SPARC_R24,  SPARC_R25,  SPARC_R26,  SPARC_R27,  SPARC_R28,  SPARC_R29,  SPARC_R30,  SPARC_R31,
	SPARC_R32,  SPARC_R33,  SPARC_R34,  SPARC_R35,  SPARC_R36,  SPARC_R37,  SPARC_R38,  SPARC_R39,  SPARC_R40,  SPARC_R41,  SPARC_R42,  SPARC_R43,  SPARC_R44,  SPARC_R45,  SPARC_R46,  SPARC_R47,
	SPARC_R48,  SPARC_R49,  SPARC_R50,  SPARC_R51,  SPARC_R52,  SPARC_R53,  SPARC_R54,  SPARC_R55,  SPARC_R56,  SPARC_R57,  SPARC_R58,  SPARC_R59,  SPARC_R60,  SPARC_R61,  SPARC_R62,  SPARC_R63,
	SPARC_R64,  SPARC_R65,  SPARC_R66,  SPARC_R67,  SPARC_R68,  SPARC_R69,  SPARC_R70,  SPARC_R71,  SPARC_R72,  SPARC_R73,  SPARC_R74,  SPARC_R75,  SPARC_R76,  SPARC_R77,  SPARC_R78,  SPARC_R79,
	SPARC_R80,  SPARC_R81,  SPARC_R82,  SPARC_R83,  SPARC_R84,  SPARC_R85,  SPARC_R86,  SPARC_R87,  SPARC_R88,  SPARC_R89,  SPARC_R90,  SPARC_R91,  SPARC_R92,  SPARC_R93,  SPARC_R94,  SPARC_R95,
	SPARC_R96,  SPARC_R97,  SPARC_R98,  SPARC_R99,  SPARC_R100, SPARC_R101, SPARC_R102, SPARC_R103, SPARC_R104, SPARC_R105, SPARC_R106, SPARC_R107, SPARC_R108, SPARC_R109, SPARC_R110, SPARC_R111
};

enum
{
	SPARC_IRQ1,
	SPARC_IRQ2,
	SPARC_IRQ3,
	SPARC_IRQ4,
	SPARC_IRQ5,
	SPARC_IRQ6,
	SPARC_IRQ7,
	SPARC_IRQ8,
	SPARC_IRQ9,
	SPARC_IRQ10,
	SPARC_IRQ11,
	SPARC_IRQ12,
	SPARC_IRQ13,
	SPARC_IRQ14,
	SPARC_NMI,
	SPARC_MAE
};

#endif /* __SPARC_H__ */
