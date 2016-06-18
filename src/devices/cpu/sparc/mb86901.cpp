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
//      - Ops: FBFcc, LDF, STF, SPARCv8 ops
//      - FPU support
//      - Coprocessor support
//
//================================================================

#include "emu.h"
#include "debugger.h"
#include "sparc.h"
#include "sparcdefs.h"

CPU_DISASSEMBLE( sparc );

const device_type MB86901 = &device_creator<mb86901_device>;

const int mb86901_device::WINDOW_COUNT = 7;

//-------------------------------------------------
//  mb86901_device - constructor
//-------------------------------------------------

mb86901_device::mb86901_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, MB86901, "Fujitsu MB86901", tag, owner, clock, "mb86901", __FILE__)
	, m_as8_config("user_insn", ENDIANNESS_BIG, 32, 32)
	, m_as9_config("supr_insn", ENDIANNESS_BIG, 32, 32)
	, m_as10_config("user_data", ENDIANNESS_BIG, 32, 32)
	, m_as11_config("supr_data", ENDIANNESS_BIG, 32, 32)
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

	m_user_insn = &space(AS_USER_INSN);
	m_super_insn = &space(AS_SUPER_INSN);
	m_user_data = &space(AS_USER_DATA);
	m_super_data = &space(AS_SUPER_DATA);

	for (int i = 0; i < 256; i++)
	{
		m_spaces[i] = m_super_insn;
	}
	m_spaces[0] = m_super_insn;
	m_spaces[8] = m_user_insn;
	m_spaces[9] = m_super_insn;
	m_spaces[10] = m_user_data;
	m_spaces[11] = m_super_data;

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

	// set our instruction counter
	m_icountptr = &m_icount;
}

void mb86901_device::device_stop()
{
}

void mb86901_device::device_reset()
{
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
	m_pil = 0; // double-check this
	m_s = true;
	m_ps = true; // double-check this
	m_et = true; // double-check this
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
		case AS_USER_INSN:
			return &m_as8_config;
			break;
		case AS_SUPER_INSN:
			return &m_as9_config;
			break;
		case AS_USER_DATA:
			return &m_as10_config;
			break;
		case AS_SUPER_DATA:
			return &m_as11_config;
			break;
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

	// TODO: check for traps
	return LOAD_UBA(asi, address);
}

INT32 mb86901_device::read_signed_byte(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: check for traps
	return LOAD_SBA(asi, address);
}

UINT32 mb86901_device::read_half(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return LOAD_UHA(asi, address);
}

INT32 mb86901_device::read_signed_half(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return LOAD_SHA(asi, address);
}

UINT32 mb86901_device::read_word(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return LOAD_WA(asi, address);
}

UINT64 mb86901_device::read_doubleword(UINT8 asi, UINT32 address)
{
	m_asi = asi;
	// TODO: data_access_exception, data_access_error traps
	return LOAD_DA(asi, address);
}

void mb86901_device::write_byte(UINT8 asi, UINT32 address, UINT8 data)
{
	m_asi = asi;
	// TODO: check for traps
	STORE_BA(asi, address, data);
}

void mb86901_device::write_half(UINT8 asi, UINT32 address, UINT16 data)
{
	m_asi = asi;
	// TODO: check for traps
	STORE_HA(asi, address, data);
}

void mb86901_device::write_word(UINT8 asi, UINT32 address, UINT32 data)
{
	m_asi = asi;
	// TODO: check for traps
	STORE_WA(asi, address, data);
}

void mb86901_device::write_doubleword(UINT8 asi, UINT32 address, UINT64 data)
{
	m_asi = asi;
	// TODO: check for traps
	STORE_DA(asi, address, data);
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
	extern CPU_DISASSEMBLE( sparc );
	return CPU_DISASSEMBLE_NAME(sparc)(this, buffer, pc, oprom, opram, options);
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
		case 10:	// umul, SPARCv8
			break;
		case 11:	// smul, SPARCv8
			break;
		case 12:	// subx
			SET_RDREG(arg1 - arg2 - (ICC_C_SET ? 1 : 0));
			break;
		case 14:	// udiv, SPARCv8
			break;
		case 15:	// sdiv, SPARCv8
			break;
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
			if ((arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg2 & 0x80000000) != (result & 0x80000000))
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
		case 26:	// umulcc, SPARCv8
			break;
		case 27:	// smulcc, SPARCv8
			break;
		case 28:	// subxcc
		{
			UINT32 c = (ICC_C_SET ? 1 : 0);
			UINT32 argt = arg2 - c;
			UINT32 result = arg1 - argt;
			TEST_ICC_NZ(result);
			if (((arg1 & 0x80000000) == (argt & 0x80000000) && (arg1 & 0x80000000) != (result & 0x80000000)) || (c != 0 && arg2 == 0x80000000))
				SET_ICC_V_FLAG;
			if (result > arg1 || (result == arg1 && arg2 != 0))
				SET_ICC_C_FLAG;
			SET_RDREG(result);
			break;
		}
		case 30:	// udivcc, SPARCv8
			break;
		case 31:	// sdivcc, SPARCv8
			break;
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
				queue_trap(sparc_tag_overflow);
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
				queue_trap(sparc_tag_overflow);
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
			else if (RS1 == 15 && RD == 0)
			{	// stbar, SPARCv8
			}
			else
			{	// rd asr, SPARCv8
			}
			break;
		case 41:	// rd psr
			// TODO: check privilege
			SET_RDREG(m_psr);
			break;
		case 42:	// rd wim
			// TODO: check privilege
			SET_RDREG(m_wim);
			break;
		case 43:	// rd tbr
			// TODO: check privilege
			SET_RDREG(m_tbr);
			break;
		case 48:
			if (RD == 0)
			{	// wr y
				m_y = arg1 ^ arg2;
			}
			else
			{	// wr asr, SPARCv8
			}
			break;
		case 49:	// wr psr
			// TODO: check privilege
			m_psr = (arg1 ^ arg2) & ~PSR_ZERO_MASK;
			BREAK_PSR;
			break;
		case 50:	// wr wim
			// TODO: check privilege
			m_wim = (arg1 ^ arg2) & 0x7f;
			break;
		case 51:	// wr tbr
			// TODO: check privilege
			m_tbr = (arg1 ^ arg2) & 0xfffff000;
			break;
		case 52: // FPop1
			break;
		case 53: // FPop2
			break;
		case 56:	// jmpl
		{
			UINT32 addr = ADDRESS;
			m_icount--;
			if (addr & 3)
			{
				queue_trap(sparc_mem_address_not_aligned);
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
					queue_trap(sparc_privileged_instruction);
				}
				else
				{
					queue_trap(sparc_illegal_instruction);
				}
				break;
			}
			else
			{
				if (IS_USER)
				{
					queue_trap(sparc_reset, sparc_privileged_instruction);
					break;
				}
				else if (m_wim & (1 << new_cwp))
				{
					queue_trap(sparc_reset, sparc_window_underflow);
					break;
				}
				else if (ADDRESS & 3)
				{
					queue_trap(sparc_reset, sparc_mem_address_not_aligned);
					break;
				}
			}

			m_cwp = new_cwp;

			m_s = m_ps;
			m_et = true;

			UINT32 target = arg1 + arg2;
			PC = nPC;
			nPC = target;
			return false;
		}
		case 58:	// ticc
			return execute_ticc(op);
		case 59:
			if (RD == 0)
			{	// flush, SPARCv8
			}
			break;
		case 60:	// save
		{
			UINT8 new_cwp = ((m_cwp + WINDOW_COUNT) - 1) % WINDOW_COUNT;
			if (m_wim & (1 << new_cwp))
			{
				queue_trap(sparc_window_overflow);
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
				queue_trap(sparc_window_overflow);
			}
			else
			{
				save_restore_update_cwp(op, new_cwp);
			}
			break;
		}
		default:
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
		queue_trap(sparc_privileged_instruction);
		trap_queued = true;
	}
	if (alignment & ADDRESS)
	{
		queue_trap(sparc_mem_address_not_aligned);
		trap_queued = true;
	}
	if ((registeralign & RD) || (noimmediate && USEIMM))
	{
		queue_trap(sparc_illegal_instruction);
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
			check_main_traps(op, false, 3, 0, false);
			SET_RDREG(read_word(m_s ? 10 : 11, ADDRESS));
			break;
		case 1:		// ldub
			SET_RDREG(read_byte(m_s ? 10 : 11, ADDRESS));
			break;
		case 2:		// lduh
			check_main_traps(op, false, 1, 0, false);
			SET_RDREG(read_half(m_s ? 10 : 11, ADDRESS));
			break;
		case 3:		// ldd
			check_main_traps(op, false, 7, 1, false);
			SET_RDREG(read_word(m_s ? 10 : 11, ADDRESS));
			REG(RD+1) = read_word(m_s ? 10 : 11, ADDRESS+4);
			break;
		case 4:		// st
			check_main_traps(op, false, 3, 0, false);
			write_word(m_s ? 10 : 11, ADDRESS, RDREG);
			break;
		case 5:		// stb
			write_byte(m_s ? 10 : 11, ADDRESS, UINT8(RDREG));
			break;
		case 6:		// sth
			check_main_traps(op, false, 1, 0, false);
			write_word(m_s ? 10 : 11, ADDRESS, UINT16(RDREG));
			break;
		case 7:		// std
			check_main_traps(op, false, 7, 1, false);
			write_word(m_s ? 10 : 11, ADDRESS, RDREG);
			write_word(m_s ? 10 : 11, ADDRESS, REG(RD+1));
			break;
		case 9:		// ldsb
			SET_RDREG(read_signed_byte(m_s ? 10 : 11, ADDRESS));
			break;
		case 10:	// lsdh
			check_main_traps(op, false, 1, 0, false);
			SET_RDREG(read_signed_half(m_s ? 10 : 11, ADDRESS));
			break;
		case 13:	// ldstub
			SET_RDREG(read_byte(m_s ? 10 : 11, ADDRESS));
			write_byte(m_s ? 10 : 11, ADDRESS, 0xff);
			break;
		case 15:	// swap, SPARCv8
			break;
		case 16:	// lda
			check_main_traps(op, true, 3, 0, true);
			SET_RDREG(read_word(ASI, ADDRESS));
			break;
		case 17:	// lduba
			check_main_traps(op, true, 0, 0, true);
			SET_RDREG(read_byte(ASI, ADDRESS));
			break;
		case 18:	// lduha
			check_main_traps(op, true, 1, 0, true);
			SET_RDREG(read_half(ASI, ADDRESS));
			break;
		case 19:	// ldda
			check_main_traps(op, true, 7, 1, true);
			SET_RDREG(read_word(ASI, ADDRESS));
			REG(RD+1) = read_word(ASI, ADDRESS+4);
			break;
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
			write_word(ASI, ADDRESS+4, REG(RD+1));
			break;
		case 25:	// ldsba
			check_main_traps(op, true, 0, 0, true);
			SET_RDREG(read_signed_byte(ASI, ADDRESS));
			break;
		case 26:	// ldsha
			check_main_traps(op, true, 1, 0, true);
			SET_RDREG(read_signed_half(ASI, ADDRESS));
			break;
		case 29:	// ldstuba
			check_main_traps(op, true, 0, 0, true);
			SET_RDREG(read_byte(ASI, ADDRESS));
			write_byte(ASI, ADDRESS, 0xff);
			break;
		case 31:	// swapa, SPARCv8
			break;
		case 32:	// ld fpr
			break;
		case 33:	// ld fsr
			break;
		case 35:	// ldd fpr
			break;
		case 36:	// st fpr
			break;
		case 37:	// st fsr
			break;
		case 38:	// std fq, SPARCv8
			break;
		case 39:	// std fpr
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

	if (trap_taken)
	{
		UINT32 arg2 = USEIMM ? SIMM7 : RS2REG;
		UINT8 tt = 128 + ((RS1REG + arg2) & 0x7f);
		queue_trap(sparc_trap_instruction, tt);
		return false;
	}

	return true;
}


//-------------------------------------------------
//  queue_trap - flag an incoming trap of a given
//  type
//-------------------------------------------------

void mb86901_device::queue_trap(UINT8 type, UINT8 tt_override)
{
	if (type == sparc_reset)
	{
		m_queued_priority = m_trap_priorities[0];
		m_queued_tt = tt_override;
	}
	else
	{
		if (type == sparc_trap_instruction)
		{
			type = tt_override;
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
	if (m_queued_priority > 0)
	{
		m_queued_priority = 0;

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
		debugger_instruction_hook(this, m_pc);

		UINT32 op = GET_OPCODE;

		bool update_npc = true;

		switch (OP)
		{
		case 0:	// Bicc, SETHI, FBfcc
			switch (OP2)
			{
			case 0: // unimp
				break;
			case 2: // branch on integer condition codes
				update_npc = execute_bicc(op);
				break;
			case 4:	// sethi
				SET_RDREG(IMM22);
				break;
			case 6: // branch on floating-point condition codes
				break;
			case 7: // branch on coprocessor condition codes, SPARCv8
				break;
			default:
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
		if (!trap_taken && update_npc)
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
