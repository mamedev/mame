// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    First-gen DEC PDP-8 emulator skeleton

    Written by Ryan Holtz
*/

#include "emu.h"
#include "pdp8.h"
#include "pdp8dasm.h"

#define OP          ((op >> 011) & 07)

#define MR_IND      ((op >> 010) & 01)
#define MR_PAGE     ((op >> 07) & 01)
#define MR_ADDR     (op & 0177)

#define IOT_DEVICE  ((op >> 03) & 077)
#define IOT_IOP1    (op & 01)
#define IOT_IOP2    ((op >> 01) & 01)
#define IOT_IOP4    ((op >> 02) & 01)

#define OPR_GROUP   ((op >> 010) & 01)
#define OPR_CLA     ((op >> 07) & 01)
#define OPR_CLL     ((op >> 06) & 01)
#define OPR_CMA     ((op >> 05) & 01)
#define OPR_CML     ((op >> 04) & 01)
#define OPR_ROR     ((op >> 03) & 01)
#define OPR_ROL     ((op >> 02) & 01)
#define OPR_ROT2    ((op >> 01) & 01)
#define OPR_IAC     (op & 01)

#define OPR_SMA     OPR_CLL
#define OPR_SZA     OPR_CMA
#define OPR_SNL     OPR_CML
#define OPR_REVSKIP OPR_ROR
#define OPR_OSR     OPR_ROL
#define OPR_HLT     OPR_ROT2

#define OPR_GROUP_MASK  0401
#define OPR_GROUP1_VAL  0000
#define OPR_GROUP2_VAL  0400

DEFINE_DEVICE_TYPE(PDP8, pdp8_device, "pdp8_cpu", "DEC PDP8")

//-------------------------------------------------
//  pdp8_device - constructor
//-------------------------------------------------

pdp8_device::pdp8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, PDP8, tag, owner, clock),
		m_program_config("program", ENDIANNESS_BIG, 16, 12, -1),
		m_pc(0),
		m_ac(0),
		m_mb(0),
		m_ma(0),
		m_sr(0),
		m_l(0),
		m_ir(0),
		m_halt(true),
		m_icount(0)
{
	// Allocate & setup
	m_program_config.m_is_octal = true;
}


void pdp8_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_l).callimport().callexport().formatstr("%1s").noshow();
	state_add(PDP8_PC,         "PC",        m_pc).mask(0xfff);
	state_add(PDP8_AC,         "AC",        m_ac).mask(0xfff);
	state_add(PDP8_MB,         "MB",        m_mb).mask(0xfff);
	state_add(PDP8_MA,         "MA",        m_ma).mask(0xfff);
	state_add(PDP8_SR,         "SR",        m_sr).mask(0xfff);
	state_add(PDP8_L,          "L",         m_l).mask(0xf);
	state_add(PDP8_IR,         "IR",        m_ir).mask(0xff);
	state_add(PDP8_HALT,       "HLT",       m_halt).mask(0xf);

	// setup regtable
	save_item(NAME(m_pc));
	save_item(NAME(m_ac));
	save_item(NAME(m_mb));
	save_item(NAME(m_ma));
	save_item(NAME(m_sr));
	save_item(NAME(m_l));
	save_item(NAME(m_ir));
	save_item(NAME(m_halt));

	// set our instruction counter
	set_icountptr(m_icount);
}

void pdp8_device::device_stop()
{
}

void pdp8_device::device_reset()
{
	m_pc = 0;
	m_ac = 0;
	m_mb = 0;
	m_ma = 0;
	m_sr = 0;
	m_l = 0;
	m_ir = 0;
	m_halt = true;
}


//-------------------------------------------------
//  memory_space_config - return a vector of
//  address space configurations for this device
//-------------------------------------------------

device_memory_interface::space_config_vector pdp8_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
	};
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void pdp8_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = util::string_format("%c", m_halt ? 'H' : '.');
			break;
	}
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> pdp8_device::create_disassembler()
{
	return std::make_unique<pdp8_disassembler>();
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t pdp8_device::execute_min_cycles() const noexcept
{
	return 1; // TODO
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t pdp8_device::execute_max_cycles() const noexcept
{
	return 3; // TODO
}


//-------------------------------------------------
//  execute_set_input - set the state of an input
//  line during execution
//-------------------------------------------------

void pdp8_device::execute_set_input(int inputnum, int state)
{
	// TODO
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void pdp8_device::execute_run()
{
	while (m_icount > 0)
	{
		m_pc &= 07777;

		debugger_instruction_hook(m_pc);

		uint16_t op [[maybe_unused]] = m_program->read_word(m_pc);

		--m_icount;
	}
}
