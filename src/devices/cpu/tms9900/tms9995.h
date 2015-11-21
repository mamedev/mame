// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
  tms9995.h

  See tms9995.c for documentation
  Also see tms9900.h for types of TMS99xx processors.
*/

#ifndef __TMS9995_H__
#define __TMS9995_H__

#include "emu.h"
#include "debugger.h"
#include "tms99com.h"

// device type definition
extern const device_type TMS9995;
extern const device_type TMS9995_MP9537;

enum
{
	INT_9995_RESET = 0,
	INT_9995_INTREQ = 1,
	INT_9995_INT1 = 2,
	INT_9995_INT4 = 3
};

#define MCFG_TMS9995_EXTOP_HANDLER( _extop) \
	devcb = &tms9995_device::static_set_extop_callback( *device, DEVCB_##_extop );

#define MCFG_TMS9995_IAQ_HANDLER( _iaq )    \
	devcb = &tms9995_device::static_set_iaq_callback( *device, DEVCB_##_iaq );

#define MCFG_TMS9995_CLKOUT_HANDLER( _clkout ) \
	devcb = &tms9995_device::static_set_clkout_callback( *device, DEVCB_##_clkout );

#define MCFG_TMS9995_HOLDA_HANDLER( _holda ) \
	devcb = &tms9995_device::static_set_holda_callback( *device, DEVCB_##_holda );

#define MCFG_TMS9995_DBIN_HANDLER( _dbin ) \
	devcb = &tms9995_device::static_set_dbin_callback( *device, DEVCB_##_dbin );

#define MCFG_TMS9995_ENABLE_OVINT( _ovint ) \
	downcast<tms9995_device*>(device)->set_overflow_interrupt( _ovint );


class tms9995_device : public cpu_device
{
public:
	tms9995_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms9995_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// READY input line. When asserted (high), the memory is ready for data exchange.
	// We chose to use a direct method instead of a delegate to keep performance
	// footprint low; this method may be called very frequently.
	void set_ready(int state);

	// HOLD input line. When asserted (low), the CPU is requested to release the
	// data and address bus and enter the HOLD state. The entrance of this state
	// is acknowledged by the HOLDA output line.
	void set_hold(int state);

	// Callbacks
	template<class _Object> static devcb_base &static_set_extop_callback(device_t &device, _Object object) { return downcast<tms9995_device &>(device).m_external_operation.set_callback(object); }
	template<class _Object> static devcb_base &static_set_iaq_callback(device_t &device, _Object object) { return downcast<tms9995_device &>(device).m_iaq_line.set_callback(object); }
	template<class _Object> static devcb_base &static_set_clkout_callback(device_t &device, _Object object) { return downcast<tms9995_device &>(device).m_clock_out_line.set_callback(object); }
	template<class _Object> static devcb_base &static_set_holda_callback(device_t &device, _Object object) { return downcast<tms9995_device &>(device).m_holda_line.set_callback(object); }
	template<class _Object> static devcb_base &static_set_dbin_callback(device_t &device, _Object object) { return downcast<tms9995_device &>(device).m_dbin_line.set_callback(object); }

	// For debugger access
	UINT8 debug_read_onchip_memory(offs_t addr) { return m_onchip_memory[addr & 0xff]; };
	bool is_onchip(offs_t addrb) { return (((addrb & 0xff00)==0xf000 && (addrb < 0xf0fc)) || ((addrb & 0xfffc)==0xfffc)) && !m_mp9537; }

	void set_overflow_interrupt( int enable ) { m_check_overflow = (enable!=0); }

protected:
	// device-level overrides
	virtual void        device_start();
	virtual void        device_stop();
	virtual void        device_reset();

	// device_execute_interface overrides
	virtual UINT32      execute_min_cycles() const;
	virtual UINT32      execute_max_cycles() const;
	virtual UINT32      execute_input_lines() const;
	virtual void        execute_set_input(int irqline, int state);
	virtual void        execute_run();

	// device_disasm_interface overrides
	virtual UINT32      disasm_min_opcode_bytes() const;
	virtual UINT32      disasm_max_opcode_bytes() const;
	virtual offs_t      disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	const address_space_config* memory_space_config(address_spacenum spacenum) const;

	UINT64 execute_clocks_to_cycles(UINT64 clocks) const { return clocks / 4.0; }
	UINT64 execute_cycles_to_clocks(UINT64 cycles) const { return cycles * 4.0; }

	// Variant of the TMS9995 without internal RAM and decrementer
	bool    m_mp9537;

private:
	// State / debug management
	UINT16  m_state_any;
	static const char* s_statename[];
	void    state_import(const device_state_entry &entry);
	void    state_export(const device_state_entry &entry);
	void    state_string_export(const device_state_entry &entry, std::string &str);
	UINT16  read_workspace_register_debug(int reg);
	void    write_workspace_register_debug(int reg, UINT16 data);

	// TMS9995 hardware registers
	UINT16  WP;     // Workspace pointer
	UINT16  PC;     // Program counter
	UINT16  ST;     // Status register

	// The TMS9995 has a prefetch feature which causes a wrong display of the PC.
	// We use this additional member for the debugger only.
	UINT16  PC_debug;

	// 256 bytes of onchip memory
	UINT8   m_onchip_memory[256];

	const address_space_config      m_program_config;
	const address_space_config      m_io_config;
	address_space*                  m_prgspace;
	address_space*                  m_cru;


	// Processor states
	bool    m_idle_state;
	bool    m_nmi_state;
//	bool    m_irq_state;
	bool    m_hold_state;

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

	// For parity operations
	int     m_parity;

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
	bool    m_int_decrementer;
	bool    m_int_overflow;

	bool    m_reset;
	bool    m_from_reset;
	bool    m_mid_flag;
	bool    m_mid_active;

	int     m_decrementer_clkdiv;
	bool    m_servicing_interrupt;

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
	inline void set_hold_state(bool state);

	// Only used for the DIV(S) operations. It seems sufficient to let the
	// command terminate at this point, so this method just calls command_terminated.
	void    abort_operation(void);

	// Decode the given 16-bit value which has been retrieved by a prefetch or
	// during an X operation.
	void    decode(UINT16 inst);

	// Store the interrupt mask part of the ST. This is used when processing
	// an interrupt, passing the new mask from the service_interrupt part to
	// the program part.
	int     m_intmask;

	// Stored address
	UINT16  m_address;

	// Stores the recently read word or the word to be written
	UINT16  m_current_value;

	// Stores the value of the source operand in multi-operand instructions
	UINT16  m_source_value;

	// During indexed addressing, this value is added to get the final address value.
	UINT16  m_address_add;

	// During indirect/auto-increment addressing, this copy of the address must
	// be preserved while writing the new value to the register.
	UINT16  m_address_saved;

	// Another copy of the address
	UINT16  m_address_copy;

	// Copy of the value
	UINT16  m_value_copy;

	// Stores the recent register number. Only used to pass the register
	// number during the operand address derivation.
	int     m_regnumber;

	// Stores the number of bits or shift operations
	int     m_count;

	// ============== Decrementer =======================
	void trigger_decrementer();

	// Start value
	UINT16  m_starting_count_storage_register;

	// Current decrementer value.
	UINT16  m_decrementer_value;

	// ============== CRU support ======================

	UINT16  m_cru_address;
	UINT16  m_cru_value;
	bool    m_cru_first_read;
	int     m_cru_bits_left;
	UINT32  m_cru_read;

	// CPU-internal CRU flags
	bool    m_flag[16];

	// ============== Prefetch support =====================

	struct decoded_instruction
	{
		UINT16          IR;
		UINT16          command;
		const UINT8*    program;
		bool            byteop;
		int             state;
	};

	int     m_instindex;

	// We implement the prefetch mechanism by two separate datasets for
	// the decoded commands. When the previous command has completed, the
	// pointer is just switched to the other one.
	tms9995_device::decoded_instruction     m_decoded[2];
	tms9995_device::decoded_instruction*    m_instruction;

	// ================ Microprogram support ========================

	// Set up lookup table
	void build_command_lookup_table();

	// Sequence of micro-operations
	typedef const UINT8* microprogram;

	// Method pointer
	typedef void (tms9995_device::*ophandler)(void);

	// Opcode list entry
	struct tms_instruction
	{
		UINT16              opcode;
		int                 id;
		int                 format;
		microprogram        prog;       // Microprogram
	};

	// Lookup table entry
	struct lookup_entry
	{
		lookup_entry *next_digit;
		const tms_instruction *entry;
	};

	// Pointer to the lookup table; the entry point for searching the command
	lookup_entry*   m_command_lookup_table;

	// List of allocated tables (used for easy clean-up on exit)
	lookup_entry*   m_lotables[32];

	// List of pointers for micro-operations
	static const tms9995_device::ophandler s_microoperation[];

	static const tms9995_device::tms_instruction s_command[];

	// Micro-operation program counter (as opposed to the program counter PC)
	int     MPC;

	// Calling microprogram (used when data derivation is called)
	const UINT8*    m_caller;
	int             m_caller_MPC;

	// Table of microprograms
	static const microprogram mp_table[];

	// Used to display the number of consumed cycles in the log.
	int     m_first_cycle;

	// Status register update
	inline void set_status_bit(int bit, bool state);
	inline void compare_and_set_lae(UINT16 value1, UINT16 value2);
	void set_status_parity(UINT8 value);

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
//	void alu_multiply_signed();
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
	// is not that simple to emulate. For the sake of homogenity between the
	// chip emulations we use a dedicated callback.
	devcb_write8   m_external_operation;

	// Signal to the outside world that we are now getting an instruction (IAQ).
	// In the real hardware this line is shared with the HOLDA line, and the
	// /MEMEN line is used to decide which signal we have on the line. We do not
	// emulate the /MEMEN line, so we have to use two separate lines.
	devcb_write_line   m_iaq_line;

	// Clock output.
	devcb_write_line   m_clock_out_line;

	// Asserted when the CPU is in a HOLD state
	devcb_write_line   m_holda_line;

	// DBIN line. When asserted (high), the CPU has disabled the data bus output buffers.
	devcb_write_line   m_dbin_line;
};


/*
    Variant of the TMS9995 without on-chip RAM; used in the TI-99/8 console
*/
class tms9995_mp9537_device : public tms9995_device
{
public:
	tms9995_mp9537_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms9995_device(mconfig, TMS9995_MP9537, "TMS9995-MP9537", tag, owner, clock, "tms9995_mp9537", __FILE__)
	{
		m_mp9537 = true;
	}
};

#endif /* __TMS9995_H__ */
