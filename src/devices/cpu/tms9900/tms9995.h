// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
  tms9995.h

  See tms9995.cpp for documentation
  Also see tms9900.h for types of TMS99xx processors.
*/

#ifndef MAME_CPU_TMS9995_TMS9995_H
#define MAME_CPU_TMS9995_TMS9995_H

#pragma once

#include "tms99com.h"

// device type definition
DECLARE_DEVICE_TYPE(TMS9995, tms9995_device)
DECLARE_DEVICE_TYPE(TMS9995_MP9537, tms9995_mp9537_device)

enum
{
	INT_9995_RESET = 0,
	INT_9995_INTREQ = 1,
	INT_9995_INT1 = 2,
	INT_9995_INT4 = 3
};

class tms9995_device : public cpu_device
{
public:
	static constexpr int AS_SETADDRESS = 4;

	tms9995_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// READY input line. When asserted (high), the memory is ready for data exchange.
	// We chose to use a direct method instead of a delegate to keep performance
	// footprint low; this method may be called very frequently.
	void ready_line(int state);

	// HOLD input line. When asserted (low), the CPU is requested to release the
	// data and address bus and enter the HOLD state. The entrance of this state
	// is acknowledged by the HOLDA output line.
	void hold_line(int state);

	// RESET input line. Unlike the standard set_input_line, this input method
	// is synchronous and will immediately lead to a reset of the CPU.
	void reset_line(int state);

	// Callbacks
	auto extop_cb() { return m_external_operation.bind(); }
	auto clkout_cb() { return m_clock_out_line.bind(); }
	auto holda_cb() { return m_holda_line.bind(); }

	// For debugger access
	uint8_t debug_read_onchip_memory(offs_t addr) { return m_onchip_memory[addr & 0xff]; }
	void debug_write_onchip_memory(offs_t addr, uint8_t data) { m_onchip_memory[addr & 0xff] = data; }
	bool is_onchip(offs_t addrb) { return (((addrb & 0xff00)==0xf000 && (addrb < 0xf0fc)) || ((addrb & 0xfffc)==0xfffc)) && !m_mp9537; }

	void set_overflow_interrupt( int enable ) { m_check_overflow = (enable!=0); }

protected:
	tms9995_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void        device_start() override;

	// device_execute_interface overrides
	virtual uint32_t    execute_min_cycles() const noexcept override;
	virtual uint32_t    execute_max_cycles() const noexcept override;
	virtual void        execute_set_input(int irqline, int state) override;
	virtual void        execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual space_config_vector memory_space_config() const override;

	uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return clocks / 4.0; }
	uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return cycles * 4.0; }

	// Variant of the TMS9995 without internal RAM and decrementer
	bool    m_mp9537;

private:
	// State / debug management
	uint16_t  m_state_any;
	static char const *const s_statename[];
	void    state_import(const device_state_entry &entry) override;
	void    state_export(const device_state_entry &entry) override;
	void    state_string_export(const device_state_entry &entry, std::string &str) const override;
	uint16_t  read_workspace_register_debug(int reg);
	void    write_workspace_register_debug(int reg, uint16_t data);

	// TMS9995 hardware registers
	uint16_t  WP;     // Workspace pointer
	uint16_t  PC;     // Program counter
	uint16_t  ST;     // Status register

	// The TMS9995 has a prefetch feature which causes a wrong display of the PC.
	// We use this additional member for the debugger only.
	uint16_t  PC_debug;

	// Indicates the instruction acquisition phase
	bool    m_iaq;

	// 256 bytes of onchip memory
	uint8_t   m_onchip_memory[256];

	const address_space_config      m_program_config;
	const address_space_config      m_setaddress_config;
	const address_space_config      m_io_config;
	address_space*                  m_prgspace;
	address_space*                  m_setaddr;
	address_space*                  m_cru;

	// Processor states
	bool    m_idle_state;
	bool    m_nmi_state;
	bool    m_hold_state;
	bool    m_hold_requested;

	// READY handling. The READY line is operated before the clock
	// pulse falls. As the ready line is only set once in this emulation we
	// keep the level in a buffer (like a latch)
	bool    m_ready_bufd;   // buffered state
	bool    m_ready;        // sampled value

	// Auto-wait state generation
	bool    m_request_auto_wait_state;
	bool    m_auto_wait;

	// Cycle counter
	int     m_icount;

	// Phase of the memory access
	int     m_mem_phase;

	// Check the READY line?
	bool    m_check_ready;

	// Check the HOLD line
	bool    m_check_hold;

	// For multi-pass operations. For instance, memory word accesses are
	// executed as two consecutive byte accesses. CRU accesses are repeated
	// single-bit accesses.
	int     m_pass;

	// For Format 1 instruction; determines whether the next operand address
	// derivation is for the source or address operand
	bool    m_get_destination;

	// Used for situations when a command is byte-oriented, but the memory access
	// must be word-oriented. Example: MOVB *R1,R0; we must read the full word
	// from R1 to get the address.
	bool    m_word_access;

	// Interrupt handling
	bool    m_nmi_active;
	bool    m_int1_active;
	bool    m_int4_active;
	bool    m_int_overflow;

	bool    m_reset;
	bool    m_from_reset;
	bool    m_mid_flag;
	bool    m_mid_active;

	int     m_decrementer_clkdiv;
	bool    m_log_interrupt;

	// Flag field
	int     m_int_pending;

	// The TMS9995 is capable of raising an internal interrupt on
	// arithmetic overflow, depending on the status register Overflow Enable bit.
	// However, the specs also say that this feature is non-functional in the
	// currently available chip. Thus we have an option to turn it off so that
	// software will not change its behavior on overflows.
	bool    m_check_overflow;

	// Service pending interrupts
	void    service_interrupt();

	// Issue clock pulses. The TMS9995 uses one (output) clock cycle per machine cycle.
	inline void pulse_clock(int count);

	// Signal the hold state via the external line
	void set_hold_state(bool state);

	// Only used for the DIV(S) operations. It seems sufficient to let the
	// command terminate at this point, so this method just calls command_terminated.
	void    abort_operation(void);

	// Decode the given 16-bit value which has been retrieved by a prefetch or
	// during an X operation.
	void    decode(uint16_t inst);

	// Store the interrupt mask part of the ST. This is used when processing
	// an interrupt, passing the new mask from the service_interrupt part to
	// the program part.
	int     m_intmask;

	// Stored address
	uint16_t  m_address;

	// Stores the recently read word or the word to be written
	uint16_t  m_current_value;

	// Stores the value of the source operand in multi-operand instructions
	uint16_t  m_source_value;

	// During indexed addressing, this value is added to get the final address value.
	uint16_t  m_address_add;

	// During indirect/auto-increment addressing, this copy of the address must
	// be preserved while writing the new value to the register.
	uint16_t  m_address_saved;

	// Another copy of the address
	uint16_t  m_address_copy;

	// Copy of the value
	uint16_t  m_value_copy;

	// Stores the recent register number. Only used to pass the register
	// number during the operand address derivation.
	int     m_regnumber;

	// Stores the number of bits or shift operations
	int     m_count;

	// ============== Decrementer =======================
	void trigger_decrementer();

	// Start value
	uint16_t  m_starting_count_storage_register;

	// Current decrementer value.
	uint16_t  m_decrementer_value;

	// ============== CRU support ======================

	uint16_t  m_cru_address;
	uint16_t  m_cru_value;
	bool    m_cru_first_read;

	// CPU-internal CRU flags
	bool    m_flag[16];

	// ============== Prefetch support =====================

	// We implement the prefetch mechanism by two separate datasets for
	// the decoded commands. When the next instruction shall be started,
	// the contents from the pre* members are copied to the main members.

	uint16_t  IR;
	uint16_t  m_command;
	int     m_index;
	bool    m_byteop;

	uint16_t  m_pre_IR;
	uint16_t  m_pre_command;
	int     m_pre_index;
	bool    m_pre_byteop;

	// State of the currently executed instruction
	int     m_inst_state;

	// ================ Microprogram support ========================

	// Set up lookup table
	void build_command_lookup_table();

	// Sequence of micro-operations
	typedef const uint8_t* microprogram;

	// Method pointer
	typedef void (tms9995_device::*ophandler)(void);

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

	// Pointer to the lookup table; the entry point for searching the command
	std::unique_ptr<lookup_entry[]>   m_command_lookup_table;

	// List of pointers for micro-operations
	static const tms9995_device::ophandler s_microoperation[];

	static const tms9995_device::tms_instruction s_command[];

	// Index of the interrupt program
	int     m_interrupt_mp_index;

	// Index of the operand address derivation subprogram
	int     m_operand_address_derivation_index;

	// Micro-operation program counter (as opposed to the program counter PC)
	int     MPC;

	// Calling microprogram (used when data derivation is called)
	int     m_caller_index;
	int     m_caller_MPC;

	// Table of microprograms
	static const microprogram mp_table[];

	// Used to display the number of consumed cycles in the log.
	int     m_first_cycle;

	// Status register update
	inline void set_status_bit(int bit, bool state);
	inline void compare_and_set_lae(uint16_t value1, uint16_t value2);
	void set_status_parity(uint8_t value);

	// Micro-operation declarations
	void int_prefetch_and_decode();
	void prefetch_and_decode();
	void mem_read();
	void mem_write();
	inline void word_read();
	inline void word_write();
	void operand_address_subprogram();
	void increment_register();
	void indexed_addressing();
	void set_immediate();
	void return_with_address();
	void return_with_address_copy();
	void cru_input_operation();
	void cru_output_operation();
	void command_completed();
	void next_command();

	// ALU operations for specific commands
	void alu_nop();
	void alu_add_s_sxc();
	void alu_b();
	void alu_blwp();
	void alu_c();
	void alu_ci();
	void alu_clr_seto();
	void alu_divide();
	void alu_divide_signed();
	void alu_external();
	void alu_f3();
	void alu_imm_arithm();
	void alu_jump();
	void alu_ldcr();
	void alu_li();
	void alu_limi_lwpi();
	void alu_lst_lwp();
	void alu_mov();
	void alu_multiply();
	void alu_rtwp();
	void alu_sbo_sbz();
	void alu_shift();
	void alu_single_arithm();
	void alu_stcr();
	void alu_stst_stwp();
	void alu_tb();
	void alu_x();
	void alu_xop();
	void alu_int();

	// ================ Connections ====================

	// Trigger external operation. This is achieved by putting a special value in
	// the most significant three bits of the data bus and pulsing the CRUCLK line.
	// Accordingly, we have
	//
	// D0 D1 D2
	// 0  0  0   normal CRU access
	// 0  1  0   IDLE
	// 0  1  1   RSET
	// 1  0  1   CKON
	// 1  1  0   CKOF
	// 1  1  1   LREX
	//
	// We could realize this via the CRU access as well, but the data bus access
	// is not that simple to emulate. For the sake of homogeneity between the
	// chip emulations we use a dedicated callback.
	devcb_write8   m_external_operation;

	// Clock output.
	devcb_write_line   m_clock_out_line;

	// Asserted when the CPU is in a HOLD state
	devcb_write_line   m_holda_line;
};


/*
    Variant of the TMS9995 without on-chip RAM; used in the TI-99/8 console
*/
class tms9995_mp9537_device : public tms9995_device
{
public:
	tms9995_mp9537_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: tms9995_device(mconfig, TMS9995_MP9537, tag, owner, clock)
	{
		m_mp9537 = true;
	}
};

#endif // MAME_CPU_TMS9995_TMS9995_H
