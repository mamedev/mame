// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    SPARC v7 emulator
*/

#pragma once

#ifndef __SPARC_H__
#define __SPARC_H__

#define AS_USER_INSN		AS_0
#define AS_SUPER_INSN		AS_1
#define AS_USER_DATA		AS_2
#define AS_SUPER_DATA		AS_3

class mb86901_device : public cpu_device
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

	UINT8 fetch_asi() { return m_asi; }
	UINT32 pc() { return m_pc; }

protected:
	void queue_trap(UINT8 type, UINT8 tt_override = 0);
	bool invoke_queued_traps();
	bool check_main_traps(UINT32 op, bool privileged, UINT32 alignment, UINT8 registeralign, bool noimmediate);

	void update_gpr_pointers();
	void save_restore_update_cwp(UINT32 op, UINT8 new_cwp);
	bool execute_group2(UINT32 op);
	void execute_group3(UINT32 op);
	bool execute_bicc(UINT32 op);
	bool execute_ticc(UINT32 op);
	bool evaluate_condition(UINT32 op);

	// address spaces
	const address_space_config m_as8_config;
	const address_space_config m_as9_config;
	const address_space_config m_as10_config;
	const address_space_config m_as11_config;

	// memory access
	UINT32 read_byte(UINT8 asi, UINT32 address);
	INT32 read_signed_byte(UINT8 asi, UINT32 address);
	UINT32 read_half(UINT8 asi, UINT32 address);
	INT32 read_signed_half(UINT8 asi, UINT32 address);
	UINT32 read_word(UINT8 asi, UINT32 address);
	UINT64 read_doubleword(UINT8 asi, UINT32 address);
	void write_byte(UINT8 asi, UINT32 address, UINT8 data);
	void write_half(UINT8 asi, UINT32 address, UINT16 data);
	void write_word(UINT8 asi, UINT32 address, UINT32 data);
	void write_doubleword(UINT8 asi, UINT32 address, UINT64 data);

	// general-purpose registers
	UINT32 m_r[120];

	// control/status registers
	UINT32 m_pc;
	UINT32 m_npc;
	UINT32 m_psr;
	UINT32 m_wim;
	UINT32 m_tbr;
	UINT32 m_y;

	// fields separated out from PSR (Processor State Register)
	UINT8 m_impl;	// implementation (always 0 in MB86901)
	UINT8 m_ver;	// version (always 0 in MB86901)
	UINT8 m_icc;	// integer condition codes
	bool m_ec;		// enable coprocessor
	bool m_ef;		// enable FPU
	UINT8 m_pil;	// processor interrupt level
	bool m_s;		// supervisor mode
	bool m_ps;		// prior S state
	bool m_et;		// enable traps
	UINT8 m_cwp;	// current window pointer

	// register windowing helpers
	UINT32* m_regs[32];

	// addressing helpers
	UINT8 m_insn_asi;
	UINT8 m_data_asi;
	UINT8 m_asi;

	// other internal states
	UINT8 m_trap_priorities[256];
	UINT8 m_queued_tt;
	UINT8 m_queued_priority;
	int m_icount;

	// debugger helpers
	UINT32 m_dbgregs[24];

	// address spaces
	address_space *m_user_insn;
	address_space *m_super_insn;
	address_space *m_user_data;
	address_space *m_super_data;
	address_space *m_spaces[256];

	// processor configuration
	static const int WINDOW_COUNT;
};

// device type definition
extern const device_type MB86901;

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	SPARC_PC = 1,
	SPARC_NPC,
	SPARC_PSR,
	SPARC_WIM,
	SPARC_TBR,
	SPARC_Y,

	SPARC_ICC,
	SPARC_CWP,

	SPARC_G0,	SPARC_G1,	SPARC_G2,	SPARC_G3,	SPARC_G4,	SPARC_G5,	SPARC_G6,	SPARC_G7,
	SPARC_O0,	SPARC_O1,	SPARC_O2,	SPARC_O3,	SPARC_O4,	SPARC_O5,	SPARC_O6,	SPARC_O7,
	SPARC_L0,	SPARC_L1,	SPARC_L2,	SPARC_L3,	SPARC_L4,	SPARC_L5,	SPARC_L6,	SPARC_L7,
	SPARC_I0,	SPARC_I1,	SPARC_I2,	SPARC_I3,	SPARC_I4,	SPARC_I5,	SPARC_I6,	SPARC_I7,

	SPARC_EC,
	SPARC_EF,
	SPARC_ET,
	SPARC_PIL,
	SPARC_S,
	SPARC_PS,

	SPARC_R0,	SPARC_R1,	SPARC_R2,	SPARC_R3,	SPARC_R4,	SPARC_R5,	SPARC_R6,	SPARC_R7,	SPARC_R8,	SPARC_R9,	SPARC_R10,	SPARC_R11,	SPARC_R12,	SPARC_R13,	SPARC_R14,	SPARC_R15,
	SPARC_R16,	SPARC_R17,	SPARC_R18,	SPARC_R19,	SPARC_R20,	SPARC_R21,	SPARC_R22,	SPARC_R23,	SPARC_R24,	SPARC_R25,	SPARC_R26,	SPARC_R27,	SPARC_R28,	SPARC_R29,	SPARC_R30,	SPARC_R31,
	SPARC_R32,	SPARC_R33,	SPARC_R34,	SPARC_R35,	SPARC_R36,	SPARC_R37,	SPARC_R38,	SPARC_R39,	SPARC_R40,	SPARC_R41,	SPARC_R42,	SPARC_R43,	SPARC_R44,	SPARC_R45,	SPARC_R46,	SPARC_R47,
	SPARC_R48,	SPARC_R49,	SPARC_R50,	SPARC_R51,	SPARC_R52,	SPARC_R53,	SPARC_R54,	SPARC_R55,	SPARC_R56,	SPARC_R57,	SPARC_R58,	SPARC_R59,	SPARC_R60,	SPARC_R61,	SPARC_R62,	SPARC_R63,
	SPARC_R64,	SPARC_R65,	SPARC_R66,	SPARC_R67,	SPARC_R68,	SPARC_R69,	SPARC_R70,	SPARC_R71,	SPARC_R72,	SPARC_R73,	SPARC_R74,	SPARC_R75,	SPARC_R76,	SPARC_R77,	SPARC_R78,	SPARC_R79,
	SPARC_R80,	SPARC_R81,	SPARC_R82,	SPARC_R83,	SPARC_R84,	SPARC_R85,	SPARC_R86,	SPARC_R87,	SPARC_R88,	SPARC_R89,	SPARC_R90,	SPARC_R91,	SPARC_R92,	SPARC_R93,	SPARC_R94,	SPARC_R95,
	SPARC_R96,	SPARC_R97,	SPARC_R98,	SPARC_R99,	SPARC_R100,	SPARC_R101,	SPARC_R102,	SPARC_R103,	SPARC_R104,	SPARC_R105,	SPARC_R106,	SPARC_R107,	SPARC_R108,	SPARC_R109,	SPARC_R110,	SPARC_R111
};

CPU_DISASSEMBLE( sparc );

#endif /* __SPARC_H__ */
