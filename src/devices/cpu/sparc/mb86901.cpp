// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  mb86901.cpp - Emulation for the Fujitsu MB86901 / LSI L64801
//                processors. Both chips are identical both
//                electrically and functionally, and implement
//                the integer instructions in a SPARC v7
//                compatible instruction set.
//
//  To-Do:
//      - Ops: FBFcc, LDF, STF
//		- Test: SPARCv8 ops are untested
//		- Test: Traps are untested
//      - FPU support
//      - Coprocessor support
//
//================================================================

#include "emu.h"
#include "debugger.h"
#include "sparc.h"
#include "sparcdefs.h"

#define SPARCV8		(0)

const device_type MB86901 = &device_creator<mb86901_device>;

const int mb86901_device::WINDOW_COUNT = 7;

//-------------------------------------------------
//  mb86901_device - constructor
//-------------------------------------------------

mb86901_device::mb86901_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, MB86901, "Fujitsu MB86901", tag, owner, clock, "mb86901", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32)
	, m_dasm(7)
{
}


void mb86901_device::device_start()
{
	m_trap_priorities[0]  = 1;
	m_trap_priorities[1]  = 2;
	m_trap_priorities[2]  = 3;
	m_trap_priorities[3]  = 4;
	m_trap_priorities[4]  = 5;
	m_trap_priorities[5]  = 6;
	m_trap_priorities[6]  = 6;
	m_trap_priorities[7]  = 7;
	m_trap_priorities[8]  = 8;
	m_trap_priorities[9]  = 10;
	m_trap_priorities[10] = 11;
	for (int i = 11; i <= 16; i++)
		m_trap_priorities[i] = 31;
	m_trap_priorities[17] = 27;
	m_trap_priorities[18] = 26;
	m_trap_priorities[19] = 25;
	m_trap_priorities[20] = 24;
	m_trap_priorities[21] = 23;
	m_trap_priorities[22] = 22;
	m_trap_priorities[23] = 21;
	m_trap_priorities[24] = 20;
	m_trap_priorities[25] = 19;
	m_trap_priorities[26] = 18;
	m_trap_priorities[27] = 17;
	m_trap_priorities[28] = 16;
	m_trap_priorities[29] = 15;
	m_trap_priorities[30] = 14;
	m_trap_priorities[31] = 13;
	for (int i = 32; i < 128; i++)
		m_trap_priorities[i] = 31;
	for (int i = 128; i < 256; i++)
		m_trap_priorities[i] = 12;

	memset(m_dbgregs, 0, 24 * sizeof(UINT32));

	m_program = &space(AS_PROGRAM);

	// register our state for the debugger
	state_add(STATE_GENPC,		"GENPC",	m_pc).noshow();
	state_add(STATE_GENFLAGS,	"GENFLAGS",	m_psr).callimport().callexport().formatstr("%6s").noshow();
	state_add(SPARC_PC,			"PC",		m_pc).formatstr("%08X");
	state_add(SPARC_NPC,		"nPC",		m_npc).formatstr("%08X");
	state_add(SPARC_PSR,		"PSR",		m_psr).formatstr("%08X");
	state_add(SPARC_WIM,		"WIM",		m_wim).formatstr("%08X");
	state_add(SPARC_TBR,		"TBR",		m_tbr).formatstr("%08X");
	state_add(SPARC_Y,			"Y",		m_y).formatstr("%08X");
	state_add(SPARC_ICC,		"icc",		m_icc).formatstr("%4s");
	state_add(SPARC_CWP,		"CWP",		m_cwp).formatstr("%2d");
	char regname[3] = "g0";
	for (int i = 0; i < 8; i++)
	{
		regname[1] = 0x30 + i;
		state_add(SPARC_G0 + i, regname, m_r[i]).formatstr("%08X");
	}
	regname[0] = 'o';
	for (int i = 0; i < 8; i++)
	{
		regname[1] = 0x30 + i;
		state_add(SPARC_O0 + i, regname, m_dbgregs[i]).formatstr("%08X");
	}

	regname[0] = 'l';
	for (int i = 0; i < 8; i++)
	{
		regname[1] = 0x30 + i;
		state_add(SPARC_L0 + i, regname, m_dbgregs[8+i]).formatstr("%08X");
	}
	regname[0] = 'i';
	for (int i = 0; i < 8; i++)
	{
		regname[1] = 0x30 + i;
		state_add(SPARC_I0 + i, regname, m_dbgregs[16+i]).formatstr("%08X");
	}

	state_add(SPARC_EC,		"EC",		m_ec).formatstr("%1d");
	state_add(SPARC_EF,		"EF",		m_ef).formatstr("%1d");
	state_add(SPARC_ET,		"ET",		m_et).formatstr("%1d");
	state_add(SPARC_PIL,	"PIL",		m_pil).formatstr("%2d");
	state_add(SPARC_S,		"S",		m_s).formatstr("%1d");
	state_add(SPARC_PS,		"PS",		m_ps).formatstr("%1d");

	char rname[5];
	for (int i = 0; i < 120; i++)
	{
		sprintf(rname, "r%d", i);
		state_add(SPARC_R0 + i, rname, m_r[i]).formatstr("%08X");
	}

	// register with the savestate system
	save_item(NAME(m_r));
	save_item(NAME(m_pc));
	save_item(NAME(m_npc));
	save_item(NAME(m_psr));
	save_item(NAME(m_wim));
	save_item(NAME(m_tbr));
	save_item(NAME(m_y));
	save_item(NAME(m_impl));
	save_item(NAME(m_ver));
	save_item(NAME(m_icc));
	save_item(NAME(m_ec));
	save_item(NAME(m_ef));
	save_item(NAME(m_pil));
	save_item(NAME(m_s));
	save_item(NAME(m_ps));
	save_item(NAME(m_et));
	save_item(NAME(m_cwp));
	save_item(NAME(m_insn_asi));
	save_item(NAME(m_data_asi));
	save_item(NAME(m_asi));
	save_item(NAME(m_queued_tt));
	save_item(NAME(m_queued_priority));

	// set our instruction counter
	m_icountptr = &m_icount;
}

void mb86901_device::device_stop()
{
}

void mb86901_device::device_reset()
{
	m_queued_tt = 0;
	m_queued_priority = SPARC_NO_TRAP;
	m_asi = 0;
	MAE = false;
	HOLD_BUS = false;

	PC = 0;
	nPC = 4;
	memset(m_r, 0, sizeof(UINT32) * 120);

	m_wim = 0;
	m_tbr = 0;
	m_y = 0;

	m_impl = 0;
	m_ver = 0;
	m_icc = 0;
	m_ec = false;
	m_ef = false;
	m_pil = 0;
	m_s = true;
	m_ps = true;
	m_et = false;
	m_cwp = 0;

	m_insn_asi = 9;
	m_data_asi = 11;

	MAKE_PSR;
	for (int i = 0; i < 8; i++)
	{
		m_regs[i] = m_r + i;
	}
	update_gpr_pointers();
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *mb86901_device::memory_space_config(address_spacenum spacenum) const
{
	switch (spacenum)
	{
		case AS_PROGRAM:
			return &m_program_config;
		default:
			return nullptr;
	}
}

//-------------------------------------------------
//  read_byte - read an 8-bit value from a given
//  address space
//-------------------------------------------------

UINT32 mb86901_device::read_byte(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return m_program->read_byte(address);
}

INT32 mb86901_device::read_signed_byte(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return (((INT32)m_program->read_byte(address) << 24) >> 24);
}

UINT32 mb86901_device::read_half(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return m_program->read_word(address);
}

INT32 mb86901_device::read_signed_half(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return (((INT32)m_program->read_word(address) << 16) >> 16);
}

UINT32 mb86901_device::read_word(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return m_program->read_dword(address);
}

UINT64 mb86901_device::read_doubleword(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return (((UINT64)m_program->read_dword(address) << 32) | m_program->read_dword(address+4));
}

void mb86901_device::write_byte(UINT8 asi, UINT32 address, UINT8 data)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	m_program->write_byte(address, data);
}

void mb86901_device::write_half(UINT8 asi, UINT32 address, UINT16 data)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	m_program->write_word(address, data);
}

void mb86901_device::write_word(UINT8 asi, UINT32 address, UINT32 data)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	m_program->write_dword(address, data);
}

void mb86901_device::write_doubleword(UINT8 asi, UINT32 address, UINT64 data)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	m_program->write_dword(address, (UINT32)(data >> 32));
	m_program->write_dword(address+4, (UINT32)data);
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void mb86901_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		case SPARC_ICC:
			str = string_format("%c%c%c%c", ICC_N_SET ? 'n' : ' ', ICC_Z_SET ? 'z' : ' ', ICC_V_SET ? 'v' : ' ', ICC_C_SET ? 'c' : ' ');
			break;
		case SPARC_O0:	case SPARC_O1:	case SPARC_O2:	case SPARC_O3:	case SPARC_O4:	case SPARC_O5:	case SPARC_O6:	case SPARC_O7:
			str = string_format("%08X", m_dbgregs[entry.index() - SPARC_O0]);
			break;
		case SPARC_L0:	case SPARC_L1:	case SPARC_L2:	case SPARC_L3:	case SPARC_L4:	case SPARC_L5:	case SPARC_L6:	case SPARC_L7:
			str = string_format("%08X", m_dbgregs[8 + (entry.index() - SPARC_L0)]);
			break;
		case SPARC_I0:	case SPARC_I1:	case SPARC_I2:	case SPARC_I3:	case SPARC_I4:	case SPARC_I5:	case SPARC_I6:	case SPARC_I7:
			str = string_format("%08X", m_dbgregs[16 + (entry.index() - SPARC_I0)]);
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 mb86901_device::disasm_min_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 mb86901_device::disasm_max_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t mb86901_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	UINT32 op = *reinterpret_cast<const UINT32 *>(oprom);
	return m_dasm.dasm(buffer, pc, BIG_ENDIANIZE_INT32(op));
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 mb86901_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 mb86901_device::execute_max_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 mb86901_device::execute_input_lines() const
{
	return 0;
}


//-------------------------------------------------
//  execute_set_input - set the state of an input
//  line during execution
//-------------------------------------------------

void mb86901_device::execute_set_input(int inputnum, int state)
{
}


//-------------------------------------------------
//  save_restore_update_cwp - update cwp after
//  a save or restore opcode with no trap taken
//-------------------------------------------------

void mb86901_device::save_restore_update_cwp(UINT32 op, UINT8 new_cwp)
{
	UINT32 arg1 = RS1REG;
	UINT32 arg2 = USEIMM ? SIMM13 : RS2REG;

	m_cwp = new_cwp;
	MAKE_PSR;
	update_gpr_pointers();

	SET_RDREG(arg1 + arg2);
}

//-------------------------------------------------
//  execute_group2 - execute an opcode in group 2,
//  mostly ALU ops
//-------------------------------------------------

bool mb86901_device::execute_group2(UINT32 op)
{
	UINT32 arg1 = RS1REG;
	UINT32 arg2 = USEIMM ? SIMM13 : RS2REG;

	switch (OP3)
	{
		case 0: 	// add
			SET_RDREG(arg1 + arg2);
			break;
		case 1: 	// and
			SET_RDREG(arg1 & arg2);
			break;
		case 2:		// or
			SET_RDREG(arg1 | arg2);
			break;
		case 3:		// xor
			SET_RDREG(arg1 ^ arg2);
			break;
		case 4:		// sub
			SET_RDREG(arg1 - arg2);
			break;
		case 5:		// andn
			SET_RDREG(arg1 & ~arg2);
			break;
		case 6:		// orn
			SET_RDREG(arg1 | ~arg2);
			break;
		case 7:		// xnor
			SET_RDREG(arg1 ^ ~arg2);
			break;
		case 8:		// addx
			SET_RDREG(arg1 + arg2 + (ICC_C_SET ? 1 : 0));
			break;
#if SPARCV8
		case 10:	// umul, SPARCv8
		{
			UINT64 result = (UINT64)arg1 * (UINT64)arg2;
			Y = (UINT32)(result >> 32);
			SET_RDREG((UINT32)result);
			break;
		}
		case 11:	// smul, SPARCv8
		{
			INT64 result = (INT64)(INT32)arg1 * (INT64)(INT32)arg2;
			Y = (UINT32)((UINT64)result >> 32);
			SET_RDREG((UINT32)result);
			break;
		}
#endif
		case 12:	// subx
			SET_RDREG(arg1 - arg2 - (ICC_C_SET ? 1 : 0));
			break;
#if SPARCV8
		case 14:	// udiv, SPARCv8
		{
			UINT64 dividend = ((UINT64)Y << 32) || arg1;
			UINT32 divisor = arg2;
			UINT64 quotient = dividend / divisor;
			if (quotient > (0xffffffffL + (divisor - 1)))
			{
				quotient = 0xffffffff;
			}
			SET_RDREG((UINT32)quotient);
			break;
		}
		case 15:	// sdiv, SPARCv8
		{
			INT64 dividend = ((INT64)(INT32)Y << 32) || arg1;
			INT32 divisor = arg2;
			INT64 quotient = dividend / divisor;
			if (quotient > 0)
			{
				INT32 absdivisor = (divisor < 0) ? -divisor : divisor;
				if (quotient > (0x7fffffffL + (absdivisor - 1)))
				{
					quotient = 0x7fffffff;
				}
			}
			else if (quotient < 0)
			{
				if (quotient < (INT64)0xffffffff80000000L)
				{
					quotient = 0x80000000;
				}
			}
			SET_RDREG((UINT32)quotient);
			break;
		}
#endif
		case 16:	// addcc
		{
			UINT32 result = arg1 + arg2;
			TEST_ICC_NZ(result);
			if ((arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg2 & 0x80000000) != (result & 0x80000000))
				SET_ICC_V_FLAG;
			if (result < arg1 || result < arg2)
				SET_ICC_C_FLAG;
			SET_RDREG(result);
			break;
		}
		case 17:	// andcc
		{
			UINT32 result = arg1 & arg2;
			TEST_ICC_NZ(result);
			SET_RDREG(result);
			break;
		}
		case 18:	// orcc
		{
			UINT32 result = arg1 | arg2;
			TEST_ICC_NZ(result);
			SET_RDREG(result);
			break;
		}
		case 19:	// xorcc
		{
			UINT32 result = arg1 ^ arg2;
			TEST_ICC_NZ(result);
			SET_RDREG(result);
			break;
		}
		case 20:	// subcc
		{
			UINT32 result = arg1 - arg2;
			TEST_ICC_NZ(result);
			if ((arg1 & 0x80000000) != (arg2 & 0x80000000) && (arg2 & 0x80000000) != (result & 0x80000000))
				SET_ICC_V_FLAG;
			if (result > arg1)
				SET_ICC_C_FLAG;
			SET_RDREG(result);
			break;
		}
		case 21:	// andncc
		{
			UINT32 result = arg1 & ~arg2;
			TEST_ICC_NZ(result);
			SET_RDREG(result);
			break;
		}
		case 22:	// orncc
		{
			UINT32 result = arg1 | ~arg2;
			TEST_ICC_NZ(result);
			SET_RDREG(result);
			break;
		}
		case 23:	// xnorcc
		{
			UINT32 result = arg1 ^ ~arg2;
			TEST_ICC_NZ(result);
			SET_RDREG(result);
			break;
		}
		case 24:	// addxcc
		{
			UINT32 c = (ICC_C_SET ? 1 : 0);
			UINT32 argt = arg2 + c;
			UINT32 result = arg1 + argt;
			TEST_ICC_NZ(result);
			if (((arg1 & 0x80000000) == (argt & 0x80000000) && (arg1 & 0x80000000) != (result & 0x80000000)) || (c != 0 && arg2 == 0x7fffffff))
				SET_ICC_V_FLAG;
			if (result < arg1 || result < arg2 || (result == arg1 && arg2 != 0))
				SET_ICC_C_FLAG;
			SET_RDREG(result);
			break;
		}
#if SPARCV8
		case 26:	// umulcc, SPARCv8
		{
			UINT64 result = (UINT64)arg1 * (UINT64)arg2;
			Y = (UINT32)(result >> 32);
			TEST_ICC_NZ(result);
			SET_RDREG((UINT32)result);
			break;
		}
		case 27:	// smulcc, SPARCv8
		{
			INT64 result = (INT64)(INT32)arg1 * (INT64)(INT32)arg2;
			Y = (UINT32)((UINT64)result >> 32);
			TEST_ICC_NZ(result);
			SET_RDREG((UINT32)result);
			break;
		}
#endif
		case 28:	// subxcc
		{
			UINT32 c = (ICC_C_SET ? 1 : 0);
			UINT32 argt = arg2 + c;
			UINT32 result = arg1 - argt;
			TEST_ICC_NZ(result);
			if (((arg1 & 0x80000000) != (argt & 0x80000000) && (arg1 & 0x80000000) != (result & 0x80000000)) || (c != 0 && arg2 == 0x80000000))
				SET_ICC_V_FLAG;
			if (result > arg1 || (result == arg1 && arg2 != 0))
				SET_ICC_C_FLAG;
			SET_RDREG(result);
			break;
		}
#if SPARCV8
		case 30:	// udivcc, SPARCv8
		{
			UINT64 dividend = ((UINT64)Y << 32) || arg1;
			UINT32 divisor = arg2;
			UINT64 quotient = dividend / divisor;

			bool v = false;
			if (quotient > (0xffffffffL + (divisor - 1)))
			{
				quotient = 0xffffffff;
				v = true;
			}

			TEST_ICC_NZ((UINT32)quotient);
			if (v)
				ICC_V_SET;

			SET_RDREG((UINT32)quotient);
			break;
		}
		case 31:	// sdiv, SPARCv8
		{
			INT64 dividend = ((INT64)(INT32)Y << 32) || arg1;
			INT32 divisor = arg2;
			INT64 quotient = dividend / divisor;

			bool v = false;
			if (quotient > 0)
			{
				INT32 absdivisor = (divisor < 0) ? -divisor : divisor;
				if (quotient > (0x7fffffffL + (absdivisor - 1)))
				{
					quotient = 0x7fffffff;
					v = true;
				}
			}
			else if (quotient < 0)
			{
				if (quotient < (INT64)0xffffffff80000000L)
				{
					quotient = 0x80000000;
					v = true;
				}
			}

			if (v)
				ICC_V_SET;
			TEST_ICC_NZ((UINT32)quotient);

			SET_RDREG((UINT32)quotient);
			break;
		}
#endif
		case 32:	// taddcc
		{
			UINT32 result = arg1 + arg2;
			TEST_ICC_NZ(result);
			if (((arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg2 & 0x80000000) != (result & 0x80000000)) || ((arg1 & 3) != 0) || ((arg2 & 3) != 0))
				SET_ICC_V_FLAG;
			if (result < arg1 || result < arg2)
				SET_ICC_C_FLAG;
			SET_RDREG(result);
			break;
		}
		case 33:	// tsubcc
		{
			UINT32 result = arg1 - arg2;
			TEST_ICC_NZ(result);
			if (((arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg2 & 0x80000000) != (result & 0x80000000)) || ((arg1 & 3) != 0) || ((arg2 & 3) != 0))
				SET_ICC_V_FLAG;
			if (result > arg1)
				SET_ICC_C_FLAG;
			SET_RDREG(result);
			break;
		}
		case 34:	// taddcctv
		{
			UINT32 result = arg1 + arg2;
			bool v = ((arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg2 & 0x80000000) != (result & 0x80000000)) || ((arg1 & 3) != 0) || ((arg2 & 3) != 0);
			if (v)
			{
				trap(SPARC_TAG_OVERFLOW);
			}
			else
			{
				TEST_ICC_NZ(result);
				if (result < arg1 || result < arg2)
					SET_ICC_C_FLAG;
				SET_RDREG(result);
			}
			break;
		}
		case 35:	// tsubcctv
		{
			UINT32 result = arg1 - arg2;
			bool v = ((arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg2 & 0x80000000) != (result & 0x80000000)) || ((arg1 & 3) != 0) || ((arg2 & 3) != 0);
			if (v)
			{
				trap(SPARC_TAG_OVERFLOW);
			}
			else
			{
				TEST_ICC_NZ(result);
				if (result > arg1)
					SET_ICC_C_FLAG;
 				SET_RDREG(result);
			}
			break;
		}
		case 36:	// mulscc
		{
			// Explanatory quotes from The SPARC Architecture Manual Version 8, pg. 112 (pg. 110 in sparcv8.pdf)

			// (1) The multiplier is established as r[rs2] if the i field is zero, or sign_ext(simm13) if the i field is one.
			UINT32 multiplier = arg2;

			// (2) A 32-bit value is computed by shifting r[rs1] right by one bit with "N xor V" from the PSR replacing the
			//     high-order bit. (This is the proper sign for the previous partial product.)
			UINT32 rs1 = arg1;
			bool n = ICC_N_SET;
			bool v = ICC_V_SET;
			UINT32 shifted = (rs1 >> 1) | ((n ^ v) ? 0x80000000 : 0);

			if (m_y & 1)
			{	// (3) If the least significant bit of the Y register = 1, the shifted value from step (2) is added to the multiplier.
				arg1 = multiplier;
				arg2 = shifted;
			}
			else
			{	//    If the LSB of the Y register = 0, then 0 is added to the shifted value from step (2).
				arg1 = shifted;
				arg2 = 0;
			}

			// (4) The sum from step (3) is written into r[rd].
			UINT32 result = arg1 + arg2;
			SET_RDREG(result);

			// (5) The integer condition codes, icc, are updated according to the addition performed in step (3).
			TEST_ICC_NZ(result);
			if ((arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg2 & 0x80000000) != (result & 0x80000000))
				SET_ICC_V_FLAG;
			if (result < arg1 || result < arg2)
				SET_ICC_C_FLAG;

			// (6) The Y register is shifted right by one bit, with the LSB of the unshifted r[rs1] replacing the MSB of Y.
			m_y = (m_y >> 1) | ((rs1 & 1) ? 0x80000000 : 0);
			break;
		}
		case 37:	// sll
			SET_RDREG(arg1 << arg2);
			break;
		case 38:	// srl
			SET_RDREG(arg1 >> arg2);
			break;
		case 39:	// sra
			SET_RDREG(INT32(arg1) >> arg2);
			break;
		case 40:	// rdy
			if (RS1 == 0)
			{	// rd y
				SET_RDREG(m_y);
			}
#if SPARCV8
			else if (RS1 == 15 && RD == 0)
			{	// stbar, SPARCv8
				// no-op, as this implementation assumes Total Store Ordering
			}
			else
			{	// rd asr, SPARCv8
				logerror("Unimplemented instruction: rd asr");
			}
#endif
			break;
		case 41:	// rd psr
			if (IS_USER)
			{
				trap(SPARC_PRIVILEGED_INSTRUCTION);
			}
			else
			{
				SET_RDREG(m_psr);
			}
			break;
		case 42:	// rd wim
			if (IS_USER)
			{
				trap(SPARC_PRIVILEGED_INSTRUCTION);
			}
			else
			{
				SET_RDREG(m_wim);
			}
			break;
		case 43:	// rd tbr
			if (IS_USER)
			{
				trap(SPARC_PRIVILEGED_INSTRUCTION);
			}
			else
			{
				SET_RDREG(m_tbr);
			}
			break;
		case 48:
			if (RD == 0)
			{	// wr y
				m_y = arg1 ^ arg2;
			}
#if SPARCV8
			else
			{	// wr asr, SPARCv8
				logerror("Unimplemented instruction: wr asr");
			}
#endif
			break;
		case 49:	// wr psr
			if (IS_USER)
			{
				trap(SPARC_PRIVILEGED_INSTRUCTION);
			}
			else
			{
				UINT32 new_psr = (arg1 ^ arg2) & ~PSR_ZERO_MASK;
				if ((new_psr & PSR_CWP_MASK) >= WINDOW_COUNT)
				{
					trap(SPARC_ILLEGAL_INSTRUCTION);
				}
				else
				{
					m_psr = new_psr;
					BREAK_PSR;
				}
			}
			break;
		case 50:	// wr wim
			if (IS_USER)
			{
				trap(SPARC_PRIVILEGED_INSTRUCTION);
			}
			else
			{
				m_wim = (arg1 ^ arg2) & 0x7f;
			}
			break;
		case 51:	// wr tbr
			if (IS_USER)
			{
				trap(SPARC_PRIVILEGED_INSTRUCTION);
			}
			else
			{
				m_tbr = (arg1 ^ arg2) & 0xfffff000;
				printf("wr (%08x ^ %08x) & 0xfffff000 (%08x),tbr", arg1, arg2, m_tbr);
			}
			break;
		case 52: // FPop1
			if (FPU_DISABLED)
			{
				trap(SPARC_FLOATING_POINT_DISABLED);
			}
			break;
		case 53: // FPop2
			if (FPU_DISABLED)
			{
				trap(SPARC_FLOATING_POINT_DISABLED);
			}
			break;
		case 56:	// jmpl
		{
			UINT32 addr = ADDRESS;
			m_icount--;
			if (addr & 3)
			{
				trap(SPARC_MEM_ADDRESS_NOT_ALIGNED);
			}
			else
			{
				SET_RDREG(PC);
				PC = nPC;
				nPC = addr;
				return false;
			}
			break;
		}
		case 57:	// rett
		{
			UINT8 new_cwp = (m_cwp + 1) % WINDOW_COUNT;
			if (TRAPS_ENABLED)
			{
				if (IS_USER)
				{
					trap(SPARC_PRIVILEGED_INSTRUCTION);
				}
				else
				{
					trap(SPARC_ILLEGAL_INSTRUCTION);
				}
				break;
			}
			else
			{
				if (IS_USER)
				{
					trap(SPARC_RESET, SPARC_PRIVILEGED_INSTRUCTION);
					break;
				}
				else if (m_wim & (1 << new_cwp))
				{
					trap(SPARC_RESET, SPARC_WINDOW_UNDERFLOW);
					break;
				}
				else if (ADDRESS & 3)
				{
					trap(SPARC_RESET, SPARC_MEM_ADDRESS_NOT_ALIGNED);
					break;
				}
			}

			m_cwp = new_cwp;

			m_s = m_ps;
			m_insn_asi = m_s ? 9 : 8;
			m_data_asi = m_s ? 11 : 10;
			m_et = true;

			UINT32 target = arg1 + arg2;
			PC = nPC;
			nPC = target;
			return false;
		}
		case 58:	// ticc
			return execute_ticc(op);
#if SPARCV8
		case 59:
			// SPARCv8
			if (RD == 0)
			{	// flush, SPARCv8
			}
			break;
#endif
		case 60:	// save
		{
			UINT8 new_cwp = ((m_cwp + WINDOW_COUNT) - 1) % WINDOW_COUNT;
			if (m_wim & (1 << new_cwp))
			{
				trap(SPARC_WINDOW_OVERFLOW);
			}
			else
			{
				save_restore_update_cwp(op, new_cwp);
			}
			break;
		}
		case 61:	// restore
		{
			UINT8 new_cwp = (m_cwp + 1) % WINDOW_COUNT;
			if (m_wim & (1 << new_cwp))
			{
				trap(SPARC_WINDOW_UNDERFLOW);
			}
			else
			{
				save_restore_update_cwp(op, new_cwp);
			}
			break;
		}
		default:
			trap(SPARC_ILLEGAL_INSTRUCTION);
			break;
	}

	return true;
}



//-------------------------------------------------
//  update_gpr_pointers - cache pointers to
//  the registers in our current window
//-------------------------------------------------

void mb86901_device::update_gpr_pointers()
{
	for (int i = 0; i < 8; i++)
	{
		m_regs[ 8 + i] = &m_r[8 + (( 0 + m_cwp * 16 + i) % (WINDOW_COUNT * 16))];
		m_regs[16 + i] = &m_r[8 + (( 8 + m_cwp * 16 + i) % (WINDOW_COUNT * 16))];
		m_regs[24 + i] = &m_r[8 + ((16 + m_cwp * 16 + i) % (WINDOW_COUNT * 16))];
	}
}

bool mb86901_device::check_main_traps(UINT32 op, bool privileged, UINT32 alignment, UINT8 registeralign, bool noimmediate)
{
	bool trap_queued = false;
	if (privileged && !m_s)
	{
		trap(SPARC_PRIVILEGED_INSTRUCTION);
		trap_queued = true;
	}
	if (alignment & ADDRESS)
	{
		trap(SPARC_MEM_ADDRESS_NOT_ALIGNED);
		trap_queued = true;
	}
	if ((registeralign & RD) || (noimmediate && USEIMM))
	{
		trap(SPARC_ILLEGAL_INSTRUCTION);
		trap_queued = true;
	}
	return trap_queued;
}

//-------------------------------------------------
//  execute_group3 - execute an opcode in group 3
//  (load/store)
//-------------------------------------------------

void mb86901_device::execute_group3(UINT32 op)
{
	static const int ldst_cycles[64] = {
		1, 1, 1, 2, 2, 2, 2, 3,
		0, 1, 1, 0, 0, 3, 0, 0,
		1, 1, 1, 2, 2, 2, 2, 3,
		0, 1, 1, 0, 0, 3, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

	switch (OP3)
	{
		case 0:		// ld
		{
			check_main_traps(op, false, 3, 0, false);
			UINT32 result = read_word(m_data_asi, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 1:		// ldub
		{
			UINT32 result = read_byte(m_data_asi, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 2:		// lduh
		{
			check_main_traps(op, false, 1, 0, false);
			UINT32 result = read_half(m_data_asi, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 3:		// ldd
		{
			check_main_traps(op, false, 7, 1, false);
			UINT32 result = read_word(m_data_asi, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			result = read_word(m_data_asi, ADDRESS+4);
			if (MAE || HOLD_BUS)
				break;
			REG(RD+1) = result;
			break;
		}
		case 4:		// st
			check_main_traps(op, false, 3, 0, false);
			write_word(m_data_asi, ADDRESS, RDREG);
			break;
		case 5:		// stb
			write_byte(m_data_asi, ADDRESS, UINT8(RDREG));
			break;
		case 6:		// sth
			check_main_traps(op, false, 1, 0, false);
			write_word(m_data_asi, ADDRESS, UINT16(RDREG));
			break;
		case 7:		// std
			check_main_traps(op, false, 7, 1, false);
			write_word(m_data_asi, ADDRESS, RDREG);
			if (MAE || HOLD_BUS)
				break;
			write_word(m_data_asi, ADDRESS+4, REG(RD+1));
			break;
		case 9:		// ldsb
		{
			UINT32 result = read_signed_byte(m_data_asi, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 10:	// lsdh
		{
			check_main_traps(op, false, 1, 0, false);
			UINT32 result = read_signed_half(m_data_asi, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 13:	// ldstub
		{
			UINT32 result = read_byte(m_data_asi, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			write_byte(m_data_asi, ADDRESS, 0xff);
			break;
		}
		case 15:	// swap, SPARCv8
			break;
		case 16:	// lda
		{
			check_main_traps(op, true, 3, 0, true);
			UINT32 result = read_word(ASI, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 17:	// lduba
		{
			check_main_traps(op, true, 0, 0, true);
			UINT32 result = read_byte(ASI, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 18:	// lduha
		{
			check_main_traps(op, true, 1, 0, true);
			UINT32 result = read_half(ASI, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 19:	// ldda
		{
			check_main_traps(op, true, 7, 1, true);
			UINT32 result = read_word(ASI, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			result = read_word(ASI, ADDRESS+4);
			if (MAE || HOLD_BUS)
				break;
			REG(RD+1) = result;
			break;
		}
		case 20:	// sta
			check_main_traps(op, true, 3, 0, true);
			write_word(ASI, ADDRESS, RDREG);
			break;
		case 21:	// stba
			check_main_traps(op, true, 0, 0, true);
			write_byte(ASI, ADDRESS, UINT8(RDREG));
			break;
		case 22:	// stha
			check_main_traps(op, true, 1, 0, true);
			write_half(ASI, ADDRESS, UINT16(RDREG));
			break;
		case 23:	// stda
			check_main_traps(op, true, 7, 1, true);
			write_word(ASI, ADDRESS, RDREG);
			if (MAE || HOLD_BUS)
				break;
			write_word(ASI, ADDRESS+4, REG(RD+1));
			break;
		case 25:	// ldsba
		{
			check_main_traps(op, true, 0, 0, true);
			UINT32 result = read_signed_byte(ASI, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 26:	// ldsha
		{
			check_main_traps(op, true, 1, 0, true);
			UINT32 result = read_signed_half(ASI, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			break;
		}
		case 29:	// ldstuba
		{
			check_main_traps(op, true, 0, 0, true);
			UINT32 result = read_byte(ASI, ADDRESS);
			if (MAE || HOLD_BUS)
				break;
			SET_RDREG(result);
			write_byte(ASI, ADDRESS, 0xff);
			break;
		}
		case 31:	// swapa, SPARCv8
			break;
		case 32:	// ld fpr
			if (FPU_DISABLED)
				trap(SPARC_FLOATING_POINT_DISABLED);
			break;
		case 33:	// ld fsr
			if (FPU_DISABLED)
				trap(SPARC_FLOATING_POINT_DISABLED);
			break;
		case 35:	// ldd fpr
			if (FPU_DISABLED)
				trap(SPARC_FLOATING_POINT_DISABLED);
			break;
		case 36:	// st fpr
			if (FPU_DISABLED)
				trap(SPARC_FLOATING_POINT_DISABLED);
			break;
		case 37:	// st fsr
			if (FPU_DISABLED)
				trap(SPARC_FLOATING_POINT_DISABLED);
			break;
		case 38:	// std fq, SPARCv8
			if (FPU_DISABLED)
				trap(SPARC_FLOATING_POINT_DISABLED);
			break;
		case 39:	// std fpr
			if (FPU_DISABLED)
				trap(SPARC_FLOATING_POINT_DISABLED);
			break;
		case 40:	// ld cpr, SPARCv8
			break;
		case 41:	// ld csr, SPARCv8
			break;
		case 43:	// ldd cpr, SPARCv8
			break;
		case 44:	// st cpr, SPARCv8
			break;
		case 45:	// st csr, SPARCv8
			break;
		case 46:	// std cq, SPARCv8
			break;
		case 47:	// std cpr, SPARCv8
			break;
	}

	if (MAE || HOLD_BUS)
		m_icount--;
	else
		m_icount -= ldst_cycles[OP3];
}


//-------------------------------------------------
//  evaluate_condition - evaluate a given integer
//  condition code
//-------------------------------------------------

bool mb86901_device::evaluate_condition(UINT32 op)
{
	bool take = false;
	bool n = ICC_N_SET;
	bool z = ICC_Z_SET;
	bool v = ICC_V_SET;
	bool c = ICC_C_SET;

	switch(COND & 7)							// COND & 8
	{											// 0		8
		case 0:		take = false; break;		// bn		ba
		case 1:		take = z; break;			// bz		bne
		case 2:		take = z | (n ^ z); break;	// ble		bg
		case 3:		take = n ^ z; break;		// bl		bge
		case 4:		take = c | z; break;		// bleu		bgu
		case 5:		take = c; break;			// bcs		bcc
		case 6:		take = n; break;			// bneg		bpos
		case 7:		take = v; break;			// bvs		bvc
	}

	if (COND & 8)
		take = !take;

	return take;
}

//-------------------------------------------------
//  execute_bicc - execute a branch opcode
//-------------------------------------------------

bool mb86901_device::execute_bicc(UINT32 op)
{
	bool branch_taken = evaluate_condition(op);

	if (branch_taken)
	{
		UINT32 brpc = PC + DISP22;
		PC = nPC;
		nPC = brpc;
		return false;
	}
	else
	{
		m_icount--;
	}

	if (ANNUL && (!branch_taken || COND == 8))
	{
		PC = nPC;
		nPC = PC + 4;
		m_icount -= 1;
		return false;
	}

	return true;
}


//-------------------------------------------------
//  execute_ticc - execute a conditional trap
//-------------------------------------------------

bool mb86901_device::execute_ticc(UINT32 op)
{
	bool trap_taken = evaluate_condition(op);

	printf("ticc @ %x\n", PC);
	if (trap_taken)
	{
		UINT32 arg2 = USEIMM ? SIMM7 : RS2REG;
		UINT8 tt = 128 + ((RS1REG + arg2) & 0x7f);
		trap(SPARC_TRAP_INSTRUCTION, tt);
		m_icount -= 3;
		return false;
	}

	return true;
}


//-------------------------------------------------
//  trap - flag an incoming trap of a given
//  type
//-------------------------------------------------

void mb86901_device::trap(UINT8 type, UINT8 tt_override)
{
	if (type == SPARC_RESET)
	{
		m_queued_priority = m_trap_priorities[0];
		m_queued_tt = tt_override;
	}
	else
	{
		if (type == SPARC_TRAP_INSTRUCTION)
		{
			type = tt_override;
		}

		if (type >= SPARC_INT1 && type <= SPARC_INT14)
		{
			if (!ET)
				return;
			int irl = (type - SPARC_INT1) + 1;
			if (irl <= PIL)
				return;
		}
		if (m_trap_priorities[type] < m_queued_priority)
		{
			m_queued_priority = m_trap_priorities[type];
			m_queued_tt = type;
		}
	}
}


//-------------------------------------------------
//  invoke_queued_traps - prioritize and invoke
//  traps that have been queued by the previous
//  instruction, if any. Returns true if a trap
//  was invoked.
//-------------------------------------------------

bool mb86901_device::invoke_queued_traps()
{
	if (m_queued_priority != SPARC_NO_TRAP)
	{
		m_et = false;
		m_ps = m_s;
		m_s = true;
		m_cwp = ((m_cwp + WINDOW_COUNT) - 1) % WINDOW_COUNT;

		MAKE_PSR;
		update_gpr_pointers();

		REG(17) = PC;
		REG(18) = nPC;

		m_tbr |= m_queued_tt << 4;

		PC = m_tbr;
		nPC = m_tbr + 4;

		m_queued_priority = SPARC_NO_TRAP;
		m_queued_tt = 0;
		return true;
	}

	return false;
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void mb86901_device::execute_run()
{
	bool debug = machine().debug_flags & DEBUG_FLAG_ENABLED;

	while (m_icount > 0)
	{
		bool trap_was_queued = invoke_queued_traps();
		if (trap_was_queued)
		{
			m_icount -= 4;
			continue;
		}

		debugger_instruction_hook(this, m_pc);

		if (HOLD_BUS)
		{
			m_icount--;
			continue;
		}
		UINT32 op = GET_OPCODE;

		bool update_npc = true;

		switch (OP)
		{
		case 0:	// Bicc, SETHI, FBfcc
			switch (OP2)
			{
			case 0: // unimp
				printf("unimp @ %x\n", PC);
				break;
			case 2: // branch on integer condition codes
				update_npc = execute_bicc(op);
				break;
			case 4:	// sethi
				SET_RDREG(IMM22);
				break;
			case 6: // branch on floating-point condition codes
				printf("fbfcc @ %x\n", PC);
				break;
#if SPARCV8
			case 7: // branch on coprocessor condition codes, SPARCv8
				break;
#endif
			default:
				printf("unknown %08x @ %x\n", op, PC);
				break;
			}
			break;
		case 1: // call
		{
			UINT32 pc = PC;
			UINT32 callpc = PC + DISP30;
			PC = nPC;
			nPC = callpc;

			REG(15) = pc;

			update_npc = false;
			break;
		}
		case 2:
			update_npc = execute_group2(op);
			break;
		case 3: // loads, stores
			execute_group3(op);
			break;
		default:
			break;
		}

		REG(0) = 0;

		bool trap_taken = invoke_queued_traps();
		if (!trap_taken && update_npc && !HOLD_BUS)
		{
			PC = nPC;
			nPC = PC + 4;
		}

		if (debug)
		{
			for (int i = 0; i < 8; i++)
			{
				m_dbgregs[i]		= *m_regs[8 + i];
				m_dbgregs[8 + i]	= *m_regs[16 + i];
				m_dbgregs[16 + i]	= *m_regs[24 + i];
			}
		}
		--m_icount;
	}
}
