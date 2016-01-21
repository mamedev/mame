// license:BSD-3-Clause
// copyright-holders:Michael Zapf

/*
    Texas Instruments TI990/10 CPU board

    The first member of the family was actually the ti990/10 minicomputer,
    released in 1975.  tms9900 was released in 1976, and has the same
    instruction set as ti990/10: however, tms9900 is slower, it does not
    support privileges and memory mapping, and illegal instructions do not
    cause an error interrupt.

    The ti990 family later evoluted into the huge ti990/12 system, with support
    for 144 different instructions, and microcode programming in case some user
    found it was not enough.  ti990/10 was eventually replaced by a cheaper
    ti990/10a board, built around a tms99000 microprocessor.

    tms99000 is the successor to both ti9900 and ti990/10.  It supports
    privileges, and has a coprocessor interface which enables the use of an
    external memory mapper.  Additionnally,  it can use a Macrostore ROM to
    emulate additional instructions.

    **** This is WORK IN PROGRESS ****
*/

#include "ti990_10.h"

/*
    The following defines can be set to 0 or 1 to disable or enable certain
    output in the log.
*/
// Emulation setup
#define TRACE_SETUP 0

// Emulation details
#define TRACE_EMU 0

/****************************************************************************
    Constructor for TI 990/10
    The CRU mask is related to the bits, not to their addresses which are
    twice their number. Accordingly, the TMS9900 has a CRU bitmask 0x0fff.
****************************************************************************/

ti990_10_device::ti990_10_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TI990_10, "TI990/10 CPU", tag, owner, clock, "ti990_10_cpu",  __FILE__),
		m_program_config("program", ENDIANNESS_BIG, 16, 16),
		m_io_config("cru", ENDIANNESS_BIG, 8, 12),
		m_prgspace(nullptr),
		m_cru(nullptr)
{
}

ti990_10_device::~ti990_10_device()
{
}

void ti990_10_device::device_start()
{
	m_prgspace = &space(AS_PROGRAM);
	m_cru = &space(AS_IO);

	// set our instruction counter
	m_icountptr = &m_icount;

	state_add(STATE_GENPC, "curpc", PC).formatstr("%4s").noshow();
	state_add(STATE_GENFLAGS, "status", m_state_any).callimport().callexport().formatstr("%16s").noshow();
}

void ti990_10_device::device_stop()
{
	if (TRACE_SETUP) logerror("ti990_10: Deleting lookup tables\n");
}

/*
    TI990_10 hard reset
    The device reset is just the emulator's trigger for the reset procedure
    which is invoked via the main loop.
*/
void ti990_10_device::device_reset()
{
	if (TRACE_EMU) logerror("ti990_10: Device reset by emulator\n");
}

const address_space_config *ti990_10_device::memory_space_config(address_spacenum spacenum) const
{
	switch (spacenum)
	{
	case AS_PROGRAM:
		return &m_program_config;

	case AS_IO:
		return &m_io_config;

	default:
		return nullptr;
	}
}

void ti990_10_device::execute_run()
{
	do
	{
		// TODO: Complete the implementation
		m_icount--;
	} while (m_icount>0);
}

void ti990_10_device::execute_set_input(int irqline, int state)
{
}

// ==========================================================================

UINT32 ti990_10_device::execute_min_cycles() const
{
	return 2;
}

// TODO: Compute this value, just a wild guess for the average
UINT32 ti990_10_device::execute_max_cycles() const
{
	return 10;
}

UINT32 ti990_10_device::execute_input_lines() const
{
	return 2;
}

// device_disasm_interface overrides
UINT32 ti990_10_device::disasm_min_opcode_bytes() const
{
	return 2;
}

UINT32 ti990_10_device::disasm_max_opcode_bytes() const
{
	return 6;
}

// TODO: check 9900dasm
offs_t ti990_10_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms9900 );
	return CPU_DISASSEMBLE_NAME(tms9900)(this, buffer, pc, oprom, opram, options);
}

const device_type TI990_10 = &device_creator<ti990_10_device>;
