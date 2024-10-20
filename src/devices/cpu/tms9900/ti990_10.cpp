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
    external memory mapper.  Additionally,  it can use a Macrostore ROM to
    emulate additional instructions.

    **** This is WORK IN PROGRESS ****
*/

#include "emu.h"
#include "ti990_10.h"
#include "9900dasm.h"

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

ti990_10_device::ti990_10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, TI990_10, tag, owner, clock),
		m_program_config("program", ENDIANNESS_BIG, 16, 21),
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
	set_icountptr(m_icount);

	state_add(STATE_GENPC, "GENPC", PC).formatstr("%4s").noshow();
	state_add(STATE_GENPCBASE, "CURPC", PC).formatstr("%4s").noshow();
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

device_memory_interface::space_config_vector ti990_10_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
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

uint32_t ti990_10_device::execute_min_cycles() const noexcept
{
	return 2;
}

// TODO: Compute this value, just a wild guess for the average
uint32_t ti990_10_device::execute_max_cycles() const noexcept
{
	return 10;
}

// TODO: check 9900dasm
std::unique_ptr<util::disasm_interface> ti990_10_device::create_disassembler()
{
	return std::make_unique<tms9900_disassembler>(TMS9900_ID);
}

DEFINE_DEVICE_TYPE(TI990_10, ti990_10_device, "ti990_10_cpu", "Texas Instruments TI990/10 CPU")
