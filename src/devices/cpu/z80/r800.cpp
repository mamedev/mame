// license:BSD-3-Clause
// copyright-holders:AJR,Wilbert Pol
/***************************************************************************

    ASCII R800 CPU

TODO:
- Internal configuration registers.
- External 24 bits address bus accessible through 9 memory mappers.
- DMA channels.
- Interrupt levels.
- Bits 3 and 5 of the flag register behave differently from the z80.
- Page break penalties.
- Refresh delays.

***************************************************************************/

#include "emu.h"
#include "r800.h"
#include "r800dasm.h"

#include "z80.inc"

#define LOG_INT   (1U << 1) // z80.lst

//#define VERBOSE (LOG_INT)
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(R800, r800_device, "r800", "ASCII R800")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  r800_device - constructor
//-------------------------------------------------

r800_device::r800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, R800, tag, owner, clock)
{
	z80_set_m1_cycles(1);
	z80_set_memrq_cycles(1);
	z80_set_iorq_cycles(1);
}

std::unique_ptr<util::disasm_interface> r800_device::create_disassembler()
{
	return std::make_unique<r800_disassembler>();
}

void r800_device::device_validity_check(validity_checker &valid) const
{
	cpu_device::device_validity_check(valid);
}


/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define T(icount) { m_icount -= icount; }

/***************************************************************
 * SLL  r8
 ***************************************************************/
u8 r800_device::r800_sll(u8 value)
{
	const u8 c = (value & 0x80) ? CF : 0;
	const u8 res = u8(value << 1);
	set_f(SZP[res] | c);
	return res;
}

void r800_device::mulub(u8 value)
{
	const u16 res = A * value;
	HL = res;
	const u8 c = (H) ? CF : 0;
	const u8 z = (res) ? 0 : ZF;
	set_f((F & (HF|NF)) | z | c);
}

void r800_device::muluw(u16 value)
{
	const u32 res = HL * value;
	DE = res >> 16;
	HL = res & 0xffff;
	const u8 c = (DE) ? CF : 0;
	const u8 z = (res) ? 0 : ZF;
	set_f((F & (HF|NF)) | z | c);
}

void r800_device::do_op()
{
	#include "cpu/z80/r800.hxx"
}
