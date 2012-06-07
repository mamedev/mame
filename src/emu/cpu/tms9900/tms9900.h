/*
    TMS9900 processor
    This is a re-implementation of the TMS9900 featuring a cycle-precise
    behaviour.

    See tms9900.c for documentation

    Types of TMS99xx processors:
    TI990/9    Early implementation, used in a few real-world applications, 1974
               very similar to mapper-less 990/10 and tms9900, but the Load
               process is different

    TI990/10   Original multi-chip implementation for minicomputer systems, 1975

    TI990/12   Multi-chip implementation, faster than 990/10. Huge instruction set

    TMS9900    Mono-chip implementation, 1976. Used in the TI-99/4(A) computer.

    TMS9940    Microcontroller with 2kb ROM, 128b RAM, decrementer, CRU bus, 1979

    TMS9980    8-bit variant of tms9900.  Two distinct chips actually : tms9980a
               and tms9981 with an extra clock and simplified power supply

    TMS9985    9940 with 8kb ROM, 256b RAM, and a 8-bit external bus, c. 1978 (never released)

    TMS9989    Improved 9980, used in military hardware.

    SBP68689   Improved 9989, built as an ASIC as 9989 was running scarce

    TMS9995    TMS9985-like, with many improvements (but no ROM). Used in the
               TI-99/8 prototype and the Geneve computer.

    TMS99000   Improved mono-chip implementation, meant to replace 990/10, 1981
    TMS99105   This chip is available in several variants which are similar
    TMS99110   but emulate additional instructions, thanks to the so-called
               macrostore feature.

    In this implementation we only consider TMS9900, 9980, and 9995. The
    remaining types are implemented on an own code base as they introduce
    significant changes (e.g. privileged mode, address mapper). For now we
    leave the implementation of the rest up to 99xxcore.h.
*/

#ifndef __TMS9900_H__
#define __TMS9900_H__

#include "emu.h"
#include "debugger.h"

enum
{
	TI990_10_ID = 1,
	TMS9900_ID = 3,
	TMS9940_ID = 4,
	TMS9980_ID = 5,
	TMS9985_ID = 6,
	TMS9989_ID = 7,
	TMS9995_ID = 9,
	TMS99000_ID = 10,
	TMS99105A_ID = 11,
	TMS99110A_ID = 12
};

#define MCFG_TMS9900_ADD(_tag, _device, _clock, _prgmap, _iomap, _config)		\
	MCFG_DEVICE_ADD(_tag, _device, _clock)		\
	MCFG_DEVICE_PROGRAM_MAP(_prgmap)			\
	MCFG_DEVICE_IO_MAP(_iomap)					\
	MCFG_DEVICE_CONFIG(_config)

enum
{
	IDLE_OP = 2,
	RSET_OP = 3,
	CKOF_OP = 5,
	CKON_OP = 6,
	LREX_OP = 7
};

typedef struct _tms9900_config
{
	devcb_write8		external_callback;
	devcb_read8			irq_level;
	devcb_write_line	instruction_acquisition;
	devcb_write_line	clock_out;
	devcb_write_line	wait_line;
	devcb_write_line	holda_line;
} tms9900_config;

#define TMS9900_CONFIG(name) \
	const tms9900_config(name) =

class tms9900_device : public cpu_device
{
public:
	tms9900_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// READY input line. When asserted (high), the memory is ready for data exchange.
	void set_ready(int state);

	// HOLD input line. When asserted (low), the CPU is requested to release the
	// data and address bus and enter the HOLD state. The entrance of this state
	// is acknowledged by the HOLDA output line.
	void set_hold(int state);

protected:
	// device-level overrides
	virtual void		device_start();
	virtual void		device_stop();
	virtual void		device_reset();

	// device_execute_interface overrides
	virtual UINT32		execute_min_cycles() const;
	virtual UINT32		execute_max_cycles() const;
	virtual UINT32		execute_input_lines() const;
	virtual void		execute_set_input(int irqline, int state);
	virtual void		execute_run();

	// device_disasm_interface overrides
	virtual UINT32		disasm_min_opcode_bytes() const;
	virtual UINT32		disasm_max_opcode_bytes() const;
	virtual offs_t		disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	const address_space_config* memory_space_config(address_spacenum spacenum) const;

private:
	// TMS9900 hardware registers
	UINT16	WP; 	// Workspace pointer
	UINT16	PC; 	// Program counter
	UINT16	ST; 	// Status register

	// Internal register
	UINT16	IR;		// Instruction register

	// Decoded command
	UINT16	m_command;

	// Indicates if this is a byte-oriented command
	inline bool 	byte_operation();

	const address_space_config		m_program_config;
	const address_space_config		m_io_config;
	address_space*					m_prgspace;
	address_space*					m_cru;

	// Processor states
	bool	m_idle_state;
	bool	m_load_state;
	bool	m_irq_state;
	bool	m_ready_state;
	bool	m_wait_state;
	bool	m_hold_state;

	bool	m_reset;

	int 	m_irq_level;	// Interrupt level as acquired from input lines IC0-IC3
	int 	m_icount;		// Cycle counter

	// State / debug management
	UINT16	m_state_any;
	static const char* s_statename[];
	void	state_import(const device_state_entry &entry);
	void	state_export(const device_state_entry &entry);
	void	state_string_export(const device_state_entry &entry, astring &string);
	UINT16	read_workspace_register_debug(int reg);
	void	write_workspace_register_debug(int reg, UINT16 data);

	// Interrupt handling
	void service_interrupt();

	// ================ Microprogram support ========================

	// Set up lookup table
	void build_command_lookup_table();

	// Sequence of micro-operations
	typedef const UINT8* microprogram;

	// Method pointer
	typedef void (tms9900_device::*ophandler)(void);

	// Opcode list entry
	typedef struct _tms_instruction
	{
		UINT16				opcode;
		int					id;
		int					format;
		microprogram		prog;		// Microprogram
	} tms_instruction;

	// Lookup table entry
	typedef struct _lookup_entry
	{
		struct _lookup_entry *next_digit;
		const tms_instruction *entry;
	} lookup_entry;

	// Pointer to the lookup table
	lookup_entry*	m_command_lookup_table;

	// List of allocated tables (used for easy clean-up on exit)
	lookup_entry*	m_lotables[32];

	// List of pointers for micro-operations
	static const tms9900_device::ophandler s_microoperation[];

	// Opcode table
	static const tms9900_device::tms_instruction s_command[];

	// Micro-operation declarations
	void	acquire_instruction(void);
	void	mem_read(void);
	void	mem_write(void);
	void	register_read(void);
	void	register_write(void);
	void	cru_operation(void);
	void	data_derivation_subprogram(void);
	void	return_from_subprogram(void);
	void	command_completed(void);

	void	alu_nop(void);
	void	alu_clear(void);
	void	alu_source(void);
	void	alu_setaddr(void);
	void	alu_addone(void);
	void	alu_setaddr_addone(void);
	void	alu_pcaddr_advance(void);
	void	alu_add_register(void);

	void	alu_imm(void);
	void	alu_reg(void);

	void	alu_f1(void);
	void	alu_comp(void);
	void	alu_f3(void);
	void	alu_multiply(void);
	void	alu_divide(void);
	void	alu_xop(void);
	void	alu_clr_swpb(void);
	void	alu_abs(void);
	void	alu_x(void);
	void	alu_b(void);
	void	alu_bl(void);
	void	alu_blwp(void);
	void	alu_ldcr(void);
	void	alu_stcr(void);
	void	alu_sbz_sbo(void);
	void	alu_tb(void);
	void	alu_jmp(void);
	void	alu_shift(void);
	void	alu_ai_ori(void);
	void	alu_ci(void);
	void	alu_li(void);
	void	alu_lwpi(void);
	void	alu_limi(void);
	void	alu_stwp_stst(void);
	void	alu_external(void);
	void	alu_rtwp(void);
	void	alu_int(void);

	void	abort_operation(void);
	UINT16	pulse_and_read_memory(UINT16 address);
	void	pulse_and_write_memory(UINT16 address, UINT16 data);
	void	decode(UINT16 inst);

	// Micro-operation
	UINT8	m_op;

	// Micro-operation program counter (as opposed to the program counter PC)
	int 	MPC;

	// Current microprogram
	const UINT8*	m_program;

	// Calling microprogram (used when data derivation is called)
	const UINT8*	m_caller;
	int 			m_caller_MPC;

	// State of the micro-operation. Needed for repeated ALU calls.
	int 	m_state;

	// Check the READY line?
	bool	m_check_ready;

	// Has HOLD been acknowledged yet?
	bool	m_hold_acknowledged;

	// Issue clock pulses. Note that each machine cycle has two clock cycles.
	inline void pulse_clock(int count);

	// Signal the wait state via the external line
	inline void set_wait_state(bool state);

	// Used to acknowledge HOLD and enter the HOLD state
	inline void acknowledge_hold();

	// Stored address
	UINT16	m_address;

	// Stores the recently read word or the word to be written
	UINT16	m_current_value;

	// Was the source operand a byte from an even address?
	bool m_source_even;

	// Was the destination operand a byte from an even address?
	bool m_destination_even;

	// Intermediate storage for the source operand
	UINT16 m_source_address;
	UINT16 m_source_value;

	// Stores the recently read register contents
	UINT16	m_register_contents;

	// Stores the register number for the next register access
	int 	m_regnumber;

	// CRU support: Indicates whether the CRU shall be configured to output mode
	bool	m_cru_output;

	// CRU support: Stores the CRU address
	UINT16	m_cru_address;

	// CRU support: Stores the number of bits to be transferred
	int		m_count;

	// Another internal register, storing intermediate values
	// Using 32 bits to support MPY
	UINT32	m_value;

	// For two-argument commands. Indicates whether this is the second operand.
	bool	m_get_destination;

	// Status register update
	inline void set_status_bit(int bit, bool state);
	inline void compare_and_set_lae(UINT16 value1, UINT16 value2);
	void set_status_parity(UINT8 value);

	// Used to display the number of consumed cycles in the log.
	int		m_first_cycle;

	/************************************************************************/

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
	devcb_resolved_write8	m_external_operation;

	// Get the value of the interrupt level lines
	devcb_resolved_read8	m_get_ic0123;

	// Signal to the outside world that we are now getting an instruction
	devcb_resolved_write_line	m_iaq_line;

	// Clock output. This is not a pin of the TMS9900 because the TMS9900
	// needs an external clock, and usually one of those external lines is
	// used for this purpose.
	devcb_resolved_write_line	m_clock_out_line;

	// Wait output. When asserted (high), the CPU is in a wait state.
	devcb_resolved_write_line	m_wait_line;

	// HOLD Acknowledge line. When asserted (high), the CPU is in HOLD state.
	devcb_resolved_write_line	m_holda_line;
};

unsigned Dasm9900(char *buffer, unsigned pc, int model_id, const UINT8 *oprom, const UINT8 *opram);

// device type definition
extern const device_type TMS9900;

#endif /* __TMS9900_H__ */
