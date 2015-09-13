// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    asap.c

    Core implementation for the portable ASAP emulator.
    ASAP = Atari Simplified Architecture Processor

    Special thanks to Mike Albaugh for clarification on a couple of fine points.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "asap.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const UINT32 PS_CFLAG           = 0x00000001;
const UINT32 PS_VFLAG           = 0x00000002;
const UINT32 PS_ZFLAG           = 0x00000004;
const UINT32 PS_NFLAG           = 0x00000008;
const UINT32 PS_IFLAG           = 0x00000010;
const UINT32 PS_PFLAG           = 0x00000020;

//const int EXCEPTION_RESET       = 0;
const int EXCEPTION_TRAP0       = 1;
const int EXCEPTION_TRAPF       = 2;
const int EXCEPTION_INTERRUPT   = 3;

const int REGBASE               = 0xffe0;



//**************************************************************************
//  MACROS
//**************************************************************************

#define SET_C_ADD(a,b)          (m_cflag = (UINT32)(b) > (UINT32)(~(a)))
#define SET_C_SUB(a,b)          (m_cflag = (UINT32)(b) <= (UINT32)(a))
#define SET_V_ADD(r,a,b)        (m_vflag = ~((a) ^ (b)) & ((a) ^ (r)))
#define SET_V_SUB(r,a,b)        (m_vflag =  ((a) ^ (b)) & ((a) ^ (r)))
#define SET_ZN(r)               (m_znflag = (r))
#define SET_ZNCV_ADD(r,a,b)     SET_ZN(r); SET_C_ADD(a,b); SET_V_ADD(r,a,b)
#define SET_ZNCV_SUB(r,a,b)     SET_ZN(r); SET_C_SUB(a,b); SET_V_SUB(r,a,b)

#define SET_VFLAG(val)          (m_vflag = (val) << 31)
#define SET_CFLAG(val)          (m_cflag = (val))

#define GET_FLAGS()             (m_cflag | \
									((m_vflag >> 30) & PS_VFLAG) | \
									((m_znflag == 0) << 2) | \
									((m_znflag >> 28) & PS_NFLAG) | \
									(m_iflag << 4) | \
									(m_pflag << 5))

#define SET_FLAGS(v)            do { \
									m_cflag = (v) & PS_CFLAG; \
									m_vflag = ((v) & PS_VFLAG) << 30; \
									m_znflag = ((v) & PS_ZFLAG) ? 0 : ((v) & PS_NFLAG) ? -1 : 1; \
									m_iflag = ((v) & PS_IFLAG) >> 4; \
									m_pflag = ((v) & PS_PFLAG) >> 5; \
								} while (0);

#define OPCODE                  (m_op >> 27)
#define DSTREG                  ((m_op >> 22) & 31)
#define DSTVAL                  m_src2val[REGBASE + DSTREG]
#define SRC1REG                 ((m_op >> 16) & 31)
#define SRC1VAL                 m_src2val[REGBASE + SRC1REG]
#define SRC2VAL                 m_src2val[m_op & 0xffff]



//**************************************************************************
//  STATIC OPCODE TABLES
//**************************************************************************

const asap_device::ophandler asap_device::s_opcodetable[32][4] =
{
	{   &asap_device::trap0,    &asap_device::trap0,    &asap_device::trap0,    &asap_device::trap0     },
	{   &asap_device::noop,     &asap_device::noop,     &asap_device::noop,     &asap_device::noop      },
	{   &asap_device::bsr,      &asap_device::bsr_0,    &asap_device::bsr,      &asap_device::bsr_0     },
	{   &asap_device::lea,      &asap_device::noop,     &asap_device::lea_c,    &asap_device::lea_c0    },
	{   &asap_device::leah,     &asap_device::noop,     &asap_device::leah_c,   &asap_device::leah_c0   },
	{   &asap_device::subr,     &asap_device::noop,     &asap_device::subr_c,   &asap_device::subr_c0   },
	{   &asap_device::xor_,     &asap_device::noop,     &asap_device::xor_c,    &asap_device::xor_c0    },
	{   &asap_device::xorn,     &asap_device::noop,     &asap_device::xorn_c,   &asap_device::xorn_c0   },
	{   &asap_device::add,      &asap_device::noop,     &asap_device::add_c,    &asap_device::add_c0    },
	{   &asap_device::sub,      &asap_device::noop,     &asap_device::sub_c,    &asap_device::sub_c0    },
	{   &asap_device::addc,     &asap_device::noop,     &asap_device::addc_c,   &asap_device::addc_c0   },
	{   &asap_device::subc,     &asap_device::noop,     &asap_device::subc_c,   &asap_device::subc_c0   },
	{   &asap_device::and_,     &asap_device::noop,     &asap_device::and_c,    &asap_device::and_c0    },
	{   &asap_device::andn,     &asap_device::noop,     &asap_device::andn_c,   &asap_device::andn_c0   },
	{   &asap_device::or_,      &asap_device::noop,     &asap_device::or_c,     &asap_device::or_c0     },
	{   &asap_device::orn,      &asap_device::noop,     &asap_device::orn_c,    &asap_device::orn_c0    },
	{   &asap_device::ld,       &asap_device::ld_0,     &asap_device::ld_c,     &asap_device::ld_c0     },
	{   &asap_device::ldh,      &asap_device::ldh_0,    &asap_device::ldh_c,    &asap_device::ldh_c0    },
	{   &asap_device::lduh,     &asap_device::lduh_0,   &asap_device::lduh_c,   &asap_device::lduh_c0   },
	{   &asap_device::sth,      &asap_device::sth_0,    &asap_device::sth_c,    &asap_device::sth_c0    },
	{   &asap_device::st,       &asap_device::st_0,     &asap_device::st_c,     &asap_device::st_c0     },
	{   &asap_device::ldb,      &asap_device::ldb_0,    &asap_device::ldb_c,    &asap_device::ldb_c0    },
	{   &asap_device::ldub,     &asap_device::ldub_0,   &asap_device::ldub_c,   &asap_device::ldub_c0   },
	{   &asap_device::stb,      &asap_device::stb_0,    &asap_device::stb_c,    &asap_device::stb_c0    },
	{   &asap_device::ashr,     &asap_device::noop,     &asap_device::ashr_c,   &asap_device::ashr_c0   },
	{   &asap_device::lshr,     &asap_device::noop,     &asap_device::lshr_c,   &asap_device::lshr_c0   },
	{   &asap_device::ashl,     &asap_device::noop,     &asap_device::ashl_c,   &asap_device::ashl_c0   },
	{   &asap_device::rotl,     &asap_device::noop,     &asap_device::rotl_c,   &asap_device::rotl_c0   },
	{   &asap_device::getps,    &asap_device::noop,     &asap_device::getps,    &asap_device::noop      },
	{   &asap_device::putps,    &asap_device::putps,    &asap_device::putps,    &asap_device::putps     },
	{   &asap_device::jsr,      &asap_device::jsr_0,    &asap_device::jsr_c,    &asap_device::jsr_c0    },
	{   &asap_device::trapf,    &asap_device::trapf,    &asap_device::trapf,    &asap_device::trapf     }
};

const asap_device::ophandler asap_device::s_conditiontable[16] =
{
	&asap_device::bsp, &asap_device::bmz, &asap_device::bgt, &asap_device::ble,
	&asap_device::bge, &asap_device::blt, &asap_device::bhi, &asap_device::bls,
	&asap_device::bcc, &asap_device::bcs, &asap_device::bpl, &asap_device::bmi,
	&asap_device::bne, &asap_device::beq, &asap_device::bvc, &asap_device::bvs
};



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

// device type definition
const device_type ASAP = &device_creator<asap_device>;

//-------------------------------------------------
//  asap_device - constructor
//-------------------------------------------------

asap_device::asap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, ASAP, "ASAP", tag, owner, clock, "asap", __FILE__),
		m_program_config("program", ENDIANNESS_LITTLE, 32, 32),
		m_pc(0),
		m_pflag(0),
		m_iflag(0),
		m_cflag(0),
		m_vflag(0),
		m_znflag(0),
		m_flagsio(0),
		m_op(0),
		m_ppc(0),
		m_nextpc(0),
		m_irq_state(0),
		m_icount(0),
		m_program(NULL),
		m_direct(NULL)
{
	// initialize the src2val table to contain immediates for low values
	for (int i = 0; i < REGBASE; i++)
		m_src2val[i] = i;

	// build the opcode table
	for (int op = 0; op < 32; op++)
		for (int dst = 0; dst < 32; dst++)
			for (int cond = 0; cond < 2; cond++)
				if (op == 1)
					m_opcode[(op << 6) + (dst << 1) + cond] = s_conditiontable[dst & 15];
				else if (cond && dst == 0)
					m_opcode[(op << 6) + (dst << 1) + cond] = s_opcodetable[op][3];
				else if (cond)
					m_opcode[(op << 6) + (dst << 1) + cond] = s_opcodetable[op][2];
				else if (dst == 0)
					m_opcode[(op << 6) + (dst << 1) + cond] = s_opcodetable[op][1];
				else
					m_opcode[(op << 6) + (dst << 1) + cond] = s_opcodetable[op][0];
}


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void asap_device::device_start()
{
	// get our address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// register our state for the debugger
	std::string tempstr;
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
	state_add(STATE_GENSP,     "GENSP",     m_src2val[REGBASE + 31]).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_flagsio).callimport().callexport().formatstr("%6s").noshow();
	state_add(ASAP_PC,         "PC",        m_pc);
	state_add(ASAP_PS,         "PS",        m_flagsio).callimport().callexport();
	for (int regnum = 0; regnum < 32; regnum++)
		state_add(ASAP_R0 + regnum, strformat(tempstr, "R%d", regnum).c_str(), m_src2val[REGBASE + regnum]);

	// register our state for saving
	save_item(NAME(m_pc));
	save_item(NAME(m_pflag));
	save_item(NAME(m_iflag));
	save_item(NAME(m_cflag));
	save_item(NAME(m_vflag));
	save_item(NAME(m_znflag));
	save_item(NAME(m_op));
	save_item(NAME(m_ppc));
	save_item(NAME(m_nextpc));
	save_item(NAME(m_irq_state));

	// set our instruction counter
	m_icountptr = &m_icount;
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void asap_device::device_reset()
{
	// initialize the state
	m_src2val[REGBASE + 0] = 0;
	m_pc = 0;
	m_iflag = 0;

	m_ppc = 0;
	m_nextpc = ~0;
	m_irq_state = 0;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *asap_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_program_config : NULL;
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void asap_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		case ASAP_PS:
			SET_FLAGS(m_flagsio);
			break;
	}
}


//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void asap_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		case ASAP_PS:
			m_flagsio = GET_FLAGS();
			break;
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void asap_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c",
							m_pflag ? 'P' : '.',
							m_iflag ? 'I' : '.',
							((INT32)m_znflag < 0) ? 'N' : '.',
							(m_znflag == 0) ? 'Z' : '.',
							((m_vflag >> 30) & PS_VFLAG) ? 'V' : '.',
							m_cflag ? 'C' : '.');
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 asap_device::disasm_min_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 asap_device::disasm_max_opcode_bytes() const
{
	return 12;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t asap_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( asap );
	return CPU_DISASSEMBLE_NAME(asap)(this, buffer, pc, oprom, opram, options);
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readop - read an opcode at the given address
//-------------------------------------------------

inline UINT32 asap_device::readop(offs_t pc)
{
	return m_direct->read_dword(pc);
}


//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT8 asap_device::readbyte(offs_t address)
{
	// no alignment issues with bytes
	return m_program->read_byte(address);
}


//-------------------------------------------------
//  readword - read a word at the given address
//-------------------------------------------------

inline UINT16 asap_device::readword(offs_t address)
{
	// aligned reads are easy
	if (!(address & 1))
		return m_program->read_word(address);

	// misaligned reads are tricky
	return m_program->read_dword(address & ~3) >> (address & 3);
}


//-------------------------------------------------
//  readlong - read a long at the given address
//-------------------------------------------------

inline UINT32 asap_device::readlong(offs_t address)
{
	// aligned reads are easy
	if (!(address & 3))
		return m_program->read_dword(address);

	// misaligned reads are tricky
	return m_program->read_dword(address & ~3) >> (address & 3);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void asap_device::writebyte(offs_t address, UINT8 data)
{
	// no alignment issues with bytes
	m_program->write_byte(address, data);
}


//-------------------------------------------------
//  writeword - write a word at the given address
//-------------------------------------------------

inline void asap_device::writeword(offs_t address, UINT16 data)
{
	// aligned writes are easy
	if (!(address & 1))
	{
		m_program->write_word(address, data);
		return;
	}

	// misaligned writes are tricky
	if (!(address & 2))
	{
		m_program->write_byte(address + 1, data);
		m_program->write_byte(address + 2, data >> 8);
	}
	else
		m_program->write_byte(address + 1, data);
}


//-------------------------------------------------
//  writelong - write a long at the given address
//-------------------------------------------------

inline void asap_device::writelong(offs_t address, UINT32 data)
{
	// aligned writes are easy
	if (!(address & 3))
	{
		m_program->write_dword(address, data);
		return;
	}

	// misaligned writes are tricky
	switch (address & 3)
	{
		case 1:
			m_program->write_byte(address, data);
			m_program->write_word(address + 1, data >> 8);
			break;
		case 2:
			m_program->write_word(address, data);
			break;
		case 3:
			m_program->write_byte(address, data);
			break;
	}
}


//-------------------------------------------------
//  generate_exception - generate an exception of
//  the requested type
//-------------------------------------------------

inline void asap_device::generate_exception(int exception)
{
	m_pflag = m_iflag;
	m_iflag = 0;

	m_src2val[REGBASE + 30] = m_pc;
	m_src2val[REGBASE + 31] = (m_nextpc == ~0) ? m_pc + 4 : m_nextpc;

	m_pc = 0x40 * exception;
	m_nextpc = ~0;

	m_icount--;
}


//-------------------------------------------------
//  check_irqs - check for pending IRQs
//-------------------------------------------------

inline void asap_device::check_irqs()
{
	if (m_irq_state && m_iflag)
	{
		generate_exception(EXCEPTION_INTERRUPT);
		standard_irq_callback(ASAP_IRQ0);
	}
}



//**************************************************************************
//  CORE EXECUTION
//**************************************************************************

inline void asap_device::fetch_instruction()
{
	// debugging
	m_ppc = m_pc;

	// instruction fetch
	m_op = readop(m_pc);
	m_pc += 4;
}

inline void asap_device::fetch_instruction_debug()
{
	// debugging
	m_ppc = m_pc;
	debugger_instruction_hook(this, m_pc);

	// instruction fetch
	m_op = readop(m_pc);
	m_pc += 4;
}

inline void asap_device::execute_instruction()
{
	// parse the instruction
	(this->*m_opcode[m_op >> 21])();
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 asap_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 asap_device::execute_max_cycles() const
{
	return 2;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 asap_device::execute_input_lines() const
{
	return 1;
}


void asap_device::execute_set_input(int inputnum, int state)
{
	m_irq_state = (state != CLEAR_LINE);
}


void asap_device::execute_run()
{
	// check for IRQs
	check_irqs();

	// core execution loop
	if ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		do
		{
			// fetch and execute the next instruction
			fetch_instruction();
			execute_instruction();

			// fetch and execute the next instruction
			fetch_instruction();
			execute_instruction();

			// fetch and execute the next instruction
			fetch_instruction();
			execute_instruction();

			// fetch and execute the next instruction
			fetch_instruction();
			execute_instruction();

			m_icount -= 4;

		} while (m_icount > 0);
	}
	else
	{
		do
		{
			// fetch and execute the next instruction
			fetch_instruction_debug();
			execute_instruction();

			// fetch and execute the next instruction
			fetch_instruction_debug();
			execute_instruction();

			// fetch and execute the next instruction
			fetch_instruction_debug();
			execute_instruction();

			// fetch and execute the next instruction
			fetch_instruction_debug();
			execute_instruction();

			m_icount -= 4;

		} while (m_icount > 0);
	}
}



//**************************************************************************
//  OPCODE IMPLEMENTATIONS
//**************************************************************************

void asap_device::noop()
{
}

/**************************** TRAP 0 ******************************/

void asap_device::trap0()
{
	generate_exception(EXCEPTION_TRAP0);
}

/**************************** Bcc ******************************/

void asap_device::bsp()
{
	if ((INT32)m_znflag > 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bmz()
{
	if ((INT32)m_znflag <= 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bgt()
{
	if (m_znflag != 0 && (INT32)(m_znflag ^ m_vflag) >= 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::ble()
{
	if (m_znflag == 0 || (INT32)(m_znflag ^ m_vflag) < 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bge()
{
	if ((INT32)(m_znflag ^ m_vflag) >= 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::blt()
{
	if ((INT32)(m_znflag ^ m_vflag) < 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bhi()
{
	if (m_znflag != 0 && m_cflag)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bls()
{
	if (m_znflag == 0 || !m_cflag)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bcc()
{
	if (!m_cflag)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bcs()
{
	if (m_cflag)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bpl()
{
	if ((INT32)m_znflag >= 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bmi()
{
	if ((INT32)m_znflag < 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bne()
{
	if (m_znflag != 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::beq()
{
	if (m_znflag == 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bvc()
{
	if ((INT32)m_vflag >= 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

void asap_device::bvs()
{
	if ((INT32)m_vflag < 0)
	{
		m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

		fetch_instruction();
		m_pc = m_nextpc;
		m_nextpc = ~0;

		execute_instruction();
		m_icount--;
	}
}

/**************************** BSR ******************************/

void asap_device::bsr()
{
	DSTVAL = m_pc + 4;
	m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

	fetch_instruction();
	m_pc = m_nextpc;
	m_nextpc = ~0;

	execute_instruction();
	m_icount--;
}

void asap_device::bsr_0()
{
	m_nextpc = m_ppc + ((INT32)(m_op << 10) >> 8);

	fetch_instruction();
	m_pc = m_nextpc;
	m_nextpc = ~0;

	execute_instruction();
	m_icount--;
}

/**************************** LEA ******************************/

void asap_device::lea()
{
	DSTVAL = SRC1VAL + (SRC2VAL << 2);
}

void asap_device::lea_c()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + (src2 << 2);

	SET_ZNCV_ADD(dst, src1, src2);
	if (src1 & 0xc0000000)
		SET_CFLAG(1);
	if (((src1 ^ (src1 >> 1)) & 0x20000000) || (src1 ^ (src1 >> 2)) & 0x20000000)
		SET_VFLAG(1);
	DSTVAL = dst;
}

void asap_device::lea_c0()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + (src2 << 2);

	SET_ZNCV_ADD(dst, src1, src2);
	if (src1 & 0xc0000000)
		SET_CFLAG(1);
	if (((src1 ^ (src1 >> 1)) & 0x20000000) || (src1 ^ (src1 >> 2)) & 0x20000000)
		SET_VFLAG(1);
}

/**************************** LEAH ******************************/

void asap_device::leah()
{
	DSTVAL = SRC1VAL + (SRC2VAL << 1);
}

void asap_device::leah_c()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + (src2 << 1);

	SET_ZNCV_ADD(dst, src1, src2);
	if (src1 & 0x80000000)
		SET_CFLAG(1);
	if ((src1 ^ (src1 >> 1)) & 0x40000000)
		SET_VFLAG(1);
	DSTVAL = dst;
}

void asap_device::leah_c0()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + (src2 << 1);

	SET_ZNCV_ADD(dst, src1, src2);
	if (src1 & 0x80000000)
		SET_CFLAG(1);
	if ((src1 ^ (src1 >> 1)) & 0x40000000)
		SET_VFLAG(1);
}

/**************************** SUBR ******************************/

void asap_device::subr()
{
	DSTVAL = SRC2VAL - SRC1VAL;
}

void asap_device::subr_c()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src2 - src1;

	SET_ZNCV_SUB(dst, src2, src1);
	DSTVAL = dst;
}

void asap_device::subr_c0()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src2 - src1;

	SET_ZNCV_SUB(dst, src2, src1);
}

/**************************** XOR ******************************/

void asap_device::xor_()
{
	DSTVAL = SRC1VAL ^ SRC2VAL;
}

void asap_device::xor_c()
{
	UINT32 dst = SRC1VAL ^ SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::xor_c0()
{
	UINT32 dst = SRC1VAL ^ SRC2VAL;
	SET_ZN(dst);
}

/**************************** XOR ******************************/

void asap_device::xorn()
{
	DSTVAL = SRC1VAL ^ ~SRC2VAL;
}

void asap_device::xorn_c()
{
	UINT32 dst = SRC1VAL ^ ~SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::xorn_c0()
{
	UINT32 dst = SRC1VAL ^ ~SRC2VAL;
	SET_ZN(dst);
}

/**************************** ADD ******************************/

void asap_device::add()
{
	DSTVAL = SRC1VAL + SRC2VAL;
}

void asap_device::add_c()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + src2;

	SET_ZNCV_ADD(dst, src1, src2);
	DSTVAL = dst;
}

void asap_device::add_c0()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + src2;

	SET_ZNCV_ADD(dst, src1, src2);
}

/**************************** ADD ******************************/

void asap_device::sub()
{
	DSTVAL = SRC1VAL - SRC2VAL;
}

void asap_device::sub_c()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 - src2;

	SET_ZNCV_SUB(dst, src1, src2);
	DSTVAL = dst;
}

void asap_device::sub_c0()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 - src2;

	SET_ZNCV_SUB(dst, src1, src2);
}

/**************************** ADDC ******************************/

void asap_device::addc()
{
	DSTVAL = SRC1VAL + SRC2VAL + m_cflag;
}

void asap_device::addc_c()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + src2 + m_cflag;

	SET_ZNCV_ADD(dst, src1, src2);
	DSTVAL = dst;
}

void asap_device::addc_c0()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + src2 + m_cflag;

	SET_ZNCV_ADD(dst, src1, src2);
}

/**************************** SUBC ******************************/

void asap_device::subc()
{
	DSTVAL = SRC1VAL - SRC2VAL - 1 + m_cflag;
}

void asap_device::subc_c()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 - src2 - 1 + m_cflag;

	SET_ZNCV_SUB(dst, src1, src2);
	DSTVAL = dst;
}

void asap_device::subc_c0()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 - src2 - 1 + m_cflag;

	SET_ZNCV_SUB(dst, src1, src2);
}

/**************************** AND ******************************/

void asap_device::and_()
{
	DSTVAL = SRC1VAL & SRC2VAL;
}

void asap_device::and_c()
{
	UINT32 dst = SRC1VAL & SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::and_c0()
{
	UINT32 dst = SRC1VAL & SRC2VAL;
	SET_ZN(dst);
}

/**************************** ANDN ******************************/

void asap_device::andn()
{
	DSTVAL = SRC1VAL & ~SRC2VAL;
}

void asap_device::andn_c()
{
	UINT32 dst = SRC1VAL & ~SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::andn_c0()
{
	UINT32 dst = SRC1VAL & ~SRC2VAL;
	SET_ZN(dst);
}

/**************************** OR ******************************/

void asap_device::or_()
{
	DSTVAL = SRC1VAL | SRC2VAL;
}

void asap_device::or_c()
{
	UINT32 dst = SRC1VAL | SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::or_c0()
{
	UINT32 dst = SRC1VAL | SRC2VAL;
	SET_ZN(dst);
}

/**************************** ORN ******************************/

void asap_device::orn()
{
	DSTVAL = SRC1VAL | ~SRC2VAL;
}

void asap_device::orn_c()
{
	UINT32 dst = SRC1VAL | ~SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::orn_c0()
{
	UINT32 dst = SRC1VAL | ~SRC2VAL;
	SET_ZN(dst);
}

/**************************** LD ******************************/

void asap_device::ld()
{
	DSTVAL = readlong(SRC1VAL + (SRC2VAL << 2));
}

void asap_device::ld_0()
{
	readlong(SRC1VAL + (SRC2VAL << 2));
}

void asap_device::ld_c()
{
	UINT32 dst = readlong(SRC1VAL + (SRC2VAL << 2));
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::ld_c0()
{
	UINT32 dst = readlong(SRC1VAL + (SRC2VAL << 2));
	SET_ZN(dst);
}

/**************************** LDH ******************************/

void asap_device::ldh()
{
	DSTVAL = (INT16)readword(SRC1VAL + (SRC2VAL << 1));
}

void asap_device::ldh_0()
{
	readword(SRC1VAL + (SRC2VAL << 1));
}

void asap_device::ldh_c()
{
	UINT32 dst = (INT16)readword(SRC1VAL + (SRC2VAL << 1));
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::ldh_c0()
{
	UINT32 dst = (INT16)readword(SRC1VAL + (SRC2VAL << 1));
	SET_ZN(dst);
}

/**************************** LDUH ******************************/

void asap_device::lduh()
{
	DSTVAL = readword(SRC1VAL + (SRC2VAL << 1));
}

void asap_device::lduh_0()
{
	readword(SRC1VAL + (SRC2VAL << 1));
}

void asap_device::lduh_c()
{
	UINT32 dst = readword(SRC1VAL + (SRC2VAL << 1));
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::lduh_c0()
{
	UINT32 dst = readword(SRC1VAL + (SRC2VAL << 1));
	SET_ZN(dst);
}

/**************************** STH ******************************/

void asap_device::sth()
{
	writeword(SRC1VAL + (SRC2VAL << 1), DSTVAL);
}

void asap_device::sth_0()
{
	writeword(SRC1VAL + (SRC2VAL << 1), 0);
}

void asap_device::sth_c()
{
	UINT32 dst = (UINT16)DSTVAL;
	SET_ZN(dst);
	writeword(SRC1VAL + (SRC2VAL << 1), dst);
}

void asap_device::sth_c0()
{
	SET_ZN(0);
	writeword(SRC1VAL + (SRC2VAL << 1), 0);
}

/**************************** ST ******************************/

void asap_device::st()
{
	writelong(SRC1VAL + (SRC2VAL << 2), DSTVAL);
}

void asap_device::st_0()
{
	writelong(SRC1VAL + (SRC2VAL << 2), 0);
}

void asap_device::st_c()
{
	UINT32 dst = DSTVAL;
	SET_ZN(dst);
	writelong(SRC1VAL + (SRC2VAL << 2), dst);
}

void asap_device::st_c0()
{
	SET_ZN(0);
	writelong(SRC1VAL + (SRC2VAL << 2), 0);
}

/**************************** LDB ******************************/

void asap_device::ldb()
{
	DSTVAL = (INT8)readbyte(SRC1VAL + SRC2VAL);
}

void asap_device::ldb_0()
{
	readbyte(SRC1VAL + SRC2VAL);
}

void asap_device::ldb_c()
{
	UINT32 dst = (INT8)readbyte(SRC1VAL + SRC2VAL);
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::ldb_c0()
{
	UINT32 dst = (INT8)readbyte(SRC1VAL + SRC2VAL);
	SET_ZN(dst);
}

/**************************** LDUB ******************************/

void asap_device::ldub()
{
	DSTVAL = readbyte(SRC1VAL + SRC2VAL);
}

void asap_device::ldub_0()
{
	readbyte(SRC1VAL + SRC2VAL);
}

void asap_device::ldub_c()
{
	UINT32 dst = readbyte(SRC1VAL + SRC2VAL);
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::ldub_c0()
{
	UINT32 dst = readbyte(SRC1VAL + SRC2VAL);
	SET_ZN(dst);
}

/**************************** STB ******************************/

void asap_device::stb()
{
	writebyte(SRC1VAL + SRC2VAL, DSTVAL);
}

void asap_device::stb_0()
{
	writebyte(SRC1VAL + SRC2VAL, 0);
}

void asap_device::stb_c()
{
	UINT32 dst = (UINT8)DSTVAL;
	SET_ZN(dst);
	writebyte(SRC1VAL + SRC2VAL, dst);
}

void asap_device::stb_c0()
{
	SET_ZN(0);
	writebyte(SRC1VAL + SRC2VAL, 0);
}

/**************************** ASHR ******************************/

void asap_device::ashr()
{
	UINT32 src2 = SRC2VAL;
	DSTVAL = (src2 < 32) ? ((INT32)SRC1VAL >> src2) : ((INT32)SRC1VAL >> 31);
}

void asap_device::ashr_c()
{
	UINT32 src2 = SRC2VAL;
	m_cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = (INT32)src1 >> src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			m_cflag = src1 >> 31;
		}
		DSTVAL = dst;
	}
	else
	{
		UINT32 dst = (INT32)SRC1VAL >> 31;
		SET_ZN(dst);
		DSTVAL = dst;
	}
}

void asap_device::ashr_c0()
{
	UINT32 src2 = SRC2VAL;
	m_cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = (INT32)src1 >> src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			m_cflag = src1 >> 31;
		}
	}
	else
	{
		UINT32 dst = (INT32)SRC1VAL >> 31;
		SET_ZN(dst);
	}
}

/**************************** LSHR ******************************/

void asap_device::lshr()
{
	UINT32 src2 = SRC2VAL;
	DSTVAL = (src2 < 32) ? (SRC1VAL >> src2) : (SRC1VAL >> 31);
}

void asap_device::lshr_c()
{
	UINT32 src2 = SRC2VAL;
	m_cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = src1 >> src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			m_cflag = src1 >> 31;
		}
		DSTVAL = dst;
	}
	else
	{
		UINT32 dst = SRC1VAL >> 31;
		SET_ZN(dst);
		DSTVAL = dst;
	}
}

void asap_device::lshr_c0()
{
	UINT32 src2 = SRC2VAL;
	m_cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = src1 >> src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			m_cflag = src1 >> 31;
		}
	}
	else
	{
		SET_ZN(0);
		DSTVAL = 0;
	}
}

/**************************** ASHL ******************************/

void asap_device::ashl()
{
	UINT32 src2 = SRC2VAL;
	DSTVAL = (src2 < 32) ? (SRC1VAL << src2) : 0;
}

void asap_device::ashl_c()
{
	UINT32 src2 = SRC2VAL;
	m_cflag = m_vflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = src1 << src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = (INT32)src1 >> (32 - src2);
			m_cflag = src1 & PS_CFLAG;
			m_vflag = (src1 != ((INT32)dst >> 31)) << 31;
		}
		DSTVAL = dst;
	}
	else
	{
		SET_ZN(0);
		DSTVAL = 0;
	}
}

void asap_device::ashl_c0()
{
	UINT32 src2 = SRC2VAL;
	m_cflag = m_vflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = src1 << src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = (INT32)src1 >> (32 - src2);
			m_cflag = src1 & PS_CFLAG;
			m_vflag = (src1 != ((INT32)dst >> 31)) << 31;
		}
	}
	else
		SET_ZN(0);
}

/**************************** ROTL ******************************/

void asap_device::rotl()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL & 31;
	DSTVAL = (src1 << src2) | (src1 >> (32 - src2));
}

void asap_device::rotl_c()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL & 31;
	UINT32 dst = (src1 << src2) | (src1 >> (32 - src2));
	SET_ZN(dst);
	DSTVAL = dst;
}

void asap_device::rotl_c0()
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL & 31;
	UINT32 dst = (src1 << src2) | (src1 >> (32 - src2));
	SET_ZN(dst);
}

/**************************** GETPS ******************************/

void asap_device::getps()
{
	DSTVAL = GET_FLAGS();
}

/**************************** PUTPS ******************************/

void asap_device::putps()
{
	UINT32 src2 = SRC2VAL & 0x3f;
	SET_FLAGS(src2);
	check_irqs();
}

/**************************** JSR ******************************/

void asap_device::jsr()
{
	DSTVAL = m_pc + 4;
	m_nextpc = SRC1VAL + (SRC2VAL << 2);

	fetch_instruction();
	m_pc = m_nextpc;
	m_nextpc = ~0;

	execute_instruction();
	m_icount--;
}

void asap_device::jsr_0()
{
	m_nextpc = SRC1VAL + (SRC2VAL << 2);

	fetch_instruction();
	m_pc = m_nextpc;
	m_nextpc = ~0;

	execute_instruction();
	m_icount--;
}

void asap_device::jsr_c()
{
	DSTVAL = m_pc + 4;
	m_nextpc = SRC1VAL + (SRC2VAL << 2);
	m_iflag = m_pflag;

	fetch_instruction();
	m_pc = m_nextpc;
	m_nextpc = ~0;

	execute_instruction();
	m_icount--;
	check_irqs();
}

void asap_device::jsr_c0()
{
	m_nextpc = SRC1VAL + (SRC2VAL << 2);
	m_iflag = m_pflag;

	fetch_instruction();
	m_pc = m_nextpc;
	m_nextpc = ~0;

	execute_instruction();
	m_icount--;
	check_irqs();
}

/**************************** TRAP F ******************************/

void asap_device::trapf()
{
	generate_exception(EXCEPTION_TRAPF);
}
