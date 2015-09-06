// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
// V60.C
// Undiscover the beast!
// Main hacking and coding by Farfetch'd
// Portability fixes by R. Belmont
//
// Emulation for the NEC V60 (uPD70615) and V70 (uPD70632) CPUs
//

/*
Taken from the NEC Semiconductor Selection Guide Guide Book (Oct. 1995):

uPD70615 (V60)
Features:
- Virtual memory (paging method)
- Level protection architecture - 4-level hierarchical protection function
    for system multi-programming.
- Abundant general registers - Thirty two 32-bit general registers for
    optimizing compiler
- Refined instruction set - 2-address method: Arbitrary addressing mode
    can be used independently for source operand and destination operand.
- Abundant address modes and data types - Auto increment/decrement mode
    for string process, and memory indirect addressing for pointer operation
- High cost-to performance chip
- No multiprocessor system - no FRM function for increasing system
    reliability using two or more processors.
- No V20/V30 simulation mode
Address bus: 24 bits
Data bus: 16 bits
Memory space: 4G bytes
Operating frequency: 16 MHz
Package: 120-pin QFP

uPD70616 (V60)
Features:
- Virtual memory (paging method)
- Level protection architecture - 4-level hierarchical protection function
    for system multi-programming.
- Abundant general registers - Thirty two 32-bit general registers for
    optimizing compiler
- Refined instruction set - 2-address method: Arbitrary addressing mode
    can be used independently for source operand and destination operand.
- Abundant address modes and data types - Auto increment/decrement mode
    for string process, and memory indirect addressing for pointer operation
- Multiprocessor system - FRM function for increasing system reliability
    using two or more processors.
- V20/V30 simulation mode
Address bus: 24 bits
Data bus: 16 bits
Memory space: 4G bytes
Operating frequency: 16 MHz
Package: 68-pin PGA

uPD70632 (V70)
Features:
- Virtual memory (paging method)
- Level protection architecture - 4-level hierarchical protection function
    for system multi-programming.
- Abundant general registers - Thirty two 32-bit general registers for
    optimizing compiler
- Refined instruction set - 2-address method: Arbitrary addressing mode
    can be used independently for source operand and destination operand.
- Abundant address modes and data types - Auto increment/decrement mode
    for string process, and memory indirect addressing for pointer operation
- Multiprocessor system - FRM function for increasing system reliability
    using two or more processors.
- V20/V30 simulation mode
Address bus: 32 bits
Data bus: 32 bits
Memory space: 4G bytes
Operating frequency: 20 MHz
Package: 132-pin PGA, 200-pin QFP
*/

#include "emu.h"
#include "debugger.h"
#include "v60.h"

const device_type V60 = &device_creator<v60_device>;
const device_type V70 = &device_creator<v70_device>;


v60_device::v60_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, V60, "V60", tag, owner, clock, "v60", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_fetch_xor(BYTE_XOR_LE(0))
	, m_start_pc(0xfffff0)
{
	// Set m_PIR (Processor ID) for NEC m_ LSB is reserved to NEC,
	// so I don't know what it contains.
	m_reg[45] = 0x00006000;
}


v60_device::v60_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_fetch_xor(BYTE4_XOR_LE(0))
	, m_start_pc(0xfffffff0)
{
	// Set m_PIR (Processor ID) for NEC v70. LSB is reserved to NEC,
	// so I don't know what it contains.
	m_reg[45] = 0x00007000;
}

v70_device::v70_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: v60_device(mconfig, V70, "V70", tag, owner, clock, "v70", __FILE__)
{
}


offs_t v60_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( v60 );
	return CPU_DISASSEMBLE_NAME(v60)(this, buffer, pc, oprom, opram, options);
}


offs_t v70_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( v70 );
	return CPU_DISASSEMBLE_NAME(v70)(this, buffer, pc, oprom, opram, options);
}


// memory accessors
#if defined(LSB_FIRST) && !defined(ALIGN_INTS)
#define OpRead8(a)   (m_direct->read_byte(a))
#define OpRead16(a)  (m_direct->read_word(a))
#define OpRead32(a)  (m_direct->read_dword(a))
#else
#define OpRead8(a)   (m_direct->read_byte((a), m_fetch_xor))
#define OpRead16(a)  ((m_direct->read_byte(((a)+0), m_fetch_xor) << 0) | \
							(m_direct->read_byte(((a)+1), m_fetch_xor) << 8))
#define OpRead32(a)  ((m_direct->read_byte(((a)+0), m_fetch_xor) << 0) | \
							(m_direct->read_byte(((a)+1), m_fetch_xor) << 8) | \
							(m_direct->read_byte(((a)+2), m_fetch_xor) << 16) | \
							(m_direct->read_byte(((a)+3), m_fetch_xor) << 24))
#endif


// macros stolen from MAME for flags calc
// note that these types are in x86 naming:
// byte = 8 bit, word = 16 bit, long = 32 bit

// parameter x = result, y = source 1, z = source 2

#define SetOFL_Add(x, y,z)  (_OV = (((x) ^ (y)) & ((x) ^ (z)) & 0x80000000) ? 1: 0)
#define SetOFW_Add(x, y,z)  (_OV = (((x) ^ (y)) & ((x) ^ (z)) & 0x8000) ? 1 : 0)
#define SetOFB_Add(x, y,z)  (_OV = (((x) ^ (y)) & ((x) ^ (z)) & 0x80) ? 1 : 0)

#define SetOFL_Sub(x, y,z)  (_OV = (((z) ^ (y)) & ((z) ^ (x)) & 0x80000000) ? 1 : 0)
#define SetOFW_Sub(x, y,z)  (_OV = (((z) ^ (y)) & ((z) ^ (x)) & 0x8000) ? 1 : 0)
#define SetOFB_Sub(x, y,z)  (_OV = (((z) ^ (y)) & ((z) ^ (x)) & 0x80) ? 1 : 0)

#define SetCFB(x)           {_CY = ((x) & 0x100) ? 1 : 0; }
#define SetCFW(x)           {_CY = ((x) & 0x10000) ? 1 : 0; }
#define SetCFL(x)           {_CY = ((x) & (((UINT64)1) << 32)) ? 1 : 0; }

#define SetSF(x)            (_S = (x))
#define SetZF(x)            (_Z = (x))

#define SetSZPF_Byte(x)     {_Z = ((UINT8)(x) == 0);  _S = ((x)&0x80) ? 1 : 0; }
#define SetSZPF_Word(x)     {_Z = ((UINT16)(x) == 0);  _S = ((x)&0x8000) ? 1 : 0; }
#define SetSZPF_Long(x)     {_Z = ((UINT32)(x) == 0);  _S = ((x)&0x80000000) ? 1 : 0; }

#define ORB(dst, src)       { (dst) |= (src); _CY = _OV = 0; SetSZPF_Byte(dst); }
#define ORW(dst, src)       { (dst) |= (src); _CY = _OV = 0; SetSZPF_Word(dst); }
#define ORL(dst, src)       { (dst) |= (src); _CY = _OV = 0; SetSZPF_Long(dst); }

#define ANDB(dst, src)      { (dst) &= (src); _CY = _OV = 0; SetSZPF_Byte(dst); }
#define ANDW(dst, src)      { (dst) &= (src); _CY = _OV = 0; SetSZPF_Word(dst); }
#define ANDL(dst, src)      { (dst) &= (src); _CY = _OV = 0; SetSZPF_Long(dst); }

#define XORB(dst, src)      { (dst) ^= (src); _CY = _OV = 0; SetSZPF_Byte(dst); }
#define XORW(dst, src)      { (dst) ^= (src); _CY = _OV = 0; SetSZPF_Word(dst); }
#define XORL(dst, src)      { (dst) ^= (src); _CY = _OV = 0; SetSZPF_Long(dst); }

#define SUBB(dst, src)      { unsigned res = (dst) - (src); SetCFB(res); SetOFB_Sub(res, src, dst); SetSZPF_Byte(res); dst = (UINT8)res; }
#define SUBW(dst, src)      { unsigned res = (dst) - (src); SetCFW(res); SetOFW_Sub(res, src, dst); SetSZPF_Word(res); dst = (UINT16)res; }
#define SUBL(dst, src)      { UINT64 res = (UINT64)(dst) - (INT64)(src); SetCFL(res); SetOFL_Sub(res, src, dst); SetSZPF_Long(res); dst = (UINT32)res; }

#define ADDB(dst, src)      { unsigned res = (dst) + (src); SetCFB(res); SetOFB_Add(res, src, dst); SetSZPF_Byte(res); dst = (UINT8)res; }
#define ADDW(dst, src)      { unsigned res = (dst) + (src); SetCFW(res); SetOFW_Add(res, src, dst); SetSZPF_Word(res); dst = (UINT16)res; }
#define ADDL(dst, src)      { UINT64 res = (UINT64)(dst) + (UINT64)(src); SetCFL(res); SetOFL_Add(res, src, dst); SetSZPF_Long(res); dst = (UINT32)res; }

#define SETREG8(a, b)       (a) = ((a) & ~0xff) | ((b) & 0xff)
#define SETREG16(a, b)      (a) = ((a) & ~0xffff) | ((b) & 0xffff)


/*
 * Prevent warnings on NetBSD.  All identifiers beginning with an underscore
 * followed by an uppercase letter are reserved by the C standard (ISO / IEC
 * 9899:1999, 7.1.3) to be used by the implementation.  It'd be best to rename
 * all such instances, but this is less intrusive and error-prone.
 */
#undef _S

#define _CY     m_flags.CY
#define _OV     m_flags.OV
#define _S      m_flags.S
#define _Z      m_flags.Z


// Defines of all v60 register...
#define R0      m_reg[0]
#define R1      m_reg[1]
#define R2      m_reg[2]
#define R3      m_reg[3]
#define R4      m_reg[4]
#define R5      m_reg[5]
#define R6      m_reg[6]
#define R7      m_reg[7]
#define R8      m_reg[8]
#define R9      m_reg[9]
#define R10     m_reg[10]
#define R11     m_reg[11]
#define R12     m_reg[12]
#define R13     m_reg[13]
#define R14     m_reg[14]
#define R15     m_reg[15]
#define R16     m_reg[16]
#define R17     m_reg[17]
#define R18     m_reg[18]
#define R19     m_reg[19]
#define R20     m_reg[20]
#define R21     m_reg[21]
#define R22     m_reg[22]
#define R23     m_reg[23]
#define R24     m_reg[24]
#define R25     m_reg[25]
#define R26     m_reg[26]
#define R27     m_reg[27]
#define R28     m_reg[28]
#define AP      m_reg[29]
#define FP      m_reg[30]
#define SP      m_reg[31]

#define PC      m_reg[32]
#define PSW     m_reg[33]

// Privileged registers
#define ISP     m_reg[36]
#define L0SP    m_reg[37]
#define L1SP    m_reg[38]
#define L2SP    m_reg[39]
#define L3SP    m_reg[40]
#define SBR     m_reg[41]
#define TR      m_reg[42]
#define SYCW    m_reg[43]
#define TKCW    m_reg[44]
#define PIR     m_reg[45]
//10-14 reserved
#define PSW2    m_reg[51]
#define ATBR0   m_reg[52]
#define ATLR0   m_reg[53]
#define ATBR1   m_reg[54]
#define ATLR1   m_reg[55]
#define ATBR2   m_reg[56]
#define ATLR2   m_reg[57]
#define ATBR3   m_reg[58]
#define ATLR3   m_reg[59]
#define TRMODE  m_reg[60]
#define ADTR0   m_reg[61]
#define ADTR1   m_reg[62]
#define ADTMR0  m_reg[63]
#define ADTMR1  m_reg[64]
//29-31 reserved

// Defines...
#define NORMALIZEFLAGS() \
{ \
	_S    = _S  ? 1 : 0; \
	_OV   = _OV ? 1 : 0; \
	_Z    = _Z  ? 1 : 0; \
	_CY   = _CY ? 1 : 0; \
}


void v60_device::v60SaveStack()
{
	if (PSW & 0x10000000)
		ISP = SP;
	else
		m_reg[37 + ((PSW >> 24) & 3)] = SP;
}

void v60_device::v60ReloadStack()
{
	if (PSW & 0x10000000)
		SP = ISP;
	else
		SP = m_reg[37 + ((PSW >> 24) & 3)];
}

UINT32 v60_device::v60ReadPSW()
{
	PSW &= 0xfffffff0;
	PSW |= (_Z?1:0) | (_S?2:0) | (_OV?4:0) | (_CY?8:0);
	return PSW;
}

void v60_device::v60WritePSW(UINT32 newval)
{
	/* determine if we need to save / restore the stacks */
	int updateStack = 0;

	/* if the interrupt state is changing, we definitely need to update */
	if ((newval ^ PSW) & 0x10000000)
		updateStack = 1;

	/* if we are not in interrupt mode and the level is changing, we also must update */
	else if (!(PSW & 0x10000000) && ((newval ^ PSW) & 0x03000000))
		updateStack = 1;

	/* save the previous stack value */
	if (updateStack)
		v60SaveStack();

	/* set the new value and update the flags */
	PSW = newval;
	_Z =  (UINT8)(PSW & 1);
	_S =  (UINT8)(PSW & 2);
	_OV = (UINT8)(PSW & 4);
	_CY = (UINT8)(PSW & 8);

	/* fetch the new stack value */
	if (updateStack)
		v60ReloadStack();
}


UINT32 v60_device::v60_update_psw_for_exception(int is_interrupt, int target_level)
{
	UINT32 oldPSW = v60ReadPSW();
	UINT32 newPSW = oldPSW;

	// Change to interrupt context
	newPSW &= ~(3 << 24);  // PSW.EL = 0
	newPSW |= target_level << 24; // set target level
	newPSW &= ~(1 << 18);  // PSW.IE = 0
	newPSW &= ~(1 << 16);  // PSW.TE = 0
	newPSW &= ~(1 << 27);  // PSW.TP = 0
	newPSW &= ~(1 << 17);  // PSW.AE = 0
	newPSW &= ~(1 << 29);  // PSW.EM = 0
	if (is_interrupt)
		newPSW |=  (1 << 28);// PSW.IS = 1
	newPSW |=  (1 << 31);  // PSW.ASA = 1
	v60WritePSW(newPSW);

	return oldPSW;
}


#define GETINTVECT(nint)                    m_program->read_dword((SBR & ~0xfff) + (nint) * 4)
#define EXCEPTION_CODE_AND_SIZE(code, size) (((code) << 16) | (size))


// Addressing mode decoding functions
#include "am.inc"

// Opcode functions
#include "op12.inc"
#include "op2.inc"
#include "op3.inc"
#include "op4.inc"
#include "op5.inc"
#include "op6.inc"
#include "op7a.inc"

UINT32 v60_device::opUNHANDLED()
{
	fatalerror("Unhandled OpCode found : %02x at %08x\n", OpRead16(PC), PC);
	//return 0; /* never reached, fatalerror won't return */
}

// Opcode jump table
#include "optable.inc"

void v60_device::device_start()
{
	m_stall_io = 0;
	m_irq_line = CLEAR_LINE;
	m_nmi_line = CLEAR_LINE;

	for ( int i = 0; i < 68; i++ )
	{
		// Don't set SP (31), PCi (32), PSW (33), SBR (41), SYCW (43), TKCW (44), PIR (45), PSW2 (51)
		if ( i != 31 && i != 32 && i != 33 && i != 41 && i != 43 && i != 44 && i != 45 && i != 51 )
		{
			m_reg[i] = 0;
		}
	}

	m_flags.CY = 0;
	m_flags.OV = 0;
	m_flags.S = 0;
	m_flags.Z = 0;

	m_op1 = 0;
	m_op2 = 0;
	m_flag1 = 0;
	m_flag2 = 0;
	m_instflags = 0;
	m_lenop1 = 0;
	m_lenop2 = 0;
	m_subop = 0;
	m_bamoffset1 = 0;
	m_bamoffset2 = 0;
	m_amflag = 0;
	m_amout = 0;
	m_bamoffset = 0;
	m_amlength1 = 0;
	m_amlength2 = 0;
	m_modadd = 0;
	m_modm = 0;
	m_modval = 0;
	m_modval2 = 0;
	m_modwritevalb = 0;
	m_modwritevalh = 0;
	m_modwritevalw = 0;
	m_moddim = 0;

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	save_item(NAME(m_reg));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_nmi_line));
	save_item(NAME(m_PPC));
	save_item(NAME(_CY));
	save_item(NAME(_OV));
	save_item(NAME(_S));
	save_item(NAME(_Z));

	state_add( V60_R0,     "R0",     R0).formatstr("%08X");
	state_add( V60_R1,     "R1",     R1).formatstr("%08X");
	state_add( V60_R2,     "R2",     R2).formatstr("%08X");
	state_add( V60_R3,     "R3",     R3).formatstr("%08X");
	state_add( V60_R4,     "R4",     R4).formatstr("%08X");
	state_add( V60_R5,     "R5",     R5).formatstr("%08X");
	state_add( V60_R6,     "R6",     R6).formatstr("%08X");
	state_add( V60_R7,     "R7",     R7).formatstr("%08X");
	state_add( V60_R8,     "R8",     R8).formatstr("%08X");
	state_add( V60_R9,     "R9",     R9).formatstr("%08X");
	state_add( V60_R10,    "R10",    R10).formatstr("%08X");
	state_add( V60_R11,    "R11",    R11).formatstr("%08X");
	state_add( V60_R12,    "R12",    R12).formatstr("%08X");
	state_add( V60_R13,    "R13",    R13).formatstr("%08X");
	state_add( V60_R14,    "R14",    R14).formatstr("%08X");
	state_add( V60_R15,    "R15",    R15).formatstr("%08X");
	state_add( V60_R16,    "R16",    R16).formatstr("%08X");
	state_add( V60_R17,    "R17",    R17).formatstr("%08X");
	state_add( V60_R18,    "R18",    R18).formatstr("%08X");
	state_add( V60_R19,    "R19",    R19).formatstr("%08X");
	state_add( V60_R20,    "R20",    R20).formatstr("%08X");
	state_add( V60_R21,    "R21",    R21).formatstr("%08X");
	state_add( V60_R22,    "R22",    R22).formatstr("%08X");
	state_add( V60_R23,    "R23",    R23).formatstr("%08X");
	state_add( V60_R24,    "R24",    R24).formatstr("%08X");
	state_add( V60_R25,    "R25",    R25).formatstr("%08X");
	state_add( V60_R26,    "R26",    R26).formatstr("%08X");
	state_add( V60_R27,    "R27",    R27).formatstr("%08X");
	state_add( V60_R28,    "R28",    R28).formatstr("%08X");
	state_add( V60_AP,     "AP",     AP).formatstr("%08X");
	state_add( V60_FP,     "FP",     FP).formatstr("%08X");
	state_add( V60_SP,     "SP",     SP).formatstr("%08X");
	state_add( V60_PC,     "PC",     PC).formatstr("%08X");
	state_add( V60_PSW,    "PSW",    m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add( V60_ISP,    "ISP",    ISP).formatstr("%08X");
	state_add( V60_L0SP,   "L0SP",   L0SP).formatstr("%08X");
	state_add( V60_L1SP,   "L1SP",   L1SP).formatstr("%08X");
	state_add( V60_L2SP,   "L2SP",   L2SP).formatstr("%08X");
	state_add( V60_L3SP,   "L3SP",   L3SP).formatstr("%08X");
	state_add( V60_SBR,    "SBR",    SBR).formatstr("%08X");
	state_add( V60_TR,     "TR",     TR).formatstr("%08X");
	state_add( V60_SYCW,   "SYCW",   SYCW).formatstr("%08X");
	state_add( V60_TKCW,   "TKCW",   TKCW).formatstr("%08X");
	state_add( V60_PIR,    "PIR",    PIR).formatstr("%08X");
	state_add( V60_PSW2,   "PSW2",   PSW2).formatstr("%08X");
	state_add( V60_ATBR0,  "ATBR0",  ATBR0).formatstr("%08X");
	state_add( V60_ATLR0,  "ATLR0",  ATLR0).formatstr("%08X");
	state_add( V60_ATBR1,  "ATBR1",  ATBR1).formatstr("%08X");
	state_add( V60_ATLR1,  "ATLR1",  ATLR1).formatstr("%08X");
	state_add( V60_ATBR2,  "ATBR2",  ATBR2).formatstr("%08X");
	state_add( V60_ATLR2,  "ATLR2",  ATLR2).formatstr("%08X");
	state_add( V60_ATBR3,  "ATBR3",  ATBR3).formatstr("%08X");
	state_add( V60_ATLR3,  "ATLR3",  ATLR3).formatstr("%08X");
	state_add( V60_TRMODE, "TRMODE", TRMODE).formatstr("%08X");
	state_add( V60_ADTR0,  "ADTR0",  ADTR0).formatstr("%08X");
	state_add( V60_ADTR1,  "ADTR1",  ADTR1).formatstr("%08X");
	state_add( V60_ADTMR0, "ADTMR0", ADTMR0).formatstr("%08X");
	state_add( V60_ADTMR1, "ADTMR1", ADTMR1).formatstr("%08X");

	state_add( STATE_GENPC, "GENPC", PC).noshow();
	state_add( STATE_GENPCBASE, "GENPCBASE", m_PPC ).noshow();
	state_add( STATE_GENSP, "GENSP", SP ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).noshow();

	m_icountptr = &m_icount;
}


void v60_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case V60_PSW:
			m_debugger_temp = v60ReadPSW();
			break;
	}
}


void v60_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case V60_PSW:
			v60WritePSW( m_debugger_temp );
			break;
	}
}


void v60_device::device_reset()
{
	PSW   = 0x10000000;
	PC    = m_start_pc;
	SBR   = 0x00000000;
	SYCW  = 0x00000070;
	TKCW  = 0x0000e000;
	PSW2  = 0x0000f002;

	_CY   = 0;
	_OV   = 0;
	_S    = 0;
	_Z    = 0;
}


void v60_device::stall()
{
	m_stall_io = 1;
}


void v60_device::v60_do_irq(int vector)
{
	UINT32 oldPSW = v60_update_psw_for_exception(1, 0);

	// Push PC and PSW onto the stack
	SP-=4;
	m_program->write_dword_unaligned(SP, oldPSW);
	SP-=4;
	m_program->write_dword_unaligned(SP, PC);

	// Jump to vector for user interrupt
	PC = GETINTVECT(vector);
}

void v60_device::v60_try_irq()
{
	if(m_irq_line == CLEAR_LINE)
		return;
	if((PSW & (1 << 18)) != 0) {
		int vector;
		if(m_irq_line != ASSERT_LINE)
			m_irq_line = CLEAR_LINE;

		vector = standard_irq_callback(0);

		v60_do_irq(vector + 0x40);
	}
}

void v60_device::execute_set_input(int irqline, int state)
{
	if(irqline == INPUT_LINE_NMI) {
		switch(state) {
		case ASSERT_LINE:
			if(m_nmi_line == CLEAR_LINE) {
				m_nmi_line = ASSERT_LINE;
				v60_do_irq(2);
			}
			break;
		case CLEAR_LINE:
			m_nmi_line = CLEAR_LINE;
			break;
		}
	} else {
		m_irq_line = state;
		v60_try_irq();
	}
}

// Actual cycles / instruction is unknown

void v60_device::execute_run()
{
	if (m_irq_line != CLEAR_LINE)
		v60_try_irq();

	while (m_icount > 0)
	{
		UINT32 inc;
		m_PPC = PC;
		debugger_instruction_hook(this, PC);
		m_icount -= 8;  /* fix me -- this is just an average */
		inc = (this->*s_OpCodeTable[OpRead8(PC)])();
		PC += inc;
		if (m_irq_line != CLEAR_LINE)
			v60_try_irq();
	}
}
