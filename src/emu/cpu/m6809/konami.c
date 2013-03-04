/*********************************************************************

    konami.c

	Portable Konami cpu emulator

    Copyright Nicola Salmoria and the MAME Team

    Based on M6809 cpu core copyright John Butler

    References:

        6809 Simulator V09, By L.C. Benschop, Eidnhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    UINT16 must be 16 bit unsigned int
                            UINT8 must be 8 bit unsigned int
                            UINT32 must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

    History:

March 2013 NPW:
	Rewrite of 6809/6309/Konami CPU; overall core is now unified and
	supports mid-instruction timings.

	Some of the instruction timings have changed with the new core; the
	old core had some nonsensical timings.  For example (from scontra):
		
		819A    3A 07 1F 8C        STA $1f8C

	Under the old core, this took four clock cycles, which is dubious
	because this instruction would have to do four opcode reads and one
	write.  OGalibert says that the current timings are just a guess and
	nobody has done precise readings, so I'm replacing the old guesses
	with new guesses.

991022 HJB:
    Tried to improve speed: Using bit7 of cycles1 as flag for multi
    byte opcodes is gone, those opcodes now instead go through opcode2().
    Inlined fetch_effective_address() into that function as well.
    Got rid of the slow/fast flags for stack (S and U) memory accesses.
    Minor changes to use 32 bit values as arguments to memory functions
    and added defines for that purpose (e.g. X = 16bit XD = 32bit).

990720 EHC:
    Created this file

*****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "konami.h"
#include "m6809inl.h"


//**************************************************************************
//  PARAMETERS
//**************************************************************************

// turn off 'unreferenced label' errors
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-label"
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4102 )
#endif


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

const device_type KONAMI = &device_creator<konami_cpu_device>;


//-------------------------------------------------
//  konami_cpu_device - constructor
//-------------------------------------------------

konami_cpu_device::konami_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: m6809_base_device(mconfig, "KONAMI", tag, owner, clock, KONAMI, 1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void konami_cpu_device::device_start()
{
	super::device_start();

	// initialize variables
	m_set_lines = NULL;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t konami_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern offs_t konami_cpu_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	return konami_cpu_disassemble(buffer, pc, oprom, opram, options);
}


//-------------------------------------------------
//  ireg
//-------------------------------------------------

ATTR_FORCE_INLINE UINT16 &konami_cpu_device::ireg()
{
	switch(m_opcode & 0x70)
	{
		case 0x20:	return m_x.w;
		case 0x30:	return m_y.w;
		case 0x50:	return m_u.w;
		case 0x60:	return m_s.w;
		case 0x70:	return m_pc.w;
		default:
			fatalerror("Should not get here");
			return m_x.w;
	}
}


//-------------------------------------------------
//  safe_shift_right
//-------------------------------------------------

template<class T> T konami_cpu_device::safe_shift_right(T value, UINT32 shift)
{
	T result;

	if (shift < (sizeof(T) * 8))
		result = value >> shift;
	else if (value < 0)
		result = (T) -1;
	else
		result = 0;

	return result;
}


//-------------------------------------------------
//  safe_shift_left
//-------------------------------------------------

template<class T> T konami_cpu_device::safe_shift_left(T value, UINT32 shift)
{
	T result;

	if (shift < (sizeof(T) * 8))
		result = value << shift;
	else
		result = 0;

	return result;
}


//-------------------------------------------------
//  lmul
//-------------------------------------------------

ATTR_FORCE_INLINE void konami_cpu_device::lmul()
{
	PAIR result;

	// do the multiply
	result.d = (UINT32)m_x.w * m_y.w;

	// set the result registers
	m_x.w = result.w.h;
	m_y.w = result.w.l;

	// set Z flag
	set_flags<UINT32>(CC_Z, result.d);

	// set C flag
	if (result.d & 0x8000)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;
}


//-------------------------------------------------
//  divx
//-------------------------------------------------

ATTR_FORCE_INLINE void konami_cpu_device::divx()
{
	UINT16 result;
	UINT8 remainder;

	if (m_d.b.l != 0)
	{
		result = m_x.w / m_d.b.l;
		remainder = m_x.w % m_d.b.l;
	}
	else
	{
		// divide by zero; not sure what happens
		result = 0;
		remainder = 0;
	}

	// set results and Z flag
	m_x.w = set_flags<UINT16>(CC_Z, result);
	m_d.b.l = remainder;

	// set C flag
	if (result & 0x0080)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;
}


//-------------------------------------------------
//  execute_one - try to execute a single instruction
//-------------------------------------------------

void konami_cpu_device::set_lines(UINT8 data)
{
	if (m_set_lines != NULL)
		(*m_set_lines)(this, data);
}


//-------------------------------------------------
//  execute_one - try to execute a single instruction
//-------------------------------------------------

ATTR_FORCE_INLINE void konami_cpu_device::execute_one()
{
	switch(pop_state())
	{
#include "cpu/m6809/konami.inc"
	}
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void konami_cpu_device::execute_run()
{
	do
	{
		execute_one();
	} while(m_icount > 0);
}


//-------------------------------------------------
//  konami_configure_set_lines
//-------------------------------------------------

void konami_configure_set_lines(device_t *device, konami_set_lines_func func)
{
	konami_cpu_device *cpu = dynamic_cast<konami_cpu_device*>(device);
	cpu->configure_set_lines(func);
}
