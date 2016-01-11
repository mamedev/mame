// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    tms32031.c

    TMS32031/2 emulator

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tms32031.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// indexes into the register file
enum
{
	TMR_R0 = 0,
	TMR_R1,
	TMR_R2,
	TMR_R3,
	TMR_R4,
	TMR_R5,
	TMR_R6,
	TMR_R7,
	TMR_AR0,
	TMR_AR1,
	TMR_AR2,
	TMR_AR3,
	TMR_AR4,
	TMR_AR5,
	TMR_AR6,
	TMR_AR7,
	TMR_DP,
	TMR_IR0,
	TMR_IR1,
	TMR_BK,
	TMR_SP,
	TMR_ST,
	TMR_IE,
	TMR_IF,
	TMR_IOF,
	TMR_RS,
	TMR_RE,
	TMR_RC,
	TMR_R8,     // 3204x only
	TMR_R9,     // 3204x only
	TMR_R10,    // 3204x only
	TMR_R11,    // 3204x only
	TMR_TEMP1,  // used by the interpreter
	TMR_TEMP2,  // used by the interpreter
	TMR_TEMP3   // used by the interpreter
};

// flags
const int CFLAG     = 0x0001;
const int VFLAG     = 0x0002;
const int ZFLAG     = 0x0004;
const int NFLAG     = 0x0008;
const int UFFLAG    = 0x0010;
const int LVFLAG    = 0x0020;
const int LUFFLAG   = 0x0040;
const int OVMFLAG   = 0x0080;
const int RMFLAG    = 0x0100;
//const int CFFLAG    = 0x0400;
//const int CEFLAG    = 0x0800;
//const int CCFLAG    = 0x1000;
const int GIEFLAG   = 0x2000;



//**************************************************************************
//  MACROS
//**************************************************************************

#define IREG(rnum)  (m_r[rnum].i32[0])



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type TMS32031 = &device_creator<tms32031_device>;
const device_type TMS32032 = &device_creator<tms32032_device>;


// internal memory maps
static ADDRESS_MAP_START( internal_32031, AS_PROGRAM, 32, tms32031_device )
	AM_RANGE(0x809800, 0x809fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( internal_32032, AS_PROGRAM, 32, tms32032_device )
	AM_RANGE(0x87fe00, 0x87ffff) AM_RAM
ADDRESS_MAP_END


// ROM definitions for the internal boot loader programs
// (Using assembled versions until the code ROMs are extracted from both DSPs)
ROM_START( tms32031 )
	ROM_REGION(0x4000, "tms32031", 0)
	ROM_LOAD( "c31boot.bin", 0x0000, 0x4000, BAD_DUMP CRC(bddc2763) SHA1(96b2170ecee5bec5abaa1741bb2d3b6096ecc262) ) // Assembled from c31boot.asm (02-07-92)
ROM_END

ROM_START( tms32032 )
	ROM_REGION(0x4000, "tms32032", 0)
	ROM_LOAD( "c32boot.bin", 0x0000, 0x4000, BAD_DUMP CRC(ecf84729) SHA1(4d32ead450f921f563514b061ea561a222283616) ) // Assembled from c32boot.asm (03-04-96)
ROM_END



//**************************************************************************
//  TMSREG REGISTER
//**************************************************************************

//-------------------------------------------------
//  as_float - interpret the contents of a tmsreg
//  as a DSP-encoded floating-point value, and
//  extract a 32-bit IEEE float from it
//-------------------------------------------------

float tms3203x_device::tmsreg::as_float() const
{
	int_double id;

	// map 0 to 0
	if (mantissa() == 0 && exponent() == -128)
		return 0;

	// handle positive numbers
	else if (mantissa() >= 0)
	{
		int exp = (exponent() + 127) << 23;
		id.i[0] = exp + (mantissa() >> 8);
	}

	// handle negative numbers
	else
	{
		int exp = (exponent() + 127) << 23;
		INT32 man = -mantissa();
		id.i[0] = 0x80000000 + exp + ((man >> 8) & 0x00ffffff);
	}

	// return the converted float
	return id.f[0];
}


//-------------------------------------------------
//  as_double - interpret the contents of a tmsreg
//  as a DSP-encoded floating-point value, and
//  extract a 64-bit IEEE double from it
//-------------------------------------------------

double tms3203x_device::tmsreg::as_double() const
{
	int_double id;

	// map 0 to 0
	if (mantissa() == 0 && exponent() == -128)
		return 0;

	// handle positive numbers
	else if (mantissa() >= 0)
	{
		int exp = (exponent() + 1023) << 20;
		id.i[BYTE_XOR_BE(0)] = exp + (mantissa() >> 11);
		id.i[BYTE_XOR_BE(1)] = (mantissa() << 21) & 0xffe00000;
	}

	// handle negative numbers
	else
	{
		int exp = (exponent() + 1023) << 20;
		INT32 man = -mantissa();
		id.i[BYTE_XOR_BE(0)] = 0x80000000 + exp + ((man >> 11) & 0x001fffff);
		id.i[BYTE_XOR_BE(1)] = (man << 21) & 0xffe00000;
	}

	// return the converted double
	return id.d;
}


//-------------------------------------------------
//  from_double - import a 64-bit IEEE double into
//  the DSP's internal floating point format
//-------------------------------------------------

void tms3203x_device::tmsreg::from_double(double val)
{
	// extract mantissa and exponent from the IEEE input
	int_double id;
	id.d = val;
	INT32 mantissa = ((id.i[BYTE_XOR_BE(0)] & 0x000fffff) << 11) | ((id.i[BYTE_XOR_BE(1)] & 0xffe00000) >> 21);
	INT32 exponent = ((id.i[BYTE_XOR_BE(0)] & 0x7ff00000) >> 20) - 1023;

	// if we're too small, map to 0
	if (exponent < -128)
	{
		set_mantissa(0);
		set_exponent(-128);
	}

	// if we're too large, map to the maximum value
	else if (exponent > 127)
	{
		if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
			set_mantissa(0x7fffffff);
		else
			set_mantissa(0x80000001);
		set_exponent(127);
	}

	// if we're positive, map directly
	else if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
	{
		set_mantissa(mantissa);
		set_exponent(exponent);
	}

	// if we're negative with a non-zero mantissa, remove the leading sign bit
	else if (mantissa != 0)
	{
		set_mantissa(0x80000000 | -mantissa);
		set_exponent(exponent);
	}

	// if we're negative with a zero mantissa, normalize
	else
	{
		set_mantissa(0x80000000);
		set_exponent(exponent - 1);
	}
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  tms3203x_device - constructor
//-------------------------------------------------

tms3203x_device::tms3203x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 chiptype, address_map_constructor internal_map, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_program_config("program", ENDIANNESS_LITTLE, 32, 24, -2, internal_map),
		m_chip_type(chiptype),
		m_pc(0),
		m_bkmask(0),
		m_irq_state(0),
		m_delayed(false),
		m_irq_pending(false),
		m_is_idling(false),
		m_icount(0),
		m_program(nullptr),
		m_direct(nullptr),
		m_mcbl_mode(false),
		m_xf0_cb(*this),
		m_xf1_cb(*this),
		m_iack_cb(*this)
{
	// initialize remaining state
	memset(&m_r, 0, sizeof(m_r));

	// set our instruction counter
	m_icountptr = &m_icount;

#if (TMS_3203X_LOG_OPCODE_USAGE)
	memset(m_hits, 0, sizeof(m_hits));
#endif
}

tms32031_device::tms32031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms3203x_device(mconfig, TMS32031, "TMS32031", tag, owner, clock, CHIP_TYPE_TMS32031, ADDRESS_MAP_NAME(internal_32031), "tms32031", __FILE__)
{
}

tms32032_device::tms32032_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms3203x_device(mconfig, TMS32032, "TMS32032", tag, owner, clock, CHIP_TYPE_TMS32032, ADDRESS_MAP_NAME(internal_32032), "tms32032", __FILE__)
{
}


DIRECT_UPDATE_MEMBER( tms3203x_device::direct_handler )
{
	// internal boot loader ROM
	if (m_mcbl_mode && address < (0x1000 << 2))
	{
		direct.explicit_configure(0x000000, 0x003fff, 0x003fff, m_bootrom);
		return (offs_t)-1;
	}

	return address;
}


//-------------------------------------------------
//  ~tms3203x_device - destructor
//-------------------------------------------------

tms3203x_device::~tms3203x_device()
{
#if (TMS_3203X_LOG_OPCODE_USAGE)
	for (int i = 0; i < ARRAY_LENGTH(m_hits); i++)
		if (m_hits[i] != 0)
			printf("%10d - %03X.%X\n", m_hits[i], i / 4, i % 4);
#endif
}


//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const rom_entry *tms3203x_device::device_rom_region() const
{
	switch (m_chip_type)
	{
		default:
		case CHIP_TYPE_TMS32031:    return ROM_NAME( tms32031 );
		case CHIP_TYPE_TMS32032:    return ROM_NAME( tms32032 );
	}
}

//-------------------------------------------------
//  ROPCODE - fetch an opcode
//-------------------------------------------------

inline UINT32 tms3203x_device::ROPCODE(offs_t pc)
{
	return m_direct->read_dword(pc << 2);
}


//-------------------------------------------------
//  RMEM - read memory
//-------------------------------------------------

inline UINT32 tms3203x_device::RMEM(offs_t addr)
{
	if (m_mcbl_mode && addr < 0x1000)
		return m_bootrom[addr];

	return m_program->read_dword(addr << 2);
}


//-------------------------------------------------
//  WMEM - write memory
//-------------------------------------------------

inline void tms3203x_device::WMEM(offs_t addr, UINT32 data)
{
	m_program->write_dword(addr << 2, data);
}


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void tms3203x_device::device_start()
{
	// find address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// resolve devcb handlers
	m_xf0_cb.resolve_safe();
	m_xf1_cb.resolve_safe();
	m_iack_cb.resolve_safe();

	// set up the internal boot loader ROM
	m_bootrom = reinterpret_cast<UINT32*>(memregion(shortname())->base());
	m_direct->set_direct_update(direct_update_delegate(FUNC(tms3203x_device::direct_handler), this));

	// save state
	save_item(NAME(m_pc));
	for (int regnum = 0; regnum < 36; regnum++)
		save_item(NAME(m_r[regnum].i32), regnum);
	save_item(NAME(m_bkmask));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_delayed));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_is_idling));

	// register our state for the debugger
	state_add(TMS3203X_PC,      "PC",        m_pc);
	state_add(STATE_GENPC,      "GENPC",     m_pc).noshow();
	state_add(STATE_GENFLAGS,   "GENFLAGS",  m_r[TMR_ST].i32[0]).mask(0xff).noshow().formatstr("%8s");
	state_add(TMS3203X_R0,      "R0",        m_r[TMR_R0].i32[0]);
	state_add(TMS3203X_R1,      "R1",        m_r[TMR_R1].i32[0]);
	state_add(TMS3203X_R2,      "R2",        m_r[TMR_R2].i32[0]);
	state_add(TMS3203X_R3,      "R3",        m_r[TMR_R3].i32[0]);
	state_add(TMS3203X_R4,      "R4",        m_r[TMR_R4].i32[0]);
	state_add(TMS3203X_R5,      "R5",        m_r[TMR_R5].i32[0]);
	state_add(TMS3203X_R6,      "R6",        m_r[TMR_R6].i32[0]);
	state_add(TMS3203X_R7,      "R7",        m_r[TMR_R7].i32[0]);
	state_add(TMS3203X_R0F,     "R0F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R1F,     "R1F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R2F,     "R2F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R3F,     "R3F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R4F,     "R4F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R5F,     "R5F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R6F,     "R6F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R7F,     "R7F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_AR0,     "AR0",       m_r[TMR_AR0].i32[0]);
	state_add(TMS3203X_AR1,     "AR1",       m_r[TMR_AR1].i32[0]);
	state_add(TMS3203X_AR2,     "AR2",       m_r[TMR_AR2].i32[0]);
	state_add(TMS3203X_AR3,     "AR3",       m_r[TMR_AR3].i32[0]);
	state_add(TMS3203X_AR4,     "AR4",       m_r[TMR_AR4].i32[0]);
	state_add(TMS3203X_AR5,     "AR5",       m_r[TMR_AR5].i32[0]);
	state_add(TMS3203X_AR6,     "AR6",       m_r[TMR_AR6].i32[0]);
	state_add(TMS3203X_AR7,     "AR7",       m_r[TMR_AR7].i32[0]);
	state_add(TMS3203X_DP,      "DP",        m_r[TMR_DP].i32[0]).mask(0xff);
	state_add(TMS3203X_IR0,     "IR0",       m_r[TMR_IR0].i32[0]);
	state_add(TMS3203X_IR1,     "IR1",       m_r[TMR_IR1].i32[0]);
	state_add(TMS3203X_BK,      "BK",        m_r[TMR_BK].i32[0]);
	state_add(TMS3203X_SP,      "SP",        m_r[TMR_SP].i32[0]);
	state_add(TMS3203X_ST,      "ST",        m_r[TMR_ST].i32[0]);
	state_add(TMS3203X_IE,      "IE",        m_r[TMR_IE].i32[0]);
	state_add(TMS3203X_IF,      "IF",        m_r[TMR_IF].i32[0]);
	state_add(TMS3203X_IOF,     "IOF",       m_r[TMR_IOF].i32[0]);
	state_add(TMS3203X_RS,      "RS",        m_r[TMR_RS].i32[0]);
	state_add(TMS3203X_RE,      "RE",        m_r[TMR_RE].i32[0]);
	state_add(TMS3203X_RC,      "RC",        m_r[TMR_RC].i32[0]);
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void tms3203x_device::device_reset()
{
	m_pc = RMEM(0);

	// reset some registers
	IREG(TMR_IE) = 0;
	IREG(TMR_IF) = 0;
	IREG(TMR_ST) = 0;
	IREG(TMR_IOF) = 0;

	// update IF with the external interrupt state (required for boot loader operation)
	IREG(TMR_IF) |= m_irq_state & 0x0f;

	// reset internal stuff
	m_delayed = m_irq_pending = m_is_idling = false;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *tms3203x_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr;
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void tms3203x_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case TMS3203X_R0F:
		case TMS3203X_R1F:
		case TMS3203X_R2F:
		case TMS3203X_R3F:
		case TMS3203X_R4F:
		case TMS3203X_R5F:
		case TMS3203X_R6F:
		case TMS3203X_R7F:
			m_r[TMR_R0 + (entry.index() - TMS3203X_R0F)].from_double(*(float *)&m_iotemp);
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(tms3203x) called for unexpected value\n");
	}
}


//-------------------------------------------------
//  state_export - export state into the device,
//  before returning it to the caller
//-------------------------------------------------

void tms3203x_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case TMS3203X_R0F:
		case TMS3203X_R1F:
		case TMS3203X_R2F:
		case TMS3203X_R3F:
		case TMS3203X_R4F:
		case TMS3203X_R5F:
		case TMS3203X_R6F:
		case TMS3203X_R7F:
			*(float *)&m_iotemp = m_r[TMR_R0 + (entry.index() - TMS3203X_R0F)].as_float();
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(tms3203x) called for unexpected value\n");
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void tms3203x_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case TMS3203X_R0F:
		case TMS3203X_R1F:
		case TMS3203X_R2F:
		case TMS3203X_R3F:
		case TMS3203X_R4F:
		case TMS3203X_R5F:
		case TMS3203X_R6F:
		case TMS3203X_R7F:
			strprintf(str, "%12g", m_r[TMR_R0 + (entry.index() - TMS3203X_R0F)].as_double());
			break;

		case STATE_GENFLAGS:
			UINT32 temp = m_r[TMR_ST].i32[0];
			strprintf(str, "%c%c%c%c%c%c%c%c",
				(temp & 0x80) ? 'O':'.',
				(temp & 0x40) ? 'U':'.',
				(temp & 0x20) ? 'V':'.',
				(temp & 0x10) ? 'u':'.',
				(temp & 0x08) ? 'n':'.',
				(temp & 0x04) ? 'z':'.',
				(temp & 0x02) ? 'v':'.',
				(temp & 0x01) ? 'c':'.');
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 tms3203x_device::disasm_min_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 tms3203x_device::disasm_max_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t tms3203x_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms3203x );
	return CPU_DISASSEMBLE_NAME(tms3203x)(this, buffer, pc, oprom, opram, options);
}



//**************************************************************************
//  PUBLIC INTERFACES
//**************************************************************************

//-------------------------------------------------
//  fp_to_float - convert a 32-bit value from DSP
//  floating-point format a 32-bit IEEE float
//-------------------------------------------------

float tms3203x_device::fp_to_float(UINT32 floatdata)
{
	tmsreg gen(floatdata << 8, (INT32)floatdata >> 24);
	return gen.as_float();
}


//-------------------------------------------------
//  fp_to_double - convert a 32-bit value from DSP
//  floating-point format a 64-bit IEEE double
//-------------------------------------------------

double tms3203x_device::fp_to_double(UINT32 floatdata)
{
	tmsreg gen(floatdata << 8, (INT32)floatdata >> 24);
	return gen.as_double();
}


//-------------------------------------------------
//  float_to_fp - convert a 32-bit IEEE float to
//  a 32-bit DSP floating-point value
//-------------------------------------------------

UINT32 tms3203x_device::float_to_fp(float fval)
{
	tmsreg gen(fval);
	return (gen.exponent() << 24) | ((UINT32)gen.mantissa() >> 8);
}


//-------------------------------------------------
//  double_to_fp - convert a 64-bit IEEE double to
//  a 32-bit DSP floating-point value
//-------------------------------------------------

UINT32 tms3203x_device::double_to_fp(double dval)
{
	tmsreg gen(dval);
	return (gen.exponent() << 24) | ((UINT32)gen.mantissa() >> 8);
}



//**************************************************************************
//  EXECUTION
//**************************************************************************

//-------------------------------------------------
//  check_irqs - check for pending IRQs and take
//  them if enabled
//-------------------------------------------------

void tms3203x_device::check_irqs()
{
	// determine if we have any live interrupts
	UINT16 validints = IREG(TMR_IF) & IREG(TMR_IE) & 0x0fff;
	if (validints == 0 || (IREG(TMR_ST) & GIEFLAG) == 0)
		return;

	// find the lowest signalled value
	int whichtrap = 0;
	for (int i = 0; i < 12; i++)
		if (validints & (1 << i))
		{
			whichtrap = i + 1;
			break;
		}

	// no longer idling if we get here
	m_is_idling = false;
	if (!m_delayed)
	{
		UINT16 intmask = 1 << (whichtrap - 1);

		// bit in IF is cleared when interrupt is taken
		IREG(TMR_IF) &= ~intmask;
		trap(whichtrap);

		// after auto-clearing the interrupt bit, we need to re-trigger
		// level-sensitive interrupts
		if (m_chip_type == CHIP_TYPE_TMS32031 || (IREG(TMR_ST) & 0x4000) == 0)
			IREG(TMR_IF) |= m_irq_state & 0x0f;
	}
	else
		m_irq_pending = true;
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 tms3203x_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 tms3203x_device::execute_max_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 tms3203x_device::execute_input_lines() const
{
	return (m_chip_type == CHIP_TYPE_TMS32032) ? 13 : 12;
}


//-------------------------------------------------
//  execute_set_input - set input and IRQ lines
//-------------------------------------------------

void tms3203x_device::execute_set_input(int inputnum, int state)
{
	// ignore anything out of range
	if (inputnum >= 13)
		return;

	if (inputnum == TMS3203X_MCBL)
	{
		// switch between microcomputer/boot loader and microprocessor modes
		m_mcbl_mode = (state == ASSERT_LINE);
		m_direct->force_update();
		return;
	}

	// update the external state
	UINT16 intmask = 1 << inputnum;
	if (state == ASSERT_LINE)
	{
		m_irq_state |= intmask;
		IREG(TMR_IF) |= intmask;
	}
	else
		m_irq_state &= ~intmask;

	// external interrupts are level-sensitive on the '31 and can be
	// configured as such on the '32; in that case, if the external
	// signal is high, we need to update the value in IF accordingly
	if (m_chip_type == CHIP_TYPE_TMS32031 || (IREG(TMR_ST) & 0x4000) == 0)
		IREG(TMR_IF) |= m_irq_state & 0x0f;
}


//-------------------------------------------------
//  execute_run - execute until our icount expires
//-------------------------------------------------

void tms3203x_device::execute_run()
{
	// check IRQs up front
	check_irqs();

	// if we're idling, just eat the cycles
	if (m_is_idling)
	{
		m_icount = 0;
		return;
	}

	// non-debug case
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		while (m_icount > 0)
		{
			if ((IREG(TMR_ST) & RMFLAG) && m_pc == IREG(TMR_RE) + 1)
			{
				if ((INT32)--IREG(TMR_RC) >= 0)
					m_pc = IREG(TMR_RS);
				else
				{
					IREG(TMR_ST) &= ~RMFLAG;
					if (m_delayed)
					{
						m_delayed = false;
						if (m_irq_pending)
						{
							m_irq_pending = false;
							check_irqs();
						}
					}
				}
				continue;
			}

			execute_one();
		}
	}

	// debugging case
	else
	{
		while (m_icount > 0)
		{
			// watch for out-of-range stack pointers
			if (IREG(TMR_SP) & 0xff000000)
				debugger_break(machine());
			if ((IREG(TMR_ST) & RMFLAG) && m_pc == IREG(TMR_RE) + 1)
			{
				if ((INT32)--IREG(TMR_RC) >= 0)
					m_pc = IREG(TMR_RS);
				else
				{
					IREG(TMR_ST) &= ~RMFLAG;
					if (m_delayed)
					{
						m_delayed = false;
						if (m_irq_pending)
						{
							m_irq_pending = false;
							check_irqs();
						}
					}
				}
				continue;
			}

			debugger_instruction_hook(this, m_pc);
			execute_one();
		}
	}
}


//**************************************************************************
//  CORE OPCODES
//**************************************************************************

#include "32031ops.inc"
