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

#define LOG_UNDOC (1U << 1)
#define LOG_INT   (1U << 2)
#define LOG_TIME  (1U << 3)

//#define VERBOSE ( LOG_UNDOC /*| LOG_INT*/ )
#include "logmacro.h"

#define LOGUNDOC(...) LOGMASKED(LOG_UNDOC, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)

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



#define HAS_LDAIR_QUIRK  0

/****************************************************************************
 * The Z80 registers. halt is set to 1 when the CPU is halted, the refresh
 * register is calculated as follows: refresh = (r & 127) | (r2 & 128)
 ****************************************************************************/
#define CF      0x01
#define NF      0x02
#define PF      0x04
#define VF      PF
#define XF      0x08
#define HF      0x10
#define YF      0x20
#define ZF      0x40
#define SF      0x80

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

#define PRVPC   m_prvpc.d     // previous program counter

#define PCD     m_pc.d
#define PC      m_pc.w.l

#define SPD     m_sp.d
#define SP      m_sp.w.l

#define AFD     m_af.d
#define AF      m_af.w.l
#define A       m_af.b.h
#define F       m_af.b.l
#define Q       m_q
#define QT      m_qtemp
#define I       m_i
#define R       m_r
#define R2      m_r2

#define BCD     m_bc.d
#define BC      m_bc.w.l
#define B       m_bc.b.h
#define C       m_bc.b.l

#define DED     m_de.d
#define DE      m_de.w.l
#define D       m_de.b.h
#define E       m_de.b.l

#define HLD     m_hl.d
#define HL      m_hl.w.l
#define H       m_hl.b.h
#define L       m_hl.b.l

#define IXD     m_ix.d
#define IX      m_ix.w.l
#define HX      m_ix.b.h
#define LX      m_ix.b.l

#define IYD     m_iy.d
#define IY      m_iy.w.l
#define HY      m_iy.b.h
#define LY      m_iy.b.l

#define WZ      m_wz.w.l
#define WZ_H    m_wz.b.h
#define WZ_L    m_wz.b.l

#define TADR     m_shared_addr.w   // Typically represents values from A0..15 pins. 16bit input in steps.
#define TADR_H   m_shared_addr.b.h
#define TADR_L   m_shared_addr.b.l
#define TDAT     m_shared_data.w   // 16bit input(if use as second parameter) or output in steps.
#define TDAT2    m_shared_data2.w
#define TDAT_H   m_shared_data.b.h
#define TDAT_L   m_shared_data.b.l
#define TDAT8    m_shared_data.b.l // Typically represents values from D0..8 pins. 8bit input or output in steps.


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
