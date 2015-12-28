// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    Texas Instruments TMS9900

                    +--------------------+
               V_BB | 1  o             64| /HOLD
               V_CC | 2                63| /MEMEN
               WAIT | 3                62| READY
              /LOAD | 4                61| /WE
              HOLDA | 5                60| CRUCLK
             /RESET | 6                59| V_CC
                IAQ | 7                58| -
               PHI1 | 8                57| -
               PHI2 | 9                56| D15   -+  LSB
     LSB  +-    A14 |10                55| D14    |
          |     A13 |11                54| D13    |
          |     A12 |12                53| D12    |
          |     A11 |13                52| D11    |
  Address |     A10 |14   +--------+   51| D10    | Data
    bus   |      A9 |15   |        |   50| D9     | bus
   32K *  |      A8 |16   |        |   49| D8     | 16 bit
   16bit  |      A7 |17   |        |   48| D7     |
          |      A6 |18   |        |   47| D6     |
          |      A5 |19   +--------+   46| D5     |
          |      A4 |20                45| D4     |
          |      A3 |21                44| D3     |
          |      A2 |22                43| D2     |
          |      A1 |23                42| D1     |
     MSB  +-     A0 |24                41| D0    -+  MSB
               PHI4 |25                40| V_SS
               V_SS |26                39| -
               V_DD |27                38| -
               PHI3 |28                37| -
               DBIN |29                36| IC0   -+ MSB
             CRUOUT |30                35| IC1    | Interrupt
              CRUIN |31                34| IC2    | level
            /INTREQ |32                33| IC3   -+ LSB
                    +--------------------+

       WAIT   out   Processor in wait state
      /LOAD    in   Non-maskable interrupt
      HOLDA   out   Hold acknowledge
     /RESET    in   Reset
        IAQ   out   Instruction acquisition
     PHI1-4    in   Clock phase inputs
       DBIN   out   Data bus in input mode
     CRUOUT   out   Communication register unit data output
      CRUIN    in   Communication register unit data input
    /INTREQ    in   Interrupt request
     CRUCLK   out   Communication register unit clock output
        /WE   out   Data available for memory write
      READY    in   Memory ready for access
     /MEMEN   out   Address bus contains memory address
      /HOLD    in   External device acquires address and data bus lines

      V_BB     -5V  supply
      V_CC     +5V  supply (pins 2 and 59 connected in parallel)
      V_DD    +12V  supply
      V_SS      0V  Ground reference (pins 26 and 40 connected in parallel)

      A0-A14  out   Address bus (32768 words of 16 bit width)
      D0-A15  i/o   Data bus
     IC0-IC3   in   Interrupt level (0-15)

     Note that Texas Instruments' bit numberings define bit 0 as the
     most significant bit (different to most other systems). Also, the
     system uses big-endian memory organisation: Storing the word 0x1234 at
     address 0x0000 means that the byte 0x12 is stored at 0x0000 and byte 0x34
     is stored at 0x0001.

     The processor also knows byte-oriented operations (like add byte (AB),
     move byte (MOVB)). This makes it necessary for the CPU to read the word
     from the target memory location first, change the respective byte, and
     write it back.

     See the TI-99/4A driver for an application of the TMS9900 processor
     within an 8-bit data bus board layout (using a data bus multiplexer).

     Subcycle handling

     In this implementation we try to emulate the internal operations as
     precisely as possible, following the technical specifications. We need
     not try to be clock-precise with every tick; it suffices to perform
     the proper number of operations within a given time span.

     For each command the CPU executes a microprogram which requires some
     amount of cycles to complete. During this time the external clock continues
     to issue pulses which can be used to control wait state creation. As we
     do not emulate external clocks this implementation offers an extra output
     "clock_out" (which, however, is available for the TMS9995) which pulses
     at a rate of 3 MHz. External devices (e.g. memory controllers) may count
     the pulses and pull down the READY line (with set_ready) as needed.

     Another possibility for creating wait states is to pull down the line
     for some time set by a timer. This is done, for example, by circuits like
     GROMs or speech synthesis processors (TMS52xx).

    TODO:
    - Fine-tune cycles
    - State save
    - HOLD state should be tested; I don't have test cases yet

    Michael Zapf, June 2012
*/

#include "tms9900.h"

/* tms9900 ST register bits. */
enum
{
	ST_LH = 0x8000,     // Logical higher (unsigned comparison)
	ST_AGT = 0x4000,    // Arithmetical greater than (signed comparison)
	ST_EQ = 0x2000,     // Equal
	ST_C = 0x1000,      // Carry
	ST_OV = 0x0800,     // Overflow (when using signed operations)
	ST_OP = 0x0400,     // Odd parity (used with byte operations)
	ST_X = 0x0200,      // XOP
	ST_IM = 0x000f      // Interrupt mask
};

/*
    The following defines can be set to 0 or 1 to disable or enable certain
    output in the log.
*/
// Emulation setup
#define TRACE_SETUP 0

// Emulation details
#define TRACE_EMU 0

// Location and command
#define TRACE_EXEC 0

// Memory operation
#define TRACE_MEM 0

// Address bus operation
#define TRACE_ADDRESSBUS 0

// Cycle count
#define TRACE_CYCLES 0

// Clock ticks
#define TRACE_CLOCK 0

// Wait states
#define TRACE_WAIT 0

// Interrupts
#define TRACE_INT 0

// CRU operation
#define TRACE_CRU 0

// Status register
#define TRACE_STATUS 0

// ALU details
#define TRACE_ALU 0

// Microinstruction level
#define TRACE_MICRO 0

/****************************************************************************
    Common constructor for TMS9900 and TMS9980A
    The CRU mask is related to the bits, not to their addresses which are
    twice their number. Accordingly, the TMS9900 has a CRU bitmask 0x0fff.
****************************************************************************/

tms99xx_device::tms99xx_device(const machine_config &mconfig, device_type type,  const char *name, const char *tag, int databus_width, int prg_addr_bits, int cru_addr_bits, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_program_config("program", ENDIANNESS_BIG, databus_width, prg_addr_bits),
		m_io_config("cru", ENDIANNESS_BIG, 8, cru_addr_bits),
		m_prgspace(nullptr),
		m_cru(nullptr),
		m_prgaddr_mask((1<<prg_addr_bits)-1),
		m_cruaddr_mask((1<<cru_addr_bits)-1),
		m_clock_out_line(*this),
		m_wait_line(*this),
		m_holda_line(*this),
		m_iaq_line(*this),
		m_get_intlevel(*this),
		m_dbin_line(*this),
		m_external_operation(*this)
{
}

tms99xx_device::~tms99xx_device()
{
}

/****************************************************************************
    Constructor for TMS9900
****************************************************************************/

tms9900_device::tms9900_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms99xx_device(mconfig, TMS9900, "TMS9900", tag, 16, 16, 12, owner, clock, "tms9900", __FILE__)
{
}

enum
{
	TMS9900_PC=0, TMS9900_WP, TMS9900_STATUS, TMS9900_IR,
	TMS9900_R0, TMS9900_R1, TMS9900_R2, TMS9900_R3,
	TMS9900_R4, TMS9900_R5, TMS9900_R6, TMS9900_R7,
	TMS9900_R8, TMS9900_R9, TMS9900_R10, TMS9900_R11,
	TMS9900_R12, TMS9900_R13, TMS9900_R14, TMS9900_R15
};

void tms99xx_device::device_start()
{
	// TODO: Restore state save feature
	resolve_lines();
	m_prgspace = &space(AS_PROGRAM);
	m_cru = &space(AS_IO);

	// set our instruction counter
	m_icountptr = &m_icount;

	m_state_any = 0;
	PC = 0;
	m_hold_state = false;

	// add the states for the debugger
	for (int i=0; i < 20; i++)
	{
		// callimport = need to use the state_import method to write to the state variable
		// callexport = need to use the state_export method to read the state variable
		state_add(i, s_statename[i], m_state_any).callimport().callexport().formatstr("%04X");
	}
	state_add(STATE_GENPC, "curpc", PC).formatstr("%4s").noshow();
	state_add(STATE_GENFLAGS, "status", m_state_any).callimport().callexport().formatstr("%16s").noshow();

	build_command_lookup_table();

	m_program = nullptr;
}

void tms99xx_device::device_stop()
{
	int k = 0;
	if (TRACE_SETUP) logerror("tms99xx: Deleting lookup tables\n");
	while (m_lotables[k]!=nullptr) delete[] m_lotables[k++];
}

/*
    External connections
*/
void tms99xx_device::resolve_lines()
{
	// Resolve our external connections
	m_external_operation.resolve();
	m_get_intlevel.resolve();
	m_iaq_line.resolve();
	m_clock_out_line.resolve();
	m_wait_line.resolve();
	m_holda_line.resolve();
	m_dbin_line.resolve();        // we need this for the set_address operation
}

/*
    TMS9900 hard reset
    The device reset is just the emulator's trigger for the reset procedure
    which is invoked via the main loop.
*/
void tms99xx_device::device_reset()
{
	if (TRACE_EMU) logerror("tms99xx: Device reset by emulator\n");
	m_reset = true;
	m_check_ready = false;
	m_wait_state = false;
	ST = 0;
	m_irq_state = false;
}

const char* tms99xx_device::s_statename[20] =
{
	"PC", "WP", "ST", "IR",
	"R0", "R1", "R2", "R3",
	"R4", "R5", "R6", "R7",
	"R8", "R9", "R10","R11",
	"R12","R13","R14","R15"
};

/*
    Write the contents of a register by external input (debugger)
*/
void tms99xx_device::state_import(const device_state_entry &entry)
{
	int index = entry.index();
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			// no action here; we do not allow import, as the flags are all
			// bits of the STATUS register
			break;
		case TMS9900_PC:
			PC = (UINT16)(m_state_any & m_prgaddr_mask & 0xfffe);
			break;
		case TMS9900_WP:
			WP = (UINT16)(m_state_any & m_prgaddr_mask & 0xfffe);
			break;
		case TMS9900_STATUS:
			ST = (UINT16)m_state_any;
			break;
		case TMS9900_IR:
			IR = (UINT16)m_state_any;
			break;
		default:
			// Workspace registers
			if (index <= TMS9900_R15)
				write_workspace_register_debug(index-TMS9900_R0, (UINT16)m_state_any);
			break;
	}
}

/*
    Reads the contents of a register for display in the debugger.
*/
void tms99xx_device::state_export(const device_state_entry &entry)
{
	int index = entry.index();
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_state_any = ST;
			break;
		case TMS9900_PC:
			m_state_any = PC;
			break;
		case TMS9900_WP:
			m_state_any = WP;
			break;
		case TMS9900_STATUS:
			m_state_any = ST;
			break;
		case TMS9900_IR:
			m_state_any = IR;
			break;
		default:
			// Workspace registers
			if (index <= TMS9900_R15)
				m_state_any = read_workspace_register_debug(index-TMS9900_R0);
			break;
	}
}

/*
    state_string_export - export state as a string for the debugger
*/
void tms99xx_device::state_string_export(const device_state_entry &entry, std::string &str)
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

/**************************************************************************/

UINT16 tms99xx_device::read_workspace_register_debug(int reg)
{
	int temp = m_icount;
	m_prgspace->set_debugger_access(true);
	UINT16 value = m_prgspace->read_word((WP+(reg<<1)) & m_prgaddr_mask & 0xfffe);
	m_prgspace->set_debugger_access(false);
	m_icount = temp;
	return value;
}

void tms99xx_device::write_workspace_register_debug(int reg, UINT16 data)
{
	int temp = m_icount;
	m_prgspace->set_debugger_access(true);
	m_prgspace->write_word((WP+(reg<<1)) & m_prgaddr_mask & 0xfffe, data);
	m_prgspace->set_debugger_access(false);
	m_icount = temp;
}

const address_space_config *tms99xx_device::memory_space_config(address_spacenum spacenum) const
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
	IAQ = 0,
	MEMORY_READ,
	MEMORY_WRITE,
	REG_READ,
	REG_WRITE,
	CRU_INPUT,
	CRU_OUTPUT,
	DATA_DERIVE,
	RET,
	ABORT,
	END,

	ALU_NOP,
	ALU_CLR,
	ALU_SETADDR,
	ALU_ADDONE,
	ALU_SETADDR_ADDONE,
	ALU_PCADDR_ADVANCE,
	ALU_SOURCE,
	ALU_ADDREG,
	ALU_IMM,
	ALU_REG,
	ALU_F1,
	ALU_COMP,
	ALU_F3,
	ALU_MPY,
	ALU_DIV,
	ALU_XOP,
	ALU_CLR_SWPB,
	ALU_ABS,
	ALU_X,
	ALU_B,
	ALU_BLWP,
	ALU_LDCR,
	ALU_STCR,
	ALU_SBZ_SBO,
	ALU_TB,
	ALU_JMP,
	ALU_SHIFT,
	ALU_AI_ORI,
	ALU_CI,
	ALU_LI,
	ALU_LWPI,
	ALU_LIMI,
	ALU_STWP_STST,
	ALU_EXT,
	ALU_RTWP,
	ALU_INT
};


#define MICROPROGRAM(_MP) \
	static const UINT8 _MP[] =

/*
    This is a kind of subroutine with 6 variants. Might be done in countless
    better ways, but will suffice for now. Each variant has at most 8 steps
    RET will return to the caller.
    The padding simplifies the calculation of the start address: We just
    take the Ts field as an index. In the last two cases we add an offset of 8
    if we have an indexed (resp. a byte) operation.
*/
MICROPROGRAM(data_derivation)
{
	REG_READ, RET, 0, 0, 0, 0, 0, 0,                                                // Rx           (00)
	0, 0, 0, 0, 0, 0, 0, 0,
	REG_READ, ALU_SETADDR, MEMORY_READ, RET, 0, 0, 0, 0,                            // *Rx          (01)
	0, 0, 0, 0, 0, 0, 0, 0,
	ALU_CLR, ALU_PCADDR_ADVANCE, MEMORY_READ, ALU_ADDREG, MEMORY_READ, RET, 0, 0,   // @sym         (10)
	REG_READ, ALU_PCADDR_ADVANCE, MEMORY_READ, ALU_ADDREG, MEMORY_READ, RET, 0, 0,  // @sym(Rx)     (10)
	REG_READ, ALU_SETADDR_ADDONE, ALU_ADDONE, REG_WRITE, MEMORY_READ, RET, 0, 0,    // *Rx+ (word)  (11)
	REG_READ, ALU_SETADDR_ADDONE, REG_WRITE, MEMORY_READ, RET, 0, 0, 0              // *Rx+ (byte)  (11)
};

MICROPROGRAM(f1_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_SOURCE,         // Store the word
	DATA_DERIVE,
	ALU_F1,
	MEMORY_WRITE,
	END
};

MICROPROGRAM(comp_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_SOURCE,
	DATA_DERIVE,
	ALU_COMP,
	ALU_NOP,        // Compare operations do not write back any data
	END
};

MICROPROGRAM(f3_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_F3,
	MEMORY_READ,    // We have to distinguish this from the C/CB microprogram above
	ALU_F3,
	ALU_NOP,        // Compare operations do not write back any data
	END
};

MICROPROGRAM(xor_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_F3,
	MEMORY_READ,
	ALU_F3,
	MEMORY_WRITE,   // XOR again must write back data, cannot reuse f3_mp
	END
};

MICROPROGRAM(mult_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_MPY,        // Save the value; put register number in m_regnumber
	MEMORY_READ,
	ALU_MPY,        // 18 cycles for multiplication
	MEMORY_WRITE,       // Write the high word
	ALU_MPY,        // Get low word, increase m_address
	MEMORY_WRITE,
	END
};

MICROPROGRAM(div_mp)
{
	ALU_NOP,
	DATA_DERIVE,    // Get divisor
	ALU_DIV,        // 0 Store divisor and get register number
	MEMORY_READ,    // Read register
	ALU_DIV,        // 1 Check overflow, increase address (or abort here)
	ABORT,
	MEMORY_READ,    // Read subsequent word (if reg=15 this is behind the workspace)
	ALU_DIV,        // 2 Calculate quotient (takes variable amount of cycles; at least 32 machine cycles), set register number
	MEMORY_WRITE,   // Write quotient into register
	ALU_DIV,        // 3 Get remainder
	MEMORY_WRITE,   // Write remainder
	END
};

MICROPROGRAM(xop_mp)
{
	ALU_NOP,
	DATA_DERIVE,    // Get argument
	ALU_XOP,        // 0 Save the address of the source operand, set address = 0x0040 + xopNr*4, 6 cycles
	MEMORY_READ,    // Read the new WP
	ALU_XOP,        // 1 Save old WP, set new WP, get the source operand address
	MEMORY_WRITE,   // Write the address of the source operand into the new R11
	ALU_XOP,        // 2
	MEMORY_WRITE,   // Write the ST into the new R15
	ALU_XOP,        // 3
	MEMORY_WRITE,   // Write the PC into the new R14
	ALU_XOP,        // 4
	MEMORY_WRITE,   // Write the WP into the new R13
	ALU_XOP,        // 5 Set the X bit in the ST
	MEMORY_READ,    // Read the new PC
	ALU_XOP,        // 6 Set the new PC
	END
};

MICROPROGRAM(clr_swpb_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_CLR_SWPB,
	MEMORY_WRITE,
	END
};

MICROPROGRAM(abs_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_ABS,        // two cycles
	MEMORY_WRITE,   // skipped when ABS is not performed
	ALU_NOP,
	END
};

MICROPROGRAM(x_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_X,
	END
};

MICROPROGRAM(b_mp)      // Branch
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_B,
	END
};

MICROPROGRAM(bl_mp)     // Branch and Link
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_B,
	ALU_NOP,
	MEMORY_WRITE,
	END
};

MICROPROGRAM(blwp_mp)       // Branch and Load WP
{
	ALU_NOP,
	DATA_DERIVE,            // Get argument
	ALU_BLWP,               // 0 Save old WP, set new WP, save position
	ALU_NOP,
	MEMORY_WRITE,           // write ST to R15
	ALU_BLWP,               // 1
	MEMORY_WRITE,           // write PC to R14
	ALU_BLWP,               // 2
	MEMORY_WRITE,           // write WP to R13
	ALU_BLWP,               // 3 Get saved position
	MEMORY_READ,            // Read new PC
	ALU_BLWP,               // 4 Set new PC
	END
};

MICROPROGRAM(ldcr_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_SOURCE,
	ALU_NOP,
	ALU_LDCR,
	ALU_NOP,
	MEMORY_READ,
	ALU_LDCR,
	CRU_OUTPUT,
	ALU_NOP,
	END
};

MICROPROGRAM(stcr_mp)
{
	ALU_NOP,
	DATA_DERIVE,
	ALU_SOURCE,         // Store address and value
	ALU_STCR,           // 0 Set register_number = 12; 0 cycles (already done before)
	MEMORY_READ,
	ALU_STCR,           // 1 Prepare CRU access
	ALU_NOP,
	CRU_INPUT,
	ALU_STCR,           // 2 Create result; Cycles = 5 + (8-#C-1) or + (16-#C)
	ALU_NOP,
	ALU_NOP,
	ALU_NOP,
	MEMORY_WRITE,
	END
};

MICROPROGRAM(sbz_sbo_mp)
{
	ALU_SBZ_SBO,
	ALU_NOP,
	MEMORY_READ,
	ALU_SBZ_SBO,
	CRU_OUTPUT,
	END
};

MICROPROGRAM(tb_mp)
{
	ALU_TB,
	MEMORY_READ,
	ALU_TB,
	CRU_INPUT,
	ALU_TB,
	END
};

MICROPROGRAM(jmp_mp)
{
	ALU_NOP,
	ALU_JMP,
	ALU_JMP,
	ALU_NOP,
	END
};

MICROPROGRAM(shift_mp)
{
	ALU_SHIFT,
	MEMORY_READ,
	ALU_SHIFT,              // 2 cycles if count != 0, else 4
	MEMORY_READ,            // skipped if count != 0
	ALU_SHIFT,              // skipped if count != 0  (4 cycles)
	ALU_SHIFT,
	MEMORY_WRITE,
	ALU_NOP,
	END
};

MICROPROGRAM(ai_ori_mp)
{
	ALU_REG,
	MEMORY_READ,
	ALU_IMM,
	MEMORY_READ,
	ALU_AI_ORI,
	MEMORY_WRITE,
	END
};

MICROPROGRAM(ci_mp)
{
	ALU_REG,
	MEMORY_READ,
	ALU_IMM,
	MEMORY_READ,
	ALU_CI,
	ALU_NOP,
	END
};

MICROPROGRAM(li_mp)
{
	ALU_IMM,
	MEMORY_READ,
	ALU_LI,             // sets status bits
	ALU_REG,            // set register number
	MEMORY_WRITE,
	END
};

MICROPROGRAM(lwpi_mp)
{
	ALU_IMM,
	MEMORY_READ,
	ALU_NOP,
	ALU_LWPI,               // sets WP
	END
};

MICROPROGRAM(limi_mp)
{
	ALU_IMM,
	MEMORY_READ,
	ALU_NOP,
	ALU_LIMI,               // sets interrupt mask in ST
	ALU_NOP,
	ALU_NOP,
	END
};

MICROPROGRAM(stwp_stst_mp)
{
	ALU_STWP_STST,
	ALU_REG,
	MEMORY_WRITE,
	END
};

MICROPROGRAM(external_mp)
{
	ALU_NOP,
	ALU_NOP,
	ALU_EXT,
	ALU_NOP,
	ALU_NOP,
	END
};

MICROPROGRAM(rtwp_mp)
{
	ALU_NOP,
	ALU_RTWP,
	MEMORY_READ,
	ALU_RTWP,               // no cycles
	MEMORY_READ,
	ALU_RTWP,               // no cycles
	MEMORY_READ,
	ALU_RTWP,
	END
};

MICROPROGRAM(int_mp)
{
	ALU_NOP,
	ALU_INT,                // 0 Set address = 0
	MEMORY_READ,
	ALU_INT,                // 1 Save old WP, set new WP, save position
	MEMORY_WRITE,           // write ST to R15
	ALU_INT,                // 2
	MEMORY_WRITE,           // write PC to R14
	ALU_INT,                // 3
	MEMORY_WRITE,           // write WP to R13
	ALU_INT,                // 4 Get saved position
	MEMORY_READ,            // Read new PC
	ALU_INT,                // 5 Set new PC
	END
};

const tms99xx_device::ophandler tms99xx_device::s_microoperation[] =
{
	&tms99xx_device::acquire_instruction,
	&tms99xx_device::mem_read,
	&tms99xx_device::mem_write,
	&tms99xx_device::register_read,
	&tms99xx_device::register_write,
	&tms99xx_device::cru_input_operation,
	&tms99xx_device::cru_output_operation,
	&tms99xx_device::data_derivation_subprogram,
	&tms99xx_device::return_from_subprogram,
	&tms99xx_device::abort_operation,
	&tms99xx_device::command_completed,

	&tms99xx_device::alu_nop,
	&tms99xx_device::alu_clear,
	&tms99xx_device::alu_setaddr,
	&tms99xx_device::alu_addone,
	&tms99xx_device::alu_setaddr_addone,
	&tms99xx_device::alu_pcaddr_advance,
	&tms99xx_device::alu_source,
	&tms99xx_device::alu_add_register,
	&tms99xx_device::alu_imm,
	&tms99xx_device::alu_reg,

	&tms99xx_device::alu_f1,
	&tms99xx_device::alu_comp,
	&tms99xx_device::alu_f3,
	&tms99xx_device::alu_multiply,
	&tms99xx_device::alu_divide,
	&tms99xx_device::alu_xop,
	&tms99xx_device::alu_clr_swpb,
	&tms99xx_device::alu_abs,
	&tms99xx_device::alu_x,
	&tms99xx_device::alu_b,
	&tms99xx_device::alu_blwp,
	&tms99xx_device::alu_ldcr,
	&tms99xx_device::alu_stcr,
	&tms99xx_device::alu_sbz_sbo,
	&tms99xx_device::alu_tb,
	&tms99xx_device::alu_jmp,
	&tms99xx_device::alu_shift,
	&tms99xx_device::alu_ai_ori,
	&tms99xx_device::alu_ci,
	&tms99xx_device::alu_li,
	&tms99xx_device::alu_lwpi,
	&tms99xx_device::alu_limi,
	&tms99xx_device::alu_stwp_stst,
	&tms99xx_device::alu_external,
	&tms99xx_device::alu_rtwp,
	&tms99xx_device::alu_int
};

/*****************************************************************************
    CPU instructions
*****************************************************************************/

/*
    Available instructions
*/
enum
{
	ILL=0, A, AB, ABS, AI, ANDI, B, BL, BLWP, C,
	CB, CI, CKOF, CKON, CLR, COC, CZC, DEC, DECT, DIV,
	IDLE, INC, INCT, INV, JEQ, JGT, JH, JHE, JL, JLE,
	JLT, JMP, JNC, JNE, JNO, JOC, JOP, LDCR, LI, LIMI,
	LREX, LWPI, MOV, MOVB, MPY, NEG, ORI, RSET, RTWP, S,
	SB, SBO, SBZ, SETO, SLA, SOC, SOCB, SRA, SRC, SRL,
	STCR, STST, STWP, SWPB, SZC, SZCB, TB, X, XOP, XOR,
	INTR
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
*/

/*
    Defines the number of bits from the left which are significant for the
    command in the respective format.
*/
static const int format_mask_len[] =
{
	0, 4, 8, 6, 6, 8, 10, 16, 12, 6
};

const tms99xx_device::tms_instruction tms99xx_device::s_command[] =
{
	// Opcode, ID, format, microprg
	{ 0x0200, LI, 8, li_mp },
	{ 0x0220, AI, 8, ai_ori_mp },
	{ 0x0240, ANDI, 8, ai_ori_mp },
	{ 0x0260, ORI, 8, ai_ori_mp },
	{ 0x0280, CI, 8, ci_mp },
	{ 0x02a0, STWP, 8, stwp_stst_mp },
	{ 0x02c0, STST, 8, stwp_stst_mp },
	{ 0x02e0, LWPI, 8, lwpi_mp },
	{ 0x0300, LIMI, 8, limi_mp },
	{ 0x0340, IDLE, 7, external_mp },
	{ 0x0360, RSET, 7, external_mp },
	{ 0x0380, RTWP, 7, rtwp_mp },
	{ 0x03a0, CKON, 7, external_mp },
	{ 0x03c0, CKOF, 7, external_mp },
	{ 0x03e0, LREX, 7, external_mp },
	{ 0x0400, BLWP, 6, blwp_mp },
	{ 0x0440, B, 6, b_mp },
	{ 0x0480, X, 6, x_mp },
	{ 0x04c0, CLR, 6, clr_swpb_mp },
	{ 0x0500, NEG, 6, clr_swpb_mp },
	{ 0x0540, INV, 6, clr_swpb_mp },
	{ 0x0580, INC, 6, clr_swpb_mp },
	{ 0x05c0, INCT, 6, clr_swpb_mp },
	{ 0x0600, DEC, 6, clr_swpb_mp },
	{ 0x0640, DECT, 6, clr_swpb_mp },
	{ 0x0680, BL, 6, bl_mp },
	{ 0x06c0, SWPB, 6, clr_swpb_mp },
	{ 0x0700, SETO, 6, clr_swpb_mp },
	{ 0x0740, ABS, 6, abs_mp },
	{ 0x0800, SRA, 5, shift_mp },
	{ 0x0900, SRL, 5, shift_mp },
	{ 0x0a00, SLA, 5, shift_mp },
	{ 0x0b00, SRC, 5, shift_mp },
	{ 0x1000, JMP, 2, jmp_mp },
	{ 0x1100, JLT, 2, jmp_mp },
	{ 0x1200, JLE, 2, jmp_mp },
	{ 0x1300, JEQ, 2, jmp_mp },
	{ 0x1400, JHE, 2, jmp_mp },
	{ 0x1500, JGT, 2, jmp_mp },
	{ 0x1600, JNE, 2, jmp_mp },
	{ 0x1700, JNC, 2, jmp_mp },
	{ 0x1800, JOC, 2, jmp_mp },
	{ 0x1900, JNO, 2, jmp_mp },
	{ 0x1a00, JL, 2, jmp_mp },
	{ 0x1b00, JH, 2, jmp_mp },
	{ 0x1c00, JOP, 2, jmp_mp },
	{ 0x1d00, SBO, 2, sbz_sbo_mp },
	{ 0x1e00, SBZ, 2, sbz_sbo_mp },
	{ 0x1f00, TB, 2, tb_mp },
	{ 0x2000, COC, 3, f3_mp },
	{ 0x2400, CZC, 3, f3_mp },
	{ 0x2800, XOR, 3, xor_mp },
	{ 0x2c00, XOP, 3, xop_mp },
	{ 0x3000, LDCR, 4, ldcr_mp },
	{ 0x3400, STCR, 4, stcr_mp },
	{ 0x3800, MPY, 9, mult_mp },
	{ 0x3c00, DIV, 9, div_mp },
	{ 0x4000, SZC, 1, f1_mp },
	{ 0x5000, SZCB, 1, f1_mp },
	{ 0x6000, S, 1, f1_mp },
	{ 0x7000, SB, 1, f1_mp },
	{ 0x8000, C, 1, comp_mp },
	{ 0x9000, CB, 1, comp_mp },
	{ 0xa000, A, 1, f1_mp },
	{ 0xb000, AB, 1, f1_mp },
	{ 0xc000, MOV, 1, f1_mp },
	{ 0xd000, MOVB, 1, f1_mp },
	{ 0xe000, SOC, 1, f1_mp },
	{ 0xf000, SOCB, 1, f1_mp }
};

/*
    Create a B-tree for looking up the commands. Each node can carry up to
    16 entries, indexed by 4 consecutive bits in the opcode.

    Works as follows:

    Opcode = 0201 (Load immediate value into register 1)
    Opcode = 0284 (Compare immediate value with register 4)

    Table: [ Table0, table1, table2, ... tableF ]
               |
       +-------+
       v
    table0: [ table00, table01, table02, ... table0f ]
                                  |
        +-------------------------+
        v
    table02: [ table020, table021, ... table028, ... table02f ]
                   |         |             |
                   v         v             v
                 Entry      NULL          Entry
                for LI                   for CI

    For each level in the tree, four more bits are compared. The search
    terminates when the number of compared bits is equal or higher than
    the number of significant bits of the format of this opcode. The entry
    points to the respective line in s_command.

    This way we can decode all format 1 commands by a single pass (including the
    most frequent command MOV), and almost all commands by less than four passes.

    The disadvantage is that we have to build these tables from the opcode
    list at runtime, and many positions are empty. But we do not need more
    than 20 tables for the TMS command set.
*/
void tms99xx_device::build_command_lookup_table()
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
		if (TRACE_SETUP) logerror("tms99xx: === opcode=%04x, len=%d\n", inst->opcode, format_mask_len[inst->format]);
		bitcount = 4;
		opcode = inst->opcode;
		cmdindex = (opcode>>12) & 0x000f;

		while (bitcount < format_mask_len[inst->format])
		{
			// Descend
			if (table[cmdindex].next_digit == nullptr)
			{
				if (TRACE_SETUP) logerror("tms99xx: create new table at bitcount=%d for index=%d\n", bitcount, cmdindex);
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
				if (TRACE_SETUP) logerror("tms99xx: found a table at bitcount=%d\n", bitcount);
			}

			table = table[cmdindex].next_digit;

			bitcount = bitcount+4;
			opcode <<= 4;
			cmdindex = (opcode>>12) & 0x000f;
			if (TRACE_SETUP) logerror("tms99xx: next index=%x\n", cmdindex);
		}

		if (TRACE_SETUP) logerror("tms99xx: bitcount=%d\n", bitcount);
		// We are at the target level
		// Need to fill in the same entry for all values in the bitcount
		// (if a command needs 10 bits we have to copy it four
		// times for all combinations with 12 bits)
		for (int j=0; j < (1<<(bitcount-format_mask_len[inst->format])); j++)
		{
			if (TRACE_SETUP) logerror("tms99xx: opcode=%04x at position %d\n", inst->opcode, cmdindex+j);
			table[cmdindex+j].entry = inst;
		}

		i++;
	} while (inst->opcode != 0xf000);

	m_lotables[k++] = nullptr;
	if (TRACE_SETUP) logerror("tms99xx: Allocated %d tables\n", k);
}

/*
    Main execution loop

    For each invocation of execute_run, a number of loop iterations has been
    calculated before (m_icount). Each loop iteration is one clock cycle.
    The loop must be executed for the number of times that corresponds to the
    time until the next timer event.

    In this implementation, each loop iteration also causes the clock line to
    pulse once. External devices may use this pulse to decrement counters
    which control the READY line.

    Machine cycles to clock input:

      +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+
      | | | | | | | | | | | | | | | | | |  clock (1 of 4 phases)
    +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +
    |-------|-------|-------|-------|----  cycles (2 clock pulses each)

    Wait states only have effect for memory operations. They are processed as
    follows:

    1) The CPU sets the address bus for reading. If READY is low, the CPU
    waits for the next clock tick repeatedly until READY is high again.
    When this is the case, the data bus is sampled on the next clock tick
    and the read operation is complete.

    As we do not have a split-phase read operation in this emulation
    we actually read the data bus instantly but wait for the READY line to
    be high again.

    2) The CPU sets the address bus for writing. In the same moment, the data
    bus is loaded with the word to be written. On the next clock tick,
    the CPU checks the READY line and waits until it is high. When READY
    is high at a clock tick, the operation is complete on the next clock tick.
*/
void tms99xx_device::execute_run()
{
	if (m_reset) service_interrupt();

	if (TRACE_EMU) logerror("tms99xx: calling execute_run for %d cycles\n", m_icount);
	do
	{
		// Only when last instruction has completed
		if (m_program == nullptr)
		{
			if (m_load_state)
			{
				logerror("tms99xx: LOAD interrupt\n");
				m_irq_level = LOAD_INT;
				m_irq_state = false;
				service_interrupt();
			}
			else
			{
				// Interrupts are serviced when
				// - an interrupt condition is signaled over INTREQ and
				// - the level indicated by IC0-IC3 is lower than the interrupt mask value and
				// - the previous instruction is not an XOP or BLWP
				if (m_irq_state && (m_irq_level <= (ST & 0x000f)) && (m_command != XOP && m_command != BLWP))
					service_interrupt();
			}
		}

		if (m_program == nullptr && m_idle_state)
		{
			if (TRACE_WAIT) logerror("tms99xx: idle state\n");
			pulse_clock(1);
			if (!m_external_operation.isnull())
			{
				m_external_operation(IDLE_OP, 0, 0xff);
				m_external_operation(IDLE_OP, 1, 0xff);
			}
		}
		else
		{
			// Handle HOLD
			// A HOLD request is signalled through the input line HOLD.
			// The hold state will be entered with the next non-memory access cycle.
			if (m_hold_state &&
				(m_program==nullptr ||
				(m_program[MPC] != IAQ &&
				m_program[MPC] != MEMORY_READ && m_program[MPC] != MEMORY_WRITE &&
				m_program[MPC] != REG_READ && m_program[MPC] != REG_WRITE)))
			{
				if (TRACE_WAIT) logerror("tms99xx: hold\n");
				if (!m_hold_acknowledged) acknowledge_hold();
				pulse_clock(1);
			}
			else
			{
				// Normal operation
				if (m_check_ready && m_ready == false)
				{
					// We are in a wait state
					set_wait_state(true);
					if (TRACE_WAIT) logerror("tms99xx: wait\n");
					// The clock output should be used to change the state of an outer
					// device which operates the READY line
					pulse_clock(1);
				}
				else
				{
					set_wait_state(false);
					m_check_ready = false;

					if (m_program==nullptr) m_op = IAQ;
					else
					{
						m_op = m_program[MPC];
					}
					if (TRACE_MICRO) logerror("tms99xx: MPC = %d, m_op = %d\n", MPC, m_op);
					// Call the operation of the microprogram
					(this->*s_microoperation[m_op])();
					// If we have multiple passes (as in the TMS9980)
					m_pass--;
					if (m_pass<=0)
					{
						m_pass = 1;
						MPC++;
						m_mem_phase = 1;
						if (!m_iaq_line.isnull()) m_iaq_line(CLEAR_LINE);
					}
				}
			}
		}
	} while (m_icount>0 && !m_reset);
	if (TRACE_EMU) logerror("tms99xx: cycles expired; will return soon.\n");
}

/**************************************************************************/

/*
    Interrupt input
*/
void tms99xx_device::execute_set_input(int irqline, int state)
{
	if (irqline==INT_9900_RESET && state==ASSERT_LINE)
	{
		m_reset = true;
	}
	else
	{
		if (irqline == INT_9900_LOAD)
		{
			m_load_state = (state==ASSERT_LINE);
			m_irq_level = -1;
			m_reset = false;
		}
		else
		{
			m_irq_state = (state==ASSERT_LINE);
			if (state==ASSERT_LINE)
			{
				m_irq_level = get_intlevel(state);
				if (TRACE_INT) logerror("tms99xx: /INT asserted, level=%d, ST=%04x\n", m_irq_level, ST);
			}
			else
			{
				if (TRACE_INT) logerror("tms99xx: /INT cleared\n");
			}
		}
	}
}

/*
    This can be overloaded by variants of TMS99xx.
*/
int tms99xx_device::get_intlevel(int state)
{
	if (!m_get_intlevel.isnull()) return m_get_intlevel(0);
	return 0;
}

void tms99xx_device::service_interrupt()
{
	m_program = int_mp;
	m_command = INTR;
	m_idle_state = false;
	if (!m_external_operation.isnull()) m_external_operation(IDLE_OP, 0, 0xff);

	m_state = 0;

	if (!m_dbin_line.isnull()) m_dbin_line(ASSERT_LINE);

	// If reset, we just start with execution, otherwise we put the MPC
	// on the first microinstruction, which also means that the main loop shall
	// leave it where it is. So we pretend we have another pass to do.
	m_pass = m_reset? 1 : 2;

	if (m_reset)
	{
		m_irq_level = RESET_INT;

		m_ready_bufd = true;
		m_ready = true;
		m_load_state = false;
		m_hold_state = false;
		m_hold_acknowledged = false;
		m_wait_state = false;
		IR = 0;
		ST = 0;
		m_mem_phase = 1;

		m_reset = false;
	}
	if (TRACE_INT)
	{
		switch (m_irq_level)
		{
		case RESET_INT: logerror("tms99xx: **** triggered a RESET interrupt\n"); break;
		case LOAD_INT: logerror("tms99xx: **** triggered a LOAD (NMI) interrupt\n"); break;
		default: logerror("tms99xx: ** triggered an interrupt on level %d\n", m_irq_level); break;
		}
	}

	MPC = 0;
	m_first_cycle = m_icount;
}

/*
    Issue a pulse on the clock line.
*/
void tms99xx_device::pulse_clock(int count)
{
	for (int i=0; i < count; i++)
	{
		if (!m_clock_out_line.isnull()) m_clock_out_line(ASSERT_LINE);
		m_ready = m_ready_bufd;              // get the latched READY state
		if (!m_clock_out_line.isnull()) m_clock_out_line(CLEAR_LINE);
		m_icount--;                         // This is the only location where we count down the cycles.
		if (TRACE_CLOCK)
		{
			if (m_check_ready) logerror("tms99xx: pulse_clock, READY=%d\n", m_ready? 1:0);
			else logerror("tms99xx: pulse_clock\n");
		}
	}
}

/*
    Enter the hold state.
*/
void tms99xx_device::set_hold(int state)
{
	m_hold_state = (state==ASSERT_LINE);
	if (!m_hold_state)
	{
		m_hold_acknowledged = false;
		if (!m_holda_line.isnull()) m_holda_line(CLEAR_LINE);
	}
}

/*
    Acknowledge the HOLD request.
*/
inline void tms99xx_device::acknowledge_hold()
{
	m_hold_acknowledged = true;
	if (!m_holda_line.isnull()) m_holda_line(ASSERT_LINE);
}

/*
    Signal READY to the CPU. When cleared, the CPU enters wait states. This
    becomes effective on a clock pulse.
*/
void tms99xx_device::set_ready(int state)
{
	m_ready_bufd = (state==ASSERT_LINE);
}

void tms99xx_device::abort_operation()
{
	command_completed();
}

/*
    Enter or leave the wait state. We only operate the WAIT line when there is a change.
*/
inline void tms99xx_device::set_wait_state(bool state)
{
	if (m_wait_state != state)
		if (!m_wait_line.isnull()) m_wait_line(state? ASSERT_LINE : CLEAR_LINE);
	m_wait_state = state;
}

/*
    Acquire the next word as an instruction. The program counter advances by
    one word.
*/
void tms99xx_device::decode(UINT16 inst)
{
	int index = 0;
	lookup_entry* table = m_command_lookup_table;
	UINT16 opcode = inst;
	bool complete = false;
	const tms_instruction *decoded;

	m_state = 0;
	IR = inst;
	m_get_destination = false;
	m_byteop = false;

	while (!complete)
	{
		index = (opcode >> 12) & 0x000f;
		if (TRACE_MICRO) logerror("tms99xx: Check next hex digit of instruction %x\n", index);
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
		logerror("tms99xx: Illegal opcode %04x\n", inst);
		IR = 0;
		// This will cause another instruction acquisition in the next machine cycle
		// with an asserted IAQ line (can be used to indicate this illegal opcode detection).
		m_program = nullptr;
	}
	else
	{
		m_program = decoded->prog;
		MPC = -1;
		m_command = decoded->id;
		if (TRACE_MICRO) logerror("tms99xx: Command decoded as id %d, %s, base opcode %04x\n", m_command, opname[m_command], decoded->opcode);
		// Byte operations are either format 1 with the byte flag set
		// or format 4 (CRU multi bit operations) with 1-8 bits to transfer.
		m_byteop = ((decoded->format==1 && ((IR & 0x1000)!=0))
				|| (decoded->format==4 && (((IR >> 6)&0x000f) > 0) && (((IR >> 6)&0x000f) > 9)));
	}
	m_pass = 1;
}

inline bool tms99xx_device::byte_operation()
{
	return (IR & 0x1000)!=0;
}

void tms99xx_device::acquire_instruction()
{
	if (m_mem_phase == 1)
	{
		if (!m_iaq_line.isnull()) m_iaq_line(ASSERT_LINE);
		m_address = PC;
		m_first_cycle = m_icount;
	}

	mem_read();

	if (m_mem_phase == 1)
	{
		decode(m_current_value);
		if (TRACE_EXEC) logerror("tms99xx: %04x: %04x (%s)\n", PC, IR, opname[m_command]);
		debugger_instruction_hook(this, PC);
		PC = (PC + 2) & 0xfffe & m_prgaddr_mask;
		// IAQ will be cleared in the main loop
	}
}

/*
    Memory read
    Clock cycles: 2 + W, W = number of wait states
*/
void tms99xx_device::mem_read()
{
	// After set_address, any device attached to the address bus may pull down
	// READY in order to put the CPU into wait state before the read_word
	// operation will be performed
	// set_address and read_word should pass the same address as argument
	if (m_mem_phase==1)
	{
		if (!m_dbin_line.isnull()) m_dbin_line(ASSERT_LINE);
		m_prgspace->set_address(m_address & m_prgaddr_mask & 0xfffe);
		m_check_ready = true;
		m_mem_phase = 2;
		m_pass = 2;
		if (TRACE_ADDRESSBUS) logerror("tms99xx: set address (r) %04x\n", m_address);

		pulse_clock(1); // Concludes the first cycle
		// If READY has been found to be low, the CPU will now stay in the wait state loop
	}
	else
	{
		// Second phase (after READY was raised again)
		m_current_value = m_prgspace->read_word(m_address & m_prgaddr_mask & 0xfffe);
		pulse_clock(1);
		if (!m_dbin_line.isnull()) m_dbin_line(CLEAR_LINE);
		m_mem_phase = 1;        // reset to phase 1
		if (TRACE_MEM) logerror("tms99xx: mem r %04x -> %04x\n", m_address, m_current_value);
	}
}

void tms99xx_device::mem_write()
{
	if (m_mem_phase==1)
	{
		if (!m_dbin_line.isnull()) m_dbin_line(CLEAR_LINE);
		// When writing, the data bus is asserted immediately after the address bus
		if (TRACE_ADDRESSBUS) logerror("tms99xx: set address (w) %04x\n", m_address);
		m_prgspace->set_address(m_address & m_prgaddr_mask & 0xfffe);
		if (TRACE_MEM) logerror("tms99xx: mem w %04x <- %04x\n",  m_address, m_current_value);
		m_prgspace->write_word(m_address & m_prgaddr_mask & 0xfffe, m_current_value);
		m_check_ready = true;
		m_mem_phase = 2;
		m_pass = 2;
		pulse_clock(1);
	}
	else
	{
		// Second phase (we arrive here when the wait states are over)
		pulse_clock(1);
	}
}

void tms99xx_device::register_read()
{
	// Need to set m_address for F1/F3 (we don't know what the data_derive did)
	if (m_mem_phase==1)
	{
		m_address = WP + (m_regnumber<<1);
	}

	mem_read();

	if (m_mem_phase==1)
	{
		m_register_contents = m_current_value;
	}
}

/*
    Memory write:

    Clock cycles: 2 + W, W = number of wait states
*/
void tms99xx_device::register_write()
{
	// This will be called twice; m_pass is set by the embedded mem_write
	UINT16 addr_save = m_address;
	m_address = (WP + (m_regnumber<<1)) & m_prgaddr_mask & 0xfffe;
	mem_write();
	m_address = addr_save;
}

/*
    CRU support code

    The CRU bus is a 1-bit-wide I/O bus.  The CPU can read or write bits at random address.
    Special instructions are dedicated to reading and writing one or several consecutive bits.

    The CRU uses the same address bus as the normal memory access. For writing,
    the CRUCLK line is pulsed, but not for reading where CRUCLK stays cleared.
    This means that each normal memory access also causes read accesses on the
    CRU side. The /MEMEN line may be used to distinguish the kinds of accesses
    as it stays cleared during CRU operations.

    We do not emulate this here as it seems there are no real applications of
    this side effect. Real designs must ensure that CRU read operations are
    idempotent (i.e. they must not change the state of the queried device).

    Read returns the number of consecutive CRU bits, with increasing CRU address
    from the least significant to the most significant bit; right-aligned

    There seems to be no handling of wait states during CRU operations on the
    TMS9900. The TMS9995, in contrast, respects wait states during the transmission
    of each single bit.

    Usage of this method:
       CRU write: First bit is at rightmost position of m_value.
*/

void tms99xx_device::cru_input_operation()
{
	int value, value1;
	int offset, location;

	location = (m_cru_address >> 4) & (m_cruaddr_mask>>3);
	offset   = (m_cru_address>>1) & 0x07;

	// Read 8 bits (containing the desired bits)
	value = m_cru->read_byte(location);

	if ((offset + m_count) > 8) // spans two 8 bit cluster
	{
		// Read next 8 bits
		location = (location + 1) & (m_cruaddr_mask>>3);
		value1 = m_cru->read_byte(location);
		value |= (value1 << 8);

		if ((offset + m_count) > 16)    // spans three 8 bit cluster
		{
			// Read next 8 bits
			location = (location + 1) & (m_cruaddr_mask>>3);
			value1 = m_cru->read_byte(location);
			value |= (value1 << 16);
		}
	}

	// On each machine cycle (2 clocks) only one CRU bit is transmitted
	pulse_clock(m_count<<1);

	// Shift back the bits so that the first bit is at the rightmost place
	m_value = (value >> offset);

	// Mask out what we want
	m_value &= (0x0000ffff >> (16-m_count));
}

void tms99xx_device::cru_output_operation()
{
	int value;
	int location;
	location = (m_cru_address >> 1) & m_cruaddr_mask;
	value = m_value;

	// Write m_count bits from cru_address
	for (int i=0; i < m_count; i++)
	{
		if (TRACE_CRU) logerror("tms99xx: CRU output operation, address %04x, value %d\n", location<<1, value & 0x01);
		m_cru->write_byte(location, (value & 0x01));
		value >>= 1;
		location = (location + 1) & m_cruaddr_mask;
		pulse_clock(2);
	}
}

void tms99xx_device::return_from_subprogram()
{
	// Return from data derivation
	// The result should be in m_current_value
	// and the address in m_address
	m_program = m_caller;
	MPC = m_caller_MPC; // will be increased on return
}

void tms99xx_device::command_completed()
{
	// Pseudo state at the end of the current instruction cycle sequence
	if (TRACE_CYCLES)
	{
		logerror("tms99xx: ------");
		int cycles =  m_first_cycle - m_icount;
		// Avoid nonsense values due to expired and resumed main loop
		if (cycles > 0 && cycles < 10000) logerror(" %d cycles", cycles);
		logerror("\n");
	}
	m_program = nullptr;
}

/*
    This is a switch to a subprogram; there is only one, the data
    derivation. In terms of cycles, it does not take any time; execution
    continues with the first instruction of the subprogram.
*/
void tms99xx_device::data_derivation_subprogram()
{
	UINT16 ircopy = IR;

	// Save the return program and position
	m_caller = m_program;
	m_caller_MPC = MPC;

	// Source or destination argument?
	if (m_get_destination) ircopy >>= 6;

	m_regnumber = ircopy & 0x000f;

	m_program = (UINT8*)data_derivation;
	MPC = ircopy & 0x0030;

	if (((MPC == 0x0020) && (m_regnumber != 0))         // indexed
		|| ((MPC == 0x0030) && m_byteop))       // byte operation
	{
		MPC += 8;   // the second option
	}
	m_get_destination = true;   // when we call this the second time before END it's the destination
	m_pass = 2;
}


/**************************************************************************
    Status bit operations
**************************************************************************/

inline void tms99xx_device::set_status_bit(int bit, bool state)
{
	if (state) ST |= bit;
	else ST &= ~bit;
}

void tms99xx_device::set_status_parity(UINT8 value)
{
	int count = 0;
	for (int i=0; i < 8; i++)
	{
		if ((value & 0x80)!=0) count++;
		value <<= 1;
	}
	set_status_bit(ST_OP, (count & 1)!=0);
}

inline void tms99xx_device::compare_and_set_lae(UINT16 value1, UINT16 value2)
{
	set_status_bit(ST_EQ, value1 == value2);
	set_status_bit(ST_LH, value1 > value2);
	set_status_bit(ST_AGT, (INT16)value1 > (INT16)value2);
	if (TRACE_STATUS) logerror("tms99xx: ST = %04x (val1=%04x, val2=%04x)\n", ST, value1, value2);
}

/**************************************************************************
    ALU operations
**************************************************************************/

void tms99xx_device::alu_nop()
{
	// Do nothing (or nothing that is externally visible)
	pulse_clock(2);
	return;
}

void tms99xx_device::alu_source()
{
	// Copy the current value into the source data register
	m_source_even = ((m_address & 1)==0);
	m_source_value = m_current_value;
	m_source_address = m_address;
	pulse_clock(2);
}

void tms99xx_device::alu_clear()
{
	// Clears the register contents
	m_register_contents = 0;
	pulse_clock(2);
}

void tms99xx_device::alu_setaddr()
{
	// Load the current value into the address register
	m_address = m_current_value;
	pulse_clock(2);
}

void tms99xx_device::alu_addone()
{
	m_current_value++;
	pulse_clock(2);
}

void tms99xx_device::alu_setaddr_addone()
{
	// Set the address register and increase the recent value
	m_address = m_current_value;
	m_current_value++;
	pulse_clock(2);
}

void tms99xx_device::alu_pcaddr_advance()
{
	// Set PC as new read address, increase by 2
	m_address = PC;
	PC = (PC + 2) & 0xfffe & m_prgaddr_mask;
	pulse_clock(2);
}

void tms99xx_device::alu_add_register()
{
	// Add the register contents to the current value and set as address
	m_address = m_current_value + m_register_contents;
	pulse_clock(2);
}

void tms99xx_device::alu_imm()
{
	m_value_copy = m_current_value;
	m_address_copy = m_address;
	m_address = PC;
	PC = (PC + 2) & 0xfffe & m_prgaddr_mask;
	pulse_clock(2);
}

void tms99xx_device::alu_reg()
{
	m_address = (WP + ((IR & 0x000f)<<1)) & m_prgaddr_mask;
	pulse_clock(2);
}

void tms99xx_device::alu_f1()
{
	UINT32 dest_new = 0;

	// Save the destination value
	UINT16 prev_dest_value = m_current_value;

	m_destination_even = ((m_address & 1)==0);  // this is the destination address; the source address has already been saved
	bool byteop = byte_operation();

	if (byteop)
	{
		if (!m_destination_even) m_current_value <<= 8;
		if (!m_source_even) m_source_value <<= 8;
		// We have to strip away the low byte, or byte operations may fail
		// e.g. 0x10ff + 0x0101 = 0x1200
		// or   0x2000 - 0x0101 = 0x1eff
		m_source_value &= 0xff00;
		m_current_value &= 0xff00;
	}

	switch (m_command)
	{
		case A:
		case AB:
			// Add the contents of the source data to the destination data
			// May exceed 0xffff (for carry check)
			dest_new = m_current_value + m_source_value;

			// 1000 + e000 = f000 (L)
			// c000 + c000 = 8000 (LC)
			// 7000 + 4000 = b000 (LO)
			// 2000 + f000 = 1000 (LAC)
			// c000 + b000 = 7000 (LACO)
			// 2000 + e000 = 0000 (EC)
			// 8000 + 8000 = 0000 (ECO)

			// When adding, a carry occurs when we exceed the 0xffff value.
			set_status_bit(ST_C, (dest_new & 0x10000) != 0);
			// If the result has a sign bit that is different from both arguments, we have an overflow
			// (i.e. getting a negative value from two positive values and vice versa)
			set_status_bit(ST_OV, ((dest_new ^ m_current_value) & (dest_new ^ m_source_value) & 0x8000)!=0);
			break;

		case S:
		case SB:
			// Subtract the contents of the source data from the destination data
			dest_new = m_current_value + ((~m_source_value) & 0xffff) + 1;
			// LAECO(P)
			// 8000 - 8000 = 0000 (EC)
			// 2000 - 8000 = a000 (LO)
			// 8000 - 2000 = 6000 (LACO)
			// 2000 - 1000 = 1000 (LAC)
			// 1000 - 2000 = f000 (L)
			// 1000 - 1000 = 0000 (EC)
			// 1000 - f000 = 2000 (LA)
			// f000 - 2000 = d000 (LC)

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
			// OR the contents of the source data on the destination data
			dest_new = m_current_value | m_source_value;
			break;

		case SZC:
		case SZCB:
			// AND the one's complement of the contents of the source data on the destination data
			dest_new = m_current_value & ~m_source_value;
			break;

		case MOV:
		case MOVB:
			// Copy the source data to the destination data
			dest_new = m_source_value;
			break;
	}

	if (byteop)
	{
		set_status_parity((UINT8)(dest_new>>8));

		// destnew is the new value to be written (high byte); needs to be
		// merged with the existing word
		if (m_destination_even)
			m_current_value = (prev_dest_value & 0x00ff) | (dest_new & 0xff00);
		else
			m_current_value = (prev_dest_value & 0xff00) | ((dest_new >> 8) & 0x00ff);
		compare_and_set_lae((UINT16)(dest_new & 0xff00), 0);
	}
	else
	{
		m_current_value = (UINT16)(dest_new & 0xffff);
		compare_and_set_lae((UINT16)(dest_new & 0xffff), 0);
	}

	pulse_clock(2);
}

void tms99xx_device::alu_comp()
{
	m_destination_even = ((m_address & 1)==0);  // this is the destination address; the source address has already been saved
	if (byte_operation())
	{
		if (!m_destination_even) m_current_value <<= 8;
		if (!m_source_even) m_source_value <<= 8;
		set_status_parity((UINT8)(m_source_value>>8));
		compare_and_set_lae(m_source_value & 0xff00, m_current_value & 0xff00);
	}
	else
		compare_and_set_lae(m_source_value, m_current_value);

	pulse_clock(2);
}

void tms99xx_device::alu_f3()
{
	switch (m_state)
	{
	case 0:
		// Get register address
		m_address = WP + ((IR >> 5) & 0x001e);
		m_source_value = m_current_value;
		break;
	case 1:
		if (m_command == COC)
		{
			set_status_bit(ST_EQ, (m_current_value & m_source_value) == m_source_value);
		}
		else
		{
			if (m_command == CZC)
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
		if (TRACE_STATUS) logerror("tms99xx: ST = %04x\n", ST);
		break;
	}

	m_state++;
	pulse_clock(2);
}

void tms99xx_device::alu_multiply()
{
	UINT32 result;

	switch (m_state)
	{
	case 0: // After data derivation
		m_source_value = m_current_value;
		m_address = ((IR >> 5) & 0x001e) + WP;
		break;
	case 1: // After reading the register (multiplier)
		if (TRACE_ALU) logerror("tms99xx: Multiply %04x by %04x\n", m_current_value, m_source_value);
		result = (m_source_value & 0x0000ffff) * (m_current_value & 0x0000ffff);
		m_current_value = (result >> 16) & 0xffff;
		m_value_copy = result & 0xffff;
		pulse_clock(34);                                // add 36 clock cycles (18 machine cycles); last one in main loop
		break;
	case 2: // After writing the high word to the destination register
		m_current_value = m_value_copy;                     // Prepare to save low word
		m_address = (m_address + 2) & m_prgaddr_mask;
		break;
	}
	pulse_clock(2);
	m_state++;
}

void tms99xx_device::alu_divide()
{
	// Format is DIV Divisor,REG(dividend)
	UINT32 uval32;
	bool overflow = true;
	UINT16 value1;

	switch (m_state)
	{
	case 0:
		m_source_value = m_current_value;   // store divisor
		// Set address of register
		m_address = WP + ((IR >> 5) & 0x001e);
		m_address_copy = m_address;
		break;
	case 1:
		// We have an overflow when the quotient cannot be stored in 16 bits
		// This is the case when the dividend / divisor >= 0x10000,
		// or equivalently, dividend / 0x10000 >= divisor

		if (m_current_value < m_source_value)   // also if source=0
		{
			MPC++;  // skip the abort
			overflow = false;
		}
		set_status_bit(ST_OV, overflow);
		m_value_copy = m_current_value;         // Save the high word
		m_address = (m_address + 2) & m_prgaddr_mask;       // Read next word
		break;
	case 2:
		// W2 is in m_current_value
		// Create full word and perform division
		uval32 = (m_value_copy << 16) | m_current_value;

		if (TRACE_ALU) logerror("tms99xx: Dividing %08x by %04x\n", uval32, m_source_value);
		m_current_value = uval32 / m_source_value;
		m_value_copy = uval32 % m_source_value;

		if (TRACE_ALU) logerror("tms99xx: Quotient %04x, remainder %04x\n", m_current_value, m_value_copy);

		m_address = m_address_copy;

		// The number of ALU cycles depends on the number of steps in
		// the division algorithm. The number of cycles is between 32 and
		// 48 (*2 for clock cycles)
		// As I don't have a description of the actual algorithm, I'll use
		// the following heuristic: We use 32 ALU cycles in general, then
		// we need as many cycles as it takes to
		// shift away the dividend. Thus, bigger dividends need more cycles.

		pulse_clock(62);    // one pulse is at the start, one at the end
		value1 = m_value_copy & 0xffff;

		while (value1 != 0)
		{
			value1 = (value1 >> 1) & 0xffff;
			pulse_clock(2);
		}
		// We still have m_regnumber; this is where m_current_value will go to
		break;
	case 3:
		// Prepare to write the remainder
		m_current_value = m_value_copy;
		m_address = m_address + 2;
		if (TRACE_STATUS) logerror("tms99xx: ST = %04x (div)\n", ST);
		break;
	}
	pulse_clock(2);
	m_state++;
}

void tms99xx_device::alu_xop()
{
	switch (m_state)
	{
	case 0:
		// We have the effective address of the source operand in m_address
		m_address_saved = m_address;
		// Now we take the XOP number from the instruction register
		// and calculate the vector location
		// [0010 11xx xx tt SSSS]  shift 6 right, then *4 => shift 4 right
		m_address = 0x0040 + ((IR >> 4) & 0x003c);
		// Takes some additional cycles
		pulse_clock(4);
		break;
	case 1:
		m_value_copy = WP;                      // save the old WP
		WP = m_current_value & m_prgaddr_mask & 0xfffe;  // the new WP has been read in the previous microoperation
		m_current_value = m_address_saved;      // we saved the address of the source operand; retrieve it
		m_address = WP + 0x0016;                // Next register is R11
		break;
	case 2:
		m_address = WP + 0x001e;
		m_current_value = ST;
		break;
	case 3:
		m_address = WP + 0x001c;
		m_current_value = PC;
		break;
	case 4:
		m_address = WP + 0x001a;
		m_current_value = m_value_copy;         // old WP into new R13
		break;
	case 5:
		m_address =  0x0042 + ((IR >> 4) & 0x003c);     // location of new PC
		set_status_bit(ST_X, true);
		break;
	case 6:
		PC = m_current_value & m_prgaddr_mask & 0xfffe;
		break;
	}
	pulse_clock(2);
	m_state++;
}

void tms99xx_device::alu_clr_swpb()
{
	UINT32 dest_new = 0;
	UINT32 src_val = m_current_value & 0x0000ffff;
	UINT16 sign = 0;

	bool setstatus = true;
	bool check_ov = true;

	switch (m_command)
	{
	case CLR:
		// no status bits
		m_current_value = 0x0000;
		setstatus = false;
		break;
	case SETO:
		// no status bits
		m_current_value = 0xffff;
		setstatus = false;
		break;
	case INV:
		// LAE
		dest_new = ~src_val & 0xffff;
		check_ov = false;
		break;
	case NEG:
		// LAECO
		// Overflow occurs for value=0x8000
		dest_new = ((~src_val) & 0x0000ffff) + 1;
		check_ov = false;
		set_status_bit(ST_OV, src_val == 0x8000);
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
	case SWPB:
		m_current_value = ((m_current_value << 8) | (m_current_value >> 8)) & 0xffff;
		setstatus = false;
		break;
	}

	if (setstatus)
	{
		if (check_ov) set_status_bit(ST_OV, ((src_val & 0x8000)==sign) && ((dest_new & 0x8000)!=sign));
		set_status_bit(ST_C, (dest_new & 0x10000) != 0);
		m_current_value = dest_new & 0xffff;
		compare_and_set_lae(m_current_value, 0);
	}

	pulse_clock(2);
	// No states here
}

void tms99xx_device::alu_abs()
{
	// LAECO (from original word!)
	// O if >8000
	// C is alwas reset
	set_status_bit(ST_OV, m_current_value == 0x8000);
	set_status_bit(ST_C, false);
	compare_and_set_lae(m_current_value, 0);

	if ((m_current_value & 0x8000)!=0)
	{
		m_current_value = (((~m_current_value) & 0x0000ffff) + 1) & 0xffff;
		pulse_clock(2);     // If ABS is performed it takes one machine cycle more
	}
	else
	{
		MPC++; // skips over the next micro operation (MEMORY_WRITE)
	}
	pulse_clock(2);
}

void tms99xx_device::alu_x()
{
	if (TRACE_ALU) logerror("tms99xx: Substituting current command by %04x\n", m_current_value);
	decode(m_current_value);
	pulse_clock(2);
}

/*
    Also used by other microprograms
*/
void tms99xx_device::alu_b()
{
	// no status bits
	// Although we got the contents of the source data, we do not use them
	// but directly branch there. That is, we are only interested in the
	// address of the source data.
	// If we have a B *R5 and R5 contains the value 0xa000, the CPU actually
	// retrieves the value at 0xa000, but in fact it will load the PC
	// with the address 0xa000
	m_current_value = PC;
	PC = m_address & m_prgaddr_mask & 0xfffe;
	m_address = WP + 22;
	if (TRACE_ALU) logerror("tms99xx: Set new PC = %04x\n", PC);
	pulse_clock(2);
}

void tms99xx_device::alu_blwp()
{
	switch (m_state)
	{
	case 0:
		m_value_copy = WP;
		WP = m_current_value & m_prgaddr_mask & 0xfffe;              // set new WP (*m_destination)
		m_address_saved = (m_address + 2) & m_prgaddr_mask; // Save the location of the WP
		m_address = WP + 30;
		m_current_value = ST;                           // get status register
		break;
	case 1:
		m_current_value = PC;                           // get program counter
		m_address = m_address - 2;
		break;
	case 2:
		m_current_value = m_value_copy;                 // retrieve the old WP
		m_address = m_address - 2;
		break;
	case 3:
		m_address = m_address_saved;                    // point to PC component of branch vector
		break;
	case 4:
		PC = m_current_value & m_prgaddr_mask & 0xfffe;
		if (TRACE_ALU) logerror("tms9900: Context switch complete; WP=%04x, PC=%04x, ST=%04x\n", WP, PC, ST);
		break;
	}
	pulse_clock(2);
	m_state++;
}

void tms99xx_device::alu_ldcr()
{
	UINT16 value;

	// Spec: "If the source operand address is odd, the address is truncated
	//        to an even address prior to data transfer."
	// (Editor/Assembler, page 151)
	// This refers to transfers with more than 8 bits. In this case, for
	// LDCR the first bit is taken from the least significant bit of the
	// source word. If the address is odd (e.g. 0x1001), it is
	// treated as 0x1000, that is, truncated to an even address.
	// For transfers with 1-8 bits, the first bit is the least significant
	// bit of the source byte (any address).

	if (m_state == 0)
	{
		m_address = WP + 24;
	}
	else
	{
		value = m_source_value; // copied by ALU_SOURCE
		m_count = (IR >> 6) & 0x000f;
		if (m_count == 0) m_count = 16;
		if (m_count <= 8)
		{
			if (m_source_even) value>>=8;
			set_status_parity((UINT8)(value & 0xff));
			compare_and_set_lae(value<<8, 0);
		}
		else
		{
			compare_and_set_lae(value, 0);
		}
		m_cru_address = m_current_value;
		m_value = value;
		if (TRACE_CRU) logerror("tms99xx: Load CRU address %04x (%d bits), value = %04x\n", m_cru_address, m_count, m_value);
	}
	m_state++;
	pulse_clock(2);
}

void tms99xx_device::alu_stcr()
{
	UINT16 value;
	int n = 2;
	// For STCR transfers with more than 8 bits, the first CRU bit is
	// always put into the least significant bit of the destination word.
	// If the address is odd (e.g. 0x1001), it is treated as 0x1000, that is,
	// truncated to an even boundary.
	// For transfers with 1-8 bits, the destination address is handled as
	// in MOVB operations, i.e. the other byte of the word is kept unchanged.

	switch (m_state)
	{
	case 0: // After getting the destination operand and saving the address/value
		m_address = WP + 24;
		n = 0;
		break;
	case 1: // After getting R12
		m_cru_address = m_current_value;
		m_count = (IR >> 6) & 0x000f;
		if (m_count == 0) m_count = 16;
		break;
	case 2: // After the cru operation; value starts at LSB of m_value
		value = m_value & 0xffff;
		if (m_count < 9)
		{
			if (TRACE_CRU) logerror("tms99xx: Store CRU at %04x (%d bits) in %04x, result = %02x\n", m_cru_address, m_count, m_source_address, value);
			set_status_parity((UINT8)(value & 0xff));
			compare_and_set_lae(value<<8, 0);
			if (m_source_even)
				m_current_value = (m_source_value & 0x00ff) | (value<<8);
			else
				m_current_value = (m_source_value & 0xff00) | (value & 0xff);

			pulse_clock(2*(5 + (8-m_count)));
		}
		else
		{
			if (TRACE_CRU) logerror("tms99xx: Store CRU at %04x (%d bits) in %04x, result = %04x\n", m_cru_address, m_count, m_source_address, value);
			m_current_value = value;
			compare_and_set_lae(value, 0);
			pulse_clock(2*(5 + (16-m_count)));
		}
		m_address = m_source_address;
		break;
	}

	m_state++;
	pulse_clock(n);
}

void tms99xx_device::alu_sbz_sbo()
{
	INT8 displacement;
	if (m_state==0)
	{
		m_address = WP + 24;
	}
	else
	{
		m_value = (m_command==SBO)? 1 : 0;
		displacement = (INT8)(IR & 0xff);
		m_cru_address = m_current_value + (displacement<<1);
		m_count = 1;
	}
	m_state++;
	pulse_clock(2);
}

void tms99xx_device::alu_tb()
{
	INT8 displacement;
	switch (m_state)
	{
	case 0:
		m_address = WP + 24;
		break;
	case 1:
		displacement = (INT8)(IR & 0xff);
		m_cru_address = m_current_value + (displacement<<1);
		m_count = 1;
		break;
	case 2:
		set_status_bit(ST_EQ, m_value!=0);
		if (TRACE_STATUS) logerror("tms99xx: ST = %04x\n", ST);
		break;
	}
	m_state++;
	pulse_clock(2);
}

void tms99xx_device::alu_jmp()
{
	INT8 displacement;
	bool cond = false;

	if (m_state==0)
	{
		switch (m_command)
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
			if (TRACE_ALU) logerror("tms99xx: Jump condition false\n");
			MPC+=1; // skip next ALU call
		}
		else
			if (TRACE_ALU) logerror("tms99xx: Jump condition true\n");
	}
	else
	{
		displacement = (IR & 0xff);
		PC = (PC + (displacement<<1)) & m_prgaddr_mask & 0xfffe;
	}
	m_state++;
	pulse_clock(2);
}

void tms99xx_device::alu_shift()
{
	bool carry = false;
	bool overflow = false;
	UINT16 sign = 0;
	UINT32 value;
	int count;

	switch (m_state)
	{
	case 0:
		m_address = WP + ((IR & 0x000f)<<1);
		pulse_clock(2);
		break;
	case 1:
		// we have the value of the register in m_current_value
		// Save it (we may have to read R0)
		m_value_copy = m_current_value;
		m_address_saved = m_address;
		m_address = WP;
		m_current_value = (IR >> 4) & 0x000f;

		if (m_current_value != 0)
		{
			// skip the next read and ALU operation
			MPC = MPC+2;
			m_state++;
		}
		else
		{
			if (TRACE_ALU) logerror("tms99xx: Shift operation gets count from R0\n");
			pulse_clock(2);
		}
		pulse_clock(2);
		break;
	case 2:
		// after READ
		pulse_clock(2);
		pulse_clock(2);
		break;
	case 3:
		count = m_current_value & 0x000f; // from the instruction or from R0
		if (count==0) count = 16;

		value = m_value_copy;

		// we are re-implementing the shift operations because we have to pulse
		// the clock at each single shift anyway.
		// Also, it is easier to implement the status bit setting.
		// Note that count is never 0
		if (m_command == SRA) sign = value & 0x8000;

		for (int i=0; i < count; i++)
		{
			switch (m_command)
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
			pulse_clock(2);
		}

		m_current_value = value & 0xffff;
		set_status_bit(ST_C, carry);
		set_status_bit(ST_OV, overflow);
		compare_and_set_lae(m_current_value, 0);
		m_address = m_address_saved;        // Register address
		if (TRACE_STATUS) logerror("tms99xx: ST = %04x (val=%04x)\n", ST, m_current_value);
		break;
	}
	m_state++;
}

void tms99xx_device::alu_ai_ori()
{
	UINT32 dest_new = 0;
	switch (m_command)
	{
	case AI:
		dest_new = m_current_value + m_value_copy;
		// See status bit handling for Add
		set_status_bit(ST_C, (dest_new & 0x10000) != 0);
		set_status_bit(ST_OV, ((dest_new ^ m_current_value) & (dest_new ^ m_value_copy) & 0x8000)!=0);
		break;
	case ANDI:
		dest_new = m_current_value & m_value_copy;
		break;
	case ORI:
		dest_new = m_current_value | m_value_copy;
		break;
	}
	m_current_value = dest_new & 0xffff;
	m_address = m_address_copy;
	compare_and_set_lae(m_current_value, 0);
	pulse_clock(2);
}

void tms99xx_device::alu_ci()
{
	compare_and_set_lae(m_value_copy, m_current_value);
	pulse_clock(2);
}

void tms99xx_device::alu_li()
{
	compare_and_set_lae(m_current_value, 0);
	pulse_clock(2);
}

void tms99xx_device::alu_lwpi()
{
	WP = m_current_value & m_prgaddr_mask & 0xfffe;
	pulse_clock(2);
}

void tms99xx_device::alu_limi()
{
	ST = (ST & 0xfff0) | (m_current_value & 0x000f);
	if (TRACE_STATUS) logerror("tms99xx: ST = %04x\n", ST);
	pulse_clock(2);
}

void tms99xx_device::alu_stwp_stst()
{
	if (m_command==STST) m_current_value = ST;
	else m_current_value = WP;
	pulse_clock(2);
}

void tms99xx_device::alu_external()
{
	// Call some possibly attached external device
	// We pass the bit pattern of the address bus to the external function

	// IDLE = 0000 0011 0100 0000
	// RSET = 0000 0011 0110 0000
	// CKON = 0000 0011 1010 0000
	// CKOF = 0000 0011 1100 0000
	// LREX = 0000 0011 1110 0000
	//                  ---
	if (m_command == IDLE)
		m_idle_state = true;

	if (!m_external_operation.isnull()) m_external_operation((IR >> 5) & 0x07, 1, 0xff);
	pulse_clock(2);
}

void tms99xx_device::alu_rtwp()
{
	switch (m_state)
	{
	case 0:
		m_address = WP + 30;        // R15
		pulse_clock(2);
		break;
	case 1:
		ST = m_current_value;
		m_address -= 2;             // R14
		break;
	case 2:
		PC = m_current_value & m_prgaddr_mask & 0xfffe;
		m_address -= 2;             // R13
		break;
	case 3:
		WP = m_current_value & m_prgaddr_mask & 0xfffe;
		pulse_clock(2);
		break;
	}
	m_state++;
}


void tms99xx_device::alu_int()
{
	if (TRACE_EMU) logerror("tms99xx: INT state %d; irq_level %d\n", m_state, m_irq_level);
	switch (m_state)
	{
	case 0:
		if (m_irq_level == RESET_INT)
		{
			m_address = 0;
			pulse_clock(2);
		}
		else
		{
			if (m_irq_level == LOAD_INT) m_address = 0xfffc; // will be truncated for TMS9980
			else
			{
				m_address = (m_irq_level << 2);
			}
		}
		break;
	case 1:
		m_address_copy = m_address;
		m_value_copy = WP;                          // old WP
		WP = m_current_value & m_prgaddr_mask & 0xfffe;      // new WP
		m_current_value = ST;
		m_address = (WP + 30) & m_prgaddr_mask;
		break;
	case 2:
		m_current_value = PC;
		m_address = (WP + 28) & m_prgaddr_mask;
		break;
	case 3:
		m_current_value = m_value_copy; // old WP
		m_address = (WP + 26) & m_prgaddr_mask;
		break;
	case 4:
		m_address = (m_address_copy + 2) & 0xfffe & m_prgaddr_mask;
		if (TRACE_ALU) logerror("tms99xx: read from %04x\n", m_address);
		break;
	case 5:
		PC = m_current_value & m_prgaddr_mask & 0xfffe;
		if (m_irq_level > 0 )
		{
			ST = (ST & 0xfff0) | (m_irq_level - 1);
		}
		break;
	}
	m_state++;
	pulse_clock(2);
}

/**************************************************************************/
UINT32 tms99xx_device::execute_min_cycles() const
{
	return 2;
}

// TODO: Compute this value, just a wild guess for the average
UINT32 tms99xx_device::execute_max_cycles() const
{
	return 10;
}

UINT32 tms99xx_device::execute_input_lines() const
{
	return 2;
}

// clocks to cycles, cycles to clocks = id
// execute_default_irq_vector = 0
// execute_burn = nop

// device_disasm_interface overrides
UINT32 tms99xx_device::disasm_min_opcode_bytes() const
{
	return 2;
}

UINT32 tms99xx_device::disasm_max_opcode_bytes() const
{
	return 6;
}

offs_t tms99xx_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms9900 );
	return CPU_DISASSEMBLE_NAME(tms9900)(this, buffer, pc, oprom, opram, options);
}


const device_type TMS9900 = &device_creator<tms9900_device>;
