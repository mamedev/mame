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
//      - Test: SPARCv8 ops are untested
//      - Test: Traps are untested
//      - FPU support
//      - Coprocessor support
//
//================================================================

#include "emu.h"
#include "debugger.h"
#include "sparc.h"
#include "sparcdefs.h"

const device_type MB86901 = &device_creator<mb86901_device>;

const int mb86901_device::NWINDOWS = 7;

//-------------------------------------------------
//  mb86901_device - constructor
//-------------------------------------------------

mb86901_device::mb86901_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, MB86901, "Fujitsu MB86901", tag, owner, clock, "mb86901", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32)
	, m_dasm(this, 7)
{
}


void mb86901_device::device_start()
{
	m_bp_reset_in = false;
	m_bp_irl = 0;
	m_bp_fpu_present = false;
	m_bp_cp_present = false;
	m_pb_error = false;
	m_pb_block_ldst_byte = false;
	m_pb_block_ldst_word = false;

	memset(m_dbgregs, 0, 24 * sizeof(UINT32));

	memset(m_illegal_instruction_asr, 0, 32 * sizeof(bool));
	memset(m_privileged_asr, 1, 32 * sizeof(bool));
	m_privileged_asr[0] = false;

	memset(m_alu_op3_assigned, 0, 64 * sizeof(bool));
	m_alu_op3_assigned[OP3_ADD] = true;
	m_alu_op3_assigned[OP3_AND] = true;
	m_alu_op3_assigned[OP3_OR] = true;
	m_alu_op3_assigned[OP3_XOR] = true;
	m_alu_op3_assigned[OP3_SUB] = true;
	m_alu_op3_assigned[OP3_ANDN] = true;
	m_alu_op3_assigned[OP3_ORN] = true;
	m_alu_op3_assigned[OP3_XNOR] = true;
	m_alu_op3_assigned[OP3_ADDX] = true;
	m_alu_op3_assigned[OP3_SUBX] = true;
	m_alu_op3_assigned[OP3_ADDCC] = true;
	m_alu_op3_assigned[OP3_ANDCC] = true;
	m_alu_op3_assigned[OP3_ORCC] = true;
	m_alu_op3_assigned[OP3_XORCC] = true;
	m_alu_op3_assigned[OP3_SUBCC] = true;
	m_alu_op3_assigned[OP3_ANDNCC] = true;
	m_alu_op3_assigned[OP3_ORNCC] = true;
	m_alu_op3_assigned[OP3_XNORCC] = true;
	m_alu_op3_assigned[OP3_ADDXCC] = true;
	m_alu_op3_assigned[OP3_SUBXCC] = true;
	m_alu_op3_assigned[OP3_TADDCC] = true;
	m_alu_op3_assigned[OP3_TSUBCC] = true;
	m_alu_op3_assigned[OP3_TADDCCTV] = true;
	m_alu_op3_assigned[OP3_TSUBCCTV] = true;
	m_alu_op3_assigned[OP3_MULSCC] = true;
	m_alu_op3_assigned[OP3_SLL] = true;
	m_alu_op3_assigned[OP3_SRL] = true;
	m_alu_op3_assigned[OP3_SRA] = true;
	m_alu_op3_assigned[OP3_RDASR] = true;
	m_alu_op3_assigned[OP3_RDPSR] = true;
	m_alu_op3_assigned[OP3_RDWIM] = true;
	m_alu_op3_assigned[OP3_RDTBR] = true;
	m_alu_op3_assigned[OP3_WRASR] = true;
	m_alu_op3_assigned[OP3_WRPSR] = true;
	m_alu_op3_assigned[OP3_WRWIM] = true;
	m_alu_op3_assigned[OP3_WRTBR] = true;
	m_alu_op3_assigned[OP3_FPOP1] = true;
	m_alu_op3_assigned[OP3_FPOP2] = true;
	m_alu_op3_assigned[OP3_JMPL] = true;
	m_alu_op3_assigned[OP3_RETT] = true;
	m_alu_op3_assigned[OP3_TICC] = true;
	m_alu_op3_assigned[OP3_SAVE] = true;
	m_alu_op3_assigned[OP3_RESTORE] = true;
#if SPARCV8
	m_alu_op3_assigned[OP3_UMUL] = true;
	m_alu_op3_assigned[OP3_SMUL] = true;
	m_alu_op3_assigned[OP3_UDIV] = true;
	m_alu_op3_assigned[OP3_SDIV] = true;
	m_alu_op3_assigned[OP3_UMULCC] = true;
	m_alu_op3_assigned[OP3_SMULCC] = true;
	m_alu_op3_assigned[OP3_UDIVCC] = true;
	m_alu_op3_assigned[OP3_SDIVCC] = true;
	m_alu_op3_assigned[OP3_CPOP1] = true;
	m_alu_op3_assigned[OP3_CPOP2] = true;
#endif
	m_program = &space(AS_PROGRAM);

	memset(m_ldst_op3_assigned, 0, 64 * sizeof(bool));
	m_ldst_op3_assigned[OP3_LD] = true;
	m_ldst_op3_assigned[OP3_LDUB] = true;
	m_ldst_op3_assigned[OP3_LDUH] = true;
	m_ldst_op3_assigned[OP3_LDD] = true;
	m_ldst_op3_assigned[OP3_ST] = true;
	m_ldst_op3_assigned[OP3_STB] = true;
	m_ldst_op3_assigned[OP3_STH] = true;
	m_ldst_op3_assigned[OP3_STD] = true;
	m_ldst_op3_assigned[OP3_LDSB] = true;
	m_ldst_op3_assigned[OP3_LDSH] = true;
	m_ldst_op3_assigned[OP3_LDSTUB] = true;
	m_ldst_op3_assigned[OP3_LDA] = true;
	m_ldst_op3_assigned[OP3_LDUBA] = true;
	m_ldst_op3_assigned[OP3_LDUHA] = true;
	m_ldst_op3_assigned[OP3_LDDA] = true;
	m_ldst_op3_assigned[OP3_STA] = true;
	m_ldst_op3_assigned[OP3_STBA] = true;
	m_ldst_op3_assigned[OP3_STHA] = true;
	m_ldst_op3_assigned[OP3_STDA] = true;
	m_ldst_op3_assigned[OP3_LDSBA] = true;
	m_ldst_op3_assigned[OP3_LDSHA] = true;
	m_ldst_op3_assigned[OP3_LDSTUBA] = true;
	m_ldst_op3_assigned[OP3_LDFPR] = true;
	m_ldst_op3_assigned[OP3_LDFSR] = true;
	m_ldst_op3_assigned[OP3_LDDFPR] = true;
	m_ldst_op3_assigned[OP3_STFPR] = true;
	m_ldst_op3_assigned[OP3_STFSR] = true;
	m_ldst_op3_assigned[OP3_STDFQ] = true;
	m_ldst_op3_assigned[OP3_STDFPR] = true;
#if SPARCV8
	m_ldst_op3_assigned[OP3_SWAP] = true;
	m_ldst_op3_assigned[OP3_SWAPA] = true;
	m_ldst_op3_assigned[OP3_LDCPR] = true;
	m_ldst_op3_assigned[OP3_LDCSR] = true;
	m_ldst_op3_assigned[OP3_LDDCPR] = true;
	m_ldst_op3_assigned[OP3_STCPR] = true;
	m_ldst_op3_assigned[OP3_STCSR] = true;
	m_ldst_op3_assigned[OP3_STDCQ] = true;
	m_ldst_op3_assigned[OP3_STDCPR] = true;
#endif

	// register our state for the debugger
	state_add(STATE_GENPC,      "GENPC",    m_pc).noshow();
	state_add(STATE_GENFLAGS,   "GENFLAGS", m_psr).callimport().callexport().formatstr("%6s").noshow();
	state_add(SPARC_PC,         "PC",       m_pc).formatstr("%08X");
	state_add(SPARC_NPC,        "nPC",      m_npc).formatstr("%08X");
	state_add(SPARC_PSR,        "PSR",      m_psr).formatstr("%08X");
	state_add(SPARC_WIM,        "WIM",      m_wim).formatstr("%08X");
	state_add(SPARC_TBR,        "TBR",      m_tbr).formatstr("%08X");
	state_add(SPARC_Y,          "Y",        m_y).formatstr("%08X");
	state_add(SPARC_ANNUL,      "ANNUL",    m_annul).formatstr("%01d");
	state_add(SPARC_ICC,        "icc",      m_icc).formatstr("%4s");
	state_add(SPARC_CWP,        "CWP",      m_cwp).formatstr("%2d");
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

	state_add(SPARC_EC,     "EC",       m_ec).formatstr("%1d");
	state_add(SPARC_EF,     "EF",       m_ef).formatstr("%1d");
	state_add(SPARC_ET,     "ET",       m_et).formatstr("%1d");
	state_add(SPARC_PIL,    "PIL",      m_pil).formatstr("%2d");
	state_add(SPARC_S,      "S",        m_s).formatstr("%1d");
	state_add(SPARC_PS,     "PS",       m_ps).formatstr("%1d");

	char rname[5];
	for (int i = 0; i < 120; i++)
	{
		sprintf(rname, "r%d", i);
		state_add(SPARC_R0 + i, rname, m_r[i]).formatstr("%08X");
	}

	save_item(NAME(m_r));
	save_item(NAME(m_fpr));
	save_item(NAME(m_fsr));
	save_item(NAME(m_ftt));
	save_item(NAME(m_pc));
	save_item(NAME(m_npc));
	save_item(NAME(m_psr));
	save_item(NAME(m_wim));
	save_item(NAME(m_tbr));
	save_item(NAME(m_y));
	save_item(NAME(m_bp_reset_in));
	save_item(NAME(m_bp_irl));
	save_item(NAME(m_bp_fpu_present));
	save_item(NAME(m_bp_cp_present));
	save_item(NAME(m_pb_block_ldst_byte));
	save_item(NAME(m_pb_block_ldst_word));
	save_item(NAME(m_trap));
	save_item(NAME(m_tt));
	save_item(NAME(m_ticc_trap_type));
	save_item(NAME(m_interrupt_level));
	save_item(NAME(m_privileged_instruction));
	save_item(NAME(m_illegal_instruction));
	save_item(NAME(m_mem_address_not_aligned));
	save_item(NAME(m_fp_disabled));
	save_item(NAME(m_fp_exception));
	save_item(NAME(m_cp_disabled));
	save_item(NAME(m_cp_exception));
	save_item(NAME(m_unimplemented_FLUSH));
	save_item(NAME(m_r_register_access_error));
	save_item(NAME(m_instruction_access_error));
	save_item(NAME(m_instruction_access_exception));
	save_item(NAME(m_data_access_error));
	save_item(NAME(m_data_store_error));
	save_item(NAME(m_data_access_exception));
	save_item(NAME(m_division_by_zero));
	save_item(NAME(m_trap_instruction));
	save_item(NAME(m_window_underflow));
	save_item(NAME(m_window_overflow));
	save_item(NAME(m_tag_overflow));
	save_item(NAME(m_reset_mode));
	save_item(NAME(m_reset_trap));
	save_item(NAME(m_execute_mode));
	save_item(NAME(m_error_mode));
	save_item(NAME(m_fpu_sequence_err));
	save_item(NAME(m_cp_sequence_err));
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
	save_item(NAME(m_asi));
	save_item(NAME(m_mae));
	save_item(NAME(m_annul));
	save_item(NAME(m_hold_bus));

	// set our instruction counter
	m_icountptr = &m_icount;
}


void mb86901_device::device_stop()
{
}


void mb86901_device::device_reset()
{
	m_trap = 0;
	m_tt = 0;
	m_ticc_trap_type = 0;
	m_privileged_instruction = 0;
	m_illegal_instruction = 0;
	m_mem_address_not_aligned = 0;
	m_fp_disabled = 0;
	m_cp_disabled = 0;
	m_instruction_access_exception = 0;
	m_trap_instruction = 0;
	m_window_underflow = 0;
	m_window_overflow = 0;
	m_tag_overflow = 0;
	m_reset_mode = 1;
	m_reset_trap = 0;
	m_execute_mode = 0;
	m_error_mode = 0;
	m_fpu_sequence_err = 0;
	m_cp_sequence_err = 0;

	m_asi = 0;
	MAE = false;
	HOLD_BUS = false;
	m_annul = false;

	PC = 0;
	nPC = 4;
	memset(m_r, 0, sizeof(UINT32) * 120);
	memset(m_fpr, 0, sizeof(UINT32) * 32);

	WIM = 0;
	TBR = 0;
	Y = 0;

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
//  read_sized_word - read a value from a given
//  address space and address, shifting the data
//  that is read into the appropriate location of
//  a 32-bit word in a big-endian system.
//-------------------------------------------------

UINT32 mb86901_device::read_sized_word(UINT8 asi, UINT32 address, int size)
{
	m_asi = asi;
	if (size == 1)
	{
		return m_program->read_byte(address) << ((3 - (address & 3)) * 8);
	}
	else if (size == 2)
	{
		return m_program->read_word(address) << ((2 - (address & 2)) * 8);
	}
	else
	{
		return m_program->read_dword(address);
	}
}


//-------------------------------------------------
//  write_sized_word - write a value to a given
//  address space and address, shifting the data
//  that is written into the least significant
//  bits as appropriate in order to write the
//  value to a memory system with separate data
//  size handlers
//-------------------------------------------------

void mb86901_device::write_sized_word(UINT8 asi, UINT32 address, UINT32 data, int size)
{
	m_asi = asi;
	if (size == 1)
	{
		m_program->write_byte(address, data >> ((3 - (address & 3)) * 8));
	}
	else if (size == 2)
	{
		m_program->write_word(address, data >> ((2 - (address & 2)) * 8));
	}
	else
	{
		m_program->write_dword(address, data);
	}
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
		case SPARC_O0:  case SPARC_O1:  case SPARC_O2:  case SPARC_O3:  case SPARC_O4:  case SPARC_O5:  case SPARC_O6:  case SPARC_O7:
			str = string_format("%08X", m_dbgregs[entry.index() - SPARC_O0]);
			break;
		case SPARC_L0:  case SPARC_L1:  case SPARC_L2:  case SPARC_L3:  case SPARC_L4:  case SPARC_L5:  case SPARC_L6:  case SPARC_L7:
			str = string_format("%08X", m_dbgregs[8 + (entry.index() - SPARC_L0)]);
			break;
		case SPARC_I0:  case SPARC_I1:  case SPARC_I2:  case SPARC_I3:  case SPARC_I4:  case SPARC_I5:  case SPARC_I6:  case SPARC_I7:
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


#if SPARCV8
#include "sparcv8ops.ipp"
#endif

//-------------------------------------------------
//  execute_add - execute an add-type opcode
//-------------------------------------------------

void mb86901_device::execute_add(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 173, "Appendix C - ISP Descriptions - Add Instructions" (SPARCv8.pdf, pg. 170)

	operand2 := if (i = 0) then r[rs2] else sign_extend(simm13);

	if (ADD or ADDcc) then
	    result <- r[rs1] + operand2;
	else if (ADDX or ADDXcc) then
	    result <= r[rs1] + operand2 + C;
	next;

	if (rd != 0) then
	    r[rd] <- result;

	if (ADDcc or ADDXcc) then (
	    N <- result<31>;
	    Z <- if (result = 0) then 1 else 0;
	    V <- (r[rs1]<31> and operand2<31> and (not result<31>)) or
	        ((not r[rs1]<31>) and (not operand2<31>) and result<31>);
	    C <- (r[rs1]<31> and operand2<31>) or
	        ((not result<31>) and (r[rs1]<31> or operand2<31>))
	);
	*/
	UINT32 operand2 = USEIMM ? SIMM13 : RS2REG;

	UINT32 result = 0;
	if (ADD || ADDCC)
		result = RS1REG + operand2;
	else if (ADDX || ADDXCC)
		result = RS1REG + operand2 + ICC_C;


	if (RD != 0)
		RDREG = result;

	if (ADDCC || ADDXCC)
	{
		CLEAR_ICC;
		PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
		PSR |= (result == 0) ? PSR_Z_MASK : 0;
		PSR |= ((BIT31(RS1REG) && BIT31(operand2) && !BIT31(result)) ||
				(!BIT31(RS1REG) && !BIT31(operand2) && BIT31(result))) ? PSR_V_MASK : 0;
		PSR |= ((BIT31(RS1REG) && BIT31(operand2)) ||
				(!BIT31(result) && (BIT31(RS1REG) || BIT31(operand2)))) ? PSR_C_MASK : 0;
	}
}


//-------------------------------------------------
//  execute_taddcc - execute a tagged add-type
//  opcode
//-------------------------------------------------

void mb86901_device::execute_taddcc(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 173, "Appendix C - ISP Descriptions - Tagged Add Instructions" (SPARCv8.pdf, pg. 170)

	operand2 := if (i = 0) then r[rs2] else sign_extend(simm13);

	result <- r[rs1] + operand2;
	next;

	temp_V <- (r[rs1]<31> and operand2<31> and (not result<31>)) or
	          ((not r[rs1]<31>) and (not operand2<31>) and result<31>) or
	          (r[rs1]<1:0> != 0 or operand2<1:0> != 0);
	next;

	if (TADDccTV and (temp_V = 1)) then (
	    trap <- 1;
	    tag_overflow <- 1
	) else (
	    N <- result<31>;
	    Z <- if (result = 0) then 1 else 0;
	    V <- temp_V;
	    C <- (r[rs1]<31> and operand2<31>) or
	         ((not result<31>) and (r[rs1]<31> or operand2<31>));
	    if (rd != 0) then
	        r[rd] <- result;
	);
	*/
	UINT32 operand2 = USEIMM ? SIMM13 : RS2REG;

	UINT32 result = RS1REG + operand2;

	bool temp_v = (BIT31(RS1REG) && BIT31(operand2) && !BIT31(result)) ||
					(!BIT31(RS1REG) && !BIT31(operand2) && BIT31(result)) ||
					((RS1REG & 3) != 0 || (RS1REG & 3) != 0) ? true : false;

	if (TADDCCTV && temp_v)
	{
		m_trap = 1;
		m_tag_overflow = true;
	}
	else
	{
		CLEAR_ICC;
		PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
		PSR |= (result == 0) ? PSR_Z_MASK : 0;
		PSR |= temp_v ? PSR_V_MASK : 0;
		PSR |= ((BIT31(RS1REG) && BIT31(operand2)) ||
				(!BIT31(result) && (BIT31(RS1REG) || BIT31(operand2)))) ? PSR_C_MASK : 0;

		if (RD != 0)
			RDREG = result;
	}
}


//-------------------------------------------------
//  execute_sub - execute a subtraction-type
//  opcode
//-------------------------------------------------

void mb86901_device::execute_sub(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 174, "Appendix C - ISP Descriptions - Subtract Instructions" (SPARCv8.pdf, pg. 171)

	operand2 := if (i = 0) then r[rs2] else sign_extend(simm13);

	if (SUB or SUBcc) then
	    result <- r[rs1] - operand2;
	else if (SUBX or SUBXcc) then
	    result <= r[rs1] - operand2 - C;
	next;

	if (rd != 0) then
	    r[rd] <- result;

	if (SUBcc or SUBXcc) then (
	    N <- result<31>;
	    Z <- if (result = 0) then 1 else 0;
	    V <- (r[rs1]<31> and (not operand2<31>) and (not result<31>)) or
	         ((not r[rs1]<31>) and operand2<31> and result<31>);
	    C <- ((not r[rs1]<31>) and operand2<31>) or
	         (result<31> and ((not r[rs1]<31>) or operand2<31>))
	);
	*/
	UINT32 operand2 = USEIMM ? SIMM13 : RS2REG;

	UINT32 result = 0;
	if (SUB || SUBCC)
		result = RS1REG - operand2;
	else if (SUBX || SUBXCC)
		result = RS1REG - operand2 - ICC_C;

	if (RD != 0)
		RDREG = result;

	if (SUBCC || SUBXCC)
	{
		CLEAR_ICC;
		PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
		PSR |= (result == 0) ? PSR_Z_MASK : 0;
		PSR |= ((BIT31(RS1REG) && !BIT31(operand2) && !BIT31(result)) ||
				(!BIT31(RS1REG) && BIT31(operand2) && BIT31(result))) ? PSR_V_MASK : 0;
		PSR |= ((!BIT31(RS1REG) && BIT31(operand2)) ||
				(BIT31(result) && (!BIT31(RS1REG) || BIT31(operand2)))) ? PSR_C_MASK : 0;
	}
}


//--------------------------------------------------
//  execute_tsubcc - execute a tagged subtract-type
//  opcode
//--------------------------------------------------

void mb86901_device::execute_tsubcc(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 174, "Appendix C - ISP Descriptions - Tagged Subtract Instructions" (SPARCv8.pdf, pg. 171)

	operand2 := if (i = 0) then r[rs2] else sign_extend(simm13);

	result <- r[rs1] - operand2;
	next;

	temp_V <- (r[rs1]<31> and (not operand2<31>) and (not result<31>)) or
	          ((not r[rs1]<31>) and operand2<31> and result<31>) or
	          (r[rs1]<1:0> != 0 or operand2<1:0> != 0);
	next;

	if (TSUBccTV and (temp_V = 1)) then (
	    trap <- 1;
	    tag_overflow <- 1
	) else (
	    N <- result<31>;
	    Z <- if (result = 0) then 1 else 0;
	    V <- temp_V;
	    C <- ((not r[rs1]<31>) and operand2<31>) or
	         (result<31> and ((not r[rs1]<31>) or operand2<31>));
	    if (rd != 0) then
	        r[rd] <- result;
	);
	*/

	UINT32 operand2 = USEIMM ? SIMM13 : RS2REG;

	UINT32 result = RS1REG - operand2;

	bool temp_v = (BIT31(RS1REG) && !BIT31(operand2) && !BIT31(result)) ||
					(!BIT31(RS1REG) && BIT31(operand2) && BIT31(result)) ||
					((RS1REG & 3) != 0 || (RS1REG & 3) != 0) ? true : false;

	if (TSUBCCTV && temp_v)
	{
		m_trap = 1;
		m_tag_overflow = 1;
	}
	else
	{
		CLEAR_ICC;
		PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
		PSR |= (result == 0) ? PSR_Z_MASK : 0;
		PSR |= temp_v ? PSR_V_MASK : 0;
		PSR |= ((!BIT31(RS1REG) && BIT31(operand2)) ||
				(BIT31(result) && (!BIT31(RS1REG) || BIT31(operand2)))) ? PSR_C_MASK : 0;

		if (RD != 0)
			RDREG = result;
	}
}


//-------------------------------------------------
//  execute_logical - execute a logical-type
//  opcode, and/or/xor/andn/orn/xnor
//-------------------------------------------------

void mb86901_device::execute_logical(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 172, "Appendix C - ISP Descriptions - Logical Instructions" (SPARCv8.pdf, pg. 169)

	operand2 := if (i = 0) then r[rs2] else sign_extend(simm13);

	if ( AND or  ANDcc) then result <- r[rs1] and operand2
	if (ANDN or ANDNcc) then result <- r[rs1] and not operand2
	if (  OR or   ORcc) then result <- r[rs1] or operand2
	if ( ORN or  ORNcc) then result <- r[rs1] or not operand2
	if ( XOR or  XORcc) then result <- r[rs1] xor operand2
	if (XNOR or XNORcc) then result <- r[rs1] xor not operand2;
	next;

	if (rd != 0) then r[rd] <- result;

	if (ANDcccc or ANDNcc or ORcc or ORNcc or XORcc or XNORcc) then (
	    N <- result<31>;
	    Z <- if (result = 0) then 1 else 0;
	    V <- 0
	    C <- 0
	);
	*/

	UINT32 operand2 = USEIMM ? SIMM13 : RS2REG;

	UINT32 result = 0;
	switch (OP3)
	{
		case OP3_AND:
		case OP3_ANDCC:
			result = RS1REG & operand2;
			break;
		case OP3_ANDN:
		case OP3_ANDNCC:
			result = RS1REG & ~operand2;
			break;
		case OP3_OR:
		case OP3_ORCC:
			result = RS1REG | operand2;
			break;
		case OP3_ORN:
		case OP3_ORNCC:
			result = RS1REG | ~operand2;
			break;
		case OP3_XOR:
		case OP3_XORCC:
			result = RS1REG ^ operand2;
			break;
		case OP3_XNOR:
		case OP3_XNORCC:
			result = RS1REG ^ ~operand2;
			break;
	}

	if (RD != 0)
		RDREG = result;

	if (ANDCC || ANDNCC || ORCC || ORNCC || XORCC || XNORCC)
	{
		CLEAR_ICC;
		PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
		PSR |= (result == 0) ? PSR_Z_MASK : 0;
	}
}


//-------------------------------------------------
//  execute_shift - execute a shift-type opcode,
//  sll/srl/sra
//-------------------------------------------------

void mb86901_device::execute_shift(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 172, "Appendix C - ISP Descriptions - Shift Instructions" (SPARCv8.pdf, pg. 169)

	shift_count := if (i = 0) then r[rs2]<4:0> else shcnt;

	if (SLL and (rd != 0) ) then
	    r[rd] <- shift_left_logical(r[rs1], shift_count)
	else if (SRL and (rd != 0) ) then
	    r[rd] <- shift_right_logical(r[rs1], shift_count)
	else if (SRA and (rd != 0) ) then
	    r[rd] <- shift_right_arithmetic(r[rs1], shift_count)
	*/
	UINT32 shift_count = USEIMM ? (SIMM13 & 31) : (RS2REG & 31);

	if (SLL && RD != 0)
		RDREG = RS1REG << shift_count;
	else if (SRL && RD != 0)
		RDREG = UINT32(RS1REG) >> shift_count;
	else if (SRA && RD != 0)
		RDREG = INT32(RS1REG) >> shift_count;
}


//--------------------------------------------------
//  execute_mulscc - execute a multiply step opcode
//--------------------------------------------------

void mb86901_device::execute_mulscc(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 175, "Appendix C - ISP Descriptions - Multiply Step Instruction" (SPARCv8.pdf, pg. 172)

	operand1 := (N xor V) [] (r[rs1]<31:1>);

	operand2 := (
	    if (Y<0> = 0) then 0
	    else if (i = 0) then r[rs2] else sign_extend(simm13)
	);

	result <- operand1 + operand2;
	Y <- r[rs1]<0> [] Y<31:1>;
	next;

	if (rd != 0) then (
	    r[rd] <- result;
	)
	N <- result<31>;
	Z <- if (result = 0) then 1 else 0;
	V <- (operand1<31> and operand2<31> and (not result<31>)) or
	     ((not operand1<31>) and (not operand2<31>) and result<31>);
	C <- (operand1<31> and operand2<31>) or
	     ((not result<31>) and (operand1<31> or operand2<31>))
	*/
	UINT32 operand1 = (ICC_N != ICC_V ? 0x80000000 : 0) | (RS1REG >> 1);

	UINT32 operand2 = (Y & 1) ? 0 : (USEIMM ? SIMM13 : RS2REG);

	UINT32 result = operand1 + operand2;
	Y = ((RS1REG & 1) ? 0x80000000 : 0) | (Y >> 1);

	if (RD != 0)
		RDREG = result;

	CLEAR_ICC;
	PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
	PSR |= (result == 0) ? PSR_Z_MASK : 0;
	PSR |= ((BIT31(operand1) && BIT31(operand2) && !BIT31(result)) ||
			(!BIT31(operand1) && !BIT31(operand2) && BIT31(result))) ? PSR_V_MASK : 0;
	PSR |= ((BIT31(operand1) && BIT31(operand2)) ||
			(!BIT31(result) && (BIT31(operand1) || BIT31(operand2)))) ? PSR_C_MASK : 0;
}


//-------------------------------------------------
//  execute_rdsr - execute a status register read
//  opcode
//-------------------------------------------------

void mb86901_device::execute_rdsr(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 182, "Appendix C - ISP Descriptions - Read State Register Instructions" (SPARCv8.pdf, pg. 179)

	if ((RDPSR or RDWIM or RDBTR
	    or (RDASR and (privileged_ASR(rs1) = 1))) and (S = 0)) then (
	    trap <- 1;
	    privileged_instruction <- 1;
	else if (illegal_instruction_ASR(rs1) = 1) then (
	    trap <- 1;
	    illegal_instruction <- 1
	else if (rd != 0) then (
	    if        (RDY) then r[rd] <- Y
	    else if (RDASR) then r[rd] <- ASR[rs1]
	    else if (RDPSR) then r[rd] <- PSR
	    else if (RDWIM) then r[rd] <- WIM
	    else if (RDTBR) then r[rd] <- TBR;
	);
	*/

	if (((WRPSR || WRWIM || WRTBR) || (WRASR && m_privileged_asr[RS1])) && IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
	}
	else if (m_illegal_instruction_asr[RS1])
	{
		m_trap = 1;
		m_illegal_instruction = 1;
	}
	else if (RD != 0)
	{
		if (RDASR)
		{
			if (RS1 == 0)
			{
				RDREG = Y;
			}
		}
		else if (RDPSR)
			RDREG = PSR;
		else if (RDWIM)
			RDREG = WIM;
		else if (RDTBR)
			RDREG = TBR;
	}
}


//-------------------------------------------------
//  execute_wrsr - execute a status register write
//  opcode
//-------------------------------------------------

void mb86901_device::execute_wrsr(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 183, "Appendix C - ISP Descriptions - Write State Register Instructions" (SPARCv8.pdf, pg. 180)

	operand2 := if (i = 0) then r[rs2] else sign_extend(simm13);
	result := r[rs1] xor operand2;

	if (WRY) then (
	    Y'''' <- result
	) else if (WRASR) then (
	    if ( (privileged_ASR(rd) = 1) and (S = 0) ) then (
	        trap <- 1;
	        privileged_instruction <- 1
	    ) else if (illegal_instruction_ASR(rd) = 1) then (
	        trap <- 1;
	        illegal_instruction <- 1
	    ) else (
	        ASR[rd]'''' <- result
	    )
	) else if (WRPSR) then (
	    if (S = 0) then (
	        trap <- 1;
	        privileged_instruction <- 1
	    ) else if (result<4:0> >= NWINDOWS) then (
	        trap <- 1;
	        illegal_instruction <- 1
	    ) else (
	        PSR'''' <- result
	    )
	) else if (WRWIM) then (
	    if (S = 0) then (
	        trap <- 1;
	        privileged_instruction <- 1
	    ) else (
	        WIM'''' <- result
	    )
	) else if (WRBTR) then (
	    if (S = 0) then (
	        trap <- 1;
	        privileged_instruction <- 1
	    ) else (
	        WIM'''' <- result
	    )
	);
	*/
	UINT32 operand2 = USEIMM ? SIMM13 : RS2REG;

	UINT32 result = RS1REG ^ operand2;

	if (WRASR && RD == 0)
	{
		Y = result;
	}
	else if (WRASR)
	{
		if (m_privileged_asr[RD] && IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
		}
		else if (m_illegal_instruction_asr[RD])
		{
			m_trap = 1;
			m_illegal_instruction = 1;
		}
		else
		{
			// SPARCv8
		}
	}
	else if (WRPSR)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
		}
		else if ((result & 31) >= NWINDOWS)
		{
			m_trap = 1;
			m_illegal_instruction = 1;
		}
		else
		{
			PSR = result &~ PSR_ZERO_MASK;
		}
	}
	else if (WRWIM)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
		}
		else
		{
			WIM = result & 0x7f;
		}
	}
	else if (WRTBR)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
		}
		else
		{
			TBR = result & 0xfffff000;
		}
	}
}


//-------------------------------------------------
//  execute_rett - execute a return-from-trap
//  opcode
//-------------------------------------------------

void mb86901_device::execute_rett(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 181, "Appendix C - ISP Descriptions - Return from Trap Instructions" (SPARCv8.pdf, pg. 178)

	new_cwp <- (CWP + 1) modulo NWINDOWS;
	address <- r[rs1] + (if (i = 0) then r[rs2] else sign_extend(simm13));
	next;
	if (ET = 1) then (
	    trap <- 1;
	    if (S = 0) then privileged_instruction <- 1
	    else if (S != 0) then illegal_instruction <- 1
	) else if (S = 0) then (
	    trap <- 1;
	    privileged_instruction <- 1
	    tt <- 00000011; { trap type for privileged_instruction }
	    execute_mode <- 0;
	    error_mode = 1
	) else if ((WIM and (1 << new_cwp)) != 0) then (
	    trap <- 1;
	    window_underflow <- 1;
	    tt <- 00000110; { trap type for window_underflow }
	    execute_mode = 0;
	    error_mode = 1
	) else if (address<1:0> != 0) then (
	    trap = 1;
	    mem_address_not_aligned = 1;
	    tt = 7; { trap type for mem_address_not_aligned }
	    execute_mode = 0;
	    error_mode = 1
	) else (
	    ET <- 1;
	    PC <- nPC;
	    nPC <- address;
	    CWP <- new_cwp;
	    S <- PS
	)
	*/

	UINT8 new_cwp = (CWP + 1) % NWINDOWS;
	UINT32 address = RS1REG + (USEIMM ? SIMM13 : RS2REG);
	if (ET)
	{
		m_trap = 1;
		if (IS_USER) m_privileged_instruction = 1;
		else m_illegal_instruction = 1;
	}
	else if (IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		m_tt = 3;
		m_execute_mode = 0;
		m_error_mode = 1;
	}
	else if ((WIM & (1 << new_cwp)) != 0)
	{
		m_trap = 1;
		m_window_underflow = 1;
		m_tt = 6;
		m_execute_mode = 0;
		m_error_mode = 1;

	}
	else if (address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		m_tt = 7;
		m_execute_mode = 0;
		m_error_mode = 1;
	}
	else
	{
		ET = 1;
		PC = nPC;
		nPC = address;
		CWP = new_cwp;
		S = PS;
	}

	MAKE_PSR;
	update_gpr_pointers();
}



//-------------------------------------------------
// execute_saverestore - execute a save or restore
// opcode
//-------------------------------------------------

void mb86901_device::execute_saverestore(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 177, "Appendix C - ISP Descriptions - SAVE and RESTORE Instructions" (SPARCv8.pdf, pg. 174)

	operand2 := if (i = 0) then r[rs2] else sign_extend(simm13);

	if (SAVE) then (
	    new_cwp <- (CWP - 1) modulo NWINDOWS;
	    next;
	    if ((WIM and (1 << new_cwp)) != 0) then (
	        trap <- 1;
	        window_overflow <- 1
	    ) else (
	        result <- r[rs1] + operand2; { operands from old window }
	        CWP <- new_cwp
	    )
	) else if (RESTORE) then (
	    new_cwp <- (CWP + 1) modulo NWINDOWS;
	    next;
	    if ((WIM and (1 << new_cwp)) != 0) then (
	        trap <- 1;
	        window_overflow <- 1
	    ) else (
	        result <- r[rs1] + operand2; { operands from old window }
	        CWP <- new_cwp
	    )
	);
	next;
	if ((trap = 0) and (rd != 0)) then
	    r[rd] <- result { destination in new window }
	*/

	UINT32 rs1 = RS1REG;
	UINT32 operand2 = USEIMM ? SIMM13 : RS2REG;

	UINT32 result = 0;
	if (SAVE)
	{
		UINT8 new_cwp = ((CWP + NWINDOWS) - 1) % NWINDOWS;
		if ((WIM & (1 << new_cwp)) != 0)
		{
			m_trap = 1;
			m_window_overflow = 1;
		}
		else
		{
			result = rs1 + operand2;
			CWP = new_cwp;
		}
	}
	else if (RESTORE)
	{
		UINT8 new_cwp = (CWP + 1) % NWINDOWS;
		if ((WIM & (1 << new_cwp)) != 0)
		{
			m_trap = 1;
			m_window_underflow = 1;
		}
		else
		{
			result = rs1 + operand2;
			CWP = new_cwp;
		}
	}

	MAKE_PSR;
	update_gpr_pointers();

	if (m_trap == 0 && RD != 0)
		RDREG = result;
}


//-------------------------------------------------
//  execute_jmpl - execute a jump and link opcode
//-------------------------------------------------

void mb86901_device::execute_jmpl(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 180, "Appendix C - ISP Descriptions - SAVE and RESTORE Instructions" (SPARCv8.pdf, pg. 177)

	jump_address <- r[rs1] + (if (i = 0) then r[rs2] else sign_extend(simm13));
	next;
	if (jump_address<1:0> != 0) then (
	    trap <- 1;
	    mem_address_not_aligned <- 1
	) else (
	    if (rd != 0) then r[rd] <- PC;
	    PC <- nPC;
	    nPC <- jump_address
	)
	*/

	UINT32 jump_address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (jump_address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
	}
	else
	{
		if (RD != 0)
			RDREG = PC;
		PC = nPC;
		nPC = jump_address;
	}
}


//-------------------------------------------------
//  execute_group2 - execute an opcode in group 2,
//  mostly ALU ops
//-------------------------------------------------

void mb86901_device::execute_group2(UINT32 op)
{
	switch (OP3)
	{
		case OP3_ADD:
		case OP3_ADDX:
		case OP3_ADDCC:
		case OP3_ADDXCC:
			execute_add(op);
			break;

		case OP3_SUB:
		case OP3_SUBX:
		case OP3_SUBCC:
		case OP3_SUBXCC:
			execute_sub(op);
			break;

		case OP3_TADDCC:
		case OP3_TADDCCTV:
			execute_taddcc(op);
			break;

		case OP3_TSUBCC:
		case OP3_TSUBCCTV:
			execute_tsubcc(op);
			break;

		case OP3_AND:
		case OP3_OR:
		case OP3_XOR:
		case OP3_ANDN:
		case OP3_ORN:
		case OP3_XNOR:
		case OP3_ANDCC:
		case OP3_ORCC:
		case OP3_XORCC:
		case OP3_ANDNCC:
		case OP3_ORNCC:
		case OP3_XNORCC:
			execute_logical(op);
			break;

		case OP3_MULSCC:
			execute_mulscc(op);
			break;

		case OP3_SLL:
		case OP3_SRL:
		case OP3_SRA:
			execute_shift(op);
			break;

		case OP3_RDASR:
		case OP3_RDPSR:
		case OP3_RDWIM:
		case OP3_RDTBR:
			execute_rdsr(op);
			break;

		case OP3_WRASR:
		case OP3_WRPSR:
		case OP3_WRWIM:
		case OP3_WRTBR:
			execute_wrsr(op);
			break;

		case OP3_FPOP1:
		case OP3_FPOP2:
			// Not yet implemented
			break;

		case OP3_JMPL:
			execute_jmpl(op);
			break;

		case OP3_RETT:
			execute_rett(op);
			break;

		case OP3_TICC:
			execute_ticc(op);
			break;

		case OP3_SAVE:
		case OP3_RESTORE:
			execute_saverestore(op);
			break;

#if SPARCV8
		case OP3_UMUL:
		case OP3_SMUL:
		case OP3_UMULCC:
		case OP3_SMULCC:
			execute_mul(op);
			break;

		case OP3_UDIV:
		case OP3_SDIV:
		case OP3_UDIVCC:
		case OP3_SDIVCC:
			execute_div(op);
			break;

		case OP3_CPOP1:
		case OP3_CPOP2:
			break;
#endif

		default:
			m_trap = 1;
			m_illegal_instruction = 1;
			break;
	}
}



//-------------------------------------------------
//  update_gpr_pointers - cache pointers to
//  the registers in our current window
//-------------------------------------------------

void mb86901_device::update_gpr_pointers()
{
	for (int i = 0; i < 8; i++)
	{
		m_regs[ 8 + i] = &m_r[8 + (( 0 + m_cwp * 16 + i) % (NWINDOWS * 16))];
		m_regs[16 + i] = &m_r[8 + (( 8 + m_cwp * 16 + i) % (NWINDOWS * 16))];
		m_regs[24 + i] = &m_r[8 + ((16 + m_cwp * 16 + i) % (NWINDOWS * 16))];
	}
}


//-------------------------------------------------
//  execute_store - execute a store-type opcode
//-------------------------------------------------

void mb86901_device::execute_store(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 165, "Appendix C - ISP Descriptions - Store Instructions" (SPARCv8.pdf, pg. 162)

	if ( (S = 0) and (STDA or STA or STHA or STBA or STDFQ or STDCQ) ) then (
	    trap <- 1;
	    privileged_instruction <- 1
	) else if ((i = 1) and (STDA or STA or STHA or STBA)) then (
	    trap <- 1;
	    illegal_instruction <- 1
	);
	next;
	if (trap = 0) then (
	    if (STD or ST or STH or STB or STF or STDF or STFSR or STDFQ or STCSR or STC or STDC or STDCQ) then (
	        address <- r[rs1] + (if (i = 0) then r[rs2] else sign_extend(simm13));
	        addr_space <- (if (S = 0) then 10 else 11)
	    ) else if (STDA or STA or STHA or STBA) then (
	        address <- r[rs1] + r[rs2];
	        addr_space <- asi
	    );
	    if ((STF or STDF or STFSR or STDFQ) and
	       ((EF = 0) or (bp_FPU_present = 0)) ) then (
	        trap <- 1;
	        fp_disabled <- 1;
	    );
	    if ((STC or STDC or STCSR or STDCQ) and
	       ((EC = 0) or (bp_CP_present = 0)) ) then (
	        trap <- 1;
	        cp_disabled <- 1;
	    )
	);
	next;
	if (trap = 0) then (
	    if ((STH or STHA) and (address<0> != 0)) then (
	        trap <- 1;
	        mem_address_not_aligned <- 1
	    ) else if ((ST or STA or STF or STFSR or STC or STCSR) and (address<1:0> != 0)) then (
	        trap <- 1;
	        mem_address_not_aligned <- 1
	    ) else if ((STD or STDA or STDF or STDFQ or STDC or STDCQ) and (address<2:0> != 0)) then (
	        trap <- 1;
	        mem_address_not_aligned <- 1
	    ) else (
	        if (STDFQ and ((implementation has no floating-point queue) or (FSR.qne = 0))) then (
	            trap <- 1;
	            fp_exception <- 1;
	            ftt <- sequence_error;
	        );
	        if (STDCQ and ((implementation has no coprocessor queue)) then (
	            trap <- 1;
	            cp_exception <- 1;
	            { possibly additional implementation-dependent actions }
	        );
	        if (STDF and (rd<0> != 0)) then (
	            trap <- 1;
	            fp_exception <- 1;
	            ftt <- invalid_fp_register;
	        )
	    )
	);
	next;
	if (trap = 0) then (
	    if (STF) then ( byte_mask <- 1111; data0 <- f[rd] )
	    else if (STC) then ( byte_mask <- 1111; data0 <- implementation_dependent_value )
	    else if (STDF) then ( byte_mask <- 1111; data0 <- f[rd & 0x1e] )
	    else if (STDC) then ( byte_mask <- 1111; data0 <- implementation_dependent_value )
	    else if (STD or STDA) then ( byte_mask <- 1111; data0 <- r[rd & 0x1e] )
	    else if (STDFQ) then ( byte_mask <- 1111; data0 <- implementation_dependent_value )
	    else if (STDCQ) then ( byte_mask <- 1111; data0 <- implementation_dependent_value )
	    else if (STFSR) then (
	        while ((FSR.qne = 1) and (trap = 0)) (
	            // wait for pending floating-point instructions to complete
	        )
	        next;
	        byte_mask <- 1111; data0 <- FSR
	    ) else if (STCSR) then (
	        { implementation-dependent actions }
	        byte_mask <- 1111; data0 <- CSR
	    ) else if (ST or STA) then ( byte_mask <- 1111; data0 = r[rd] )
	    else if (STH or STHA) then (
	        if (address<1:0> = 0) then (
	            byte_mask <- 1100; data0 <- shift_left_logical(r[rd], 16) )
	        else if (address<1:0> = 2) then (
	            byte_mask <- 0011; data0 <- r[rd] )
	    ) else if (STB or STBA) then (
	        if (address<1:0> = 0) then (
	            byte_mask <- 1000; data0 <- shift_left_logical(r[rd], 24) )
	        ) else if (address<1:0> = 1) then (
	            byte_mask <- 0100; data0 <- shift_left_logical(r[rd], 16) )
	        ) else if (address<1:0> = 2) then (
	            byte_mask <- 0010; data0 <- shift_left_logical(r[rd], 8) )
	        ) else if (address<1:0> = 3) then (
	            byte_mask <- 0001; data0 <- r[rd] )
	        )
	    );
	);
	next;
	if (trap = 0) then (
	    MAE <- memory_write(addr_space, address, byte_mask, data1);
	    next;
	    if (MAE = 1) then (
	        trap <- 1;
	        data_access_exception <- 1
	    )
	);
	if ((trap = 0) and (STD or STDA or STDF or STDC or STDFQ or STDCQ)) then (
	    if (STD or STDA) then ( data1 <- r[rd or 00001] )
	    else if (STDF) then ( data1 <- f[rd or 00001] )
	    else if (STDC) then ( data1 <- implementation_dependent_value }
	    else if (STDFQ) then ( data1 <- implementation_dependent_value }
	    else if (STDCQ) then ( data1 <- implementation_dependent_value }
	    next;
	    MAE <- memory_write(addr_space, address + 4, 1111, data1);
	    next;
	    if (MAE = 1) then ( { MAE = 1 only due to a "non-resumable machine-check error" }
	        trap <- 1;
	        data_access_exception <- 1
	    )
	);
	*/

	if (IS_USER && (STDA || STA || STHA || STBA || STDFQ || STDCQ))
	{
		m_trap = 1;
		m_privileged_instruction = 1;
	}
	else if (USEIMM && (STDA || STA || STHA || STBA))
	{
		m_trap = 1;
		m_illegal_instruction = 1;
	}

	UINT32 address = 0;
	UINT8 addr_space = 0;
	if (!m_trap)
	{
		if (STD || ST || STH || STB || STF || STDF || STFSR || STDFQ || STCSR || STC || STDC || STDCQ)
		{
			address = RS1REG + (USEIMM ? SIMM13 : RS2REG);
			addr_space = (IS_USER ? 10 : 11);
		}
		else if (STDA || STA || STHA || STBA)
		{
			address = RS1REG + RS2REG;
			addr_space = ASI;
		}
		if ((STF || STDF || STFSR || STDFQ) && (!EF || !m_bp_fpu_present))
		{
			m_trap = 1;
			m_fp_disabled = 1;
		}
		if ((STC || STDC || STCSR || STDCQ) && (!EC || !m_bp_cp_present))
		{
			m_trap = 1;
			m_cp_disabled = 1;
		}
	}

	if (!m_trap)
	{
		if ((STH || STHA) && ((address & 1) != 0))
		{
			m_trap = 1;
			m_mem_address_not_aligned = 1;
		}
		else if ((ST || STA || STF || STFSR || STC || STCSR) && ((address & 3) != 0))
		{
			m_trap = 1;
			m_mem_address_not_aligned = 1;
		}
		else if ((STD || STDA || STDF || STDFQ || STDC || STDCQ) && ((address & 7) != 0))
		{
			m_trap = 1;
			m_mem_address_not_aligned = 1;
		}
		else
		{
			if (STDFQ)
			{
				// assume no floating-point queue for now
				m_trap = 1;
				m_fp_exception = 1;
				m_ftt = m_fpu_sequence_err;
			}
			if (STDCQ)
			{
				// assume no coprocessor queue for now
				m_trap = 1;
				m_cp_exception = 1;
				// { possibly additional implementation-dependent actions }
			}
			if (STDF && ((RD & 1) != 0))
			{
				m_trap = 1;
				m_fp_exception = 1;
				m_ftt = 0xff;
			}
		}
	}

	UINT32 data0 = 0;
	if (!m_trap)
	{
		//UINT8 byte_mask;
		if (STF)
		{
			//byte_mask = 15;
			data0 = FREG(RD);
		}
		else if (STC)
		{
			//byte_mask = 15;
			data0 = 0;
		}
		else if (STDF)
		{
			//byte_mask = 15;
			data0 = FREG(RD & 0x1e);
		}
		else if (STDC)
		{
			//byte_mask = 15;
			data0 = 0;
		}
		else if (STD || STDA)
		{
			//byte_mask = 15;
			data0 = REG(RD & 0x1e);
		}
		else if (STDFQ)
		{
			//byte_mask = 15;
			data0 = 0;
		}
		else if (STDCQ)
		{
			//byte_mask = 15;
			data0 = 0;
		}
		else if (STFSR)
		{
			// while ((FSR.qne = 1) and (trap = 0)) (
			//     wait for pending floating-point instructions to complete
			// )
			// next;
			//byte_mask = 15;
			data0 = FSR;
		}
		else if (STCSR)
		{
			// { implementation-dependent actions }
			//byte_mask = 15;
			data0 = 0;
		}
		else if (ST || STA)
		{
			//byte_mask = 15;
			data0 = REG(RD);
		}
		else if (STH || STHA)
		{
			if ((address & 3) == 0)
			{
				//byte_mask = 12;
				data0 = REG(RD) << 16;
			}
			else if ((address & 3) == 2)
			{
				//byte_mask = 3;
				data0 = REG(RD);
			}
		}
		else if (STB || STBA)
		{
			if ((address & 3) == 0)
			{
				//byte_mask = 8;
				data0 = REG(RD) << 24;
			}
			else if ((address & 3) == 1)
			{
				//byte_mask = 4;
				data0 = REG(RD) << 16;
			}
			else if ((address & 3) == 2)
			{
				//byte_mask = 2;
				data0 = REG(RD) << 8;
			}
			else if ((address & 3) == 3)
			{
				//byte_mask = 1;
				data0 = REG(RD);
			}
		}
	}

	if (!m_trap)
	{
		write_sized_word(addr_space, address, data0, (ST || STA || STD || STDA || STF || STDF || STDFQ || STFSR || STC || STDC || STDCQ || STCSR) ? 4 : ((STH || STHA) ? 2 : 1));
		if (MAE)
		{
			m_trap = 1;
			m_data_access_exception = 1;
		}
	}
	if (!m_trap && (STD || STDA || STDF || STDC || STDFQ || STDCQ))
	{
		UINT32 data1 = 0;
		if (STD || STDA)
		{
			data1 = REG(RD | 1);
		}
		else if (STDF)
		{
			data1 = FREG(RD | 1);
		}
		else if (STDC)
		{
			data1 = 0;
		}
		else if (STDFQ)
		{
			data1 = 0;
		}
		else if (STDCQ)
		{
			data1 = 0;
		}

		write_sized_word(addr_space, address + 4, data1, 4);
		if (MAE)
		{
			m_trap = 1;
			m_data_access_exception = 1;
		}
	}
}

//-------------------------------------------------
//  execute_load - execute a load-type opcode
//-------------------------------------------------

void mb86901_device::execute_load(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 163, "Appendix C - ISP Descriptions - C.9. Instruction Defintions - Load Instructions" (SPARCv8.pdf, pg. 160)

	if (LDD or LD or LDSH or LDUH or LDSB or LDUB or LDDF or LDF or LDFSR or LDDC or LDC or LDCSR) then (
	    address <- r[rs1] + (if (i = 0) then r[rs2] else sign_extend(simm13));
	    addr_space <- (if (S = 0) then 10 else 11)
	) else if (LDDA or LDA or LDSHA or LDUHA or LDSBA or LDUBA) then (
	    if (S = 0) then (
	        trap <- 1;
	        privileged_instruction <- 1
	    ) else if (i = 1) then (
	        trap <- 1;
	        illegal_instruction <- 1
	    ) else (
	        address <- r[rs1] + r[rs2];
	        addr_space <- asi
	    )
	)
	next;
	if (trap = 0) then (
	    if ( (LDF or LDDF or LDFSR) and ((EF = 0) or (bp_FPU_present = 0)) then (
	        trap <- 1;
	        fp_disabled <- 1
	    ) else if ( (LDC or LDDC or LDCSR) and ((EC = 0) or (bp_CP_present = 0)) then (
	        trap <- 1;
	        cp_disabled <- 1
	    ) else if ( ( (LDD or LDDA or LDDF or LDDC) and (address<2:0> != 0)) or
	        ((LD or LDA or LDF or LDFSR or LDC or LDCSR) and (address<1:0> != 0)) or
	        ((LDSH or LDSHA or LDUH or LDUHA) and address<0> != 0) ) then (
	        trap <- 1;
	        mem_address_not_aligned <- 1
	    ) else if (LDDF and (rd<0> != 0)) then (
	        trap <- 1;
	        fp_exception <- 1;
	        ftt <- invalid_fpr_register
	    ) else if ((LDF or LDDF or LDFSR) and (an FPU sequence error is detected)) then (
	        trap <- 1;
	        fp_exception <- 1;
	        ftt <- sequence_error
	    ) else if ((LDC or LDDC or LDCSR) and (a CP sequence error is detected)) then (
	        trap <- 1;
	        cp_exception <- 1;
	        { possibly additional implementation-dependent actions }
	    )
	);
	next;
	if (trap = 0) then {
	    (data, MAE) <- memory_read(addr_space, address);
	    next;
	    if (MAE = 1) then (
	        trap <- 1;
	        data_access_exception <- 1;
	    ) else (
	        if (LDSB or LDSBA or LDUB or LDUBA) then (
	            if      (address<1:0> = 0) then byte <- data<31:24>
	            else if (address<1:0> = 1) then byte <- data<23:16>
	            else if (address<1:0> = 2) then byte <- data<15: 8>
	            else if (address<1:0> = 3) then byte <- data< 7: 0>
	            next;
	            if (LDSB or LDSBA) then
	                word0 <- sign_extend_byte(byte)
	            else
	                word0 <- zero_extend_byte(byte)
	        ) else if (LDSH or LDSHA or LDUH or LDUHA) then (
	            if      (address<1:0> = 0) then halfword <- data<31:16>
	            else if (address<1:0> = 2) then halfword <- data<15: 0>
	            next;
	            if (LDSH or LDSHA) then
	                word0 <- sign_extend_halfword(halfword)
	            else
	                word0 <- zero_extend_halfword(halfword)
	        ) else
	            word0 <- data
	    )
	);
	next;
	if (trap = 0) then (
	    if ( (rd != 0) and (LD or LDA or LDSH or LDSHA
	        or LDUHA or LDUH or LDSB or LDSBA or LDUB or LDUBA) ) then
	            r[rd] <- word0
	    else if (LDF) then f[rd] <- word0
	    else if (LDC) then { implementation-dependent actions }
	    else if (LDFSR) then FSR <- word0
	    else if (LDCSR) then CSR <- word0
	    else if (LDD or LDDA) then r[rd and 11110] <- word0
	    else if (LDDF) then f[rd and 11110] <- word0
	    else if (LDDC) then { implementation-dependent actions }
	);
	next;
	if (((trap = 0) and (LDD or LDDA or LDDF or LDDC)) then (
	    (word1, MAE) <- memory_read(addr_space, address + 4);
	    next;
	    if (MAE = 1) then ( { MAE = 1 only due to a "non-resumable machine-check error" }
	        trap <- 1;
	        data_access_exception <- 1 )
	    else if (LDD or LDDA) then r[rd or 1] <- word1
	    else if (LDDF) then f[rd or 1] <- word1
	    else if (LDDC) then { implementation-dependent actions }
	);
	*/

	UINT32 address = 0;
	UINT8 addr_space = 0;
	if (LDD || LD || LDSH || LDUH || LDSB || LDUB || LDDF || LDF || LDFSR || LDDC || LDC || LDCSR)
	{
		address = RS1REG + (USEIMM ? SIMM13 : RS2REG);
		addr_space = (IS_USER ? 10 : 11);
	}
	else if (LDDA || LDA || LDSHA || LDUHA || LDSBA || LDUBA)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
		}
		else if (USEIMM)
		{
			m_trap = 1;
			m_illegal_instruction = 1;
		}
		else
		{
			address = RS1REG + RS2REG;
			addr_space = ASI;
		}
	}

	if (!m_trap)
	{
		if ((LDF || LDDF || LDFSR) && (EF == 0 || m_bp_fpu_present == 0))
		{
			m_trap = 1;
			m_fp_disabled = 1;
		}
		else if ((LDC || LDDC || LDCSR) && (EC == 0 || m_bp_cp_present == 0))
		{
			m_trap = 1;
			m_cp_disabled = 1;
		}
		else if (((LDD || LDDA || LDDF || LDDC) && ((address & 7) != 0)) ||
				((LD || LDA || LDF || LDFSR || LDC || LDCSR) && ((address & 3) != 0)) ||
				((LDSH || LDSHA || LDUH || LDUHA) && ((address & 1) != 0)))
		{
			m_trap = 1;
			m_mem_address_not_aligned = 1;
		}
		else if (LDDF && ((RD & 1) != 0))
		{
			m_trap = 1;
			m_fp_exception = 1;
			m_ftt = 0xff;
		}
		else if ((LDF || LDDF || LDFSR) && m_fpu_sequence_err != 0)
		{
			m_trap = 1;
			m_fp_exception = 1;
			m_ftt = m_fpu_sequence_err;
		}
		else if ((LDC || LDDC || LDCSR) && m_cp_sequence_err != 0)
		{
			m_trap = 1;
			m_cp_exception = 1;
			// possibly additional implementation-dependent actions
		}
	}

	UINT32 word0(0);
	if (!m_trap)
	{
		UINT32 data = read_sized_word(addr_space, address, (LD || LDD || LDA || LDDA) ? 4 : ((LDUH || LDSH || LDUHA || LDSHA) ? 2 : 1));

		if (m_mae)
		{
			m_trap = 1;
			m_data_access_exception = 1;
		}
		else
		{
			if (LDSB || LDSBA || LDUB || LDUBA)
			{
				UINT8 byte = 0;
				if ((address & 3) == 0) byte = (data >> 24) & 0xff;
				else if ((address & 3) == 1) byte = (data >> 16) & 0xff;
				else if ((address & 3) == 2) byte = (data >> 8) & 0xff;
				else if ((address & 3) == 3) byte = data & 0xff;

				if (LDSB || LDSBA)
					word0 = (((INT32)byte) << 24) >> 24;
				else
					word0 = byte;
			}
			else if (LDSH || LDSHA || LDUH || LDUHA)
			{
				UINT16 halfword = 0;
				if ((address & 3) == 0) halfword = (data >> 16) & 0xffff;
				else if ((address & 3) == 2) halfword = data & 0xffff;

				if (LDSH || LDSHA)
				{
					word0 = (((INT32)halfword) << 16) >> 16;
				}
				else
				{
					word0 = halfword;
				}
			}
			else
			{
				word0 = data;
			}
		}
	}

	if (!m_trap)
	{
		if ((RD != 0) && (LD || LDA || LDSH || LDSHA || LDUHA || LDUH || LDSB || LDSBA || LDUB || LDUBA))
		{
			RDREG = word0;
		}
		else if (LDF) FDREG = word0;
		else if (LDC) { } // implementation-dependent actions
		else if (LDFSR) FSR = word0;
		else if (LDD || LDDA) REG(RD & 0x1e) = word0;
		else if (LDDF) FREG(RD & 0x1e) = word0;
		else if (LDDC) { } // implementation-dependent actions
	}

	if (!m_trap && (LDD || LDDA || LDDF || LDDC))
	{
		UINT32 word1 = read_sized_word(addr_space, address + 4, 4);
		if (MAE)
		{
			m_trap = 1;
			m_data_access_exception = 1;
		}
		else if (LDD || LDDA) REG(RD | 1) = word1;
		else if (LDDF) FREG(RD | 1) = word1;
		else if (LDDC) { } // implementation-dependent actions
	}
}


//-------------------------------------------------
//  execute_ldstub - execute an atomic load-store
//  instruction
//-------------------------------------------------

void mb86901_device::execute_ldstub(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 169, "Appendix C - ISP Descriptions - Atomic Load-Store Unsigned Byte Instructions" (SPARCv8.pdf, pg. 166)

	if (LDSTUB) then (
	    address <- r[rs1] + (if (i = 0) then r[rs2] else sign_extend(simm13));
	    addr_space <- (if (S = 0) then 10 else 11)
	} else if (LDSTUBA) then (
	    if (S = 0) then (
	        trap <- 1;
	        privileged_instruction <- 1
	    ) else if (i = 1) then (
	        trap <- 1;
	        illegal_instruction <- 1
	    ) else (
	        address <- r[rs1] + r[rs2];
	        addr_space <- asi
	    )
	);
	next;
	if (trap = 0) then (
	    while ( (pb_block_ldst_byte = 1) or (pb_block_ldst_word = 1) ) then (
	        { wait for lock(s) to be lifted }
	        { an implementation actually need only block when another LDSTUB or SWAP
	          is pending on the same byte in memory as the one addressed by this LDSTUB }
	    };
	    next;
	    pb_block_ldst_byte <- 1;
	    next;
	    (data, MAE) <- memory_read(addr_space, address);
	    next;
	    if (MAE = 1) then (
	        trap <- 1;
	        data_access_exception <- 1
	    )
	)
	next;
	if (trap = 0) then (
	    if      (address<1:0> = 0) then ( byte_mask <- 1000 )
	    else if (address<1:0> = 1) then ( byte_mask <- 0100 )
	    else if (address<1:0> = 2) then ( byte_mask <- 0010 )
	    else if (address<1:0> = 3) then ( byte_mask <- 0001 )
	    ;
	    next;
	    MAE <- memory_write(addr_space, address, byte_mask, FFFFFFFF);
	    next;
	    pb_block_ldst_byte <- 0;
	    if (MAE = 1) then ( { MAE = 1 only due to a "non-resumable machine-check error" }
	        trap <- 1;
	        data_access_exception <- 1
	    ) else (
	        if      (address<1:0> = 0) then word <- zero_extend_byte(data<31:24>)
	        else if (address<1:0> = 1) then word <- zero_extend_byte(data<23:24>)
	        else if (address<1:0> = 2) then word <- zero_extend_byte(data<15: 8>)
	        else if (address<1:0> = 3) then word <- zero_extend_byte(data< 7: 0>)
	        next;
	        if (rd != 0) then r[rd] <- word
	    )
	);
	*/

	UINT32 address = 0;
	UINT8 addr_space = 0;
	if (LDSTUB)
	{
		address = RS1REG + (USEIMM ? SIMM13 : RS2REG);
		addr_space = (IS_USER ? 10 : 11);
	}
	else if (LDSTUBA)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
		}
		else if (USEIMM)
		{
			m_trap = 1;
			m_illegal_instruction = 1;
		}
		else
		{
			address = RS1REG + RS2REG;
			addr_space = ASI;
		}
	}

	UINT32 data(0);
	if (!m_trap)
	{
		while (m_pb_block_ldst_byte || m_pb_block_ldst_word)
		{
			// { wait for lock(s) to be lifted }
			// { an implementation actually need only block when another LDSTUB or SWAP
			//   is pending on the same byte in memory as the one addressed by this LDSTUB }
		}

		m_pb_block_ldst_byte = 1;

		data = read_sized_word(addr_space, address, 1);

		if (MAE)
		{
			m_trap = 1;
			m_data_access_exception = 1;
		}
	}

	if (!m_trap)
	{
		//UINT8 byte_mask;
		if ((address & 3) == 0)
		{
			//byte_mask = 8;
		}
		else if ((address & 3) == 1)
		{
			//byte_mask = 4;
		}
		else if ((address & 3) == 2)
		{
			//byte_mask = 2;
		}
		else if ((address & 3) == 3)
		{
			//byte_mask = 1;
		}
		write_sized_word(addr_space, address, 0xffffffff, 1);

		m_pb_block_ldst_byte = 0;

		if (MAE)
		{
			m_trap = 1;
			m_data_access_exception = 1;
		}
		else
		{
			UINT32 word;
			if ((address & 3) == 0)
			{
				word = (data >> 24) & 0xff;
			}
			else if ((address & 3) == 1)
			{
				word = (data >> 16) & 0xff;
			}
			else if ((address & 3) == 2)
			{
				word = (data >> 8) & 0xff;
			}
			else // if ((address & 3) == 3)
			{
				word = data & 0xff;
			}
			if (RD != 0)
				RDREG = word;
		}
	}
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
		case OP3_LD:
		case OP3_LDUB:
		case OP3_LDUH:
		case OP3_LDD:
		case OP3_LDSB:
		case OP3_LDSH:
		case OP3_LDA:
		case OP3_LDUBA:
		case OP3_LDUHA:
		case OP3_LDDA:
		case OP3_LDSBA:
		case OP3_LDSHA:
		case OP3_LDFPR:
		case OP3_LDFSR:
		case OP3_LDDFPR:
		case OP3_LDCPR:
		case OP3_LDCSR:
		case OP3_LDDCPR:
			execute_load(op);
			break;

		case OP3_ST:
		case OP3_STB:
		case OP3_STH:
		case OP3_STD:
		case OP3_STA:
		case OP3_STBA:
		case OP3_STHA:
		case OP3_STDA:
		case OP3_STFPR:
		case OP3_STFSR:
		case OP3_STDFQ:
		case OP3_STDFPR:
		case OP3_STCPR:
		case OP3_STCSR:
		case OP3_STDCQ:
		case OP3_STDCPR:
			execute_store(op);
			break;

		case OP3_LDSTUB:
		case OP3_LDSTUBA:
			execute_ldstub(op);
			break;

#if SPARCV8
		case OP3_SWAP:
		case OP3_SWAPA:
			execute_swap(op);
			break;
#endif
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

	switch(COND & 7)                            // COND & 8
	{                                           // 0        8
		case 0:     take = false; break;        // bn       ba
		case 1:     take = z; break;            // bz       bne
		case 2:     take = z | (n ^ z); break;  // ble      bg
		case 3:     take = n ^ v; break;        // bl       bge
		case 4:     take = c | z; break;        // bleu     bgu
		case 5:     take = c; break;            // bcs      bcc
		case 6:     take = n; break;            // bneg     bpos
		case 7:     take = v; break;            // bvs      bvc
	}

	if (COND & 8)
		take = !take;

	return take;
}


//-------------------------------------------------
//  execute_bicc - execute a branch opcode
//-------------------------------------------------

void mb86901_device::execute_bicc(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 178, "Appendix C - ISP Descriptions - Branch on Integer Condition Instructions" (SPARCv8.pdf, pg. 175)

	eval_icc := (
	    if (BNE)    then (if (Z = 0) then 1 else 0);
	    if (BE)     then (if (Z = 1) then 1 else 0);
	    if (BG)     then (if ((Z or (N xor V)) = 0) then 1 else 0);
	    if (BLE)    then (if ((Z or (N xor V)) = 1) then 1 else 0);
	    if (BGE)    then (if ((N xor V) = 0) then 1 else 0);
	    if (BL)     then (if ((N xor V) = 1) then 1 else 0);
	    if (BGU)    then (if ((C = 0) and (Z = 0)) then 1 else 0);
	    if (BLEU)   then (if ((C = 1) or (Z = 1)) then 1 else 0);
	    if (BCC)    then (if (C = 0) then 1 else 0);
	    if (BCS)    then (if (C = 1) then 1 else 0);
	    if (BPOS)   then (if (N = 0) then 1 else 0);
	    if (BNEG)   then (if (N = 1) then 1 else 0);
	    if (BVC)    then (if (V = 0) then 1 else 0);
	    if (BVS)    then (if (V = 1) then 1 else 0);
	    if (BA)     then 1;
	    if (BN)     then 0;
	)
	PC <- nPC;
	if (eval_icc = 1) then (
	    nPC <- PC + sign_extend(disp22[]00);
	    if (BA and (a = 1)) then
	        annul <- 1 { only for annulling Branch-Always }
	) else (
	    nPC <- nPC + 4;
	    if (a = 1) then
	        annul <- 1 { only for annulling branches other than BA }
	)
	*/

	bool branch_taken = evaluate_condition(op);
	UINT32 pc = PC;
	PC = nPC;
	if (branch_taken)
	{
		nPC = pc + DISP22;
		if (COND == COND_BA && ANNUL)
			m_annul = 1;
	}
	else
	{
		nPC = nPC + 4;
		if (ANNUL)
			m_annul = 1;
	}
}


//-------------------------------------------------
//  execute_ticc - execute a conditional trap
//-------------------------------------------------

void mb86901_device::execute_ticc(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 182, "Appendix C - ISP Descriptions - Trap on Integer Condition Instructions" (SPARCv8.pdf, pg. 179)

	trap_eval_icc := (
	    if (TNE)    then (if (Z = 0) then 1 else 0);
	    if (TE)     then (if (Z = 1) then 1 else 0);
	    if (TG)     then (if ((Z or (N xor V)) = 0) then 1 else 0);
	    if (TLE)    then (if ((Z or (N xor V)) = 1) then 1 else 0);
	    if (TGE)    then (if ((N xor V) = 0) then 1 else 0);
	    if (TL)     then (if ((N xor V) = 1) then 1 else 0);
	    if (TGU)    then (if ((C = 0) and (Z = 0)) then 1 else 0);
	    if (TLEU)   then (if ((C = 1) or (Z = 1)) then 1 else 0);
	    if (TCC)    then (if (C = 0) then 1 else 0);
	    if (TCS)    then (if (C = 1) then 1 else 0);
	    if (TPOS)   then (if (N = 0) then 1 else 0);
	    if (TNEG)   then (if (N = 1) then 1 else 0);
	    if (TVC)    then (if (V = 0) then 1 else 0);
	    if (TVS)    then (if (V = 1) then 1 else 0);
	    if (TA)     then 1;
	    if (TN)     then 0;
	)

	trap_number := r[rs1] + (if (i = 0) then r[rs2] else sign_extend(software_trap#));

	if (Ticc) then (
	    if (trap_eval_icc = 1) then (
	        trap <- 1;
	        trap_instruction <- 1;
	        ticc_trap_type <- trap_number<6:0>
	    ) else (
	        PC <- nPC;
	        nPC <- nPC + 4;
	    )
	);
	*/

	bool trap_eval_icc = evaluate_condition(op);

	UINT8 trap_number = RS1REG + (USEIMM ? SIMM7 : RS2REG);

	if (COND)
	{
		if (trap_eval_icc)
		{
			m_trap = 1;
			m_trap_instruction = 1;
			m_ticc_trap_type = trap_number & 0x7f;
		}
		else
		{
			PC = nPC;
			nPC = nPC + 4;
		}
	}
}



//-------------------------------------------------
//  select_trap - prioritize traps and perform any
//  additional functions from taking them
//-------------------------------------------------

void mb86901_device::select_trap()
{
	if (!m_trap)
		return;

	if (m_reset_trap)
	{
		m_trap = 0;
		return;
	}
	else if (!ET)
	{
		m_execute_mode = 0;
		m_error_mode = 1;
	}
	else if (m_data_store_error)
		m_tt = 0x2b;
	else if (m_instruction_access_error)
		m_tt = 0x21;
	else if (m_r_register_access_error)
		m_tt = 0x20;
	else if (m_instruction_access_exception)
		m_tt = 0x01;
	else if (m_privileged_instruction)
		m_tt = 0x03;
	else if (m_illegal_instruction)
		m_tt = 0x02;
	else if (m_fp_disabled)
		m_tt = 0x04;
	else if (m_cp_disabled)
		m_tt = 0x24;
	else if (m_unimplemented_FLUSH)
		m_tt = 0x25;
	else if (m_window_overflow)
		m_tt = 0x05;
	else if (m_window_underflow)
		m_tt = 0x06;
	else if (m_mem_address_not_aligned)
		m_tt = 0x07;
	else if (m_fp_exception)
		m_tt = 0x08;
	else if (m_cp_exception)
		m_tt = 0x28;
	else if (m_data_access_error)
		m_tt = 0x29;
	else if (m_data_access_exception)
		m_tt = 0x09;
	else if (m_tag_overflow)
		m_tt = 0x0a;
	else if (m_division_by_zero)
		m_tt = 0x2a;
	else if (m_trap_instruction)
		m_tt = 0x80 | m_ticc_trap_type;
	else if (m_interrupt_level > 0)
		m_tt = 0x10 | m_interrupt_level;

	m_instruction_access_exception = 0;
	m_illegal_instruction = 0;
	m_privileged_instruction = 0;
	m_fp_disabled = 0;
	m_cp_disabled = 0;
	m_window_overflow = 0;
	m_window_underflow = 0;
	m_mem_address_not_aligned = 0;
	m_fp_exception = 0;
	m_cp_exception = 0;
	m_data_access_exception = 0;
	m_tag_overflow = 0;
	m_division_by_zero = 0;
	m_trap_instruction = 0;
	m_interrupt_level = 0;
}


//-------------------------------------------------
//  execute_trap - prioritize and invoke traps
//  that have been flagged by the previous
//  instructions, if any.
//-------------------------------------------------

void mb86901_device::execute_trap()
{
	/* The SPARC Instruction Manual: Version 8, page 161, "Appendix C - C.8. Traps" (SPARCv8.pdf, pg. 158)

	select_trap; { see below }
	next;

	if (error_mode = 0) then (
	    ET <- 0;
	    PS <- S;
	    CWP <- (CWP - 1) modulo NWINDOWS;

	    next;
	    if (annul = 0) then (
	        r[17] <- PC;
	        r[18] <- nPC;
	    ) else { annul != 0) } (
	        r[17] <- nPC;
	        r[18] <- nPC + 4;
	        annul <- 0;
	    )

	    next;
	    S <- 1;
	    if (reset_trap = 0) then (
	        PC <- TBR;
	        nPC <- TBR + 4;
	    ) else { reset_trap = 1 } (
	        PC <- 0;
	        nPC <- 4;
	        reset_trap <- 0;
	    )
	);

	select_trap := (
	    if (reset_trap = 1) then { ignore ET, and leave tt unchanged }
	    else if (ET = 0) then (
	        execute_mode <- 0;
	        error_mode <- 1 )
	    else if (data_store_error = 1) then tt <- 00101011
	    else if (instruction_access_error = 1) then tt <- 00100001
	    else if (r_register_access_error = 1) then tt <- 00100000
	    else if (instruction_access_exception = 1) then tt <- 00000001
	    else if (privileged_instruction = 1) then tt <- 00000011
	    else if (illegal_instruction = 1) then tt <- 00000010
	    else if (fp_disabled = 1) then tt <- 00000100
	    else if (cp_disabled = 1) then tt <- 00100100
	    else if (unimplemented_FLUSH = 1) then tt <- 00100101
	    else if (window_overflow = 1) then tt <- 00000101
	    else if (window_underflow = 1) then tt <- 00000110
	    else if (mem_address_not_aligned = 1) then tt <- 00000111
	    else if (fp_exception = 1) then tt <- 00001000
	    else if (cp_exception = 1) then tt <- 00101000
	    else if (data_access_error = 1) then tt <- 00101001
	    else if (data_access_exception = 1) then tt <- 00001001
	    else if (tag_overflow = 1) then tt <- 00001010
	    else if (division_by_zero = 1) then tt <- 00101010
	    else if (trap_instruction = 1) then tt <- 1[]ticc_trap_type
	    else if (interrupt_level > 0) then tt <- 0001[]interrupt_level;

	    next;

	    trap <- 0;
	    instruction_access_exception <- 0;
	    illegal_instruction <- 0;
	    privileged_instruction <- 0;
	    fp_disabled <- 0;
	    cp_disabled <- 0;
	    window_overflow <- 0;
	    window_underflow <- 0;
	    mem_address_not_aligned <- 0;
	    fp_exception <- 0;
	    cp_exception <- 0;
	    data_access_exception <- 0;
	    tag_overflow <- 0;
	    division_by_zero <- 0;
	    trap_instruction <- 0;
	    interrupt_level <- 0;
	);
	*/

	if (!m_trap)
		return;

	select_trap();

	if (!m_error_mode)
	{
		ET = 0;
		PS = S;
		CWP = ((CWP + NWINDOWS) - 1) % NWINDOWS;

		if (m_annul == 0)
		{
			REG(17) = PC;
			REG(18) = nPC;
		}
		else
		{
			REG(17) = nPC;
			REG(18) = nPC + 4;
			m_annul = 0;
		}

		S = 1;
		if (!m_reset_trap)
		{
			PC = TBR;
			nPC = TBR + 4;
		}
		else
		{
			PC = 0;
			nPC = 4;
			m_reset_trap = 0;
		}
	}

	MAKE_PSR;
	update_gpr_pointers();
}


//-------------------------------------------------
//  complete_instruction_execution - execute a
//  single fetched instruction that has been
//  checked for FP-disabled, CP-disabled, and
//  validity.
//-------------------------------------------------

void mb86901_device::complete_instruction_execution(UINT32 op)
{
	switch (OP)
	{
	case OP_TYPE0:  // Bicc, SETHI, FBfcc
		switch (OP2)
		{
		case OP2_UNIMP: // unimp
			printf("unimp @ %x\n", PC);
			break;
		case OP2_BICC: // branch on integer condition codes
			execute_bicc(op);
			break;
		case OP2_SETHI: // sethi
			SET_RDREG(IMM22);
			break;
		case OP2_FBFCC: // branch on floating-point condition codes
			printf("fbfcc @ %x\n", PC);
			break;
#if SPARCV8
		case OP2_CBCCC: // branch on coprocessor condition codes, SPARCv8
			break;
#endif
		default:
			printf("unknown %08x @ %x\n", op, PC);
			break;
		}
		break;

	case OP_CALL: // call
	{
		UINT32 pc = PC;
		UINT32 callpc = PC + DISP30;
		PC = nPC;
		nPC = callpc;

		REG(15) = pc;
		break;
	}

	case OP_ALU:
		execute_group2(op);
		break;

	case OP_LDST:
		execute_group3(op);
		break;
	default:
		break;
	}
}


//-------------------------------------------------
//  dispatch_instruction - dispatch the previously
//  fetched instruction
//-------------------------------------------------

void mb86901_device::dispatch_instruction(UINT32 op)
{
	/* The SPARC Instruction Manual: Version 8, page 159, "Appendix C - ISP Descriptions - C.6. Instruction Dispatch" (SPARCv8.pdf, pg. 156)

	illegal_IU_instr :- (
	    if ( ( (op == 00) and (op2 == 000) ) { UNIMP instruction }
	       or
	       ( ((op=11) or (op=10)) and (op3=unassigned) )
	       then 1 else 0

	if (illegal_IU_instr = 1) then (
	    trap <- 1
	    illegal_instruction <- 1
	);
	if ((FPop1 or FPop2 or FBfcc) and ((EF = 0) or (bp_FPU_present = 0))) then (
	    trap <- 1;
	    fp_disabled <- 1
	);
	if (CPop1 or CPop2 or CBccc) and ((EC = 0) or (bp_CP_present = 0))) then (
	    trap <- 1;
	    cp_disabled <- 1
	);
	next;
	if (trap = 0) then (
	    { code for specific instruction, defined below }
	);
	*/
	bool illegal_IU_instr = (OP == 0 && OP2 == 0) || ((OP == 3 && !m_ldst_op3_assigned[OP3]) || (OP == 2 && !m_alu_op3_assigned[OP3]));

	if (illegal_IU_instr)
	{
		m_trap = 1;
		m_illegal_instruction = 1;

	}
	if (((OP == OP_ALU && (FPOP1 || FPOP2)) || (OP == OP_TYPE0 && OP2 == OP2_FBFCC)) && (!EF || !m_bp_fpu_present))
	{
		m_trap = 1;
		m_fp_disabled = 1;
	}
	if (((OP == OP_ALU && (CPOP1 || CPOP2)) || (OP == OP_TYPE0 && OP2 == OP2_CBCCC)) && (!EC || !m_bp_cp_present))
	{
		m_trap = 1;
		m_cp_disabled = 1;
	}

	if (!m_trap)
	{
		complete_instruction_execution(op);
	}
}


//-------------------------------------------------
//  complete_fp_execution - completes execution
//  of a floating-point operation
//-------------------------------------------------

void mb86901_device::complete_fp_execution(UINT32 /*op*/)
{
}


//-------------------------------------------------
//  execute_step - perform one step in execute
//  mode (versus error or reset modes)
//-------------------------------------------------

void mb86901_device::execute_step()
{
	/* The SPARC Instruction Manual: Version 8, page 156, "Appendix C - ISP Descriptions - C.5. Processor States and Instruction Dispatch" (SPARCv8.pdf, pg. 153)

	if (bp_reset_in = 1) then (
	    execute_mode <- 0;
	    reset_mode <- 1;
	    break { out of while (execute_mode = 1) loop }
	) else if ((ET = 1) and ((bp_IRL = 15) or (bp_IRL > PIL))) then (
	    trap <- 1;
	    interrupt_level <- bp_IRL
	);
	next;

	if (trap = 1) then execute_trap; { See Section C.8 }

	if (execute_mode = 1) then ( { execute_trap may have set execute_mode to 0 }

	    { the following code emulates the delayed nature of the write-state-register instructions.
	    PSR <- PSR'; PSR' <- PSR''; PSR'' <- PSR'''; PSR''' <- PSR'''';
	    ASR <- ASR'; ASR' <- ASR''; ASR'' <- ASR'''; ASR''' <- ASR'''';
	    TBR <- TBR'; TBR' <- TBR''; TBR'' <- TBR'''; TBR''' <- TBR'''';
	    WIM <- WIM'; WIM' <- WIM''; WIM'' <- WIM'''; WIM''' <- WIM'''';
	      Y <-   Y';   Y' <-   Y'';   Y'' <-   Y''';   Y''' <-   Y'''';
	    next;

	    addr_space := (if (S = 0) then 8 else 9);
	    (instruction, MAE) <- memory_read(addr_space, PC);
	    next;

	    if ( (MAE = 1) and (annul = 0) ) then (
	        trap <- 1;
	        instruction_access_exception <- 1
	    ) else (
	        if (annul = 0) then (
	            dispatch_instruction ; { See Section C.6 }
	            next;
	            if (FPop1 or FPop2) then (
	                complete_fp_execution { See Section C.7 }
	            )
	            next;
	            if ( (trap = 0) and
	                  not (CALL or RETT or JMPL or Bicc or FBfcc or CBccc or Ticc) ) then (
	                PC <- nPC;
	                nPC <- nPC + 4
	            )
	        ) else { annul != 0 } (
	            annul <- 0;
	            PC <- nPC;
	            nPC <- nPC + 4
	        )
	    )
	)
	*/
	if (m_bp_reset_in)
	{
		m_execute_mode = 0;
		m_reset_mode = 1;
		return;
	}
	else if (ET && (m_bp_irl == 15 || m_bp_irl > PIL))
	{
		m_trap = 1;
		m_interrupt_level = m_bp_irl;
	}

	if (m_trap) execute_trap();

	if (m_execute_mode)
	{
		// write-state-register delay not yet implemented

		UINT32 addr_space = (IS_USER ? 8 : 9);
		UINT32 op = read_sized_word(addr_space, PC, 4);

#if 0
		if (PC == 0xffef0000)
		{
			UINT32 opcode = read_sized_word(11, REG(5), 2);
			if (!(REG(5) & 2))
			{
				opcode >>= 16;
			}
			UINT32 l1 = opcode << 2;
			l1 += REG(2);
			UINT32 handler_offset = read_sized_word(11, l1, 2);
			if (!(l1 & 2))
			{
				handler_offset >>= 16;
			}
			handler_offset <<= 2;
			handler_offset += REG(2);
			if (handler_offset == 0xffe87964)
			{
				printf("Opcode at %08x: %04x, handler is at %08x // call %08x\n", REG(5), opcode, handler_offset, l1 + 2);
			}
			else if (handler_offset == 0xffe8799c)
			{
				UINT32 address = REG(5) + 2;
				UINT32 half = read_sized_word(11, address, 2);
				if (!(address & 2)) half >>= 16;

				printf("Opcode at %08x: %04x, handler is at %08x // push_data current result (%08x) + load address %08x\n", REG(5), opcode, handler_offset, REG(4), REG(3) + half);
			}
			else if (handler_offset == 0xffe879e4)
			{
				UINT32 address = l1 + 2;
				UINT32 half0 = read_sized_word(11, address, 2);
				if (address & 2) half0 <<= 16;

				address = l1 + 4;
				UINT32 half1 = read_sized_word(11, address, 2);
				if (!(address & 2)) half1 >>= 16;

				UINT32 value = half0 | half1;

				printf("Opcode at %08x: %04x, handler is at %08x // push_data current result (%08x) + load immediate word from handler table (%08x)\n", REG(5), opcode, handler_offset, REG(4), value);
			}
			else if (handler_offset == 0xffe879c4)
			{
				UINT32 address = l1 + 2;
				UINT32 l0 = read_sized_word(11, address, 2);
				if (!(address & 2)) l0 >>= 16;

				address = REG(3) + l0;
				UINT32 l1_2 = read_sized_word(11, address, 2);
				if (!(address & 2)) l1_2 >>= 16;

				address = REG(2) + (l1_2 << 2);
				UINT32 l0_2 = read_sized_word(11, address, 2);
				if (!(address & 2)) l0_2 >>= 16;

				UINT32 dest = REG(2) + (l0_2 << 2);

				printf("Opcode at %08x: %04x, handler is at %08x // SPARC branch to %08x, calcs: g2(%08x) + halfword[g2(%04x) + (halfword[g3(%08x) + halfword[entry_point(%04x) + 2](%04x)](%04x) << 2)](%08x)\n", REG(5), opcode, handler_offset, dest, REG(2), REG(2), REG(3), l1, l0, l1_2, l0_2);
				printf("                                                 // Target func: %08x\n", l0_2 << 2);
				switch (l0_2 << 2)
				{
					case 0x10: // call
						printf("                                                 // call %08x\n", (REG(2) + (l1_2 << 2)) + 2);
						break;
					default:
						printf("                                                 // unknown handler address: %08x\n", REG(2) + (l0_2 << 2));
						break;
				}
			}
			else if (handler_offset == 0xffe8c838)
			{
				UINT32 address = l1 + 2;
				UINT32 half0 = read_sized_word(11, address, 2);
				if (address & 2) half0 <<= 16;

				address = l1 + 4;
				UINT32 half1 = read_sized_word(11, address, 2);
				if (!(address & 2)) half1 >>= 16;

				UINT32 value = half0 | half1;

				printf("Opcode at %08x: %04x, handler is at %08x // add 32-bit word (%08x) from handler table to result (%08x + %08x = %08x)\n", REG(5), opcode, handler_offset, value, REG(4), value, REG(4) + value);
			}
			else if (opcode == 0x003f || opcode == 0x0066 || opcode == 0x0099 || opcode == 0x0121 || opcode == 0x0136 || opcode == 0x014f || opcode == 0x0155 || opcode == 0x01c7 || opcode == 0x01cd ||
						opcode == 0x0217 || opcode == 0x0289 || opcode == 0x0296 || opcode == 0x029d || opcode == 0x02f2 || opcode == 0x0334 || opcode == 0x0381 || opcode == 0x3d38)
			{
				switch(opcode)
				{
					case 0x003f:
					{
						UINT32 address = REG(5) + 2;
						UINT32 half0 = read_sized_word(11, address, 2);
						if (address & 2) half0 <<= 16;

						address = REG(5) + 4;
						UINT32 half1 = read_sized_word(11, address, 2);
						if (!(address & 2)) half1 >>= 16;

						UINT32 value = half0 | half1;

						printf("Opcode at %08x: %04x, handler is at %08x // push_data current result (%08x) + load immediate word from instructions (%08x)\n", REG(5), opcode, handler_offset, REG(4), value);
						break;
					}

					case 0x0066:
					{
						UINT32 address = REG(5) + 2;
						UINT32 offset = read_sized_word(11, address, 2);
						if (!(address & 2)) offset >>= 16;

						UINT32 target = REG(5) + 2 + offset;

						printf("Opcode at %08x: %04x, handler is at %08x // if result (%08x) is zero, jump to %08x\n", REG(5), opcode, handler_offset, REG(4), target);
						break;
					}

					case 0x0099:
					{
						UINT32 l1_2 = REG(4);

						UINT32 address = REG(7);
						UINT32 l0_2 = read_sized_word(11, address, 4);

						address = REG(7) + 4;
						UINT32 popped_g4 = read_sized_word(11, address, 4);

						address = REG(5) + 2;
						UINT32 offset = read_sized_word(11, address, 2);
						if (!(address & 2)) offset >>= 16;

						UINT32 target = REG(5) + 2 + offset;

						printf("Opcode at %08x: %04x, handler is at %08x // branch relative to %08x if data stack pop/top (%08x) == result (%08x), pop_data result (%08x)\n", REG(5), opcode, handler_offset, target, l0_2, l1_2, popped_g4);
						if (l1_2 == l0_2)
						{
							printf("                                                 // branch will be taken\n");
						}
						else
						{
							printf("                                                 // branch will not be taken\n");
							printf("                                                 // push pc (%08x) onto program stack\n", REG(5) + 2);
							printf("                                                 // push previous data stack top + 0x80000000 (%08x) onto program stack\n", l0_2 + 0x8000000);
							printf("                                                 // push diff of (result - stack top) (%08x - %08x = %08x)\n", popped_g4, l0_2 + 0x8000000, popped_g4 - (l0_2 + 0x80000000));
						}

						break;
					}

					case 0x0121:
					{
						UINT32 address = REG(7);
						UINT32 word = read_sized_word(11, address, 4);
						printf("Opcode at %08x: %04x, handler is at %08x // logical-AND result with data stack pop, store in result: %08x = %08x & %08x\n", REG(5), opcode, handler_offset, word & REG(4), word, REG(4));
						break;
					}

					case 0x0136:
						printf("Opcode at %08x: %04x, handler is at %08x // invert result (%08x -> %08x)\n", REG(5), opcode, handler_offset, REG(4), REG(4) ^ 0xffffffff);
						break;

					case 0x014f:
					{
						UINT32 address = REG(7);
						UINT32 word = read_sized_word(11, address, 4);
						printf("Opcode at %08x: %04x, handler is at %08x // add result to data stack pop, store in result: %08x = %08x + %08x\n", REG(5), opcode, handler_offset, word + REG(4), word, REG(4));
						break;
					}

					case 0x0155:
					{
						UINT32 address = REG(7);
						UINT32 word = read_sized_word(11, address, 4);
						printf("Opcode at %08x: %04x, handler is at %08x // subtract result from data stack pop, store in result: %08x = %08x - %08x\n", REG(5), opcode, handler_offset, word - REG(4), word, REG(4));
						break;
					}

					case 0x01c7:
					{
						UINT32 address = REG(6);
						UINT32 half0 = read_sized_word(11, address, 2);
						if (address & 2) half0 <<= 16;

						address = REG(6) + 2;
						UINT32 half1 = read_sized_word(11, address, 2);
						if (!(address & 2)) half1 >>= 16;

						UINT32 value = half0 | half1;

						printf("Opcode at %08x: %04x, handler is at %08x // return (%08x) (pops off program stack)\n", REG(5), opcode, handler_offset, value);
						break;
					}

					case 0x01cd:
						printf("Opcode at %08x: %04x, handler is at %08x // insert result (%08x) between data stack top (%08x) and next data stack entry\n", REG(5), opcode, handler_offset, REG(4), read_sized_word(11, REG(7), 4));
						break;

					case 0x0217:
					{
						UINT32 value = read_sized_word(11, REG(7), 4);
						printf("Opcode at %08x: %04x, handler is at %08x // if pop_data (%08x) >= result (%08x), set result to 0, otherwise -1 (%08x)\n", REG(5), opcode, handler_offset, value, REG(4), (value >= REG(4)) ? 0 : ~0);
						break;
					}

					case 0x0289:
						printf("Opcode at %08x: %04x, handler is at %08x // push_data(g4 (%08x))\n", REG(5), opcode, handler_offset, REG(4));
						break;

					case 0x0296:
						printf("Opcode at %08x: %04x, handler is at %08x // swap result (%08x) with top of data stack (%08x)\n", REG(5), opcode, handler_offset, REG(4), read_sized_word(11, REG(7), 4));
						break;

					case 0x029d:
					{
						UINT32 top = read_sized_word(11, REG(7), 4);
						UINT32 next = read_sized_word(11, REG(7) + 4, 4);
						printf("Opcode at %08x: %04x, handler is at %08x // swap the top two values of the data stack (%08x <-> %08x), exchange second value with result (%08x <-> %08x)\n", REG(5), opcode, handler_offset, top, next, REG(4), next);
						break;
					}

					case 0x02f2:
						printf("Opcode at %08x: %04x, handler is at %08x // decrement g4 (%08x -> %08x)\n", REG(5), opcode, handler_offset, REG(4), REG(4) - 1);
						break;

					case 0x0334:
					{
						UINT32 address = REG(4);
						UINT32 half0 = read_sized_word(11, address, 2);
						if (address & 2) half0 <<= 16;

						address = REG(4) + 2;
						UINT32 half1 = read_sized_word(11, address, 2);
						if (!(address & 2)) half1 >>= 16;

						UINT32 value = half0 | half1;

						printf("Opcode at %08x: %04x, handler is at %08x // load result with 32-bit word at result address (g4 = [%08x] (%08x)\n", REG(5), opcode, handler_offset, REG(4), value);
						break;
					}

					case 0x0381:
						printf("Opcode at %08x: %04x, handler is at %08x // word[g4 (%08x)] = pop_data(g7), result = pop_data(g7), g7 = %08x\n", REG(5), opcode, handler_offset, REG(4), REG(7));
						break;

					case 0x3d38:
						printf("Opcode at %08x: %04x, handler is at %08x // call ffe8fe90\n", REG(5), opcode, handler_offset);
						break;
				}
			}
			else
			{
				printf("Opcode at %08x: %04x, handler is at %08x\n", REG(5), opcode, handler_offset);
			}
		}
#endif

		if (MAE && !m_annul)
		{
			m_trap = 1;
			m_instruction_access_exception = 1;
		}
		else
		{
			if (!m_annul)
			{
				dispatch_instruction(op);

				if (FPOP1 || FPOP2)
				{
					complete_fp_execution(op);
				}

				if (m_trap == 0 && !(OP == OP_CALL || (OP == OP_TYPE0 && (OP2 == OP2_BICC || OP2 == OP2_FBFCC || OP2 == OP2_CBCCC)) || (OP == OP_ALU && (JMPL || TICC || RETT))))
				{
					PC = nPC;
					nPC = nPC + 4;
				}
			}
			else
			{
				m_annul = 0;
				PC = nPC;
				nPC = nPC + 4;
			}
		}
	}
}


//-------------------------------------------------
//  reset_step - step one cycle in reset mode
//-------------------------------------------------

void mb86901_device::reset_step()
{
	/* The SPARC Instruction Manual: Version 8, page 156, "Appendix C - ISP Descriptions - C.5. Processor States and Instruction Dispatch" (SPARCv8.pdf, pg. 153)

	while (reset_mode = 1) (
	    if (bp_reset_in = 0) then (
	        reset_mode <- 0;
	        execute_mode <- 1;
	        trap <- 1;
	        reset_trap <- 1;
	    )
	);
	*/

	if (!m_bp_reset_in)
	{
		m_reset_mode = 0;
		m_execute_mode = 1;
		m_trap = 1;
		m_reset_trap = 1;
	}
}


//-------------------------------------------------
//  error_step - step one cycle in error mode
//-------------------------------------------------

void mb86901_device::error_step()
{
	/* The SPARC Instruction Manual: Version 8, page 157, "Appendix C - ISP Descriptions - C.5. Processor States and Instruction Dispatch" (SPARCv8.pdf, pg. 154)

	while (error_mode = 1) (
	    if (bp_reset_in = 1) then (
	        error_mode <- 0;
	        reset_mode <- 1;
	        pb_error <- 0
	    )
	);
	*/

	if (m_bp_reset_in)
	{
		m_error_mode = 0;
		m_reset_mode = 1;
		m_pb_error = 0;
	}
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
		if (HOLD_BUS)
		{
			m_icount--;
			continue;
		}

		debugger_instruction_hook(this, m_pc);

		if (m_reset_mode)
		{
			reset_step();
		}
		else if (m_error_mode)
		{
			error_step();
		}
		else if (m_execute_mode)
		{
			execute_step();
		}

		if (debug)
		{
			for (int i = 0; i < 8; i++)
			{
				m_dbgregs[i]        = *m_regs[8 + i];
				m_dbgregs[8 + i]    = *m_regs[16 + i];
				m_dbgregs[16 + i]   = *m_regs[24 + i];
			}
		}
		--m_icount;
	}
}


//-------------------------------------------------
//  get_reg_r - get integer register value for
//  disassembler
//-------------------------------------------------

UINT64 mb86901_device::get_reg_r(unsigned index) const
{
	return REG(index & 31);
}


//-------------------------------------------------
//  get_reg_pc - get program counter value for
//  disassembler
//-------------------------------------------------

UINT64 mb86901_device::get_translated_pc() const
{
	// FIXME: how do we apply translation to the address so it's in the same space the disassembler sees?
	return m_pc;
}


//-------------------------------------------------
//  get_icc - get integer condition codes for
//  disassembler
//-------------------------------------------------

UINT8 mb86901_device::get_icc() const
{
	return (m_psr & PSR_ICC_MASK) >> PSR_ICC_SHIFT;
}


//-------------------------------------------------
//  get_icc - get extended integer condition codes
//  for disassembler
//-------------------------------------------------

UINT8 mb86901_device::get_xcc() const
{
	// not present before SPARCv9
	return 0;
}


//-------------------------------------------------
//  get_icc - get extended integer condition codes
//  for disassembler
//-------------------------------------------------

UINT8 mb86901_device::get_fcc(unsigned index) const
{
	// only one fcc instance before SPARCv9
	return (m_fsr >> 10) & 3;
}
