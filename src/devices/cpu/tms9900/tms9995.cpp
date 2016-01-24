// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    Texas Instruments TMS9995

                    +----------------+
              XTAL1 | 1     \/     40| A15,CRUOUT
        XTAL2,CLKIN | 2            39| A14
             CLKOUT | 3            38| A13
                 D7 | 4            37| A12
                 D6 | 5            36| A11
                 D5 | 6            35| A10
                 D4 | 7            34| A9
                 D3 | 8            33| A8
                 D2 | 9            32| A7
                Vcc |10            31| Vss
                 D1 |11            30| A6
                 D0 |12            29| A5
              CRUIN |13            28| A4
          /INT4,/EC |14            27| A3
              /INT1 |15            26| A2
          IAQ,HOLDA |16            25| A1
              /DBIN |17            24| A0
              /HOLD |18            23| READY
        /WE,/CRUCLK |19            22| /RESET
             /MEMEN |20            21| /NMI
                    +----------------+

      XTAL1    in   Crystal input pin for internal oscillator
      XTAL2    in   Crystal input pin for internal oscillator, or
      CLKIN    in   Input pin for external oscillator
     CLKOUT   out   Clock output signal (1:4 of the input signal frequency)
      CRUIN    in   CRU input data
      /INT4    in   Interrupt level 4 input
        /EC    in   Event counter
      /INT1    in   Interrupt level 1 input
        IAQ   out   Instruction acquisition
      HOLDA   out   Hold acknowledge
        /WE   out   Data available for memory write
    /CRUCLK   out   Communication register unit clock output
     /MEMEN   out   Address bus contains memory address
       /NMI    in   Non-maskable interrupt (/LOAD on TMS9900)
     /RESET    in   Reset interrupt
      READY    in   Memory/External CRU device ready for access
     CRUOUT   out   Communication register unit data output

        Vcc   +5V   supply
        Vss    0V   Ground reference

     A0-A15   out   Address bus
      D0-D7 in/out  Data bus

     Note that Texas Instruments' bit numberings define bit 0 as the
     most significant bit (different to most other systems). Also, the
     system uses big-endian memory organisation: Storing the word 0x1234 at
     address 0x0000 means that the byte 0x12 is stored at 0x0000 and byte 0x34
     is stored at 0x0001.

     The TMS9995 is a 16 bit microprocessor like the TMS9900, operating on
     16-bit words and using 16-bit opcodes. Memory transfer of 16-bit words
     is achieved by a transfer of the most significant byte, followed by
     the least significant byte.

     The 8-bit databus width allows the processor to exchange single bytes with
     the external memory.

     See tms9900.c for some more details on the cycle-precise implementation.

     This implementation also features all control lines and the instruction
     prefetch mechanism. Prefetching is explicitly triggered within the
     microprograms. The TMS9995 specification does not reveal the exact
     operations during the microprogram execution, so we have to look at the
     required cycle numbers to guess what is happening.

     Auto wait state:

     In order to enable automatic wait state creation, the READY line must be
     cleared on reset time. A good position to do this is MACHINE_RESET in
     the driver.


     References (see comments below)
     ----------
     [1] Texas Instruments 9900 Microprocessor series: TMS9995 16-bit Microcomputer

     TODO:
        - State save
        - Test HOLD

     Michael Zapf, June 2012
*/

#include "tms9995.h"

/* tms9995 ST register bits. */
enum
{
	ST_LH = 0x8000,     // Logical higher (unsigned comparison)
	ST_AGT = 0x4000,    // Arithmetical greater than (signed comparison)
	ST_EQ = 0x2000,     // Equal
	ST_C = 0x1000,      // Carry
	ST_OV = 0x0800,     // Overflow (when using signed operations)
	ST_OP = 0x0400,     // Odd parity (used with byte operations)
	ST_X = 0x0200,      // XOP
	ST_OE = 0x0020,     // Overflow interrupt enabled
	ST_IM = 0x000f      // Interrupt mask
};

enum
{
	PENDING_NMI = 1,
	PENDING_MID = 2,
	PENDING_LEVEL1 = 4,
	PENDING_OVERFLOW = 8,
	PENDING_DECR = 16,
	PENDING_LEVEL4 = 32
};

/*****************************************************************
    Debugging
    Set to 0 (disable) or 1 (enable)
******************************************************************/

// Log addresses of executed opcodes
#define TRACE_EXEC 0

// Log cycles
#define TRACE_CYCLES 0

// Log configuration
#define TRACE_CONFIG 1

// Log emulation details
#define TRACE_EMU 0

// Log wait/hold states
#define TRACE_WAITHOLD 0

// Log microinstruction processing
#define TRACE_MICRO 0

// Log interrupts
#define TRACE_INT 0

// Log interrupts (detailed phases)
#define TRACE_INTD 0

// Log clock pulses
#define TRACE_CLOCK 0

// Log READY line input
#define TRACE_READY 0

// Log memory access
#define TRACE_MEM 0

// Log address bus operation
#define TRACE_ADDRESSBUS 0

// Log CRU operations
#define TRACE_CRU 0

// Log status register
#define TRACE_STATUS 0

// Log operation
#define TRACE_OP 0

// Log decrementer operation
#define TRACE_DEC 0

// Log with max detail
#define TRACE_DETAIL 0

/****************************************************************************
    Constructor
****************************************************************************/

tms9995_device::tms9995_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TMS9995, "TMS9995", tag, owner, clock, "tms9995", __FILE__),
		m_state_any(0),
		PC(0),
		PC_debug(0),
		m_program_config("program", ENDIANNESS_BIG, 8, 16),
		m_io_config("cru", ENDIANNESS_BIG, 8, 16),
		m_prgspace(nullptr),
		m_cru(nullptr),
		m_external_operation(*this),
		m_iaq_line(*this),
		m_clock_out_line(*this),
		m_holda_line(*this),
		m_dbin_line(*this)
{
	m_mp9537 = false;
	m_check_overflow = false;
}

/*
    Called from subclass.
*/
tms9995_device::tms9995_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
		: cpu_device(mconfig, TMS9995, name, tag, owner, clock, shortname, source),
		m_state_any(0),
		PC(0),
		PC_debug(0),
		m_program_config("program", ENDIANNESS_BIG, 8, 16),
		m_io_config("cru", ENDIANNESS_BIG, 8, 16),
		m_prgspace(nullptr),
		m_cru(nullptr),
		m_external_operation(*this),
		m_iaq_line(*this),
		m_clock_out_line(*this),
		m_holda_line(*this),
		m_dbin_line(*this)
{
	m_check_overflow = false;
}


enum
{
	TMS9995_PC=0, TMS9995_WP, TMS9995_STATUS, TMS9995_IR,
	TMS9995_R0, TMS9995_R1, TMS9995_R2, TMS9995_R3,
	TMS9995_R4, TMS9995_R5, TMS9995_R6, TMS9995_R7,
	TMS9995_R8, TMS9995_R9, TMS9995_R10, TMS9995_R11,
	TMS9995_R12, TMS9995_R13, TMS9995_R14, TMS9995_R15
};

void tms9995_device::device_start()
{
	// TODO: Restore save state suport

	m_prgspace = &space(AS_PROGRAM);                        // dimemory.h
	m_cru = &space(AS_IO);

	// Resolve our external connections
	m_external_operation.resolve();
	m_iaq_line.resolve();
	m_clock_out_line.resolve();
	m_holda_line.resolve();
	m_dbin_line.resolve();

	// set our instruction counter
	m_icountptr = &m_icount;

	// Clear the interrupt flags
	m_int_pending = 0;

	m_mid_flag = false;
	m_mid_active = false;
	m_nmi_active = false;
	m_int_overflow = false;
	m_int_decrementer = false;

	m_idle_state = false;

	m_source_value = 0;

	// add the states for the debugger
	for (int i=0; i < 20; i++)
	{
		// callimport = need to use the state_import method to write to the state variable
		// callexport = need to use the state_export method to read the state variable
		state_add(i, s_statename[i], m_state_any).callimport().callexport().formatstr("%04X");
	}
	state_add(STATE_GENPC, "curpc", PC_debug).formatstr("%4s").noshow();
	state_add(STATE_GENFLAGS, "status", m_state_any).callimport().callexport().formatstr("%16s").noshow();

	// Set up the lookup table for command decoding
	build_command_lookup_table();

	if (TRACE_CONFIG) logerror("%s: Variant = %s, Overflow int = %s\n", tag(), m_mp9537? "MP9537 (no on-chip RAM)" : "with on-chip RAM", m_check_overflow? "check" : "no check");
}

void tms9995_device::device_stop()
{
	int k = 0;
	if (TRACE_CONFIG) logerror("%s: Deleting lookup tables\n", tag());
	while (m_lotables[k]!=nullptr) delete[] m_lotables[k++];
}

/*
    TMS9995 hard reset
    The device reset is just the emulator's trigger for the reset procedure
    which is invoked via the main loop.

    This also allows us to check the READY line at reset time, which is used
    to enable automatic wait state creation.
*/
void tms9995_device::device_reset()
{
	m_reset = true;     // for the main loop
	m_servicing_interrupt = false;   // only for debugging
	m_request_auto_wait_state = false;
	memset(m_flag, 0, sizeof(m_flag));
}

const char* tms9995_device::s_statename[20] =
{
	"PC ", "WP ", "ST ", "IR ",
	"R0 ", "R1 ", "R2 ", "R3 ",
	"R4 ", "R5 ", "R6 ", "R7 ",
	"R8 ", "R9 ", "R10", "R11",
	"R12", "R13", "R14", "R15"
};

/*
    Write the contents of a register by external input (debugger)
    Note: this is untested any may fail because of the prefetch feature of the CPU.
    In particular it may be required to adjust the PC.
*/
void tms9995_device::state_import(const device_state_entry &entry)
{
	int index = entry.index();
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			// no action here; we do not allow import, as the flags are all
			// bits of the STATUS register
			break;
		case TMS9995_PC:
			PC = (UINT16)m_state_any & 0xfffe;
			break;
		case TMS9995_WP:
			WP = (UINT16)m_state_any & 0xfffe;
			break;
		case TMS9995_STATUS:
			ST = (UINT16)m_state_any;
			break;
		case TMS9995_IR:
			m_instruction->IR = (UINT16)m_state_any;
			break;
		default:
			// Workspace registers
			if (index <= TMS9995_R15)
				write_workspace_register_debug(index-TMS9995_R0, (UINT16)m_state_any);
			break;
	}
}

/*
    Reads the contents of a register for display in the debugger.
*/
void tms9995_device::state_export(const device_state_entry &entry)
{
	int index = entry.index();
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_state_any = ST;
			break;
		case TMS9995_PC:
			m_state_any = PC_debug;
			break;
		case TMS9995_WP:
			m_state_any = WP;
			break;
		case TMS9995_STATUS:
			m_state_any = ST;
			break;
		case TMS9995_IR:
			m_state_any = m_instruction->IR;
			break;
		default:
			// Workspace registers
			if (index <= TMS9995_R15)
				m_state_any = read_workspace_register_debug(index-TMS9995_R0);
			break;
	}
}

/*
    state_string_export - export state as a string for the debugger
*/
void tms9995_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	static const char *statestr = "LAECOPX-----IIII";
	char flags[17];
	memset(flags, 0x00, ARRAY_LENGTH(flags));
	UINT16 val = 0x8000;
	if (entry.index()==STATE_GENFLAGS)
	{
		for (int i=0; i < 16; i++)
		{
			flags[i] = ((val & ST)!=0)? statestr[i] : '.';
			val = (val >> 1) & 0x7fff;
		}
	}
	str.assign(flags);
}

/*
    Provide access to the workspace registers via the debugger. We have to
    take care whether this is in onchip RAM or outside.
*/
UINT16 tms9995_device::read_workspace_register_debug(int reg)
{
	int temp = m_icount;
	UINT16 value;

	int addrb = (WP + (reg << 1)) & 0xfffe;

	if (is_onchip(addrb))
	{
		value = (m_onchip_memory[addrb & 0x00fe]<<8) | m_onchip_memory[(addrb & 0x00fe) + 1];
	}
	else
	{
		m_prgspace->set_debugger_access(true);
		value = (m_prgspace->read_byte(addrb) << 8) & 0xff00;
		value |= m_prgspace->read_byte(addrb+1);
		m_prgspace->set_debugger_access(false);
	}
	m_icount = temp;
	return value;
}

void tms9995_device::write_workspace_register_debug(int reg, UINT16 data)
{
	int temp = m_icount;
	int addrb = (WP + (reg << 1)) & 0xfffe;

	if (is_onchip(addrb))
	{
		m_onchip_memory[addrb & 0x00fe] = (data >> 8) & 0xff;
		m_onchip_memory[(addrb & 0x00fe) + 1] = data & 0xff;
	}
	else
	{
		m_prgspace->set_debugger_access(true);
		m_prgspace->write_byte(addrb, (data >> 8) & 0xff);
		m_prgspace->write_byte(addrb+1, data & 0xff);
		m_prgspace->set_debugger_access(false);
	}
	m_icount = temp;
}

const address_space_config *tms9995_device::memory_space_config(address_spacenum spacenum) const
{
	switch (spacenum)
	{
	case AS_PROGRAM:
		return &m_program_config;

	case AS_IO:
		return &m_io_config;

	default:
		return nullptr;
	}
}

/**************************************************************************
    Microprograms for the CPU instructions

    The actions which are specific to the respective instruction are
    invoked by repeated calls of ALU_xxx; each call increases a state
    variable so that on the next call, the next part can be processed.
    This saves us a lot of additional functions.
**************************************************************************/

/*
    Define the indices for the micro-operation table. This is done for the sake
    of a simpler microprogram definition as an UINT8[].
*/
enum
{
	PREFETCH,
	PREFETCH_NO_INT,
	MEMORY_READ,
	MEMORY_WRITE,
	WORD_READ,
	WORD_WRITE,
	OPERAND_ADDR,
	INCREG,
	INDX,
	SET_IMM,
	RETADDR,
	RETADDR1,
	CRU_INPUT,
	CRU_OUTPUT,
	ABORT,
	END,

	ALU_NOP,
	ALU_ADD_S_SXC,
	ALU_B,
	ALU_BLWP,
	ALU_C,
	ALU_CI,
	ALU_CLR_SETO,
	ALU_DIV,
	ALU_DIVS,
	ALU_EXTERNAL,
	ALU_F3,
	ALU_IMM_ARITHM,
	ALU_JUMP,
	ALU_LDCR,
	ALU_LI,
	ALU_LIMIWP,
	ALU_LSTWP,
	ALU_MOV,
	ALU_MPY,
	ALU_RTWP,
	ALU_SBO_SBZ,
	ALU_SHIFT,
	ALU_SINGLE_ARITHM,
	ALU_STCR,
	ALU_STSTWP,
	ALU_TB,
	ALU_X,
	ALU_XOP,
	ALU_INT
};

#define MICROPROGRAM(_MP) \
	static const UINT8 _MP[] =

/*
    Cycles:
    XXXX 1 => needs one cycle
    xxxx 1 (1) => needs one cycle when accessing internal memory, two for external mem
    PREFETCH 0 (1) => occurs during the last step in parallel, needs one more when fetching from outside
    DECODE not shown here; assumed to happen during the next memory cycle; if there is none,
    add another cycle

    OPERAND_ADDR x => needs x cycles for address derivation; see the separate table

    Prefetch always needs 1 or 2 cycles; the previous command occurs in parallel
    to the prefetch, so we assign a 0 to the previous microprogram step
*/

MICROPROGRAM(operand_address_derivation)
{
	RETADDR, 0, 0, 0,                           // Register direct                  0
	WORD_READ, RETADDR, 0, 0,                   // Register indirect                1 (1)
	WORD_READ, RETADDR, 0, 0,                   // Symbolic                         1 (1)
	WORD_READ, INCREG, WORD_WRITE, RETADDR1,    // Reg indirect auto-increment      3 (1) (1)
	WORD_READ, INDX, WORD_READ, RETADDR         // Indexed                          3 (1) (1)
};

MICROPROGRAM(add_s_sxc_mp)
{
	OPERAND_ADDR,           // x
	MEMORY_READ,            // 1 (1)
	OPERAND_ADDR,           // y
	MEMORY_READ,            // 1 (1)
	ALU_ADD_S_SXC,          // 0 (see above, occurs in parallel with PREFETCH)
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1) + decode in parallel (0)
	END
};

MICROPROGRAM(b_mp)
{
	OPERAND_ADDR,           // x
	ALU_NOP,                // 1 Don't read, just use the address
	ALU_B,                  // 0
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1 Don't save the return address
	END
};

MICROPROGRAM(bl_mp)
{
	OPERAND_ADDR,           // x
	ALU_NOP,                // 1 Don't read, just use the address
	ALU_B,                  // 0 Re-use the alu operation from B
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	MEMORY_WRITE,           // 1 (1) Write R11
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(blwp_mp)
{
	OPERAND_ADDR,           // x Determine source address
	MEMORY_READ,            // 1 (1)
	ALU_BLWP,               // 1 Got new WP, save it; increase address, save
	MEMORY_WRITE,           // 1 (1) save old ST to new R15
	ALU_BLWP,               // 1
	MEMORY_WRITE,           // 1 (1) save old PC to new R14
	ALU_BLWP,               // 1
	MEMORY_WRITE,           // 1 (1) save old WP to new R13
	ALU_BLWP,               // 1 retrieve address
	MEMORY_READ,            // 1 (1) Read new PC
	ALU_BLWP,               // 0 Set new PC
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(c_mp)
{
	OPERAND_ADDR,           // x
	MEMORY_READ,            // 1 (1)
	OPERAND_ADDR,           // y
	MEMORY_READ,            // 1 (1)
	ALU_C,                  // 0
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1 decode
	END
};

MICROPROGRAM(ci_mp)
{
	MEMORY_READ,            // 1 (1) (reg)
	SET_IMM,                // 0 belongs to next cycle
	MEMORY_READ,            // 1 (1) (imm)
	ALU_CI,                 // 0 set status
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1 decode
	END
};

MICROPROGRAM(coc_czc_mp)
{
	OPERAND_ADDR,           // x
	MEMORY_READ,            // 1 (1)
	ALU_F3,                 // 0
	MEMORY_READ,            // 1 (1)
	ALU_F3,                 // 0
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1 decode
	END
};

MICROPROGRAM(clr_seto_mp)
{
	OPERAND_ADDR,           // x
	ALU_NOP,                // 1
	ALU_CLR_SETO,           // 0
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(divide_mp)     // TODO: Verify cycles on the real machine
{
	OPERAND_ADDR,           // x Address of divisor S in Q=W1W2/S
	MEMORY_READ,            // 1 (1) Get S
	ALU_DIV,                // 1
	MEMORY_READ,            // 1 (1) Get W1
	ALU_DIV,                // 1 Check for overflow; skip next instruction if not
	ABORT,                  // 1
	MEMORY_READ,            // 1 (1) Get W2
	ALU_DIV,                // d Calculate quotient
	MEMORY_WRITE,           // 1 (1) Write quotient to &W1
	ALU_DIV,                // 0
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1) Write remainder to &W2
	END
};

MICROPROGRAM(divide_signed_mp)  // TODO: Verify cycles on the real machine
{
	OPERAND_ADDR,           // x Address of divisor S in Q=W1W2/S
	MEMORY_READ,            // 1 (1) Get S
	ALU_DIVS,               // 1
	MEMORY_READ,            // 1 (1) Get W1
	ALU_DIVS,               // 1
	MEMORY_READ,            // 1 (1) Get W2
	ALU_DIVS,               // 1 Check for overflow, skip next instruction if not
	ABORT,                  // 1
	ALU_DIVS,               // d Calculate quotient
	MEMORY_WRITE,           // 1 (1) Write quotient to &W1
	ALU_DIVS,               // 0
	PREFETCH,               // 1
	MEMORY_WRITE,           // 1 (1) Write remainder to &W2
	END
};

MICROPROGRAM(external_mp)
{
	ALU_NOP,                // 1
	ALU_NOP,                // 1
	ALU_NOP,                // 1
	ALU_NOP,                // 1
	ALU_NOP,                // 1
	ALU_EXTERNAL,           // 0
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(imm_arithm_mp)
{
	MEMORY_READ,            // 1 (1)
	SET_IMM,                // 0
	MEMORY_READ,            // 1 (1)
	ALU_IMM_ARITHM,         // 0
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(jump_mp)
{
	ALU_NOP,                // 1
	ALU_JUMP,               // 0
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(ldcr_mp)       // TODO: Verify cycles
{
	ALU_LDCR,               // 1
	OPERAND_ADDR,           // x
	MEMORY_READ,            // 1 (1) Get source data
	ALU_LDCR,               // 1 Save it, point to R12
	WORD_READ,              // 1 (1) Get R12
	ALU_LDCR,               // 1 Prepare CRU operation
	CRU_OUTPUT,             // c
	ALU_NOP,                // 0
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(li_mp)
{
	SET_IMM,                // 0
	MEMORY_READ,            // 1 (1)
	ALU_LI,                 // 0
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(limi_lwpi_mp)
{
	SET_IMM,                // 0
	MEMORY_READ,            // 1 (1)
	ALU_NOP,                // 1
	ALU_LIMIWP,             // 0 lwpi, 1 limi
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(lst_lwp_mp)
{
	MEMORY_READ,            // 1 (1)
	ALU_NOP,                // 1
	ALU_LSTWP,              // 0 lwp, 1 lst
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(mov_mp)
{
	OPERAND_ADDR,           // 0
	MEMORY_READ,            // 1 (1)
	OPERAND_ADDR,           // 0
	ALU_MOV,                // 0
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(multiply_mp)
{
	OPERAND_ADDR,           // x
	MEMORY_READ,            // 1 (1)
	ALU_MPY,                // 1
	MEMORY_READ,            // 1 (1)
	ALU_MPY,                // 17
	MEMORY_WRITE,           // 1 (1)
	ALU_MPY,                // 0
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(rtwp_mp)
{
	ALU_RTWP,               // 1
	MEMORY_READ,            // 1 (1)
	ALU_RTWP,               // 0
	MEMORY_READ,            // 1 (1)
	ALU_RTWP,               // 0
	MEMORY_READ,            // 1 (1)
	ALU_RTWP,               // 0
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(sbo_sbz_mp)
{
	ALU_SBO_SBZ,            // 1 Set address = &R12
	WORD_READ,              // 1 (1) Read R12
	ALU_SBO_SBZ,            // 1 Add offset
	CRU_OUTPUT,             // 1 output via CRU
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(shift_mp)
{
	MEMORY_READ,            // 1 (1)
	ALU_SHIFT,              // 2 skip next operation if count != 0
	MEMORY_READ,            // 1 (1) if count=0 we must read R0
	ALU_SHIFT,              // c  do the shift
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(single_arithm_mp)
{
	OPERAND_ADDR,           // x
	MEMORY_READ,            // 1 (1)
	ALU_SINGLE_ARITHM,      // 0
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(stcr_mp)       // TODO: Verify on real machine
{
	ALU_STCR,               // 1      Check for byte operation
	OPERAND_ADDR,           // x     Source operand
	ALU_STCR,               // 1      Save, set R12
	WORD_READ,              // 1 (1) Read R12
	ALU_STCR,               // 1
	CRU_INPUT,              // c
	ALU_STCR,               // 13
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(stst_stwp_mp)
{
	ALU_STSTWP,             // 0
	ALU_NOP,                // 1
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(tb_mp)
{
	ALU_TB,                 // 1
	WORD_READ,              // 1 (1)
	ALU_TB,                 // 1
	CRU_INPUT,              // 2
	ALU_TB,                 // 0
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(x_mp)
{
	OPERAND_ADDR,           // x
	MEMORY_READ,            // 1 (1)
	ALU_X,                  // 1
	END                     // should not be reached
};

MICROPROGRAM(xop_mp)
{
	OPERAND_ADDR,           // x     Determine source address
	ALU_XOP,                // 1     Save it; determine XOP number
	MEMORY_READ,            // 1 (1) Read new WP
	ALU_XOP,                // 1
	MEMORY_WRITE,           // 1 (1) save source address to new R11
	ALU_XOP,                // 1
	MEMORY_WRITE,           // 1 (1) save old ST to new R15
	ALU_XOP,                // 1
	MEMORY_WRITE,           // 1 (1) save old PC to new R14
	ALU_XOP,                // 1
	MEMORY_WRITE,           // 1 (1) save old WP to new R13
	ALU_XOP,                // 1
	MEMORY_READ,            // 1 (1) Read new PC
	ALU_XOP,                // 0 set new PC, set X flag
	PREFETCH,               // 1 (1)
	ALU_NOP,                // 1
	ALU_NOP,                // 1
	END
};

MICROPROGRAM(xor_mp)
{
	OPERAND_ADDR,           // x
	MEMORY_READ,            // 1 (1)
	ALU_F3,                 // 0
	MEMORY_READ,            // 1 (1)
	ALU_F3,                 // 0
	PREFETCH,               // 1 (1)
	MEMORY_WRITE,           // 1 (1)
	END
};

MICROPROGRAM(int_mp)
{
	ALU_INT,                // 1
	MEMORY_READ,            // 1 (1)
	ALU_INT,                // 2
	MEMORY_WRITE,           // 1 (1)
	ALU_INT,                // 1
	MEMORY_WRITE,           // 1 (1)
	ALU_INT,                // 1
	MEMORY_WRITE,           // 1 (1)
	ALU_INT,                // 1
	MEMORY_READ,            // 1 (1)
	ALU_INT,                // 0
	PREFETCH_NO_INT,        // 1 (1)  (prefetch happens in parallel to the previous operation)
	ALU_NOP,                // 1 (+decode in parallel; actually performed right after prefetch)
	ALU_NOP,                // 1
	END
};

const tms9995_device::ophandler tms9995_device::s_microoperation[] =
{
	&tms9995_device::int_prefetch_and_decode,
	&tms9995_device::prefetch_and_decode,
	&tms9995_device::mem_read,
	&tms9995_device::mem_write,
	&tms9995_device::word_read,
	&tms9995_device::word_write,
	&tms9995_device::operand_address_subprogram,
	&tms9995_device::increment_register,
	&tms9995_device::indexed_addressing,
	&tms9995_device::set_immediate,
	&tms9995_device::return_with_address,
	&tms9995_device::return_with_address_copy,
	&tms9995_device::cru_input_operation,
	&tms9995_device::cru_output_operation,
	&tms9995_device::abort_operation,
	&tms9995_device::command_completed,

	&tms9995_device::alu_nop,
	&tms9995_device::alu_add_s_sxc,
	&tms9995_device::alu_b,
	&tms9995_device::alu_blwp,
	&tms9995_device::alu_c,
	&tms9995_device::alu_ci,
	&tms9995_device::alu_clr_seto,
	&tms9995_device::alu_divide,
	&tms9995_device::alu_divide_signed,
	&tms9995_device::alu_external,
	&tms9995_device::alu_f3,
	&tms9995_device::alu_imm_arithm,
	&tms9995_device::alu_jump,
	&tms9995_device::alu_ldcr,
	&tms9995_device::alu_li,
	&tms9995_device::alu_limi_lwpi,
	&tms9995_device::alu_lst_lwp,
	&tms9995_device::alu_mov,
	&tms9995_device::alu_multiply,
	&tms9995_device::alu_rtwp,
	&tms9995_device::alu_sbo_sbz,
	&tms9995_device::alu_shift,
	&tms9995_device::alu_single_arithm,
	&tms9995_device::alu_stcr,
	&tms9995_device::alu_stst_stwp,
	&tms9995_device::alu_tb,
	&tms9995_device::alu_x,
	&tms9995_device::alu_xop,
	&tms9995_device::alu_int
};

/*****************************************************************************
    CPU instructions
*****************************************************************************/

/*
    Available instructions
    MID is not a real instruction but stands for an invalid operation which
    triggers a "macro instruction detect" interrupt. Neither is INTR which
    indicates an interrupt handling in progress.
*/
enum
{
	MID=0, A, AB, ABS, AI, ANDI, B, BL, BLWP, C,
	CB, CI, CKOF, CKON, CLR, COC, CZC, DEC, DECT, DIV,
	DIVS, IDLE, INC, INCT, INV, JEQ, JGT, JH, JHE, JL,
	JLE, JLT, JMP, JNC, JNE, JNO, JOC, JOP, LDCR, LI,
	LIMI, LREX, LST, LWP, LWPI, MOV, MOVB, MPY, MPYS, NEG,
	ORI, RSET, RTWP, S, SB, SBO, SBZ, SETO, SLA, SOC,
	SOCB, SRA, SRC, SRL, STCR, STST, STWP, SWPB, SZC, SZCB,
	TB, X, XOP, XOR, INTR
};

static const char opname[][5] =
{   "MID ", "A   ", "AB  ", "ABS ", "AI  ", "ANDI", "B   ", "BL  ", "BLWP", "C   ",
	"CB  ", "CI  ", "CKOF", "CKON", "CLR ", "COC ", "CZC ", "DEC ", "DECT", "DIV ",
	"DIVS", "IDLE", "INC ", "INCT", "INV ", "JEQ ", "JGT ", "JH  ", "JHE ", "JL  ",
	"JLE ", "JLT ", "JMP ", "JNC ", "JNE ", "JNO ", "JOC ", "JOP ", "LDCR", "LI  ",
	"LIMI", "LREX", "LST ", "LWP ", "LWPI", "MOV ", "MOVB", "MPY ", "MPYS", "NEG ",
	"ORI ", "RSET", "RTWP", "S   ", "SB  ", "SBO ", "SBZ ", "SETO", "SLA ", "SOC ",
	"SOCB", "SRA ", "SRC ", "SRL ", "STCR", "STST", "STWP", "SWPB", "SZC ", "SZCB",
	"TB  ", "X   ", "XOP ", "XOR ", "*int"
};

/*
    Formats:

          0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    ----+------------------------------------------------+
    1   | Opcode | B | Td |  RegNr     | Ts |    RegNr   |
        +--------+---+----+------------+----+------------+
    2   |  Opcode               |      Displacement      |
        +-----------------------+------------------------+
    3   |  Opcode         |  RegNr     | Ts |    RegNr   |
        +-----------------+------------+----+------------+
    4   |  Opcode         |  Count     | Ts |    RegNr   |
        +-----------------+------------+----+------------+
    5   |  Opcode               |  Count    |    RegNr   |
        +-----------------------+-----------+------------+
    6   |  Opcode                      | Ts |    RegNr   |
        +------------------------------+----+------------+
    7   |  Opcode                         |0| 0| 0| 0| 0 |
        +---------------------------------+-+--+--+--+---+
    8   |  Opcode                         |0|    RegNr   |
        +---------------------------------+-+------------+
    9   |  Opcode         |   Reg/Nr   | Ts |    RegNr   |
        +-----------------+------------+----+------------+
    10  |  Opcode                      | Ts |    RegNr   |   (DIVS, MPYS)
        +------------------------------+----+------------+
    11  |  Opcode                           |    RegNr   |   (LST, LWP)
        +-----------------------------------+------------+
*/

/*
    Defines the number of bits from the left which are significant for the
    command in the respective format.
*/
static const int format_mask_len[] =
{
	0, 4, 8, 6, 6, 8, 10, 16, 12, 6, 10, 12
};

const tms9995_device::tms_instruction tms9995_device::s_command[] =
{
	// Base opcode list
	// Opcode, ID, format, microprg
	{ 0x0080, LST, 11, lst_lwp_mp },
	{ 0x0090, LWP, 11, lst_lwp_mp },
	{ 0x0180, DIVS, 10, divide_signed_mp },
	{ 0x01C0, MPYS, 10, multiply_mp },
	{ 0x0200, LI, 8, li_mp },
	{ 0x0220, AI, 8, imm_arithm_mp },
	{ 0x0240, ANDI, 8, imm_arithm_mp },
	{ 0x0260, ORI, 8, imm_arithm_mp },
	{ 0x0280, CI, 8, ci_mp },
	{ 0x02a0, STWP, 8, stst_stwp_mp },
	{ 0x02c0, STST, 8, stst_stwp_mp },
	{ 0x02e0, LWPI, 8, limi_lwpi_mp },
	{ 0x0300, LIMI, 8, limi_lwpi_mp },
	{ 0x0340, IDLE, 7, external_mp },
	{ 0x0360, RSET, 7, external_mp },
	{ 0x0380, RTWP, 7, rtwp_mp },
	{ 0x03a0, CKON, 7, external_mp },
	{ 0x03c0, CKOF, 7, external_mp },
	{ 0x03e0, LREX, 7, external_mp },
	{ 0x0400, BLWP, 6, blwp_mp },
	{ 0x0440, B, 6, b_mp },
	{ 0x0480, X, 6, x_mp },
	{ 0x04c0, CLR, 6, clr_seto_mp },
	{ 0x0500, NEG, 6, single_arithm_mp },
	{ 0x0540, INV, 6, single_arithm_mp },
	{ 0x0580, INC, 6, single_arithm_mp },
	{ 0x05c0, INCT, 6, single_arithm_mp },
	{ 0x0600, DEC, 6, single_arithm_mp },
	{ 0x0640, DECT, 6, single_arithm_mp },
	{ 0x0680, BL, 6, bl_mp },
	{ 0x06c0, SWPB, 6, single_arithm_mp },
	{ 0x0700, SETO, 6, clr_seto_mp },
	{ 0x0740, ABS, 6, single_arithm_mp },
	{ 0x0800, SRA, 5, shift_mp },
	{ 0x0900, SRL, 5, shift_mp },
	{ 0x0a00, SLA, 5, shift_mp },
	{ 0x0b00, SRC, 5, shift_mp },
	{ 0x1000, JMP, 2, jump_mp },
	{ 0x1100, JLT, 2, jump_mp },
	{ 0x1200, JLE, 2, jump_mp },
	{ 0x1300, JEQ, 2, jump_mp },
	{ 0x1400, JHE, 2, jump_mp },
	{ 0x1500, JGT, 2, jump_mp },
	{ 0x1600, JNE, 2, jump_mp },
	{ 0x1700, JNC, 2, jump_mp },
	{ 0x1800, JOC, 2, jump_mp },
	{ 0x1900, JNO, 2, jump_mp },
	{ 0x1a00, JL, 2, jump_mp },
	{ 0x1b00, JH, 2, jump_mp },
	{ 0x1c00, JOP, 2, jump_mp },
	{ 0x1d00, SBO, 2, sbo_sbz_mp },
	{ 0x1e00, SBZ, 2, sbo_sbz_mp },
	{ 0x1f00, TB, 2, tb_mp },
	{ 0x2000, COC, 3, coc_czc_mp },
	{ 0x2400, CZC, 3, coc_czc_mp },
	{ 0x2800, XOR, 3, xor_mp },
	{ 0x2c00, XOP, 3, xop_mp },
	{ 0x3000, LDCR, 4, ldcr_mp },
	{ 0x3400, STCR, 4, stcr_mp },
	{ 0x3800, MPY, 9, multiply_mp },
	{ 0x3c00, DIV, 9, divide_mp },
	{ 0x4000, SZC, 1, add_s_sxc_mp },
	{ 0x5000, SZCB, 1, add_s_sxc_mp },
	{ 0x6000, S, 1, add_s_sxc_mp },
	{ 0x7000, SB, 1, add_s_sxc_mp },
	{ 0x8000, C, 1, c_mp },
	{ 0x9000, CB, 1, c_mp },
	{ 0xa000, A, 1, add_s_sxc_mp },
	{ 0xb000, AB, 1, add_s_sxc_mp },
	{ 0xc000, MOV, 1, mov_mp },
	{ 0xd000, MOVB, 1, mov_mp },
	{ 0xe000, SOC, 1, add_s_sxc_mp },
	{ 0xf000, SOCB, 1, add_s_sxc_mp }
};

/*
    Create a B-tree for looking up the commands. Each node can carry up to
    16 entries, indexed by 4 consecutive bits in the opcode.

    See tms9900.c for a detailed description.
*/
void tms9995_device::build_command_lookup_table()
{
	int i = 0;
	int cmdindex;
	int bitcount;
	const tms_instruction *inst;
	UINT16 opcode;
	int k = 0;

	m_command_lookup_table = new lookup_entry[16];
	// We use lotables as a list of allocated tables - to be able to delete them
	// at the end.
	m_lotables[k++] = m_command_lookup_table;

	lookup_entry* table = m_command_lookup_table;
	for (int j=0; j < 16; j++)
	{
		table[j].entry = nullptr;
		table[j].next_digit = nullptr;
	}

	do
	{
		inst = &s_command[i];
		table = m_command_lookup_table;
		if (TRACE_EMU) logerror("tms9995: === opcode=%04x, len=%d\n", inst->opcode, format_mask_len[inst->format]);
		bitcount = 4;
		opcode = inst->opcode;
		cmdindex = (opcode>>12) & 0x000f;

		while (bitcount < format_mask_len[inst->format])
		{
			// Descend
			if (table[cmdindex].next_digit == nullptr)
			{
				if (TRACE_EMU) logerror("tms9995: create new table at bitcount=%d for index=%d\n", bitcount, cmdindex);
				table[cmdindex].next_digit = new lookup_entry[16];
				m_lotables[k++] = table[cmdindex].next_digit;
				for (int j=0; j < 16; j++)
				{
					table[cmdindex].next_digit[j].next_digit = nullptr;
					table[cmdindex].next_digit[j].entry = nullptr;
				}
			}
			else
			{
				if (TRACE_EMU) logerror("tms9995: found a table at bitcount=%d\n", bitcount);
			}

			table = table[cmdindex].next_digit;

			bitcount = bitcount+4;
			opcode <<= 4;
			cmdindex = (opcode>>12) & 0x000f;
			if (TRACE_EMU) logerror("tms9995: next index=%x\n", cmdindex);
		}

		if (TRACE_EMU) logerror("tms9995: bitcount=%d\n", bitcount);
		// We are at the target level
		// Need to fill in the same entry for all values in the bitcount
		// (if a command needs 10 bits we have to copy it four
		// times for all combinations with 12 bits)
		for (int j=0; j < (1<<(bitcount-format_mask_len[inst->format])); j++)
		{
			if (TRACE_EMU) logerror("tms9995: opcode=%04x at position %d\n", inst->opcode, cmdindex+j);
			table[cmdindex+j].entry = inst;
		}

		i++;
	} while (inst->opcode != 0xf000);

	m_lotables[k++] = nullptr;
	if (TRACE_EMU) logerror("tms9995: Allocated %d tables\n", k);
}

/*
    Main execution loop

    For each invocation of execute_run, a number of loop iterations has been
    calculated before (m_icount). Each loop iteration is one clock cycle.
    The loop must be executed for the number of times that corresponds to the
    time until the next timer event.
*/
void tms9995_device::execute_run()
{
	if (m_reset) service_interrupt();

	if (TRACE_EMU) logerror("tms9995: calling execute_run for %d cycles\n", m_icount);
	do
	{
		// Normal operation
		if (m_check_ready && m_ready == false)
		{
			// We are in a wait state
			if (TRACE_WAITHOLD) logerror("tms9995: wait state\n");
			// The clock output should be used to change the state of an outer
			// device which operates the READY line
			pulse_clock(1);
		}
		else
		{
			if (m_check_hold && m_hold_state)
			{
				set_hold_state(true);
				if (TRACE_WAITHOLD) logerror("tms9995: hold state\n");
				pulse_clock(1);
			}
			else
			{
				set_hold_state(false);

				m_check_ready = false;

				if (TRACE_MICRO) logerror("tms9995: main loop, operation %s, MPC = %d\n", opname[m_instruction->command], MPC);
				(this->*s_microoperation[m_instruction->program[MPC]])();

				// For multi-pass operations where the MPC should not advance
				// or when we have put in a new microprogram
				m_pass--;
				if (m_pass<=0)
				{
					m_pass = 1;
					MPC++;
				}
			}
		}
	} while (m_icount>0 && !m_reset);
	if (TRACE_EMU) logerror("tms9995: cycles expired; will return soon.\n");
}

/**************************************************************************/

/*
    Interrupt input
    output
        m_nmi_state
        m_irq_level
        flag[2], flag[4]
*/
void tms9995_device::execute_set_input(int irqline, int state)
{
	if (irqline==INT_9995_RESET && state==ASSERT_LINE)
	{
		m_reset = true;
	}
	else
	{
		if (irqline == INPUT_LINE_NMI)
		{
			m_nmi_active = (state==ASSERT_LINE);
			if (TRACE_INT) logerror("tms9995: NMI interrupt line state=%d\n", state);
		}
		else
		{
			if (irqline == INT_9995_INT1)
			{
				m_int1_active = m_flag[2] = (state==ASSERT_LINE);
				if (TRACE_INT) logerror("tms9995: Line INT1 state=%d\n", state);
			}
			else
			{
				if (irqline == INT_9995_INT4)
				{
					if (TRACE_INT) logerror("tms9995: Line INT4/EC state=%d\n", state);
					if (m_flag[0]==false)
					{
						if (TRACE_INT) logerror("tms9995: set as interrupt\n");
						m_int4_active = m_flag[4] = (state==ASSERT_LINE);
					}
					else
					{
						if (TRACE_INT) logerror("tms9995: set as event count\n");
						trigger_decrementer();
					}
				}
				else
				{
					logerror("tms9995: Accessed invalid interrupt line %d\n", irqline);
				}
			}
		}
	}
}

/*
    Issue a pulse on the clock line.
*/
void tms9995_device::pulse_clock(int count)
{
	for (int i=0; i < count; i++)
	{
		if (!m_clock_out_line.isnull()) m_clock_out_line(ASSERT_LINE);
		m_ready = m_ready_bufd && !m_request_auto_wait_state;                // get the latched READY state
		if (!m_clock_out_line.isnull()) m_clock_out_line(CLEAR_LINE);
		m_icount--;                         // This is the only location where we count down the cycles.
		if (TRACE_CLOCK)
		{
			if (m_check_ready) logerror("tms9995: pulse_clock, READY=%d, auto_wait=%d\n", m_ready_bufd? 1:0, m_auto_wait? 1:0);
			else logerror("tms9995: pulse_clock\n");
		}
		m_request_auto_wait_state = false;
		if (m_flag[0] == false && m_flag[1] == true)
		{
			// Section 2.3.1.2.2: "by decreasing the count in the Decrementing
			// Register by one for each fourth CLKOUT cycle"
			m_decrementer_clkdiv = (m_decrementer_clkdiv+1)%4;
			if (m_decrementer_clkdiv==0) trigger_decrementer();
		}
	}
}

/*
    Enter the hold state.
*/
void tms9995_device::set_hold(int state)
{
	m_hold_state = (state==ASSERT_LINE);
	if (TRACE_WAITHOLD) logerror("tms9995: set HOLD = %d\n", state);
	if (!m_hold_state)
	{
		if (!m_holda_line.isnull()) m_holda_line(CLEAR_LINE);
	}
}

/*
    Signal READY to the CPU. When cleared, the CPU enters wait states. This
    becomes effective on a clock pulse.
*/
void tms9995_device::set_ready(int state)
{
	m_ready_bufd = (state==ASSERT_LINE);
	if (TRACE_READY) logerror("tms9995: set READY = %d\n", m_ready_bufd? 1 : 0);
}

/*
    When the divide operations fail, we get to this operation.
*/
void tms9995_device::abort_operation()
{
	int_prefetch_and_decode(); // do not forget to prefetch
	// And don't forget that prefetch is a 2-pass operation, so this method
	// will be called a second time. Only when the lowbyte has been fetched,
	// continue with the next step
	if (m_mem_phase==1) command_completed();
}

/*
    Enter or leave the hold state. We only operate the HOLDA line when there is a change.
*/
inline void tms9995_device::set_hold_state(bool state)
{
	if (m_hold_state != state)
		if (!m_holda_line.isnull()) m_holda_line(state? ASSERT_LINE : CLEAR_LINE);
	m_hold_state = state;
}

/*
    Decode the instruction. This is done in parallel to other operations
    so we just do it together with the prefetch.
*/
void tms9995_device::decode(UINT16 inst)
{
	int index = 0;
	lookup_entry* table = m_command_lookup_table;
	UINT16 opcode = inst;
	bool complete = false;
	const tms_instruction *decoded;

	int dindex = (m_instindex==0)? 1:0;

	m_mid_active = false;

	while (!complete)
	{
		index = (opcode >> 12) & 0x000f;
		if (TRACE_EMU) logerror("tms9995: Check next hex digit of instruction %x\n", index);
		if (table[index].next_digit != nullptr)
		{
			table = table[index].next_digit;
			opcode = opcode << 4;
		}
		else complete = true;
	}
	decoded = table[index].entry;
	if (decoded == nullptr)
	{
		// not found
		logerror("tms9995: Undefined opcode %04x at logical address %04x, will trigger MID\n", inst, PC);
		m_decoded[dindex].IR = 0;
		m_decoded[dindex].command = MID;
	}
	else
	{
		m_decoded[dindex].IR = inst;
		m_decoded[dindex].command = decoded->id;
		m_decoded[dindex].program = decoded->prog;
		m_decoded[dindex].byteop = ((decoded->format == 1) && ((inst & 0x1000)!=0));
		m_decoded[dindex].state = 0;
		if (TRACE_EMU) logerror("tms9995: Command decoded as id %d, %s, base opcode %04x\n", decoded->id, opname[decoded->id], decoded->opcode);
		m_pass = 1;
	}
}

/*
    Fetch the next instruction and check pending interrupts before.
    Getting an instruction is a normal memory access (plus an asserted IAQ line),
    so this is subject to wait state handling. We have to allow for a two-pass
    handling.
*/
void tms9995_device::int_prefetch_and_decode()
{
	bool check_int = (m_instruction->command != XOP && m_instruction->command != BLWP);
	int intmask = ST & 0x000f;

	if (m_mem_phase == 1)
	{
		// Check interrupt lines
		if (m_nmi_active)
		{
			if (TRACE_INT) logerror("tms9995: Checking interrupts ... NMI active\n");
			m_int_pending |= PENDING_NMI;
			m_idle_state = false;
			PC = (PC + 2) & 0xfffe;     // we have not prefetched the next instruction
			return;
		}
		else
		{
			m_int_pending = 0;

			if (check_int)
			{
				if (m_int1_active && intmask >= 1) m_int_pending |= PENDING_LEVEL1;
				if (m_int_overflow && intmask >= 2) m_int_pending |= PENDING_OVERFLOW;
				if (m_int_decrementer && intmask >= 3) m_int_pending |= PENDING_DECR;
				if (m_int4_active && intmask >= 4) m_int_pending |= PENDING_LEVEL4;
			}

			if (m_int_pending!=0)
			{
				if (m_idle_state)
				{
					m_idle_state = false;
					if (TRACE_INT) logerror("tms9995: Interrupt occurred, terminate IDLE state\n");
				}
				PC = PC + 2;        // PC must be advanced (see flow chart), but no prefetch
				if (TRACE_INT) logerror("tms9995: Interrupts pending; no prefetch; advance PC to %04x\n", PC);
				return;
			}
			else
			{
				if (TRACE_INT) logerror("tms9995: Checking interrupts ... none pending\n");
				// No pending interrupts
				if (m_idle_state)
				{
					if (TRACE_WAITHOLD) logerror("tms9995: IDLE state\n");
					// We are IDLE, stay in the loop and do not advance the PC
					m_pass = 2;
					pulse_clock(1);
					return;
				}
			}
		}
	}

	// We reach this point in phase 1 if there is no interrupt and in all other phases
	prefetch_and_decode();
}

/*
    The actual prefetch operation, but without the interrupt check. This one is
    needed when we complete the interrupt handling and need to get the next
    instruction. According to the flow chart in [1], the prefetch after the
    interrupt handling ignores other pending interrupts.
*/
void tms9995_device::prefetch_and_decode()
{
	if (m_mem_phase==1)
	{
		// Fetch next instruction
		// Save these values; they have been computed during the current instruction execution
		m_address_copy = m_address;
		m_value_copy = m_current_value;
		if (!m_iaq_line.isnull()) m_iaq_line(ASSERT_LINE);
		m_address = PC;
		if (TRACE_OP) logerror("tms9995: **** Prefetching new instruction at %04x ****\n", PC);
	}

	word_read(); // changes m_mem_phase

	if (m_mem_phase==1)
	{
		// We're back in phase 1, i.e. the whole prefetch is done
		decode(m_current_value);    // This is for free; in reality it is in parallel with the next memory operation
		m_address = m_address_copy;     // restore m_address
		m_current_value = m_value_copy; // restore m_current_value
		PC = (PC + 2) & 0xfffe;     // advance PC
		if (!m_iaq_line.isnull()) m_iaq_line(CLEAR_LINE);
		if (TRACE_OP) logerror("tms9995: ++++ Prefetch done ++++\n");
	}
}

/*
    Used by the normal command completion as well as by the X operation. We
    assume that we have a fully decoded operation which was previously
    prefetched.
*/
void tms9995_device::next_command()
{
	int next = (m_instindex==0)? 1:0;

	if (m_decoded[next].command == MID)
	{
		m_mid_flag = true;
		m_mid_active = true;
		service_interrupt();
	}
	else
	{
		m_instindex = next;
		m_instruction = &m_decoded[m_instindex];
		m_get_destination = false;
		// This is a preset for opcodes which do not need an opcode address derivation
		m_address = WP + ((m_instruction->IR & 0x000f)<<1);
		MPC = -1;
		if (TRACE_OP) logerror("tms9995: ===== Next operation %04x (%s) at %04x =====\n", m_instruction->IR, opname[m_instruction->command], PC-2);

		if (TRACE_EXEC)
		{
			if (m_servicing_interrupt) logerror("i%04x\n", PC-2);
			else logerror("%04x\n", PC-2);
		}
		PC_debug = PC - 2;
		debugger_instruction_hook(this, PC_debug);
		m_first_cycle = m_icount;
	}
}

/*
    End of command execution
*/
void tms9995_device::command_completed()
{
	// Pseudo state at the end of the current instruction cycle sequence
	if (TRACE_CYCLES)
	{
		logerror("tms9995: +++++ Instruction %04x (%s) completed", m_instruction->IR, opname[m_instruction->command]);
		int cycles =  m_first_cycle - m_icount;
		// Avoid nonsense values due to expired and resumed main loop
		if (cycles > 0 && cycles < 10000) logerror(", consumed %d cycles", cycles);
		logerror(" +++++\n");
	}

	if (m_int_pending != 0)
	{
		service_interrupt();
	}
	else
	{
		if ((ST & ST_OE)!=0 && (ST & ST_OV)!=0 && (ST & 0x000f)>2)
		{
			service_interrupt();
		}
		else
		{
			next_command();
		}
	}
}

/*
    Handle pending interrupts.
*/
void tms9995_device::service_interrupt()
{
	int vectorpos;

	if (m_reset)
	{
		vectorpos = 0;
		m_intmask = 0;  // clear interrupt mask

		m_nmi_state = false;
		m_hold_state = false;
		m_mem_phase = 1;
		m_check_hold = false;
		m_word_access = false;
		m_int1_active = false;
		m_int4_active = false;
		m_decrementer_clkdiv = 0;

		m_pass = 0;
		m_instindex = 0;
		m_instruction = &m_decoded[m_instindex];

		memset(m_flag, 0, sizeof(m_flag));

		ST = 0;

		// The auto-wait state generation is turned on when the READY line is cleared
		// on RESET.
		m_auto_wait = !m_ready_bufd;
		if (TRACE_CONFIG) logerror("tms9995: RESET; automatic wait state creation is %s\n", m_auto_wait? "enabled":"disabled");
		// We reset the READY flag, or the CPU will not start
		m_ready_bufd = true;
	}
	else
	{
		if (m_mid_active)
		{
			vectorpos = 0x0008;
			m_intmask = 0x0001;
			PC = (PC + 2) & 0xfffe;
			if (TRACE_INT) logerror("tms9995: ***** MID pending\n");
			m_mid_active = false;
		}
		else
		{
			if ((m_int_pending & PENDING_NMI)!=0)
			{
				vectorpos = 0xfffc;
				m_int_pending &= ~PENDING_NMI;
				m_intmask = 0;
				if (TRACE_INT) logerror("tms9995: ***** NMI pending\n");
			}
			else
			{
				if ((m_int_pending & PENDING_LEVEL1)!=0)
				{
					vectorpos = 0x0004;
					m_int_pending &= ~PENDING_LEVEL1;
					m_flag[2] = false;
					m_intmask = 0;
					if (TRACE_INT) logerror("tms9995: ***** INT1 pending\n");
				}
				else
				{
					if ((m_int_pending & PENDING_OVERFLOW)!=0)
					{
						vectorpos = 0x0008;
						m_int_pending &= ~PENDING_OVERFLOW;
						m_intmask = 0x0001;
						if (TRACE_INT) logerror("tms9995: ***** OVERFL pending\n");
					}
					else
					{
						if ((m_int_pending & PENDING_DECR)!=0)
						{
							vectorpos = 0x000c;
							m_intmask = 0x0002;
							m_int_pending &= ~PENDING_DECR;
							m_flag[3] = false;
							m_int_decrementer = false;
							if (TRACE_DEC) logerror("tms9995: ***** DECR pending\n");
						}
						else
						{
							vectorpos = 0x0010;
							m_intmask = 0x0003;
							m_int_pending &= ~PENDING_LEVEL4;
							m_flag[4] = false;
							if (TRACE_INT) logerror("tms9995: ***** INT4 pending\n");
						}
					}
				}
			}
		}
	}

	if (TRACE_INT) logerror("tms9995: ********* triggered an interrupt with vector %04x/%04x\n", vectorpos, vectorpos+2);

	// just for debugging purposes
	m_servicing_interrupt = true;

	// The microinstructions will do the context switch
	m_address = vectorpos;
	m_instruction->program = int_mp;
	m_instruction->state = 0;
	m_instruction->byteop = false;
	m_instruction->command = INTR;
	m_pass = m_reset? 1 : 2;
	m_from_reset = m_reset;

	if (m_reset)
	{
		m_instruction->IR = 0x0000;
		m_reset = false;
	}
	MPC = 0;
	m_first_cycle = m_icount;
	m_check_ready = false;      // set to default
}

/*
    Read memory. This method expects as input m_address, and delivers the value
    in m_current_value. For a single byte read, the byte is put into the high byte.
    This method uses the m_pass variable to achieve a two-pass handling for
    getting the complete word (high byte, low byte).

    input:
        m_address
        m_lowbyte
    output:
        m_current_value

    m_address is unchanged

    Make sure that m_lowbyte is false on the first call.
*/
void tms9995_device::mem_read()
{
	// First determine whether the memory is inside the CPU
	// On-chip memory is F000 ... F0F9, F0FA-FFF9 = off-chip, FFFA/B = Decrementer
	// FFFC-FFFF = NMI vector (on-chip)
	// There is a variant of the TMS9995 with no on-chip RAM which was used
	// for the TI-99/8 (9537).

	if ((m_address & 0xfffe)==0xfffa && !m_mp9537)
	{
		if (TRACE_DEC) logerror("tms9995: read decrementer\n");
		// Decrementer mapped into the address space
		m_current_value = m_decrementer_value;
		if (m_instruction->byteop)
		{
			if ((m_address & 1)!=1) m_current_value <<= 8;
			m_current_value &= 0xff00;
		}
		pulse_clock(1);
		return;
	}

	if (is_onchip(m_address))
	{
		// If we have a word access, we have to align the address
		// This is the case for word operations and for certain phases of
		// byte operations (e.g. when retrieving the index register)
		if (m_word_access || !m_instruction->byteop) m_address &= 0xfffe;

		if (TRACE_MEM) logerror("tms9995: read onchip memory (single pass, address %04x)\n", m_address);

		// Ignore the READY state
		m_check_ready = false;
		// We put fffc-ffff back into the f000-f0ff area
		m_current_value = m_onchip_memory[m_address & 0x00ff]<<8;
		if (m_word_access || !m_instruction->byteop)
		{
			// We have a word operation; add the low byte right here (just 1 cycle)
			m_current_value |= (m_onchip_memory[(m_address & 0x00ff)+1] & 0xff);
		}
		pulse_clock(1);
	}
	else
	{
		// This is an off-chip access
		m_check_ready = true;
		UINT8 value;
		UINT16 address = m_address;

		switch (m_mem_phase)
		{
		case 1:
			// Set address
			// If this is a word access, 4 passes, else 2 passes
			if (!m_dbin_line.isnull()) m_dbin_line(ASSERT_LINE);
			if (m_word_access || !m_instruction->byteop)
			{
				m_pass = 4;
				// For word accesses, we always start at the even address
				address &= 0xfffe;
			}
			else m_pass = 2;

			m_check_hold = false;
			if (TRACE_ADDRESSBUS) logerror("tms9995: set address bus %04x\n", m_address & ~1);
			m_prgspace->set_address(address);
			m_request_auto_wait_state = m_auto_wait;
			pulse_clock(1);
			break;
		case 2:
			// Sample the value on the data bus (high byte)
			if (m_word_access || !m_instruction->byteop) address &= 0xfffe;
			value = m_prgspace->read_byte(address);
			if (TRACE_MEM) logerror("tms9995: memory read byte %04x -> %02x\n", m_address & ~1, value);
			m_current_value = (value << 8) & 0xff00;
			break;
		case 3:
			// Set address + 1 (unless byte command)
			if (TRACE_ADDRESSBUS) logerror("tms9995: set address bus %04x\n", m_address | 1);
			m_prgspace->set_address(m_address | 1);
			pulse_clock(1);
			break;
		case 4:
			// Read low byte
			value = m_prgspace->read_byte(m_address | 1);
			m_current_value |= value;
			if (TRACE_MEM) logerror("tms9995: memory read byte %04x -> %02x, complete word = %04x\n", m_address | 1, value, m_current_value);
			m_check_hold = true;
			break;
		}

		m_mem_phase = (m_mem_phase % 4) +1;

		// Reset to 1 when we are done
		if (m_pass==1) m_mem_phase = 1;
	}
}

/*
    Read a word. This is independent of the byte flag of the instruction.
    We need this variant especially when we have to retrieve a register value
    in indexed addressing within a byte-oriented operation.
*/
inline void tms9995_device::word_read()
{
	m_word_access = true;
	mem_read();
	m_word_access = false;
}

/*
    Write memory. This method expects as input m_address and m_current_value.
    For a single byte write, the byte to be written is expected to be in the
    high byte of m_current_value.
    This method uses the m_pass variable to achieve a two-pass handling for
    writing the complete word (high byte, low byte).

    input:
        m_address
        m_lowbyte
        m_current_value

    output:
        -
    m_address is unchanged

    Make sure that m_lowbyte is false on the first call.
*/
void tms9995_device::mem_write()
{
	if ((m_address & 0xfffe)==0xfffa && !m_mp9537)
	{
		if (m_instruction->byteop)
		{
			// According to [1], section 2.3.1.2.2:
			// "The decrementer should always be accessed as a full word. [...]
			// Writing a single byte to either of the bytes of the decrementer
			// will result in the data byte being written into the byte specifically addressed
			// and random bits being written into the other byte of the decrementer."

			// So we just don't care about the low byte.
			if (m_address == 0xfffb) m_current_value >>= 8;

			// dito: "This also loads the Decrementing Register with the same count."
			m_starting_count_storage_register = m_decrementer_value = m_current_value;
		}
		else
		{
			m_starting_count_storage_register = m_decrementer_value = m_current_value;
		}
		if (TRACE_DEC) logerror("tms9995: Setting decrementer to %04x, PC=%04x\n", m_current_value, PC);
		pulse_clock(1);
		return;
	}

	if (is_onchip(m_address))
	{
		// If we have a word access, we have to align the address
		// This is the case for word operations and for certain phases of
		// byte operations (e.g. when retrieving the index register)
		if (m_word_access || !m_instruction->byteop) m_address &= 0xfffe;

		if (TRACE_MEM) logerror("tms9995: write to onchip memory (single pass, address %04x, value=%04x)\n", m_address, m_current_value);
		m_check_ready = false;
		m_onchip_memory[m_address & 0x00ff] = (m_current_value >> 8) & 0xff;
		if (m_word_access || !m_instruction->byteop)
		{
			m_onchip_memory[(m_address & 0x00ff)+1] = m_current_value & 0xff;
		}
		pulse_clock(1);
	}
	else
	{
		// This is an off-chip access
		m_check_ready = true;
		UINT16 address = m_address;
		switch (m_mem_phase)
		{
		case 1:
			// Set address
			// If this is a word access, 4 passes, else 2 passes
			if (!m_dbin_line.isnull()) m_dbin_line(CLEAR_LINE);

			if (m_word_access || !m_instruction->byteop)
			{
				m_pass = 4;
				address &= 0xfffe;
			}
			else m_pass = 2;

			m_check_hold = false;
			if (TRACE_ADDRESSBUS) logerror("tms9995: set address bus %04x\n", address);
			m_prgspace->set_address(address);
			if (TRACE_MEM) logerror("tms9995: memory write byte %04x <- %02x\n", address, (m_current_value >> 8)&0xff);
			m_prgspace->write_byte(address, (m_current_value >> 8)&0xff);
			pulse_clock(1);
			break;

		case 2:
			// no action here, just wait for READY
			break;
		case 3:
			// Set address + 1 (unless byte command)
			if (TRACE_ADDRESSBUS) logerror("tms9995: set address bus %04x\n", m_address | 1);
			m_prgspace->set_address(m_address | 1);
			if (TRACE_MEM) logerror("tms9995: memory write byte %04x <- %02x\n", m_address | 1, m_current_value & 0xff);
			m_prgspace->write_byte(m_address | 1, m_current_value & 0xff);
			pulse_clock(1);
			break;
		case 4:
			// no action here, just wait for READY
			m_check_hold = true;
			break;
		}

		m_mem_phase = (m_mem_phase % 4) +1;

		// Reset to 1 when we are done
		if (m_pass==1) m_mem_phase = 1;
	}
}

/*
    Write a word. This is independent of the byte flag of the instruction.
*/
inline void tms9995_device::word_write()
{
	m_word_access = true;
	mem_write();
	m_word_access = false;
}

/*
    Returns from the operand address derivation.
*/
void tms9995_device::return_with_address()
{
	// Return from operand address derivation
	// The result should be in m_address
	m_instruction->program = m_caller;
	MPC = m_caller_MPC; // will be increased on return
	m_address = m_current_value + m_address_add;
	if (TRACE_DETAIL) logerror("tms9995: +++ return from operand address derivation +++\n");
	// no clock pulse
}

/*
    Returns from the operand address derivation, but using the saved address.
    This is required when we use the auto-increment feature.
*/
void tms9995_device::return_with_address_copy()
{
	// Return from operand address derivation
	m_instruction->program = m_caller;
	MPC = m_caller_MPC; // will be increased on return
	m_address = m_address_saved;
	if (TRACE_DETAIL) logerror("tms9995: +++ return from operand address derivation (auto inc) +++\n");
	// no clock pulse
}

/*
    CRU support code
    See common explanations in tms9900.c

    The TMS9995 CRU address space is larger than the CRU space of the TMS9900:
    0000-fffe (even addresses) instead of 0000-1ffe. Unlike the TMS9900, the
    9995 uses the data bus lines D0-D2 to indicate external operations.

    Internal CRU locations (read/write)
    -----------------------------------
    1EE0 Flag 0     Decrementer as event counter
    1EE2 Flag 1     Decrementer enable
    1EE4 Flag 2     Level 1 interrupt present (read only, also set when interrupt mask disallows interrupts)
    1EE6 Flag 3     Level 3 interrupt present (see above)
    1EE8 Flag 4     Level 4 interrupt present (see above)
    ...
    1EFE Flag 15
    1FDA MID flag (only indication, does not trigger when set)

    The TMS9995 allows for wait states during external CRU access. Therefore
    we read one block of 8 bits in one go (as given by the MESS architecture)
    but we do iterations for each bit, checking every time for the READY line
    in the main loop.

        (write)
        m_cru_output
        m_cru_address
        m_cru_value
        m_count

*/

#define CRUREADMASK 0x0fff
#define CRUWRITEMASK 0x7fff

void tms9995_device::cru_output_operation()
{
	if (TRACE_CRU) logerror("tms9995: CRU output operation, address %04x, value %d\n", m_cru_address, m_cru_value & 0x01);

	if (m_cru_address == 0x1fda)
	{
		// [1], section 2.3.3.2.2: "setting the MID flag to one with a CRU instruction
		// will not cause the MID interrupt to be requested."
		m_check_ready = false;
		m_mid_flag = (m_cru_value & 0x01);
	}
	else
	{
		if ((m_cru_address & 0xffe0) == 0x1ee0)
		{
			m_check_ready = false;
			// FLAG2, FLAG3, and FLAG4 are read-only
			if (TRACE_CRU) logerror("tms9995: set CRU address %04x to %d\n", m_cru_address, m_cru_value&1);
			if ((m_cru_address != 0x1ee4) && (m_cru_address != 0x1ee6) && (m_cru_address != 0x1ee8))
				m_flag[(m_cru_address>>1)&0x000f] = (m_cru_value & 0x01);
		}
		else
		{
			// External access
			m_check_ready = true;
		}
	}

	// All CRU write operations are visible to the outside world, even when we
	// have internal access. This makes it possible to assign special
	// functions to the internal flag bits which are realized outside
	// of the CPU. However, no wait states are generated for internal
	// accesses. ([1], section 2.3.3.2)

	m_cru->write_byte((m_cru_address >> 1)& CRUWRITEMASK, (m_cru_value & 0x01));
	m_cru_value >>= 1;
	m_cru_address = (m_cru_address + 2) & 0xfffe;
	m_count--;

	// Repeat this operation
	m_pass = (m_count > 0)? 2 : 1;
	pulse_clock(2);
}

/*
    Input: (read)
        m_cru_multi_first
        m_cru_address
    Output:
        m_cru_value (right-shifted; i.e. first bit is LSB of the 16 bit word,
        also for byte operations)
*/

void tms9995_device::cru_input_operation()
{
	UINT16 crubit;
	UINT8 crubyte;

	// Reading is different, since MESS uses 8 bit transfers
	// We read 8 bits in one go, then iterate another min(n-1,7) times to allow
	// for wait states.

	// read_byte for CRU delivers the first bit on the rightmost position

	int offset = (m_cru_address>>1) & 0x07;

	if (m_cru_first_read || m_cru_bits_left == 0)
	{
		// Read next 8 bits
		// 00000000 0rrrrrrr r
		//                   v
		// ........ ........ X....... ........
		//
		crubyte = m_cru->read_byte((m_cru_address >> 4)& CRUREADMASK);
		if (TRACE_DETAIL) logerror("tms9995: Need to get next 8 bits (addresses %04x-%04x): %02x\n", (m_cru_address&0xfff0)+14, m_cru_address&0xfff0, crubyte);
		m_cru_read = crubyte << 15;
		m_cru_bits_left = 8;

		if (m_cru_first_read)
		{
			m_cru_read >>= offset;
			m_cru_bits_left -= offset;
			m_parity = 0;
			m_cru_value = 0;
			m_cru_first_read = false;
			m_pass = m_count;
		}
		if (TRACE_DETAIL) logerror("tms9995: adjusted value for shift: %06x\n", m_cru_read);
	}

	crubit = (m_cru_read & 0x8000);
	m_cru_value = (m_cru_value >> 1) & 0x7fff;

	// During internal reading, the CRUIN line will be ignored. We emulate this
	// by overwriting the bit which we got from outside. Also, READY is ignored.
	if (m_cru_address == 0x1fda)
	{
		crubit = m_mid_flag? 0x8000 : 0x0000;
		m_check_ready = false;
	}
	else
	{
		if ((m_cru_address & 0xffe0)==0x1ee0)
		{
			crubit = (m_flag[(m_cru_address>>1)&0x000f]==true)? 0x8000 : 0x0000;
			m_check_ready = false;
		}
		else
		{
			m_check_ready = true;
		}
	}

	if (TRACE_CRU) logerror("tms9995: CRU input operation, address %04x, value %d\n", m_cru_address, (crubit & 0x8000)>>15);

	m_cru_value |= crubit;
	if (crubit!=0) m_parity++;

	m_cru_address = (m_cru_address + 2) & 0xfffe;
	m_cru_bits_left--;

	if (m_pass > 1)
	{
		m_cru_read >>= 1;
	}
	else
	{
		// This is the final shift. For both byte and word length transfers,
		// the first bit is always m_cru_value & 0x0001.
		m_cru_value >>= (16 - m_count);
	}
	pulse_clock(2);
}

/*
    Decrementer.
*/
void tms9995_device::trigger_decrementer()
{
	if (m_starting_count_storage_register>0) // null will turn off the decrementer
	{
		m_decrementer_value--;
		if (m_decrementer_value==0)
		{
			if (TRACE_DEC) logerror("tms9995: decrementer reached 0\n");
			m_decrementer_value = m_starting_count_storage_register;
			if (m_flag[1]==true)
			{
				if (TRACE_DEC) logerror("tms9995: decrementer flags interrupt\n");
				m_flag[3] = true;
				m_int_decrementer = true;
			}
		}
	}
}

/*
    This is a switch to a subprogram. In terms of cycles
    it does not take any time; execution continues with the first instruction
    of the subprogram.

    input:
        m_get_destination
        m_instruction
        WP
        m_current_value
        m_address
    output:
        m_source_value  = m_current_value before invocation
        m_current_value = m_address
        m_address_add   = 0
        m_lowbyte       = false
        m_get_destination = true
        m_regnumber     = register number
        m_address       = address of register
    */
void tms9995_device::operand_address_subprogram()
{
	UINT16 ircopy = m_instruction->IR;
	if (m_get_destination) ircopy = ircopy >> 6;

	// Save the return program and position
	m_caller = m_instruction->program;
	m_caller_MPC = MPC;

	m_instruction->program = (UINT8*)operand_address_derivation;
	MPC = (ircopy & 0x0030) >> 2;
	m_regnumber = (ircopy & 0x000f);
	m_address = (WP + (m_regnumber<<1)) & 0xffff;

	m_source_value = m_current_value;   // will be overwritten when reading the destination
	m_current_value = m_address;        // needed for first case

	if (MPC==8) // Symbolic
	{
		if (m_regnumber != 0)
		{
			if (TRACE_DETAIL) logerror("tms9995: indexed addressing\n");
			MPC = 16; // indexed
		}
		else
		{
			if (TRACE_DETAIL) logerror("tms9995: symbolic addressing\n");
			m_address = PC;
			PC = (PC + 2) & 0xfffe;
		}
	}

	m_get_destination = true;
	m_mem_phase = 1;
	m_address_add = 0;
	MPC--;      // will be increased in the mail loop
	if (TRACE_DETAIL) logerror("tms9995: *** Operand address derivation; address=%04x; index=%d\n", m_address, MPC+1);
}

/*
    Used for register auto-increment. We have to save the address read from the
    register content so that we can return it at the end.
*/
void tms9995_device::increment_register()
{
	m_address_saved = m_current_value;  // need a special return so we do not lose the value
	m_current_value += m_instruction->byteop? 1 : 2;
	m_address = (WP + (m_regnumber<<1)) & 0xffff;
	m_mem_phase = 1;
	pulse_clock(1);
}

/*
    Used for indexed addressing. We store the contents of the index register
    in m_address_add which is set to 0 by default. Then we set the address
    pointer to the PC location and advance it.
*/
void tms9995_device::indexed_addressing()
{
	m_address_add = m_current_value;
	m_address = PC;
	PC = (PC + 2) & 0xfffe;
	m_mem_phase = 1;
	pulse_clock(1);
}

void tms9995_device::set_immediate()
{
	// Need to determine the register address
	m_address_saved = WP + ((m_instruction->IR & 0x000f)<<1);
	m_address = PC;
	m_source_value = m_current_value;       // needed for AI, ANDI, ORI
	PC = (PC + 2) & 0xfffe;
	m_mem_phase = 1;
}

/**************************************************************************
    Status bit operations
**************************************************************************/

inline void tms9995_device::set_status_bit(int bit, bool state)
{
	if (state) ST |= bit;
	else ST &= ~bit;
	m_int_overflow = (m_check_overflow && bit == ST_OV  && ((ST & ST_OE)!=0) && state == true);
}

void tms9995_device::set_status_parity(UINT8 value)
{
	int count = 0;
	for (int i=0; i < 8; i++)
	{
		if ((value & 0x80)!=0) count++;
		value <<= 1;
	}
	set_status_bit(ST_OP, (count & 1)!=0);
}

inline void tms9995_device::compare_and_set_lae(UINT16 value1, UINT16 value2)
{
	set_status_bit(ST_EQ, value1 == value2);
	set_status_bit(ST_LH, value1 > value2);
	set_status_bit(ST_AGT, (INT16)value1 > (INT16)value2);
}

/**************************************************************************
    ALU operations. The activities as implemented here are performed
    during the internal operations of the CPU, according to the current
    instruction.

    Some ALU operations are followed by the prefetch operation. In fact,
    this prefetch happens in parallel to the ALU operation. In these
    situations we do not pulse the clock here but leave this to the prefetch
    operation.
**************************************************************************/

void tms9995_device::alu_nop()
{
	// Do nothing (or nothing that is externally visible)
	pulse_clock(1);
	return;
}

void tms9995_device::alu_add_s_sxc()
{
	// We have the source operand value in m_source_value and the destination
	// value in m_current_value
	// The destination address is still in m_address
	// Prefetch will not change m_current_value and m_address

	UINT32 dest_new = 0;

	switch (m_instruction->command)
	{
	case A:
	case AB:
		// When adding, a carry occurs when we exceed the 0xffff value.
		dest_new = m_current_value + m_source_value;
		set_status_bit(ST_C, (dest_new & 0x10000) != 0);

		// If the result has a sign bit that is different from both arguments, we have an overflow
		// (i.e. getting a negative value from two positive values and vice versa)
		set_status_bit(ST_OV, ((dest_new ^ m_current_value) & (dest_new ^ m_source_value) & 0x8000)!=0);
		break;
	case S:
	case SB:
		dest_new = m_current_value + ((~m_source_value) & 0xffff) + 1;
		// Subtraction means adding the 2s complement, so the carry bit
		// is set whenever adding the 2s complement exceeds ffff
		// In fact the CPU adds the one's complement, then adds a one. This
		// explains why subtracting 0 sets the carry bit.
		set_status_bit(ST_C, (dest_new & 0x10000) != 0);

		// If the arguments have different sign bits and the result has a
		// sign bit different from the destination value, we have an overflow
		// e.g. value1 = 0x7fff, value2 = 0xffff; value1-value2 = 0x8000
		// or   value1 = 0x8000, value2 = 0x0001; value1-value2 = 0x7fff
		// value1 is the destination value
		set_status_bit(ST_OV, (m_current_value ^ m_source_value) & (m_current_value ^ dest_new) & 0x8000);
		break;
	case SOC:
	case SOCB:
		dest_new = m_current_value | m_source_value;
		break;

	case SZC:
	case SZCB:
		dest_new = m_current_value & ~m_source_value;
		break;
	}

	m_current_value = (UINT16)(dest_new & 0xffff);

	compare_and_set_lae((UINT16)(dest_new & 0xffff),0);
	if (m_instruction->byteop)
	{
		set_status_parity((UINT8)(dest_new>>8));
	}
	if (TRACE_STATUS) logerror("tms9995: ST = %04x (val=%04x)\n", ST, m_current_value);
	// No clock pulse (will be done by prefetch)
}

/*
    Branch / Branch and link. We put the source address into the PC after
    copying the PC into m_current_value. The address is R11. The B instruction
    will just ignore these settings, but BL will use them.
*/
void tms9995_device::alu_b()
{
	m_current_value = PC;
	PC = m_address & 0xfffe;
	m_address = WP + 22;
}

/*
    Branch and load workspace pointer. This is a branch to a subprogram with
    context switch.
*/
void tms9995_device::alu_blwp()
{
	int n = 1;
	switch (m_instruction->state)
	{
	case 0:
		// new WP in m_current_value
		m_value_copy = WP;
		WP = m_current_value & 0xfffe;
		m_address_saved = m_address + 2;
		m_address = WP + 30;
		m_current_value = ST;
		break;
	case 1:
		m_current_value = PC;
		m_address = m_address - 2;
		break;
	case 2:
		m_current_value = m_value_copy;     // old WP
		m_address = m_address - 2;
		break;
	case 3:
		m_address = m_address_saved;
		break;
	case 4:
		PC = m_current_value & 0xfffe;
		n = 0;
		if (TRACE_OP) logerror("tms9995: Context switch complete; WP=%04x, PC=%04x, ST=%04x\n", WP, PC, ST);
		break;
	}
	m_instruction->state++;
	pulse_clock(n);
}

/*
    Compare is similar to add, s, soc, szc, but we do not write a result.
*/
void tms9995_device::alu_c()
{
	// We have the source operand value in m_source_value and the destination
	// value in m_current_value
	// The destination address is still in m_address
	// Prefetch will not change m_current_value and m_address
	if (m_instruction->byteop)
	{
		set_status_parity((UINT8)(m_source_value>>8));
	}
	compare_and_set_lae(m_source_value, m_current_value);
	if (TRACE_STATUS) logerror("tms9995: ST = %04x (val1=%04x, val2=%04x)\n", ST, m_source_value, m_current_value);
}

/*
    Compare with immediate value.
*/
void tms9995_device::alu_ci()
{
	// We have the register value in m_source_value, the register address in m_address_saved
	// and the immediate value in m_current_value
	compare_and_set_lae(m_source_value, m_current_value);
	if (TRACE_STATUS) logerror("tms9995: ST = %04x (val1=%04x, val2=%04x)\n", ST, m_source_value, m_current_value);
}

void tms9995_device::alu_clr_seto()
{
	if (TRACE_OP) logerror("tms9995: clr/seto: Setting values for address %04x\n", m_address);
	switch (m_instruction->command)
	{
	case CLR:
		m_current_value = 0;
		break;
	case SETO:
		m_current_value = 0xffff;
		break;
	}
	// No clock pulse, as next instruction is prefetch
}

/*
    Unsigned division.
*/
void tms9995_device::alu_divide()
{
	int n=1;
	UINT32 uval32;

	bool overflow = true;
	UINT16 value1;

	switch (m_instruction->state)
	{
	case 0:
		m_source_value = m_current_value;
		// Set address of register
		m_address = WP + ((m_instruction->IR >> 5) & 0x001e);
		m_address_copy = m_address;
		break;
	case 1:
		// Value of register is in m_current_value
		// We have an overflow when the quotient cannot be stored in 16 bits
		// This is the case when the dividend / divisor >= 0x10000,
		// or equivalently, dividend / 0x10000 >= divisor

		// Check overflow for unsigned DIV
		if (m_current_value < m_source_value)   // also if source=0
		{
			MPC++;  // skip the abort
			overflow = false;
		}
		set_status_bit(ST_OV, overflow);
		m_value_copy = m_current_value;         // Save the high word
		m_address = m_address + 2;
		break;
	case 2:
		// W2 is in m_current_value
		uval32 = (m_value_copy << 16) | m_current_value;
		// Calculate
		// The number of ALU cycles depends on the number of steps in
		// the division algorithm. The number of cycles is between 1 and 16
		// As in TMS9900, this is a guess; it depends on the actual algorithm
		// used in the chip.

		m_current_value = uval32 / m_source_value;
		m_value_copy = uval32 % m_source_value;
		m_address = m_address_copy;

		value1 = m_value_copy & 0xffff;
		while (value1 != 0)
		{
			value1 = (value1 >> 1) & 0xffff;
			n++;
		}

		break;
	case 3:
		// now write the remainder
		m_current_value = m_value_copy;
		m_address = m_address + 2;
		break;
	}
	m_instruction->state++;
	pulse_clock(n);
}

/*
    Signed Division
    We cannot handle this by the same ALU operation because we can NOT decide
    whether there is an overflow before we have retrieved the whole 32 bit
    word. Also, the overflow detection is pretty complicated for signed
    division when done before the actual calculation.
*/
void tms9995_device::alu_divide_signed()
{
	int n=1;
	bool overflow;
	UINT16 w1, w2, dwait;
	INT16 divisor;
	INT32 dividend;

	switch (m_instruction->state)
	{
	case 0:
		// Got the source value (divisor)
		m_source_value = m_current_value;
		m_address = WP;  // DIVS always uses R0,R1
		break;
	case 1:
		// Value of register is in m_current_value
		m_value_copy = m_current_value;
		m_address += 2;
		break;
	case 2:
		// Now we have the dividend low word in m_current_value,
		// the dividend high word in m_value_copy, and
		// the divisor in m_source_value.

		w1 = m_value_copy;
		w2 = m_current_value;
		divisor = m_source_value;
		dividend = w1 << 16 | w2;

		// Now check for overflow
		// We need to go for four cases
		// if the divisor is not 0 anyway
		if (divisor != 0)
		{
			if (dividend >= 0)
			{
				if (divisor > 0)
				{
					overflow = (dividend > ((divisor<<15) - 1));
				}
				else
				{
					overflow = (dividend > (((-divisor)<<15) + (-divisor) - 1));
				}
			}
			else
			{
				if (divisor > 0)
				{
					overflow = ((-dividend) > ((divisor<<15) + divisor - 1));
				}
				else
				{
					overflow = ((-dividend) > (((-divisor)<<15) - 1));
				}
			}
		}
		else
		{
			overflow = true; // divisor is 0
		}
		set_status_bit(ST_OV, overflow);
		if (!overflow) MPC++;       // Skip the next microinstruction when there is no overflow
		break;
	case 3:
		// We are here because there was no overflow
		dividend = m_value_copy << 16 | m_current_value;
		// Do the calculation
		m_current_value =  (UINT16)(dividend / (INT16)m_source_value);
		m_value_copy = (UINT16)(dividend % (INT16)m_source_value);
		m_address = WP;

		// As we have not implemented the real division algorithm we must
		// simulate the number of steps required for calculating the result.
		// This is just a guess.
		dwait = m_value_copy;
		while (dwait != 0)
		{
			dwait = (dwait >> 1) & 0xffff;
			n++;
		}
		// go write the quotient into R0
		break;
	case 4:
		// Now write the remainder
		m_current_value = m_value_copy;
		m_address += 2;
		n = 0;
		break;
	}
	m_instruction->state++;
	pulse_clock(n);
}

/*
    External operations.
*/
void tms9995_device::alu_external()
{
	// Call some possibly attached external device
	// A specific bit pattern is put on the data bus, and the CRUOUT line is
	// pulsed. In our case we use a special callback function since we cannot
	// emulate this behavior in this implementation.

	// Opcodes         D012 value
	// -----------------vvv------
	// IDLE = 0000 0011 0100 0000
	// RSET = 0000 0011 0110 0000
	// CKON = 0000 0011 1010 0000
	// CKOF = 0000 0011 1100 0000
	// LREX = 0000 0011 1110 0000

	// Only IDLE has a visible effect on the CPU without external support: the
	// CPU will stop execution until an interrupt occurs. CKON, CKOF, LREX have
	// no effect without external support. Neither has RSET, it does *not*
	// cause a reset of the CPU or of the remaining computer system.
	// It only clears the interrupt mask and outputs the
	// external code on the data bus. A special line decoder could then trigger
	// a reset from outside.

	if (m_instruction->command == IDLE)
	{
		if (TRACE_OP) logerror("tms9995: Entering IDLE state\n");
		m_idle_state = true;
	}

	if (m_instruction->command == RSET)
	{
		ST &= 0xfff0;
		if (TRACE_OP) logerror("tms9995: RSET, new ST = %04x\n", ST);
	}

	if (!m_external_operation.isnull()) m_external_operation((m_instruction->IR >> 5) & 0x07, 1, 0xff);
}

/*
    Logical compare and XOR
*/
void tms9995_device::alu_f3()
{
	switch (m_instruction->state)
	{
	case 0:
		// We have the contents of the source in m_current_value and its address
		// in m_address
		m_source_value = m_current_value;
		// Get register address
		m_address = WP + ((m_instruction->IR >> 5) & 0x001e);
		break;
	case 1:
		// Register contents -> m_current_value
		// Source contents -> m_source_value
		if (m_instruction->command == COC)
		{
			set_status_bit(ST_EQ, (m_current_value & m_source_value) == m_source_value);
		}
		else
		{
			if (m_instruction->command == CZC)
			{
				set_status_bit(ST_EQ, (~m_current_value & m_source_value) == m_source_value);
			}
			else
			{
				// XOR
				// The workspace register address is still in m_address
				m_current_value = (m_current_value ^ m_source_value);
				compare_and_set_lae(m_current_value, 0);
			}
		}
		if (TRACE_STATUS) logerror("tms9995: ST = %04x\n", ST);
		break;
	}
	m_instruction->state++;
}

/*
    Handles AI, ANDI, ORI.
*/
void tms9995_device::alu_imm_arithm()
{
	UINT32 dest_new = 0;

	// We have the register value in m_source_value, the register address in m_address_saved
	// and the immediate value in m_current_value
	switch (m_instruction->command)
	{
	case AI:
		dest_new = m_current_value + m_source_value;
		set_status_bit(ST_C, (dest_new & 0x10000) != 0);

		// If the result has a sign bit that is different from both arguments, we have an overflow
		// (i.e. getting a negative value from two positive values and vice versa)
		set_status_bit(ST_OV, ((dest_new ^ m_current_value) & (dest_new ^ m_source_value) & 0x8000)!=0);
		break;
	case ANDI:
		dest_new = m_current_value & m_source_value;
		break;
	case ORI:
		dest_new = m_current_value | m_source_value;
		break;
	}

	m_current_value = (UINT16)(dest_new & 0xffff);
	compare_and_set_lae(m_current_value, 0);
	m_address = m_address_saved;
	if (TRACE_STATUS) logerror("tms9995: ST = %04x (val=%04x)\n", ST, m_current_value);
}

/*
    Handles all jump instructions.
*/
void tms9995_device::alu_jump()
{
	bool cond = false;
	INT8 displacement = (m_instruction->IR & 0xff);

	switch (m_instruction->command)
	{
	case JMP:
		cond = true;
		break;
	case JLT:   // LAECOP == x00xxx
		cond = ((ST & (ST_AGT | ST_EQ))==0);
		break;
	case JLE:   // LAECOP == 0xxxxx
		cond = ((ST & ST_LH)==0);
		break;
	case JEQ:   // LAECOP == xx1xxx
		cond = ((ST & ST_EQ)!=0);
		break;
	case JHE:   // LAECOP == 1x0xxx, 0x1xxx
		cond = ((ST & (ST_LH | ST_EQ)) != 0);
		break;
	case JGT:   // LAECOP == x1xxxx
		cond = ((ST & ST_AGT)!=0);
		break;
	case JNE:   // LAECOP == xx0xxx
		cond = ((ST & ST_EQ)==0);
		break;
	case JNC:   // LAECOP == xxx0xx
		cond = ((ST & ST_C)==0);
		break;
	case JOC:   // LAECOP == xxx1xx
		cond = ((ST & ST_C)!=0);
		break;
	case JNO:   // LAECOP == xxxx0x
		cond = ((ST & ST_OV)==0);
		break;
	case JL:    // LAECOP == 0x0xxx
		cond = ((ST & (ST_LH | ST_EQ)) == 0);
		break;
	case JH:    // LAECOP == 1xxxxx
		cond = ((ST & ST_LH)!=0);
		break;
	case JOP:   // LAECOP == xxxxx1
		cond = ((ST & ST_OP)!=0);
		break;
	}

	if (!cond)
	{
		if (TRACE_OP) logerror("tms9995: Jump condition false\n");
	}
	else
	{
		if (TRACE_OP) logerror("tms9995: Jump condition true\n");
		PC = (PC + (displacement<<1)) & 0xfffe;
	}
}

/*
    Implements LDCR.
*/
void tms9995_device::alu_ldcr()
{
	switch (m_instruction->state)
	{
	case 0:
		m_count = (m_instruction->IR >> 6) & 0x000f;
		if (m_count==0) m_count = 16;
		m_instruction->byteop = (m_count<9);
		break;
	case 1:
		// We have read the byte or word into m_current_value.
		compare_and_set_lae(m_current_value, 0);
		if (TRACE_STATUS) logerror("tms9995: ST = %04x (val=%04x)\n", ST, m_current_value);
		if (m_instruction->byteop)
		{
			m_current_value = (m_current_value>>8) & 0xff;
			set_status_parity((UINT8)m_current_value);
		}
		m_cru_value = m_current_value;
		m_address = WP + 24;
		break;
	case 2:
		// Prepare CRU operation
		m_cru_address = m_current_value;
		break;
	}
	m_instruction->state++;
	pulse_clock(1);
}

/*
    Implements LI. Almost everything has been done in the microprogram;
    this part is reached with m_address_saved = register address,
    and m_current_value = *m_address;
*/
void tms9995_device::alu_li()
{
	// Retrieve the address of the register
	// The immediate value is still in m_current_value
	m_address = m_address_saved;
	compare_and_set_lae(m_current_value, 0);
	if (TRACE_STATUS) logerror("tms9995: ST = %04x (val=%04x)\n", ST, m_current_value);
}

void tms9995_device::alu_limi_lwpi()
{
	// The immediate value is in m_current_value
	if (m_instruction->command == LIMI)
	{
		ST = (ST & 0xfff0) | (m_current_value & 0x000f);
		if (TRACE_OP) logerror("tms9995: LIMI sets ST = %04x\n", ST);
		pulse_clock(1);     // needs one more than LWPI
	}
	else
	{
		WP = m_current_value & 0xfffe;
		if (TRACE_OP) logerror("tms9995: LWPI sets new WP = %04x\n", WP);
	}
}

/*
    Load status and load workspace pointer. This is a TMS9995-specific
    operation.
*/
void tms9995_device::alu_lst_lwp()
{
	if (m_instruction->command==LST)
	{
		ST = m_current_value;
		if (TRACE_OP) logerror("tms9995: new ST = %04x\n", ST);
		pulse_clock(1);
	}
	else
	{
		WP = m_current_value & 0xfffe;
		if (TRACE_OP) logerror("tms9995: new WP = %04x\n", WP);
	}
}

/*
    The MOV operation on the TMS9995 is definitely more efficient than in the
    TMS9900. As we have only 8 data bus lines we can read or write bytes
    with only one cycle. The TMS9900 always has to read the memory word first
    in order to write back a complete word, also when doing byte operations.
*/
void tms9995_device::alu_mov()
{
	m_current_value = m_source_value;
	if (m_instruction->byteop)
	{
		set_status_parity((UINT8)(m_current_value>>8));
	}
	compare_and_set_lae(m_current_value, 0);
	if (TRACE_STATUS) logerror("tms9995: ST = %04x (val=%04x)\n", ST, m_current_value);
	// No clock pulse, as next instruction is prefetch
}

/*
    Unsigned and signed multiplication
*/
void tms9995_device::alu_multiply()
{
	int n = 0;
	UINT32 result;
	INT32 results;

	if (m_instruction->command==MPY)
	{
		switch (m_instruction->state)
		{
		case 0:
			// m_current_value <- multiplier (source)
			m_source_value = m_current_value;
			// m_address is the second multiplier (in a register)
			m_address = ((m_instruction->IR >> 5) & 0x001e) + WP;
			n = 1;
			break;
		case 1:
			// m_current_value <- register content
			result = (m_source_value & 0x0000ffff) * (m_current_value & 0x0000ffff);
			m_current_value = (result >> 16) & 0xffff;
			m_value_copy = result & 0xffff;
			// m_address is still the register
			n = 17;
			break;
		case 2:
			m_address += 2;
			m_current_value = m_value_copy;
			// now write the lower 16 bit.
			// If the register was R15, do not use R0 but continue writing after
			// R15's address
			break;
		}
	}
	else
	{
		switch (m_instruction->state)
		{
		case 0:
			// m_current_value <- multiplier (source)
			m_source_value = m_current_value;
			// m_address is the second multiplier (in R0)
			m_address = WP;
			n = 1;
			break;
		case 1:
			// m_current_value <- register content
			results = ((INT16)m_source_value) * ((INT16)m_current_value);
			m_current_value = (results >> 16) & 0xffff;
			m_value_copy = results & 0xffff;
			// m_address is still the register
			n = 16;
			break;
		case 2:
			m_address += 2;
			m_current_value = m_value_copy;
			// now write the lower 16 bit.
			break;
		}
	}
	m_instruction->state++;
	pulse_clock(n);
}

void tms9995_device::alu_rtwp()
{
	switch (m_instruction->state)
	{
	case 0:
		m_address = WP + 30;        // R15
		pulse_clock(1);
		break;
	case 1:
		ST = m_current_value;
		m_address -= 2;             // R14
		break;
	case 2:
		PC = m_current_value & 0xfffe;
		m_address -= 2;             // R13
		break;
	case 3:
		WP = m_current_value & 0xfffe;

		// Just for debugging purposes
		m_servicing_interrupt = false;

		if (TRACE_OP) logerror("tms9995: RTWP restored old context (WP=%04x, PC=%04x, ST=%04x)\n", WP, PC, ST);
		break;
	}
	m_instruction->state++;
}

void tms9995_device::alu_sbo_sbz()
{
	INT8 displacement;

	if (m_instruction->state==0)
	{
		m_address = WP + 24;
	}
	else
	{
		m_cru_value = (m_instruction->command==SBO)? 1 : 0;
		displacement = (INT8)(m_instruction->IR & 0xff);
		m_cru_address = m_current_value + (displacement<<1);
		m_count = 1;
	}
	m_instruction->state++;
	pulse_clock(1);
}

/*
    Perform the shift operation
*/
void tms9995_device::alu_shift()
{
	bool carry = false;
	bool overflow = false;
	UINT16 sign = 0;
	UINT32 value;
	int count;

	switch (m_instruction->state)
	{
	case 0:
		// we have the value of the register in m_current_value
		// Save it (we may have to read R0)
		m_value_copy = m_current_value;
		m_address_saved = m_address;
		m_address = WP;
		// store this in m_current_value where the R0 value will be put
		m_current_value = (m_instruction->IR >> 4)& 0x000f;
		if (m_current_value != 0)
		{
			// skip the next read operation
			MPC++;
		}
		else
		{
			if (TRACE_DETAIL) logerror("tms9995: Shift operation gets count from R0\n");
		}
		pulse_clock(1);
		pulse_clock(1);
		break;

	case 1:
		count = m_current_value & 0x000f; // from the instruction or from R0
		if (count==0) count = 16;

		value = m_value_copy;

		// we are re-implementing the shift operations because we have to pulse
		// the clock at each single shift anyway.
		// Also, it is easier to implement the status bit setting.
		// Note that count is never 0
		if (m_instruction->command == SRA) sign = value & 0x8000;

		for (int i=0; i < count; i++)
		{
			switch (m_instruction->command)
			{
			case SRL:
			case SRA:
				carry = ((value & 1)!=0);
				value = (value >> 1) | sign;
				break;
			case SLA:
				carry = ((value & 0x8000)!=0);
				value <<= 1;
				if (carry != ((value&0x8000)!=0)) overflow = true;
				break;
			case SRC:
				carry = ((value & 1)!=0);
				value = (value>>1) | (carry? 0x8000 : 0x0000);
				break;
			}
			pulse_clock(1);
		}

		m_current_value = value & 0xffff;
		set_status_bit(ST_C, carry);
		set_status_bit(ST_OV, overflow);
		compare_and_set_lae(m_current_value, 0);
		m_address = m_address_saved;        // Register address
		if (TRACE_STATUS) logerror("tms9995: ST = %04x (val=%04x)\n", ST, m_current_value);
		break;
	}
	m_instruction->state++;
}

/*
    Handles ABS, DEC, DECT, INC, INCT, NEG, INV
*/
void tms9995_device::alu_single_arithm()
{
	UINT32 dest_new = 0;
	UINT32 src_val = m_current_value & 0x0000ffff;
	UINT16 sign = 0;
	bool check_ov = true;

	switch (m_instruction->command)
	{
	case ABS:
		// LAECO (from original word!)
		// O if >8000
		// C is always reset
		set_status_bit(ST_OV, m_current_value == 0x8000);
		set_status_bit(ST_C, false);
		compare_and_set_lae(m_current_value, 0);

		if ((m_current_value & 0x8000)!=0)
		{
			dest_new = ((~src_val) & 0x0000ffff) + 1;
		}
		else
		{
			dest_new = src_val;
		}
		m_current_value = dest_new & 0xffff;
		return;
	case DEC:
		// LAECO
		// Carry for result value != 0xffff
		// Overflow for result value == 0x7fff
		dest_new = src_val + 0xffff;
		sign = 0x8000;
		break;
	case DECT:
		// Carry for result value != 0xffff / 0xfffe
		// Overflow for result value = 0x7fff / 0x7ffe
		dest_new = src_val + 0xfffe;
		sign = 0x8000;
		break;
	case INC:
		// LAECO
		// Overflow for result value = 0x8000
		// Carry for result value = 0x0000
		dest_new = src_val + 1;
		break;
	case INCT:
		// LAECO
		// Overflow for result value = 0x8000 / 0x8001
		// Carry for result value = 0x0000 / 0x0001
		dest_new = src_val + 2;
		break;
	case INV:
		// LAE
		dest_new = ~src_val & 0xffff;
		check_ov = false;
		break;
	case NEG:
		// LAECO
		// Overflow occurs for value=0x8000
		// Carry occurs for value=0
		dest_new = ((~src_val) & 0x0000ffff) + 1;
		check_ov = false;
		set_status_bit(ST_OV, src_val == 0x8000);
		break;
	case SWPB:
		m_current_value = ((m_current_value << 8) | (m_current_value >> 8)) & 0xffff;
		// I don't know what they are doing right now, but we lose a lot of cycles
		// according to the spec (which can indeed be proved on a real system)

		// Maybe this command is used as a forced wait between accesses to the
		// video system. Usually we have two byte writes to set an address in
		// the VDP, with a SWPB in between. Most software for the TI-99/4A using
		// the TMS9900 will run into trouble when executed on the TI-99/8 with
		// the much faster TMS9995. So the SWPB may be used to as an intentional
		// slowdown.

		// No status bits affected
		pulse_clock(10);
		return;
	}

	if (check_ov) set_status_bit(ST_OV, ((src_val & 0x8000)==sign) && ((dest_new & 0x8000)!=sign));
	set_status_bit(ST_C, (dest_new & 0x10000) != 0);
	m_current_value = dest_new & 0xffff;
	compare_and_set_lae(m_current_value, 0);

	if (TRACE_STATUS) logerror("tms9995: ST = %04x (val=%04x)\n", ST, m_current_value);
	// No clock pulse, as next instruction is prefetch
}

/*
    Store CRU.
*/
void tms9995_device::alu_stcr()
{
	int n = 1;
	switch (m_instruction->state)
	{
	case 0:
		m_count = (m_instruction->IR >> 6) & 0x000f;
		if (m_count == 0) m_count = 16;
		m_instruction->byteop = (m_count < 9);
		break;
	case 1:
		m_address_saved = m_address;
		m_address = WP + 24;
		break;
	case 2:
		m_cru_address = m_current_value;
		m_cru_first_read = true;
		break;
	case 3:
		// I don't know what is happening here, but it takes quite some time.
		// May be shift operations.
		m_current_value = m_cru_value;
		m_address = m_address_saved;
		compare_and_set_lae(m_current_value, 0);
		n = 13;
		if (m_instruction->byteop)
		{
			set_status_parity((UINT8)m_current_value);
			m_current_value <<= 8;
		}
		else n += 8;
		if (TRACE_STATUS) logerror("tms9995: ST = %04x (val=%04x)\n", ST, m_current_value);
		break;
	}
	m_instruction->state++;
	pulse_clock(n);
}


/*
    Store status and store workspace pointer. We need to determine the
    address of the register here.
*/
void tms9995_device::alu_stst_stwp()
{
	m_address = WP + ((m_instruction->IR & 0x000f)<<1);
	m_current_value = (m_instruction->command==STST)? ST : WP;
}

/*
    Test CRU bit.
*/
void tms9995_device::alu_tb()
{
	INT8 displacement;

	switch (m_instruction->state)
	{
	case 0:
		m_address = WP + 24;
		pulse_clock(1);
		break;
	case 1:
		displacement = (INT8)(m_instruction->IR & 0xff);
		m_cru_address = m_current_value + (displacement<<1);
		m_cru_first_read = true;
		m_count = 1;
		pulse_clock(1);
		break;
	case 2:
		set_status_bit(ST_EQ, m_cru_value!=0);
		if (TRACE_STATUS) logerror("tms9995: ST = %04x\n", ST);
		break;
	}
	m_instruction->state++;
}

/*
    Execute. This operation is substituted after reading the word at the
    given address.
*/
void tms9995_device::alu_x()
{
	// We have the word in m_current_value. This word must now be decoded
	// as if it has been acquired by the normal procedure.
	decode(m_current_value);
	pulse_clock(1);

	// Switch to the prefetched and decoded instruction
	next_command();
}

/*
    XOP operation.
*/
void tms9995_device::alu_xop()
{
	switch (m_instruction->state)
	{
	case 0:
		// we have the source address in m_address
		m_address_saved = m_address;
		// Format is xxxx xxnn nnxx xxxx
		m_address = 0x0040 + ((m_instruction->IR & 0x03c0)>>4);
		pulse_clock(1);
		break;
	case 1:
		// m_current_value is new WP
		m_value_copy = WP;  // store this for later
		WP = m_current_value & 0xfffe;
		m_address = WP + 0x0016; // Address of new R11
		m_current_value = m_address_saved;
		pulse_clock(1);
		break;
	case 2:
		m_address = WP + 0x001e;
		m_current_value = ST;
		pulse_clock(1);
		break;
	case 3:
		m_address = WP + 0x001c;
		m_current_value = PC;
		pulse_clock(1);
		break;
	case 4:
		m_address = WP + 0x001a;
		m_current_value = m_value_copy;
		pulse_clock(1);
		break;
	case 5:
		m_address = 0x0042 + ((m_instruction->IR & 0x03c0)>>4);
		pulse_clock(1);
		break;
	case 6:
		PC = m_current_value & 0xfffe;
		set_status_bit(ST_X, true);
		break;
	}
	m_instruction->state++;
}

/*
    Handle an interrupt. The behavior as implemented here follows
    exactly the flowchart in [1]
*/
void tms9995_device::alu_int()
{
	int pulse = 1;

	switch (m_instruction->state)
	{
	case 0:
		PC = (PC - 2) & 0xfffe;
		m_address_saved = m_address;
		if (TRACE_INTD) logerror("tms9995: interrupt service (0): Prepare to read vector\n");
		break;
	case 1:
		pulse = 2;                  // two cycles (with the one at the end)
		m_source_value = WP;        // old WP
		WP = m_current_value & 0xfffe;       // new WP
		m_current_value = ST;
		m_address = (WP + 30)&0xfffe;
		if (TRACE_INTD) logerror("tms9995: interrupt service (1): Read new WP = %04x, save ST to %04x\n", WP, m_address);
		break;
	case 2:
		m_address = (WP + 28)&0xfffe;
		m_current_value = PC;
		if (TRACE_INTD) logerror("tms9995: interrupt service (2): Save PC to %04x\n", m_address);
		break;
	case 3:
		m_address = (WP + 26)&0xfffe;
		m_current_value = m_source_value;   // old WP
		if (TRACE_INTD) logerror("tms9995: interrupt service (3): Save WP to %04x\n", m_address);
		break;
	case 4:
		m_address = (m_address_saved + 2) & 0xfffe;
		if (TRACE_INTD) logerror("tms9995: interrupt service (4): Read PC from %04x\n", m_address);
		break;
	case 5:
		PC = m_current_value & 0xfffe;
		ST = (ST & 0xfe00) | m_intmask;
		if (TRACE_INTD) logerror("tms9995: interrupt service (5): Context switch complete; WP=%04x, PC=%04x, ST=%04x\n", WP, PC, ST);

		if (((m_int_pending & PENDING_MID)!=0) && m_nmi_active)
		{
			if (TRACE_INTD) logerror("tms9995: interrupt service (6): NMI active after context switch\n");
			m_int_pending &= ~PENDING_MID;
			m_address = 0xfffc;
			m_intmask = 0;
			MPC = 0;    // redo the interrupt service for the NMI
		}
		else
		{
			if (m_from_reset)
			{
				if (TRACE_INTD) logerror("tms9995: interrupt service (6): RESET completed\n");
				// We came from the RESET interrupt
				m_from_reset = false;
				ST &= 0x01ff;
				m_mid_flag = false;
				m_mid_active = false;
				// FLAG0 and FLAG1 are also set to zero after RESET ([1], sect. 2.3.1.2.2)
				for (int i=0; i < 5; i++) m_flag[i] = false;
				m_check_hold = true;
			}
		}
		pulse = 0;
		break;

		// If next instruction is MID opcode we will detect this in command_completed
	}
	m_instruction->state++;
	pulse_clock(pulse);
}

/**************************************************************************/
UINT32 tms9995_device::execute_min_cycles() const
{
	return 2;
}

UINT32 tms9995_device::execute_max_cycles() const
{
	return 44;
}

UINT32 tms9995_device::execute_input_lines() const
{
	return 2;
}

UINT32 tms9995_device::disasm_min_opcode_bytes() const
{
	return 2;
}

UINT32 tms9995_device::disasm_max_opcode_bytes() const
{
	return 6;
}

offs_t tms9995_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms9995 );
	return CPU_DISASSEMBLE_NAME(tms9995)(this, buffer, pc, oprom, opram, options);
}


const device_type TMS9995 = &device_creator<tms9995_device>;
const device_type TMS9995_MP9537 = &device_creator<tms9995_mp9537_device>;
