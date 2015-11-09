// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    r3000.c
    Core implementation for the portable MIPS R3000 emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "r3000.h"


#define ENABLE_OVERFLOWS    0


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define COP0_Index          0
#define COP0_Random         1
#define COP0_EntryLo        2
#define COP0_Context        4
#define COP0_BadVAddr       8
#define COP0_Status         12
#define COP0_Cause          13
#define COP0_EPC            14
#define COP0_PRId           15

#define SR_IEc              0x00000001
#define SR_KUc              0x00000002
#define SR_IEp              0x00000004
#define SR_KUp              0x00000008
#define SR_IEo              0x00000010
#define SR_KUo              0x00000020
#define SR_IMSW0            0x00000100
#define SR_IMSW1            0x00000200
#define SR_IMEX0            0x00000400
#define SR_IMEX1            0x00000800
#define SR_IMEX2            0x00001000
#define SR_IMEX3            0x00002000
#define SR_IMEX4            0x00004000
#define SR_IMEX5            0x00008000
#define SR_IsC              0x00010000
#define SR_SwC              0x00020000
#define SR_PZ               0x00040000
#define SR_CM               0x00080000
#define SR_PE               0x00100000
#define SR_TS               0x00200000
#define SR_BEV              0x00400000
#define SR_RE               0x02000000
#define SR_COP0             0x10000000
#define SR_COP1             0x20000000
#define SR_COP2             0x40000000
#define SR_COP3             0x80000000

#define EXCEPTION_INTERRUPT 0
#define EXCEPTION_TLBMOD    1
#define EXCEPTION_TLBLOAD   2
#define EXCEPTION_TLBSTORE  3
#define EXCEPTION_ADDRLOAD  4
#define EXCEPTION_ADDRSTORE 5
#define EXCEPTION_BUSINST   6
#define EXCEPTION_BUSDATA   7
#define EXCEPTION_SYSCALL   8
#define EXCEPTION_BREAK     9
#define EXCEPTION_INVALIDOP 10
#define EXCEPTION_BADCOP    11
#define EXCEPTION_OVERFLOW  12
#define EXCEPTION_TRAP      13


/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define RSREG           ((m_op >> 21) & 31)
#define RTREG           ((m_op >> 16) & 31)
#define RDREG           ((m_op >> 11) & 31)
#define SHIFT           ((m_op >> 6) & 31)

#define RSVAL           m_r[RSREG]
#define RTVAL           m_r[RTREG]
#define RDVAL           m_r[RDREG]

#define SIMMVAL         ((INT16)m_op)
#define UIMMVAL         ((UINT16)m_op)
#define LIMMVAL         (m_op & 0x03ffffff)

#define ADDPC(x)        do { m_nextpc = m_pc + ((x) << 2); } while (0)
#define ADDPCL(x,l)     do { m_nextpc = m_pc + ((x) << 2); m_r[l] = m_pc + 4; } while (0)
#define ABSPC(x)        do { m_nextpc = (m_pc & 0xf0000000) | ((x) << 2); } while (0)
#define ABSPCL(x,l)     do { m_nextpc = (m_pc & 0xf0000000) | ((x) << 2); m_r[l] = m_pc + 4; } while (0)
#define SETPC(x)        do { m_nextpc = (x); } while (0)
#define SETPCL(x,l)     do { m_nextpc = (x); m_r[l] = m_pc + 4; } while (0)

#define RBYTE(x)        (this->*m_cur->m_read_byte)(x)
#define RWORD(x)        (this->*m_cur->m_read_word)(x)
#define RLONG(x)        (this->*m_cur->m_read_dword)(x)

#define WBYTE(x,v)      (this->*m_cur->m_write_byte)(x, v)
#define WWORD(x,v)      (this->*m_cur->m_write_word)(x, v)
#define WLONG(x,v)      (this->*m_cur->m_write_dword)(x, v)

#define SR              m_cpr[0][COP0_Status]
#define CAUSE           m_cpr[0][COP0_Cause]


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

const device_type R3041 = &device_creator<r3041_device>;
const device_type R3051 = &device_creator<r3051_device>;
const device_type R3052 = &device_creator<r3052_device>;
const device_type R3071 = &device_creator<r3071_device>;
const device_type R3081 = &device_creator<r3081_device>;


//-------------------------------------------------
//  r3000_device - constructor
//-------------------------------------------------

r3000_device::r3000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, chip_type chiptype, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_program_config_be("program", ENDIANNESS_BIG, 32, 29),
		m_program_config_le("program", ENDIANNESS_LITTLE, 32, 29),
		m_program(NULL),
		m_direct(NULL),
		m_chip_type(chiptype),
		m_hasfpu(false),
		m_endianness(ENDIANNESS_BIG),
		m_pc(0),
		m_nextpc(0),
		m_hi(0),
		m_lo(0),
		m_ppc(0),
		m_op(0),
		m_icount(0),
		m_interrupt_cycles(0),
		m_in_brcond0(*this),
		m_in_brcond1(*this),
		m_in_brcond2(*this),
		m_in_brcond3(*this)
{
	// set our instruction counter
	m_icountptr = &m_icount;

	// clear some additional state
	memset(m_r, 0, sizeof(m_r));
	memset(m_cpr, 0, sizeof(m_cpr));
	memset(m_ccr, 0, sizeof(m_ccr));
}


//-------------------------------------------------
//  ~r3000_device - destructor
//-------------------------------------------------

r3000_device::~r3000_device()
{
}


//-------------------------------------------------
//  r3041_device - constructor
//-------------------------------------------------

r3041_device::r3041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: r3000_device(mconfig, R3041, "R3041", tag, owner, clock, CHIP_TYPE_R3041, "r3041", __FILE__) { }


//-------------------------------------------------
//  r3051_device - constructor
//-------------------------------------------------

r3051_device::r3051_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: r3000_device(mconfig, R3051, "R3051", tag, owner, clock, CHIP_TYPE_R3051, "r3051", __FILE__) { }


//-------------------------------------------------
//  r3052_device - constructor
//-------------------------------------------------

r3052_device::r3052_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: r3000_device(mconfig, R3052, "R3052", tag, owner, clock, CHIP_TYPE_R3052, "r3052", __FILE__) { }


//-------------------------------------------------
//  r3071_device - constructor
//-------------------------------------------------

r3071_device::r3071_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: r3000_device(mconfig, R3071, "R3071", tag, owner, clock, CHIP_TYPE_R3071, "r3071", __FILE__) { }


//-------------------------------------------------
//  r3081_device - constructor
//-------------------------------------------------

r3081_device::r3081_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: r3000_device(mconfig, R3081, "R3081", tag, owner, clock, CHIP_TYPE_R3081, "r3081", __FILE__) { }


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void r3000_device::device_start()
{
	// get our address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// determine the cache sizes
	switch (m_chip_type)
	{
		case CHIP_TYPE_R3041:
		{
			m_icache_size = 2048;
			m_dcache_size = 512;
			break;
		}
		case CHIP_TYPE_R3051:
		{
			m_icache_size = 4096;
			m_dcache_size = 2048;
			break;
		}
		case CHIP_TYPE_R3052:
		{
			m_icache_size = 8192;
			m_dcache_size = 2048;
			break;
		}

		// TODO: R3071 and R3081 have configurable cache sizes
		case CHIP_TYPE_R3071:
		{
			m_icache_size = 16384;  // or 8kB
			m_dcache_size = 4096;   // or 8kB
			break;
		}
		case CHIP_TYPE_R3081:
		{
			m_icache_size = 16384;  // or 8kB
			m_dcache_size = 4096;   // or 8kB
			m_hasfpu = true;
			break;
		}
	}

	// allocate cache memory
	m_icache.resize(m_icache_size/4);
	m_dcache.resize(m_dcache_size/4);

	m_cache = &m_dcache[0];
	m_cache_size = m_dcache_size;

	// set up memory handlers
	m_memory_hand.m_read_byte = &r3000_device::readmem;
	m_memory_hand.m_read_word = &r3000_device::readmem_word;
	m_memory_hand.m_read_dword = &r3000_device::readmem_dword;
	m_memory_hand.m_write_byte = &r3000_device::writemem;
	m_memory_hand.m_write_word = &r3000_device::writemem_word;
	m_memory_hand.m_write_dword = &r3000_device::writemem_dword;

	if (m_endianness == ENDIANNESS_BIG)
	{
		m_lwl = &r3000_device::lwl_be;
		m_lwr = &r3000_device::lwr_be;
		m_swl = &r3000_device::swl_be;
		m_swr = &r3000_device::swr_be;

		m_cache_hand.m_read_byte = &r3000_device::readcache_be;
		m_cache_hand.m_read_word = &r3000_device::readcache_be_word;
		m_cache_hand.m_read_dword = &r3000_device::readcache_be_dword;
		m_cache_hand.m_write_byte = &r3000_device::writecache_be;
		m_cache_hand.m_write_word = &r3000_device::writecache_be_word;
		m_cache_hand.m_write_dword = &r3000_device::writecache_be_dword;
	}
	else
	{
		m_lwl = &r3000_device::lwl_le;
		m_lwr = &r3000_device::lwr_le;
		m_swl = &r3000_device::swl_le;
		m_swr = &r3000_device::swr_le;

		m_cache_hand.m_read_byte = &r3000_device::readcache_le;
		m_cache_hand.m_read_word = &r3000_device::readcache_le_word;
		m_cache_hand.m_read_dword = &r3000_device::readcache_le_dword;
		m_cache_hand.m_write_byte = &r3000_device::writecache_le;
		m_cache_hand.m_write_word = &r3000_device::writecache_le_word;
		m_cache_hand.m_write_dword = &r3000_device::writecache_le_dword;
	}

	// resolve conditional branch input handlers
	m_in_brcond0.resolve_safe(0);
	m_in_brcond1.resolve_safe(0);
	m_in_brcond2.resolve_safe(0);
	m_in_brcond3.resolve_safe(0);

	// register our state for the debugger
	state_add(STATE_GENPC,      "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE,  "GENPCBASE", m_ppc).noshow();
	state_add(STATE_GENSP,      "GENSP",     m_r[31]).noshow();
	state_add(STATE_GENFLAGS,   "GENFLAGS",  SR).callimport().callexport().formatstr("%6s").noshow();
	state_add(R3000_PC,         "PC",       m_pc);
	state_add(R3000_SR,         "SR",       SR);
	state_add(R3000_R0,         "R0",       m_r[0]);
	state_add(R3000_R1,         "R1",       m_r[1]);
	state_add(R3000_R2,         "R2",       m_r[2]);
	state_add(R3000_R3,         "R3",       m_r[3]);
	state_add(R3000_R4,         "R4",       m_r[4]);
	state_add(R3000_R5,         "R5",       m_r[5]);
	state_add(R3000_R6,         "R6",       m_r[6]);
	state_add(R3000_R7,         "R7",       m_r[7]);
	state_add(R3000_R8,         "R8",       m_r[8]);
	state_add(R3000_R9,         "R9",       m_r[9]);
	state_add(R3000_R10,        "R10",      m_r[10]);
	state_add(R3000_R11,        "R11",      m_r[11]);
	state_add(R3000_R12,        "R12",      m_r[12]);
	state_add(R3000_R13,        "R13",      m_r[13]);
	state_add(R3000_R14,        "R14",      m_r[14]);
	state_add(R3000_R15,        "R15",      m_r[15]);
	state_add(R3000_R16,        "R16",      m_r[16]);
	state_add(R3000_R17,        "R17",      m_r[17]);
	state_add(R3000_R18,        "R18",      m_r[18]);
	state_add(R3000_R19,        "R19",      m_r[19]);
	state_add(R3000_R20,        "R20",      m_r[20]);
	state_add(R3000_R21,        "R21",      m_r[21]);
	state_add(R3000_R22,        "R22",      m_r[22]);
	state_add(R3000_R23,        "R23",      m_r[23]);
	state_add(R3000_R24,        "R24",      m_r[24]);
	state_add(R3000_R25,        "R25",      m_r[25]);
	state_add(R3000_R26,        "R26",      m_r[26]);
	state_add(R3000_R27,        "R27",      m_r[27]);
	state_add(R3000_R28,        "R28",      m_r[28]);
	state_add(R3000_R29,        "R29",      m_r[29]);
	state_add(R3000_R30,        "R30",      m_r[30]);
	state_add(R3000_R31,        "R31",      m_r[31]);

	// register our state for saving
	save_item(NAME(m_pc));
	save_item(NAME(m_nextpc));
	save_item(NAME(m_hi));
	save_item(NAME(m_lo));
	save_item(NAME(m_r));
	save_item(NAME(m_cpr));
	save_item(NAME(m_ccr));
	save_item(NAME(m_ppc));
	save_item(NAME(m_op));
	save_item(NAME(m_interrupt_cycles));
	save_item(NAME(m_icache));
	save_item(NAME(m_dcache));
}


//-------------------------------------------------
//  device_post_load -
//-------------------------------------------------
void r3000_device::device_post_load()
{
	if (m_cpr[0][COP0_Status] & SR_IsC)
		m_cur = &m_cache_hand;
	else
		m_cur = &m_memory_hand;
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void r3000_device::device_reset()
{
	// initialize the rest of the config
	m_cur = &m_memory_hand;

	// initialize the state
	m_pc = 0xbfc00000;
	m_nextpc = ~0;
	m_cpr[0][COP0_PRId] = 0x0200;
	m_cpr[0][COP0_Status] = 0x0000;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *r3000_device::memory_space_config(address_spacenum spacenum) const
{
	if (spacenum == AS_PROGRAM)
		return (m_endianness == ENDIANNESS_BIG) ? &m_program_config_be : &m_program_config_le;
	else
		return NULL;
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void r3000_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			break;

		default:
			fatalerror("r3000_device::state_import called for unexpected value\n");
	}
}


//-------------------------------------------------
//  state_export - export state out of the device
//-------------------------------------------------

void r3000_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			break;

		default:
			fatalerror("r3000_device::state_export called for unexpected value\n");
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void r3000_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 r3000_device::disasm_min_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 r3000_device::disasm_max_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t r3000_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( r3000le );
	extern CPU_DISASSEMBLE( r3000be );

	if (m_endianness == ENDIANNESS_BIG)
		return CPU_DISASSEMBLE_NAME(r3000be)(this, buffer, pc, oprom, opram, options);
	else
		return CPU_DISASSEMBLE_NAME(r3000le)(this, buffer, pc, oprom, opram, options);
}


/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

inline UINT32 r3000_device::readop(offs_t pc)
{
	return m_direct->read_dword(pc);
}

UINT8 r3000_device::readmem(offs_t offset)
{
	return m_program->read_byte(offset);
}

UINT16 r3000_device::readmem_word(offs_t offset)
{
	return m_program->read_word(offset);
}

UINT32 r3000_device::readmem_dword(offs_t offset)
{
	return m_program->read_dword(offset);
}

void r3000_device::writemem(offs_t offset, UINT8 data)
{
	m_program->write_byte(offset, data);
}

void r3000_device::writemem_word(offs_t offset, UINT16 data)
{
	m_program->write_word(offset, data);
}

void r3000_device::writemem_dword(offs_t offset, UINT32 data)
{
	m_program->write_dword(offset, data);
}


/***************************************************************************
    BIG ENDIAN CACHE I/O
***************************************************************************/

UINT8 r3000_device::readcache_be(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < m_cache_size) ? m_cache[BYTE4_XOR_BE(offset)] : 0xff;
}

UINT16 r3000_device::readcache_be_word(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < m_cache_size) ? *(UINT16 *)&m_cache[WORD_XOR_BE(offset)] : 0xffff;
}

UINT32 r3000_device::readcache_be_dword(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < m_cache_size) ? *(UINT32 *)&m_cache[offset] : 0xffffffff;
}

void r3000_device::writecache_be(offs_t offset, UINT8 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < m_cache_size) m_cache[BYTE4_XOR_BE(offset)] = data;
}

void r3000_device::writecache_be_word(offs_t offset, UINT16 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < m_cache_size) *(UINT16 *)&m_cache[WORD_XOR_BE(offset)] = data;
}

void r3000_device::writecache_be_dword(offs_t offset, UINT32 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < m_cache_size) *(UINT32 *)&m_cache[offset] = data;
}

UINT8 r3000_device::readcache_le(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < m_cache_size) ? m_cache[BYTE4_XOR_LE(offset)] : 0xff;
}


/***************************************************************************
    LITTLE ENDIAN CACHE I/O
***************************************************************************/

UINT16 r3000_device::readcache_le_word(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < m_cache_size) ? *(UINT16 *)&m_cache[WORD_XOR_LE(offset)] : 0xffff;
}

UINT32 r3000_device::readcache_le_dword(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < m_cache_size) ? *(UINT32 *)&m_cache[offset] : 0xffffffff;
}

void r3000_device::writecache_le(offs_t offset, UINT8 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < m_cache_size) m_cache[BYTE4_XOR_LE(offset)] = data;
}

void r3000_device::writecache_le_word(offs_t offset, UINT16 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < m_cache_size) *(UINT16 *)&m_cache[WORD_XOR_LE(offset)] = data;
}

void r3000_device::writecache_le_dword(offs_t offset, UINT32 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < m_cache_size) *(UINT32 *)&m_cache[offset] = data;
}


/***************************************************************************
    EXECEPTION HANDLING
***************************************************************************/

inline void r3000_device::generate_exception(int exception)
{
	// set the exception PC
	m_cpr[0][COP0_EPC] = m_pc;

	// put the cause in the low 8 bits and clear the branch delay flag
	CAUSE = (CAUSE & ~0x800000ff) | (exception << 2);

	// if we were in a branch delay slot, adjust
	if (m_nextpc != ~0)
	{
		m_nextpc = ~0;
		m_cpr[0][COP0_EPC] -= 4;
		CAUSE |= 0x80000000;
	}

	// shift the exception bits
	SR = (SR & 0xffffffc0) | ((SR << 2) & 0x3c);

	// based on the BEV bit, we either go to ROM or RAM
	m_pc = (SR & SR_BEV) ? 0xbfc00000 : 0x80000000;

	// most exceptions go to offset 0x180, except for TLB stuff
	if (exception >= EXCEPTION_TLBMOD && exception <= EXCEPTION_TLBSTORE)
		m_pc += 0x80;
	else
		m_pc += 0x180;
}


inline void r3000_device::invalid_instruction()
{
	generate_exception(EXCEPTION_INVALIDOP);
}


/***************************************************************************
    IRQ HANDLING
***************************************************************************/

void r3000_device::check_irqs()
{
	if ((CAUSE & SR & 0xff00) && (SR & SR_IEc))
		generate_exception(EXCEPTION_INTERRUPT);
}


void r3000_device::set_irq_line(int irqline, int state)
{
	if (state != CLEAR_LINE)
		CAUSE |= 0x400 << irqline;
	else
		CAUSE &= ~(0x400 << irqline);

	check_irqs();
}


/***************************************************************************
    COP0 (SYSTEM) EXECUTION HANDLING
***************************************************************************/

inline UINT32 r3000_device::get_cop0_reg(int idx)
{
	return m_cpr[0][idx];
}

inline void r3000_device::set_cop0_reg(int idx, UINT32 val)
{
	if (idx == COP0_Cause)
	{
		CAUSE = (CAUSE & 0xfc00) | (val & ~0xfc00);

		// update interrupts -- software ints can occur this way
		check_irqs();
	}
	else if (idx == COP0_Status)
	{
		UINT32 oldsr = m_cpr[0][idx];
		UINT32 diff = oldsr ^ val;

		// handle cache isolation
		if (diff & SR_IsC)
		{
			if (val & SR_IsC)
				m_cur = &m_cache_hand;
			else
				m_cur = &m_memory_hand;
		}

		// handle cache switching
		if (diff & SR_SwC)
		{
			if (val & SR_SwC)
				m_cache = &m_icache[0], m_cache_size = m_icache_size;
			else
				m_cache = &m_dcache[0], m_cache_size = m_dcache_size;
		}
		m_cpr[0][idx] = val;

		// update interrupts
		check_irqs();
	}
	else
		m_cpr[0][idx] = val;
}

inline UINT32 r3000_device::get_cop0_creg(int idx)
{
	return m_ccr[0][idx];
}

inline void r3000_device::set_cop0_creg(int idx, UINT32 val)
{
	m_ccr[0][idx] = val;
}

inline void r3000_device::handle_cop0()
{
	if (!(SR & SR_COP0) && (SR & SR_KUc))
		generate_exception(EXCEPTION_BADCOP);

	switch (RSREG)
	{
		case 0x00:  /* MFCz */      if (RTREG) RTVAL = get_cop0_reg(RDREG);       break;
		case 0x02:  /* CFCz */      if (RTREG) RTVAL = get_cop0_creg(RDREG);      break;
		case 0x04:  /* MTCz */      set_cop0_reg(RDREG, RTVAL);                   break;
		case 0x06:  /* CTCz */      set_cop0_creg(RDREG, RTVAL);                  break;
		case 0x08:  /* BC */
			switch (RTREG)
			{
				case 0x00:  /* BCzF */  if (!m_in_brcond0()) ADDPC(SIMMVAL);    break;
				case 0x01:  /* BCzT */  if (m_in_brcond0()) ADDPC(SIMMVAL);     break;
				case 0x02:  /* BCzFL */ invalid_instruction();                         break;
				case 0x03:  /* BCzTL */ invalid_instruction();                         break;
				default:    invalid_instruction();                                     break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */
			switch (m_op & 0x01ffffff)
			{
				case 0x01:  /* TLBR */                                                          break;
				case 0x02:  /* TLBWI */                                                         break;
				case 0x06:  /* TLBWR */                                                         break;
				case 0x08:  /* TLBP */                                                          break;
				case 0x10:  /* RFE */   SR = (SR & 0xfffffff0) | ((SR >> 2) & 0x0f); break;
				case 0x18:  /* ERET */  invalid_instruction();                         break;
				default:    invalid_instruction();                                     break;
			}
			break;
		default:    invalid_instruction();                                             break;
	}
}


/***************************************************************************
    COP1 (FPU) EXECUTION HANDLING
***************************************************************************/

inline UINT32 r3000_device::get_cop1_reg(int idx)
{
	return m_cpr[1][idx];
}

inline void r3000_device::set_cop1_reg(int idx, UINT32 val)
{
	m_cpr[1][idx] = val;
}

inline UINT32 r3000_device::get_cop1_creg(int idx)
{
	return m_ccr[1][idx];
}

inline void r3000_device::set_cop1_creg(int idx, UINT32 val)
{
	m_ccr[1][idx] = val;
}

inline void r3000_device::handle_cop1()
{
	if (!(SR & SR_COP1))
		generate_exception(EXCEPTION_BADCOP);
	if (!m_hasfpu)
		return;

	switch (RSREG)
	{
		case 0x00:  /* MFCz */      if (RTREG) RTVAL = get_cop1_reg(RDREG);       break;
		case 0x02:  /* CFCz */      if (RTREG) RTVAL = get_cop1_creg(RDREG);      break;
		case 0x04:  /* MTCz */      set_cop1_reg(RDREG, RTVAL);                   break;
		case 0x06:  /* CTCz */      set_cop1_creg(RDREG, RTVAL);                  break;
		case 0x08:  /* BC */
			switch (RTREG)
			{
				case 0x00:  /* BCzF */  if (!m_in_brcond1()) ADDPC(SIMMVAL);    break;
				case 0x01:  /* BCzT */  if (m_in_brcond1()) ADDPC(SIMMVAL);     break;
				case 0x02:  /* BCzFL */ invalid_instruction();                         break;
				case 0x03:  /* BCzTL */ invalid_instruction();                         break;
				default:    invalid_instruction();                                     break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */       invalid_instruction();                             break;
		default:    invalid_instruction();                                             break;
	}
}


/***************************************************************************
    COP2 (CUSTOM) EXECUTION HANDLING
***************************************************************************/

inline UINT32 r3000_device::get_cop2_reg(int idx)
{
	return m_cpr[2][idx];
}

inline void r3000_device::set_cop2_reg(int idx, UINT32 val)
{
	m_cpr[2][idx] = val;
}

inline UINT32 r3000_device::get_cop2_creg(int idx)
{
	return m_ccr[2][idx];
}

inline void r3000_device::set_cop2_creg(int idx, UINT32 val)
{
	m_ccr[2][idx] = val;
}

inline void r3000_device::handle_cop2()
{
	if (!(SR & SR_COP2))
		generate_exception(EXCEPTION_BADCOP);

	switch (RSREG)
	{
		case 0x00:  /* MFCz */      if (RTREG) RTVAL = get_cop2_reg(RDREG);       break;
		case 0x02:  /* CFCz */      if (RTREG) RTVAL = get_cop2_creg(RDREG);      break;
		case 0x04:  /* MTCz */      set_cop2_reg(RDREG, RTVAL);                   break;
		case 0x06:  /* CTCz */      set_cop2_creg(RDREG, RTVAL);                  break;
		case 0x08:  /* BC */
			switch (RTREG)
			{
				case 0x00:  /* BCzF */  if (!m_in_brcond2()) ADDPC(SIMMVAL);    break;
				case 0x01:  /* BCzT */  if (m_in_brcond2()) ADDPC(SIMMVAL);     break;
				case 0x02:  /* BCzFL */ invalid_instruction();                         break;
				case 0x03:  /* BCzTL */ invalid_instruction();                         break;
				default:    invalid_instruction();                                     break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */       invalid_instruction();                             break;
		default:    invalid_instruction();                                             break;
	}
}


/***************************************************************************
    COP3 (CUSTOM) EXECUTION HANDLING
***************************************************************************/

inline UINT32 r3000_device::get_cop3_reg(int idx)
{
	return m_cpr[3][idx];
}

inline void r3000_device::set_cop3_reg(int idx, UINT32 val)
{
	m_cpr[3][idx] = val;
}

inline UINT32 r3000_device::get_cop3_creg(int idx)
{
	return m_ccr[3][idx];
}

inline void r3000_device::set_cop3_creg(int idx, UINT32 val)
{
	m_ccr[3][idx] = val;
}

inline void r3000_device::handle_cop3()
{
	if (!(SR & SR_COP3))
		generate_exception(EXCEPTION_BADCOP);

	switch (RSREG)
	{
		case 0x00:  /* MFCz */      if (RTREG) RTVAL = get_cop3_reg(RDREG);       break;
		case 0x02:  /* CFCz */      if (RTREG) RTVAL = get_cop3_creg(RDREG);      break;
		case 0x04:  /* MTCz */      set_cop3_reg(RDREG, RTVAL);                   break;
		case 0x06:  /* CTCz */      set_cop3_creg(RDREG, RTVAL);                  break;
		case 0x08:  /* BC */
			switch (RTREG)
			{
				case 0x00:  /* BCzF */  if (!m_in_brcond3()) ADDPC(SIMMVAL);    break;
				case 0x01:  /* BCzT */  if (m_in_brcond3()) ADDPC(SIMMVAL);         break;
				case 0x02:  /* BCzFL */ invalid_instruction();                         break;
				case 0x03:  /* BCzTL */ invalid_instruction();                         break;
				default:    invalid_instruction();                                     break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */       invalid_instruction();                             break;
		default:    invalid_instruction();                                             break;
	}
}


/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 r3000_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 r3000_device::execute_max_cycles() const
{
	return 40;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 r3000_device::execute_input_lines() const
{
	return 6;
}


//-------------------------------------------------
//  execute_set_input
//-------------------------------------------------

void r3000_device::execute_set_input(int inputnum, int state)
{
	set_irq_line(inputnum, state);
}


//-------------------------------------------------
//  execute_run
//-------------------------------------------------

void r3000_device::execute_run()
{
	// count cycles and interrupt cycles
	m_icount -= m_interrupt_cycles;
	m_interrupt_cycles = 0;

	// check for IRQs
	check_irqs();

	// core execution loop
	do
	{
		UINT64 temp64;
		int temp;

		// debugging
		m_ppc = m_pc;
		debugger_instruction_hook(this, m_pc);

		// instruction fetch
		m_op = readop(m_pc);

		// adjust for next PC
		if (m_nextpc != ~0)
		{
			m_pc = m_nextpc;
			m_nextpc = ~0;
		}
		else
			m_pc += 4;

		// parse the instruction
		switch (m_op >> 26)
		{
			case 0x00:  /* SPECIAL */
				switch (m_op & 63)
				{
					case 0x00:  /* SLL */       if (RDREG) RDVAL = RTVAL << SHIFT;                        break;
					case 0x02:  /* SRL */       if (RDREG) RDVAL = RTVAL >> SHIFT;                        break;
					case 0x03:  /* SRA */       if (RDREG) RDVAL = (INT32)RTVAL >> SHIFT;                 break;
					case 0x04:  /* SLLV */      if (RDREG) RDVAL = RTVAL << (RSVAL & 31);          break;
					case 0x06:  /* SRLV */      if (RDREG) RDVAL = RTVAL >> (RSVAL & 31);          break;
					case 0x07:  /* SRAV */      if (RDREG) RDVAL = (INT32)RTVAL >> (RSVAL & 31);   break;
					case 0x08:  /* JR */        SETPC(RSVAL);                                             break;
					case 0x09:  /* JALR */      SETPCL(RSVAL, RDREG);                                     break;
					case 0x0c:  /* SYSCALL */   generate_exception(EXCEPTION_SYSCALL);                           break;
					case 0x0d:  /* BREAK */     generate_exception(EXCEPTION_BREAK);                             break;
					case 0x0f:  /* SYNC */      invalid_instruction();                                         break;
					case 0x10:  /* MFHI */      if (RDREG) RDVAL = m_hi;                                    break;
					case 0x11:  /* MTHI */      m_hi = RSVAL;                                               break;
					case 0x12:  /* MFLO */      if (RDREG) RDVAL = m_lo;                                    break;
					case 0x13:  /* MTLO */      m_lo = RSVAL;                                               break;
					case 0x18:  /* MULT */
						temp64 = (INT64)(INT32)RSVAL * (INT64)(INT32)RTVAL;
						m_lo = (UINT32)temp64;
						m_hi = (UINT32)(temp64 >> 32);
						m_icount -= 11;
						break;
					case 0x19:  /* MULTU */
						temp64 = (UINT64)RSVAL * (UINT64)RTVAL;
						m_lo = (UINT32)temp64;
						m_hi = (UINT32)(temp64 >> 32);
						m_icount -= 11;
						break;
					case 0x1a:  /* DIV */
						if (RTVAL)
						{
							m_lo = (INT32)RSVAL / (INT32)RTVAL;
							m_hi = (INT32)RSVAL % (INT32)RTVAL;
						}
						m_icount -= 34;
						break;
					case 0x1b:  /* DIVU */
						if (RTVAL)
						{
							m_lo = RSVAL / RTVAL;
							m_hi = RSVAL % RTVAL;
						}
						m_icount -= 34;
						break;
					case 0x20:  /* ADD */
						if (ENABLE_OVERFLOWS && RSVAL > ~RTVAL) generate_exception(EXCEPTION_OVERFLOW);
						else RDVAL = RSVAL + RTVAL;
						break;
					case 0x21:  /* ADDU */      if (RDREG) RDVAL = RSVAL + RTVAL;                  break;
					case 0x22:  /* SUB */
						if (ENABLE_OVERFLOWS && RSVAL < RTVAL) generate_exception(EXCEPTION_OVERFLOW);
						else RDVAL = RSVAL - RTVAL;
						break;
					case 0x23:  /* SUBU */      if (RDREG) RDVAL = RSVAL - RTVAL;                  break;
					case 0x24:  /* AND */       if (RDREG) RDVAL = RSVAL & RTVAL;                  break;
					case 0x25:  /* OR */        if (RDREG) RDVAL = RSVAL | RTVAL;                  break;
					case 0x26:  /* XOR */       if (RDREG) RDVAL = RSVAL ^ RTVAL;                  break;
					case 0x27:  /* NOR */       if (RDREG) RDVAL = ~(RSVAL | RTVAL);               break;
					case 0x2a:  /* SLT */       if (RDREG) RDVAL = (INT32)RSVAL < (INT32)RTVAL;    break;
					case 0x2b:  /* SLTU */      if (RDREG) RDVAL = (UINT32)RSVAL < (UINT32)RTVAL;  break;
					case 0x30:  /* TEQ */       invalid_instruction();                                         break;
					case 0x31:  /* TGEU */      invalid_instruction();                                         break;
					case 0x32:  /* TLT */       invalid_instruction();                                         break;
					case 0x33:  /* TLTU */      invalid_instruction();                                         break;
					case 0x34:  /* TGE */       invalid_instruction();                                         break;
					case 0x36:  /* TNE */       invalid_instruction();                                         break;
					default:    /* ??? */       invalid_instruction();                                         break;
				}
				break;

			case 0x01:  /* REGIMM */
				switch (RTREG)
				{
					case 0x00:  /* BLTZ */      if ((INT32)RSVAL < 0) ADDPC(SIMMVAL);                     break;
					case 0x01:  /* BGEZ */      if ((INT32)RSVAL >= 0) ADDPC(SIMMVAL);                    break;
					case 0x02:  /* BLTZL */     invalid_instruction();                                         break;
					case 0x03:  /* BGEZL */     invalid_instruction();                                         break;
					case 0x08:  /* TGEI */      invalid_instruction();                                         break;
					case 0x09:  /* TGEIU */     invalid_instruction();                                         break;
					case 0x0a:  /* TLTI */      invalid_instruction();                                         break;
					case 0x0b:  /* TLTIU */     invalid_instruction();                                         break;
					case 0x0c:  /* TEQI */      invalid_instruction();                                         break;
					case 0x0e:  /* TNEI */      invalid_instruction();                                         break;
					case 0x10:  /* BLTZAL */    if ((INT32)RSVAL < 0) ADDPCL(SIMMVAL,31);                  break;
					case 0x11:  /* BGEZAL */    if ((INT32)RSVAL >= 0) ADDPCL(SIMMVAL,31);                 break;
					case 0x12:  /* BLTZALL */   invalid_instruction();                                         break;
					case 0x13:  /* BGEZALL */   invalid_instruction();                                         break;
					default:    /* ??? */       invalid_instruction();                                         break;
				}
				break;

			case 0x02:  /* J */         ABSPC(LIMMVAL);                                                          break;
			case 0x03:  /* JAL */       ABSPCL(LIMMVAL,31);                                                      break;
			case 0x04:  /* BEQ */       if (RSVAL == RTVAL) ADDPC(SIMMVAL);                        break;
			case 0x05:  /* BNE */       if (RSVAL != RTVAL) ADDPC(SIMMVAL);                        break;
			case 0x06:  /* BLEZ */      if ((INT32)RSVAL <= 0) ADDPC(SIMMVAL);                            break;
			case 0x07:  /* BGTZ */      if ((INT32)RSVAL > 0) ADDPC(SIMMVAL);                             break;
			case 0x08:  /* ADDI */
				if (ENABLE_OVERFLOWS && RSVAL > ~SIMMVAL) generate_exception(EXCEPTION_OVERFLOW);
				else if (RTREG) RTVAL = RSVAL + SIMMVAL;
				break;
			case 0x09:  /* ADDIU */     if (RTREG) RTVAL = RSVAL + SIMMVAL;                               break;
			case 0x0a:  /* SLTI */      if (RTREG) RTVAL = (INT32)RSVAL < (INT32)SIMMVAL;                 break;
			case 0x0b:  /* SLTIU */     if (RTREG) RTVAL = (UINT32)RSVAL < (UINT32)SIMMVAL;               break;
			case 0x0c:  /* ANDI */      if (RTREG) RTVAL = RSVAL & UIMMVAL;                               break;
			case 0x0d:  /* ORI */       if (RTREG) RTVAL = RSVAL | UIMMVAL;                               break;
			case 0x0e:  /* XORI */      if (RTREG) RTVAL = RSVAL ^ UIMMVAL;                               break;
			case 0x0f:  /* LUI */       if (RTREG) RTVAL = UIMMVAL << 16;                                        break;
			case 0x10:  /* COP0 */      handle_cop0();                                                         break;
			case 0x11:  /* COP1 */      handle_cop1();                                                         break;
			case 0x12:  /* COP2 */      handle_cop2();                                                         break;
			case 0x13:  /* COP3 */      handle_cop3();                                                         break;
			case 0x14:  /* BEQL */      invalid_instruction();                                                 break;
			case 0x15:  /* BNEL */      invalid_instruction();                                                 break;
			case 0x16:  /* BLEZL */     invalid_instruction();                                                 break;
			case 0x17:  /* BGTZL */     invalid_instruction();                                                 break;
			case 0x20:  /* LB */        temp = RBYTE(SIMMVAL+RSVAL); if (RTREG) RTVAL = (INT8)temp; break;
			case 0x21:  /* LH */        temp = RWORD(SIMMVAL+RSVAL); if (RTREG) RTVAL = (INT16)temp; break;
			case 0x22:  /* LWL */       (*this.*m_lwl)();                                                       break;
			case 0x23:  /* LW */        temp = RLONG(SIMMVAL+RSVAL); if (RTREG) RTVAL = temp;      break;
			case 0x24:  /* LBU */       temp = RBYTE(SIMMVAL+RSVAL); if (RTREG) RTVAL = (UINT8)temp; break;
			case 0x25:  /* LHU */       temp = RWORD(SIMMVAL+RSVAL); if (RTREG) RTVAL = (UINT16)temp; break;
			case 0x26:  /* LWR */       (*this.*m_lwr)();                                                       break;
			case 0x28:  /* SB */        WBYTE(SIMMVAL+RSVAL, RTVAL);                               break;
			case 0x29:  /* SH */        WWORD(SIMMVAL+RSVAL, RTVAL);                               break;
			case 0x2a:  /* SWL */       (*this.*m_swl)();                                                       break;
			case 0x2b:  /* SW */        WLONG(SIMMVAL+RSVAL, RTVAL);                               break;
			case 0x2e:  /* SWR */       (*this.*m_swr)();                                                       break;
			case 0x2f:  /* CACHE */     invalid_instruction();                                                 break;
			case 0x30:  /* LL */        invalid_instruction();                                                 break;
			case 0x31:  /* LWC1 */      set_cop1_reg(RTREG, RLONG(SIMMVAL+RSVAL));                 break;
			case 0x32:  /* LWC2 */      set_cop2_reg(RTREG, RLONG(SIMMVAL+RSVAL));                 break;
			case 0x33:  /* LWC3 */      set_cop3_reg(RTREG, RLONG(SIMMVAL+RSVAL));                 break;
			case 0x34:  /* LDC0 */      invalid_instruction();                                                 break;
			case 0x35:  /* LDC1 */      invalid_instruction();                                                 break;
			case 0x36:  /* LDC2 */      invalid_instruction();                                                 break;
			case 0x37:  /* LDC3 */      invalid_instruction();                                                 break;
			case 0x38:  /* SC */        invalid_instruction();                                                 break;
			case 0x39:  /* LWC1 */      WLONG(SIMMVAL+RSVAL, get_cop1_reg(RTREG));                 break;
			case 0x3a:  /* LWC2 */      WLONG(SIMMVAL+RSVAL, get_cop2_reg(RTREG));                 break;
			case 0x3b:  /* LWC3 */      WLONG(SIMMVAL+RSVAL, get_cop3_reg(RTREG));                 break;
			case 0x3c:  /* SDC0 */      invalid_instruction();                                                 break;
			case 0x3d:  /* SDC1 */      invalid_instruction();                                                 break;
			case 0x3e:  /* SDC2 */      invalid_instruction();                                                 break;
			case 0x3f:  /* SDC3 */      invalid_instruction();                                                 break;
			default:    /* ??? */       invalid_instruction();                                                 break;
		}
		m_icount--;

	} while (m_icount > 0 || m_nextpc != ~0);

	m_icount -= m_interrupt_cycles;
	m_interrupt_cycles = 0;
}


/***************************************************************************
    COMPLEX OPCODE IMPLEMENTATIONS
***************************************************************************/

void r3000_device::lwl_be()
{
	offs_t offs = SIMMVAL + RSVAL;
	UINT32 temp = RLONG(offs & ~3);
	if (RTREG)
	{
		if (!(offs & 3)) RTVAL = temp;
		else
		{
			int shift = 8 * (offs & 3);
			RTVAL = (RTVAL & (0x00ffffff >> (24 - shift))) | (temp << shift);
		}
	}
}

void r3000_device::lwr_be()
{
	offs_t offs = SIMMVAL + RSVAL;
	UINT32 temp = RLONG(offs & ~3);
	if (RTREG)
	{
		if ((offs & 3) == 3) RTVAL = temp;
		else
		{
			int shift = 8 * (offs & 3);
			RTVAL = (RTVAL & (0xffffff00 << shift)) | (temp >> (24 - shift));
		}
	}
}

void r3000_device::swl_be()
{
	offs_t offs = SIMMVAL + RSVAL;
	if (!(offs & 3)) WLONG(offs, RTVAL);
	else
	{
		UINT32 temp = RLONG(offs & ~3);
		int shift = 8 * (offs & 3);
		WLONG(offs & ~3, (temp & (0xffffff00 << (24 - shift))) | (RTVAL >> shift));
	}
}


void r3000_device::swr_be()
{
	offs_t offs = SIMMVAL + RSVAL;
	if ((offs & 3) == 3) WLONG(offs & ~3, RTVAL);
	else
	{
		UINT32 temp = RLONG(offs & ~3);
		int shift = 8 * (offs & 3);
		WLONG(offs & ~3, (temp & (0x00ffffff >> shift)) | (RTVAL << (24 - shift)));
	}
}



void r3000_device::lwl_le()
{
	offs_t offs = SIMMVAL + RSVAL;
	UINT32 temp = RLONG(offs & ~3);
	if (RTREG)
	{
		if (!(offs & 3)) RTVAL = temp;
		else
		{
			int shift = 8 * (offs & 3);
			RTVAL = (RTVAL & (0xffffff00 << (24 - shift))) | (temp >> shift);
		}
	}
}

void r3000_device::lwr_le()
{
	offs_t offs = SIMMVAL + RSVAL;
	UINT32 temp = RLONG(offs & ~3);
	if (RTREG)
	{
		if ((offs & 3) == 3) RTVAL = temp;
		else
		{
			int shift = 8 * (offs & 3);
			RTVAL = (RTVAL & (0x00ffffff >> shift)) | (temp << (24 - shift));
		}
	}
}

void r3000_device::swl_le()
{
	offs_t offs = SIMMVAL + RSVAL;
	if (!(offs & 3)) WLONG(offs, RTVAL);
	else
	{
		UINT32 temp = RLONG(offs & ~3);
		int shift = 8 * (offs & 3);
		WLONG(offs & ~3, (temp & (0x00ffffff >> (24 - shift))) | (RTVAL << shift));
	}
}

void r3000_device::swr_le()
{
	offs_t offs = SIMMVAL + RSVAL;
	if ((offs & 3) == 3) WLONG(offs & ~3, RTVAL);
	else
	{
		UINT32 temp = RLONG(offs & ~3);
		int shift = 8 * (offs & 3);
		WLONG(offs & ~3, (temp & (0xffffff00 << shift)) | (RTVAL >> (24 - shift)));
	}
}
