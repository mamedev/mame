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
DEFINE_DEVICE_TYPE(MX10EXA, mx10exa_cpu_device, "mx10exa", "Philips MX10EXA")




xa_cpu_device::xa_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor prg_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("data", ENDIANNESS_LITTLE, 16, 24, 0, prg_map)
	, m_pc(0)
	, m_program(nullptr)
	, m_icount(0)
{
}

xa_cpu_device::xa_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: xa_cpu_device(mconfig, XA, tag, owner, clock, address_map_constructor(FUNC(xa_cpu_device::internal_map), this))
{
}


mx10exa_cpu_device::mx10exa_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: xa_cpu_device(mconfig, MX10EXA, tag, owner, clock, address_map_constructor(FUNC(mx10exa_cpu_device::mx10exa_internal_map), this))
{
}

std::unique_ptr<util::disasm_interface> xa_cpu_device::create_disassembler()
{
	return std::make_unique<xa_dasm>();
}

/*****************************************************************************/

void xa_cpu_device::internal_map(address_map &map)
{
}

void mx10exa_cpu_device::mx10exa_internal_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
}


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
	// temp, as there is code here on superkds
	m_pc = m_program->read_word(2);
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
