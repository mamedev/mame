// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    TMS9900 processor
    This is a re-implementation of the TMS9900 featuring a cycle-precise
    behaviour.

    See tms9900.c for documentation
*/

#ifndef __TMS9900_H__
#define __TMS9900_H__

#include "emu.h"
#include "debugger.h"
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
	tms99xx_device(const machine_config &mconfig, device_type type,  const char *name,
				const char *tag, int databus_width, int prg_addr_bits, int cru_addr_bits,
				device_t *owner, UINT32 clock, const char *shortname, const char *source);

	~tms99xx_device();

	// READY input line. When asserted (high), the memory is ready for data exchange.
	void set_ready(int state);

	// HOLD input line. When asserted (low), the CPU is requested to release the
	// data and address bus and enter the HOLD state. The entrance of this state
	// is acknowledged by the HOLDA output line.
	void set_hold(int state);

	// Callbacks
	template<class _Object> static devcb_base &static_set_extop_callback(device_t &device, _Object object) { return downcast<tms99xx_device &>(device).m_external_operation.set_callback(object); }
	template<class _Object> static devcb_base &static_set_intlevel_callback(device_t &device, _Object object) { return downcast<tms99xx_device &>(device).m_get_intlevel.set_callback(object); }
	template<class _Object> static devcb_base &static_set_iaq_callback(device_t &device, _Object object) { return downcast<tms99xx_device &>(device).m_iaq_line.set_callback(object); }
	template<class _Object> static devcb_base &static_set_clkout_callback(device_t &device, _Object object) { return downcast<tms99xx_device &>(device).m_clock_out_line.set_callback(object); }
	template<class _Object> static devcb_base &static_set_wait_callback(device_t &device, _Object object) { return downcast<tms99xx_device &>(device).m_wait_line.set_callback(object); }
	template<class _Object> static devcb_base &static_set_holda_callback(device_t &device, _Object object) { return downcast<tms99xx_device &>(device).m_holda_line.set_callback(object); }
	template<class _Object> static devcb_base &static_set_dbin_callback(device_t &device, _Object object) { return downcast<tms99xx_device &>(device).m_dbin_line.set_callback(object); }

protected:
	// device-level overrides
	virtual void        device_start() override;
	virtual void        device_stop() override;
	virtual void        device_reset() override;

	virtual void        resolve_lines();

	// device_execute_interface overrides
	virtual UINT32      execute_min_cycles() const override;
	virtual UINT32      execute_max_cycles() const override;
	virtual UINT32      execute_input_lines() const override;
	virtual void        execute_set_input(int irqline, int state) override;
	virtual void        execute_run() override;

	// device_disasm_interface overrides
	virtual UINT32      disasm_min_opcode_bytes() const override;
	virtual UINT32      disasm_max_opcode_bytes() const override;
	virtual offs_t      disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	const address_space_config* memory_space_config(address_spacenum spacenum) const override;

	// Let these methods be overloaded by the TMS9980.
	virtual void        mem_read(void);
	virtual void        mem_write(void);
	virtual void        acquire_instruction(void);
	void                decode(UINT16 inst);

	const address_space_config  m_program_config;
	const address_space_config  m_io_config;
	address_space*          m_prgspace;
	address_space*          m_cru;

	virtual UINT16  read_workspace_register_debug(int reg);
	virtual void    write_workspace_register_debug(int reg, UINT16 data);

	// Cycle counter
	int     m_icount;

	// TMS9900 hardware registers
	UINT16  WP;     // Workspace pointer
	UINT16  PC;     // Program counter
	UINT16  ST;     // Status register

	// Internal register
	UINT16  IR;     // Instruction register

	// Stored address
	UINT16  m_address;

	// Stores the recently read word or the word to be written
	UINT16  m_current_value;

	// Decoded command
	UINT16  m_command;

	// Is it a byte operation? Only format 1 commands with the byte flag set
	// and CRU commands with less than 9 bits to transfer are byte operations.
	bool m_byteop;

	// Issue clock pulses. Note that each machine cycle has two clock cycles.
	void pulse_clock(int count);

	// For multi-pass operations. For instance, memory word accesses are
	// executed as two consecutive byte accesses. CRU accesses are repeated
	// single-bit accesses. (Needed for TMS9980)
	int     m_pass;

	// Data bus width. Needed for TMS9980.
	int     m_databus_width;

	// Check the READY line?
	bool    m_check_ready;

	// Phase of the memory access
	int     m_mem_phase;

	// Max address
	const UINT16  m_prgaddr_mask;
	const UINT16  m_cruaddr_mask;

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

	/************************************************************************/

	// Clock output. This is not a pin of the TMS9900 because the TMS9900
	// needs an external clock, and usually one of those external lines is
	// used for this purpose.
	devcb_write_line   m_clock_out_line;

	// Wait output. When asserted (high), the CPU is in a wait state.
	devcb_write_line   m_wait_line;

	// HOLD Acknowledge line. When asserted (high), the CPU is in HOLD state.
	devcb_write_line   m_holda_line;

	// Signal to the outside world that we are now getting an instruction
	devcb_write_line   m_iaq_line;

	// Get the value of the interrupt level lines
	devcb_read8    m_get_intlevel;

	// DBIN line. When asserted (high), the CPU has disabled the data bus output buffers.
	devcb_write_line   m_dbin_line;

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
	// is not that simple to emulate. For the sake of homogenity between the
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
	UINT16  m_state_any;
	static const char* s_statename[];
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// Interrupt handling
	void service_interrupt();

	// ================ Microprogram support ========================

	// Set up lookup table
	void build_command_lookup_table();

	// Sequence of micro-operations
	typedef const UINT8* microprogram;

	// Method pointer
	typedef void (tms99xx_device::*ophandler)(void);

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

	// Pointer to the lookup table
	lookup_entry*   m_command_lookup_table;

	// List of allocated tables (used for easy clean-up on exit)
	lookup_entry*   m_lotables[32];

	// List of pointers for micro-operations
	static const tms99xx_device::ophandler s_microoperation[];

	// Opcode table
	static const tms99xx_device::tms_instruction s_command[];

	// Micro-operation declarations
	void    register_read(void);
	void    register_write(void);
	void    cru_input_operation(void);
	void    cru_output_operation(void);
	void    data_derivation_subprogram(void);
	void    return_from_subprogram(void);
	void    command_completed(void);

	void    alu_nop(void);
	void    alu_clear(void);
	void    alu_source(void);
	void    alu_setaddr(void);
	void    alu_addone(void);
	void    alu_setaddr_addone(void);
	void    alu_pcaddr_advance(void);
	void    alu_add_register(void);

	void    alu_imm(void);
	void    alu_reg(void);

	void    alu_f1(void);
	void    alu_comp(void);
	void    alu_f3(void);
	void    alu_multiply(void);
	void    alu_divide(void);
	void    alu_xop(void);
	void    alu_clr_swpb(void);
	void    alu_abs(void);
	void    alu_x(void);
	void    alu_b(void);
	//void    alu_bl(void);
	void    alu_blwp(void);
	void    alu_ldcr(void);
	void    alu_stcr(void);
	void    alu_sbz_sbo(void);
	void    alu_tb(void);
	void    alu_jmp(void);
	void    alu_shift(void);
	void    alu_ai_ori(void);
	void    alu_ci(void);
	void    alu_li(void);
	void    alu_lwpi(void);
	void    alu_limi(void);
	void    alu_stwp_stst(void);
	void    alu_external(void);
	void    alu_rtwp(void);
	void    alu_int(void);

	void    abort_operation(void);

	// Micro-operation
	UINT8   m_op;

	// Micro-operation program counter (as opposed to the program counter PC)
	int     MPC;

	// Current microprogram
	const UINT8*    m_program;

	// Calling microprogram (used when data derivation is called)
	const UINT8*    m_caller;
	int             m_caller_MPC;

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
	UINT16 m_source_address;
	UINT16 m_source_value;
	UINT16  m_address_saved;

	// Another copy of the address
	UINT16  m_address_copy;

	// Stores the recently read register contents
	UINT16  m_register_contents;

	// Stores the register number for the next register access
	int     m_regnumber;

	// CRU support: Stores the CRU address
	UINT16  m_cru_address;

	// CRU support: Stores the number of bits to be transferred
	int     m_count;

	// Copy of the value
	UINT16  m_value_copy;

	// Another internal register, storing intermediate values
	// Using 32 bits to support MPY
	UINT32  m_value;

	// For two-argument commands. Indicates whether this is the second operand.
	bool    m_get_destination;

	// Status register update
	inline void set_status_bit(int bit, bool state);
	inline void compare_and_set_lae(UINT16 value1, UINT16 value2);
	void set_status_parity(UINT8 value);
};

/*****************************************************************************/

class tms9900_device : public tms99xx_device
{
public:
	tms9900_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type TMS9900;

#endif /* __TMS9900_H__ */
