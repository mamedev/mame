// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
   While advertised as a type of 80c51 (and even referred to as such by the test mode in some IGS titles)
   this is a distinct architecture.  The opcode set does extend on an 80c51, and tools were provided to
   help convert 80c51 sources to run on the XA architecture, but the encoding is entirely different and
   there is no binary compatibility.

   https://www.ceibo.com/eng/datasheets/Philips-XA-User-Guide.pdf
*/

#include "emu.h"
#include "xa.h"
#include "xadasm.h"

DEFINE_DEVICE_TYPE(XA, xa_cpu_device, "xa", "Philips 80c51 XA")

xa_cpu_device::xa_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, XA, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 24, 0)
	, m_pc(0)
	, m_program(nullptr)
	, m_icount(0)
{
}

std::unique_ptr<util::disasm_interface> xa_cpu_device::create_disassembler()
{
	return std::make_unique<xa_disassembler>();
}

/*****************************************************************************/

device_memory_interface::space_config_vector xa_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


/*****************************************************************************/

void xa_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	state_add(STATE_GENPC,      "GENPC",     m_pc).formatstr("%08X");
	state_add(STATE_GENPCBASE,  "CURPC",     m_pc).callexport().noshow();

	set_icountptr(m_icount);
}

void xa_cpu_device::device_reset()
{
}

/*****************************************************************************/

void xa_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
		m_pc++;
		m_icount--;
	}
}
