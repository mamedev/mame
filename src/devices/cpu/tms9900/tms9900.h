// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    TMS9900 processor
    This is a re-implementation of the TMS9900 featuring a cycle-precise
    behaviour.

    See tms9900.c for documentation
*/

#ifndef MAME_CPU_TMS9900_TMS9900_H
#define MAME_CPU_TMS9900_TMS9900_H

#pragma once

#include "tms99com.h"

enum
{
	INT_9900_RESET = 0,
	INT_9900_LOAD = 1,
	INT_9900_INTREQ = 2
};

enum
{
	LOAD_INT = -1,
	RESET_INT = -2
};

static const char opname[][5] =
{   "ILL ", "A   ", "AB  ", "ABS ", "AI  ", "ANDI", "B   ", "BL  ", "BLWP", "C   ",
	"CB  ", "CI  ", "CKOF", "CKON", "CLR ", "COC ", "CZC ", "DEC ", "DECT", "DIV ",
	"IDLE", "INC ", "INCT", "INV ", "JEQ ", "JGT ", "JH  ", "JHE ", "JL  ", "JLE ",
	"JLT ", "JMP ", "JNC ", "JNE ", "JNO ", "JOC ", "JOP ", "LDCR", "LI  ", "LIMI",
	"LREX", "LWPI", "MOV ", "MOVB", "MPY ", "NEG ", "ORI ", "RSET", "RTWP", "S   ",
	"SB  ", "SBO ", "SBZ ", "SETO", "SLA ", "SOC ", "SOCB", "SRA ", "SRC ", "SRL ",
	"STCR", "STST", "STWP", "SWPB", "SZC ", "SZCB", "TB  ", "X   ", "XOP ", "XOR ",
	"*int"
};

class tms99xx_device : public cpu_device
{
public:
	static constexpr int AS_SETADDRESS = 4;

	~tms99xx_device();

	// READY input line. When asserted (high), the memory is ready for data exchange.
	void set_ready(int state);

	// HOLD input line. When asserted (low), the CPU is requested to release the
	// data and address bus and enter the HOLD state. The entrance of this state
	// is acknowledged by the HOLDA output line.
	void set_hold(int state);

	// Callbacks
	auto extop_cb() { return m_external_operation.bind(); }
	auto intlevel_cb() { return m_get_intlevel.bind(); }
	auto clkout_cb() { return m_clock_out_line.bind(); }
	auto wait_cb() { return m_wait_line.bind(); }
	auto holda_cb() { return m_holda_line.bind(); }

protected:
	tms99xx_device(const machine_config &mconfig, device_type type,
				const char *tag, int data_width, int prg_addr_bits, int cru_addr_bits,
				device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void        device_start() override;
	virtual void        device_stop() override;
	virtual void        device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t    execute_min_cycles() const noexcept override;
	virtual uint32_t    execute_max_cycles() const noexcept override;
	virtual void        execute_set_input(int irqline, int state) override;
	virtual void        execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual space_config_vector memory_space_config() const override;

	// Let these methods be overloaded by the TMS9980.
	virtual void        mem_read();
	virtual void        mem_write();
	virtual void        acquire_instruction();
	void                decode(uint16_t inst);

	const address_space_config  m_program_config;
	const address_space_config  m_setaddress_config;
	const address_space_config  m_io_config;
	address_space*          m_prgspace;
	address_space*          m_setaddr;
	address_space*          m_cru;

	virtual uint16_t  read_workspace_register_debug(int reg);
	virtual void    write_workspace_register_debug(int reg, uint16_t data);

	// Cycle counter
	int     m_icount;

	// TMS9900 hardware registers
	uint16_t  WP;     // Workspace pointer
	uint16_t  PC;     // Program counter
	uint16_t  ST;     // Status register

	// Internal register
	uint16_t  IR;     // Instruction register

	// Stored address
	uint16_t  m_address;

	// Stores the recently read word or the word to be written
	uint16_t  m_current_value;

	// Decoded command
	uint16_t  m_command;

	// Is it a byte operation? Only format 1 commands with the byte flag set
	// and CRU commands with less than 9 bits to transfer are byte operations.
	bool m_byteop;

	// Issue clock pulses. Note that each machine cycle has two clock cycles.
	void pulse_clock(int count);

	// For multi-pass operations. For instance, memory word accesses are
	// executed as two consecutive byte accesses. CRU accesses are repeated
	// single-bit accesses. (Needed for TMS9980)
	int     m_pass;

	// Check the READY line?
	bool    m_check_ready;

	// Phase of the memory access
	int     m_mem_phase;

	// Max address
	const uint16_t  m_prgaddr_mask;
	const uint16_t  m_cruaddr_mask;

	bool    m_load_state;
	bool    m_irq_state;
	bool    m_reset;

	// Determine the interrupt level using the IC0-IC3 lines
	int get_intlevel(int state);

	// Interrupt level as acquired from input lines (TMS9900: IC0-IC3, TMS9980: IC0-IC2)
	// We assume all values right-justified, i.e. TMS9980 also counts up by one
	int     m_irq_level;

	// Used to display the number of consumed cycles in the log.
	int     m_first_cycle;

	// Indicates the instruction acquision phase
	bool    m_iaq;

	/************************************************************************/

	// Clock output. This is not a pin of the TMS9900 because the TMS9900
	// needs an external clock, and usually one of those external lines is
	// used for this purpose.
	devcb_write_line   m_clock_out_line;

	// Wait output. When asserted (high), the CPU is in a wait state.
	devcb_write_line   m_wait_line;

	// HOLD Acknowledge line. When asserted (high), the CPU is in HOLD state.
	devcb_write_line   m_holda_line;

	// Get the value of the interrupt level lines
	devcb_read8    m_get_intlevel;

	// Trigger external operation. This is achieved by putting a special value in
	// the most significant three bits of the address bus (TMS9995: data bus) and
	// pulsing the CRUCLK line.
	// Accordingly, we have
	//
	// A0 A1 A2 A3 A4 A5 ... A12 A13 A14 A15
	// 0  0  0  x  x  x      x   x   x   -     normal CRU access
	// 0  1  0  x  x  x      x   x   x   -     IDLE
	// 0  1  1  x  x  x      x   x   x   -     RSET
	// 1  0  1  x  x  x      x   x   x   -     CKON
	// 1  1  0  x  x  x      x   x   x   -     CKOF
	// 1  1  1  x  x  x      x   x   x   -     LREX
	//
	// so the TMS9900 can only use CRU addresses 0 - 1ffe for CRU operations.
	// By moving these three bits to the data bus, the TMS9995 can allow for the
	// full range 0000-fffe for its CRU operations.
	//
	// We could realize this via the CRU access as well, but the data bus access
	// is not that simple to emulate. For the sake of homogeneity between the
	// chip emulations we use a dedicated callback.
	devcb_write8   m_external_operation;


private:
	// Indicates if this is a byte-oriented command
	inline bool     byte_operation();

	// Processor states
	bool    m_idle_state;

	// READY handling. The READY line is operated before the phi1 clock
	// pulse rises. As the ready line is only set once in this emulation we
	// keep the level in a buffer (like a latch)
	bool    m_ready_bufd;   // buffered state
	bool    m_ready;        // sampled value

	bool    m_wait_state;
	bool    m_hold_state;

	// State / debug management
	uint16_t  m_state_any;
	static char const *const s_statename[];
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// Interrupt handling
	void service_interrupt();

	// ================ Microprogram support ========================

	// Set up lookup table
	void build_command_lookup_table();

	// Sequence of micro-operations
	typedef const uint8_t* microprogram;

	// Method pointer
	typedef void (tms99xx_device::*ophandler)();

	// Opcode list entry
	struct tms_instruction
	{
		uint16_t              opcode;
		int                 id;
		int                 format;
		microprogram        prog;       // Microprogram
	};

	// Lookup table entry
	struct lookup_entry
	{
		std::unique_ptr<lookup_entry[]> next_digit;
		int index; // pointing to the static instruction list
	};

	// Pointer to the lookup table
	std::unique_ptr<lookup_entry[]>   m_command_lookup_table;

	// List of pointers for micro-operations
	static const tms99xx_device::ophandler s_microoperation[];

	// Opcode table
	static const tms99xx_device::tms_instruction s_command[];

	// Micro-operation declarations
	void    register_read();
	void    register_write();
	void    cru_input_operation();
	void    cru_output_operation();
	void    data_derivation_subprogram();
	void    return_from_subprogram();
	void    command_completed();

	void    alu_nop();
	void    alu_clear();
	void    alu_source();
	void    alu_setaddr();
	void    alu_addone();
	void    alu_setaddr_addone();
	void    alu_pcaddr_advance();
	void    alu_add_register();

	void    alu_imm();
	void    alu_reg();

	void    alu_f1();
	void    alu_comp();
	void    alu_f3();
	void    alu_multiply();
	void    alu_divide();
	void    alu_xop();
	void    alu_clr_swpb();
	void    alu_abs();
	void    alu_x();
	void    alu_b();
	void    alu_blwp();
	void    alu_ldcr();
	void    alu_stcr();
	void    alu_sbz_sbo();
	void    alu_tb();
	void    alu_jmp();
	void    alu_shift();
	void    alu_ai_ori();
	void    alu_ci();
	void    alu_li();
	void    alu_lwpi();
	void    alu_limi();
	void    alu_stwp_stst();
	void    alu_external();
	void    alu_rtwp();
	void    alu_int();

	void    abort_operation();

	// Micro-operation program counter (as opposed to the program counter PC)
	int     MPC;

	// Current microprogram
	int     m_program_index;

	// Calling microprogram (used when data derivation is called)
	int     m_caller_index;
	int     m_caller_MPC;

	// Index of the interrupt program
	int     m_interrupt_mp_index;

	// For debugging only
	bool    m_log_interrupt;

	// State of the micro-operation. Needed for repeated ALU calls.
	int     m_state;

	// Has HOLD been acknowledged yet?
	bool    m_hold_acknowledged;

	// Signal the wait state via the external line
	inline void set_wait_state(bool state);

	// Used to acknowledge HOLD and enter the HOLD state
	inline void acknowledge_hold();

	// Was the source operand a byte from an even address?
	bool m_source_even;

	// Was the destination operand a byte from an even address?
	bool m_destination_even;

	// Intermediate storage for the source operand
	uint16_t m_source_address;
	uint16_t m_source_value;
	uint16_t  m_address_saved;

	// Another copy of the address
	uint16_t  m_address_copy;

	// Stores the recently read register contents
	uint16_t  m_register_contents;

	// Stores the register number for the next register access
	int     m_regnumber;

	// CRU support: Stores the CRU address
	uint16_t  m_cru_address;

	// CRU support: Stores the number of bits to be transferred
	int     m_count;

	// Copy of the value
	uint16_t  m_value_copy;

	// Another internal register, storing intermediate values
	// Using 32 bits to support MPY
	uint32_t  m_value;

	// For two-argument commands. Indicates whether this is the second operand.
	bool    m_get_destination;

	// Status register update
	inline void set_status_bit(int bit, bool state);
	inline void compare_and_set_lae(uint16_t value1, uint16_t value2);
	void set_status_parity(uint8_t value);
};

/*****************************************************************************/

class tms9900_device : public tms99xx_device
{
public:
	tms9900_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(TMS9900, tms9900_device)

#endif // MAME_CPU_TMS9900_TMS9900_H
