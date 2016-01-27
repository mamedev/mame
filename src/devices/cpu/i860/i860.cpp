// license:BSD-3-Clause
// copyright-holders:Jason Eckhardt
/***************************************************************************

    i860.c

    Interface file for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)

***************************************************************************/

/*
TODO: Separate out i860XR and i860XP (make different types, etc).
      Hook IRQ lines into MAME core (they're custom functions atm).
*/

#include "emu.h"
#include "debugger.h"
#include "i860.h"


/* Control register numbers.  */
enum {
	CR_FIR     = 0,
	CR_PSR     = 1,
	CR_DIRBASE = 2,
	CR_DB      = 3,
	CR_FSR     = 4,
	CR_EPSR    = 5
};


const device_type I860 = &device_creator<i860_cpu_device>;


i860_cpu_device::i860_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, I860, "i860XR", tag, owner, clock, "i860xr", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 64, 32, 0), m_pc(0), m_merge(0), m_pin_bus_hold(0), m_pin_reset(0), m_exiting_readmem(0), m_exiting_ifetch(0), m_pc_updated(0), m_pending_trap(0), m_fir_gets_trap_addr(0), m_single_stepping(0), m_program(nullptr), m_icount(0)
{
}


void i860_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	reset_i860();
	i860_set_pin(DEC_PIN_BUS_HOLD, 0);
	i860_set_pin(DEC_PIN_RESET, 0);
	m_single_stepping = 0;

	save_item(NAME(m_iregs));
	save_item(NAME(m_cregs));
	save_item(NAME(m_frg));
	save_item(NAME(m_pc));

	state_add( I860_PC,      "PC",      m_pc).formatstr("%08X");
	state_add( I860_FIR,     "FIR",     m_cregs[CR_FIR]).formatstr("%08X");
	state_add( I860_PSR,     "PSR",     m_cregs[CR_PSR]).formatstr("%08X");
	state_add( I860_DIRBASE, "DIRBASE", m_cregs[CR_DIRBASE]).formatstr("%08X");
	state_add( I860_DB,      "DB",      m_cregs[CR_DB]).formatstr("%08X");
	state_add( I860_FSR,     "FSR",     m_cregs[CR_FSR]).formatstr("%08X");
	state_add( I860_EPSR,    "EPSR",    m_cregs[CR_EPSR]).formatstr("%08X");
	state_add( I860_R0,      "R0",      m_iregs[0]).formatstr("%08X");
	state_add( I860_R1,      "R1",      m_iregs[1]).formatstr("%08X");
	state_add( I860_R2,      "R2",      m_iregs[2]).formatstr("%08X");
	state_add( I860_R3,      "R3",      m_iregs[3]).formatstr("%08X");
	state_add( I860_R4,      "R4",      m_iregs[4]).formatstr("%08X");
	state_add( I860_R5,      "R5",      m_iregs[5]).formatstr("%08X");
	state_add( I860_R6,      "R6",      m_iregs[6]).formatstr("%08X");
	state_add( I860_R7,      "R7",      m_iregs[7]).formatstr("%08X");
	state_add( I860_R8,      "R8",      m_iregs[8]).formatstr("%08X");
	state_add( I860_R9,      "R9",      m_iregs[9]).formatstr("%08X");
	state_add( I860_R10,     "R10",     m_iregs[10]).formatstr("%08X");
	state_add( I860_R11,     "R11",     m_iregs[11]).formatstr("%08X");
	state_add( I860_R12,     "R12",     m_iregs[12]).formatstr("%08X");
	state_add( I860_R13,     "R13",     m_iregs[13]).formatstr("%08X");
	state_add( I860_R14,     "R14",     m_iregs[14]).formatstr("%08X");
	state_add( I860_R15,     "R15",     m_iregs[15]).formatstr("%08X");
	state_add( I860_R16,     "R16",     m_iregs[16]).formatstr("%08X");
	state_add( I860_R17,     "R17",     m_iregs[17]).formatstr("%08X");
	state_add( I860_R18,     "R18",     m_iregs[18]).formatstr("%08X");
	state_add( I860_R19,     "R19",     m_iregs[19]).formatstr("%08X");
	state_add( I860_R20,     "R20",     m_iregs[20]).formatstr("%08X");
	state_add( I860_R21,     "R21",     m_iregs[21]).formatstr("%08X");
	state_add( I860_R22,     "R22",     m_iregs[22]).formatstr("%08X");
	state_add( I860_R23,     "R23",     m_iregs[23]).formatstr("%08X");
	state_add( I860_R24,     "R24",     m_iregs[24]).formatstr("%08X");
	state_add( I860_R25,     "R25",     m_iregs[25]).formatstr("%08X");
	state_add( I860_R26,     "R26",     m_iregs[26]).formatstr("%08X");
	state_add( I860_R27,     "R27",     m_iregs[27]).formatstr("%08X");
	state_add( I860_R28,     "R28",     m_iregs[28]).formatstr("%08X");
	state_add( I860_R29,     "R29",     m_iregs[29]).formatstr("%08X");
	state_add( I860_R30,     "R30",     m_iregs[30]).formatstr("%08X");
	state_add( I860_R31,     "R31",     m_iregs[31]).formatstr("%08X");

	state_add( I860_F0,  "F0",  m_freg[0]).callimport().callexport().formatstr("%08X");
	state_add( I860_F1,  "F1",  m_freg[1]).callimport().callexport().formatstr("%08X");
	state_add( I860_F2,  "F2",  m_freg[2]).callimport().callexport().formatstr("%08X");
	state_add( I860_F3,  "F3",  m_freg[3]).callimport().callexport().formatstr("%08X");
	state_add( I860_F4,  "F4",  m_freg[4]).callimport().callexport().formatstr("%08X");
	state_add( I860_F5,  "F5",  m_freg[5]).callimport().callexport().formatstr("%08X");
	state_add( I860_F6,  "F6",  m_freg[6]).callimport().callexport().formatstr("%08X");
	state_add( I860_F7,  "F7",  m_freg[7]).callimport().callexport().formatstr("%08X");
	state_add( I860_F8,  "F8",  m_freg[8]).callimport().callexport().formatstr("%08X");
	state_add( I860_F9,  "F9",  m_freg[9]).callimport().callexport().formatstr("%08X");
	state_add( I860_F10, "F10", m_freg[10]).callimport().callexport().formatstr("%08X");
	state_add( I860_F11, "F11", m_freg[11]).callimport().callexport().formatstr("%08X");
	state_add( I860_F12, "F12", m_freg[12]).callimport().callexport().formatstr("%08X");
	state_add( I860_F13, "F13", m_freg[13]).callimport().callexport().formatstr("%08X");
	state_add( I860_F14, "F14", m_freg[14]).callimport().callexport().formatstr("%08X");
	state_add( I860_F15, "F15", m_freg[15]).callimport().callexport().formatstr("%08X");
	state_add( I860_F16, "F16", m_freg[16]).callimport().callexport().formatstr("%08X");
	state_add( I860_F17, "F17", m_freg[17]).callimport().callexport().formatstr("%08X");
	state_add( I860_F18, "F18", m_freg[18]).callimport().callexport().formatstr("%08X");
	state_add( I860_F19, "F19", m_freg[19]).callimport().callexport().formatstr("%08X");
	state_add( I860_F20, "F20", m_freg[20]).callimport().callexport().formatstr("%08X");
	state_add( I860_F21, "F21", m_freg[21]).callimport().callexport().formatstr("%08X");
	state_add( I860_F22, "F22", m_freg[22]).callimport().callexport().formatstr("%08X");
	state_add( I860_F23, "F23", m_freg[23]).callimport().callexport().formatstr("%08X");
	state_add( I860_F24, "F24", m_freg[24]).callimport().callexport().formatstr("%08X");
	state_add( I860_F25, "F25", m_freg[25]).callimport().callexport().formatstr("%08X");
	state_add( I860_F26, "F26", m_freg[26]).callimport().callexport().formatstr("%08X");
	state_add( I860_F27, "F27", m_freg[27]).callimport().callexport().formatstr("%08X");
	state_add( I860_F28, "F28", m_freg[28]).callimport().callexport().formatstr("%08X");
	state_add( I860_F29, "F29", m_freg[29]).callimport().callexport().formatstr("%08X");
	state_add( I860_F30, "F30", m_freg[30]).callimport().callexport().formatstr("%08X");
	state_add( I860_F31, "F31", m_freg[31]).callimport().callexport().formatstr("%08X");

	state_add(STATE_GENPC, "curpc", m_pc).noshow();

	m_icountptr = &m_icount;
}


void i860_cpu_device::state_import(const device_state_entry &entry)
{
#define I860_SET_INFO_F(fnum) m_frg[0+(4*fnum)] = (m_freg[fnum] & 0x000000ff);       \
								m_frg[1+(4*fnum)] = (m_freg[fnum] & 0x0000ff00) >> 8;  \
								m_frg[2+(4*fnum)] = (m_freg[fnum] & 0x00ff0000) >> 16; \
								m_frg[3+(4*fnum)] = (m_freg[fnum] & 0xff000000) >> 24;

	switch (entry.index())
	{
		case I860_F0:  I860_SET_INFO_F(0);  break;
		case I860_F1:  I860_SET_INFO_F(1);  break;
		case I860_F2:  I860_SET_INFO_F(2);  break;
		case I860_F3:  I860_SET_INFO_F(3);  break;
		case I860_F4:  I860_SET_INFO_F(4);  break;
		case I860_F5:  I860_SET_INFO_F(5);  break;
		case I860_F6:  I860_SET_INFO_F(6);  break;
		case I860_F7:  I860_SET_INFO_F(7);  break;
		case I860_F8:  I860_SET_INFO_F(8);  break;
		case I860_F9:  I860_SET_INFO_F(9);  break;
		case I860_F10: I860_SET_INFO_F(10); break;
		case I860_F11: I860_SET_INFO_F(11); break;
		case I860_F12: I860_SET_INFO_F(12); break;
		case I860_F13: I860_SET_INFO_F(13); break;
		case I860_F14: I860_SET_INFO_F(14); break;
		case I860_F15: I860_SET_INFO_F(15); break;
		case I860_F16: I860_SET_INFO_F(16); break;
		case I860_F17: I860_SET_INFO_F(17); break;
		case I860_F18: I860_SET_INFO_F(18); break;
		case I860_F19: I860_SET_INFO_F(19); break;
		case I860_F20: I860_SET_INFO_F(20); break;
		case I860_F21: I860_SET_INFO_F(21); break;
		case I860_F22: I860_SET_INFO_F(22); break;
		case I860_F23: I860_SET_INFO_F(23); break;
		case I860_F24: I860_SET_INFO_F(24); break;
		case I860_F25: I860_SET_INFO_F(25); break;
		case I860_F26: I860_SET_INFO_F(26); break;
		case I860_F27: I860_SET_INFO_F(27); break;
		case I860_F28: I860_SET_INFO_F(28); break;
		case I860_F29: I860_SET_INFO_F(29); break;
		case I860_F30: I860_SET_INFO_F(30); break;
		case I860_F31: I860_SET_INFO_F(31); break;
	}
}

void i860_cpu_device::state_export(const device_state_entry &entry)
{
#define I860_GET_INFO_F(fnum) m_freg[fnum] = m_frg[0+(4*fnum)] | ( m_frg[1+(4*fnum)] << 8 ) | ( m_frg[2+(4*fnum)] << 16 ) | ( m_frg[3+(4*fnum)] << 24)

	switch (entry.index())
	{
		case I860_F0:  I860_GET_INFO_F(0);  break;
		case I860_F1:  I860_GET_INFO_F(1);  break;
		case I860_F2:  I860_GET_INFO_F(2);  break;
		case I860_F3:  I860_GET_INFO_F(3);  break;
		case I860_F4:  I860_GET_INFO_F(4);  break;
		case I860_F5:  I860_GET_INFO_F(5);  break;
		case I860_F6:  I860_GET_INFO_F(6);  break;
		case I860_F7:  I860_GET_INFO_F(7);  break;
		case I860_F8:  I860_GET_INFO_F(8);  break;
		case I860_F9:  I860_GET_INFO_F(9);  break;
		case I860_F10: I860_GET_INFO_F(10); break;
		case I860_F11: I860_GET_INFO_F(11); break;
		case I860_F12: I860_GET_INFO_F(12); break;
		case I860_F13: I860_GET_INFO_F(13); break;
		case I860_F14: I860_GET_INFO_F(14); break;
		case I860_F15: I860_GET_INFO_F(15); break;
		case I860_F16: I860_GET_INFO_F(16); break;
		case I860_F17: I860_GET_INFO_F(17); break;
		case I860_F18: I860_GET_INFO_F(18); break;
		case I860_F19: I860_GET_INFO_F(19); break;
		case I860_F20: I860_GET_INFO_F(20); break;
		case I860_F21: I860_GET_INFO_F(21); break;
		case I860_F22: I860_GET_INFO_F(22); break;
		case I860_F23: I860_GET_INFO_F(23); break;
		case I860_F24: I860_GET_INFO_F(24); break;
		case I860_F25: I860_GET_INFO_F(25); break;
		case I860_F26: I860_GET_INFO_F(26); break;
		case I860_F27: I860_GET_INFO_F(27); break;
		case I860_F28: I860_GET_INFO_F(28); break;
		case I860_F29: I860_GET_INFO_F(29); break;
		case I860_F30: I860_GET_INFO_F(30); break;
		case I860_F31: I860_GET_INFO_F(31); break;
	}
}

void i860_cpu_device::device_reset()
{
	reset_i860();
}


offs_t i860_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( i860 );
	return CPU_DISASSEMBLE_NAME(i860)(this, buffer, pc, oprom, opram, options);
}


/**************************************************************************
 * The actual decode and execute code.
 **************************************************************************/
#include "i860dec.inc"
