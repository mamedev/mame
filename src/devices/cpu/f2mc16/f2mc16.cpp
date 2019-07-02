// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Fujitsu Micro F2MC-16 series

    From 50,000 feet these chips look a lot like a 65C816 with no index
    registers.  As you get closer, you can see the banking includes some
    concepts from 8086 segmentation, and the interrupt handling is 68000-like.

***************************************************************************/

#include "emu.h"
#include "f2mc16.h"
#include "f2mc16dasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(F2MC16, f2mc16_device, "f2mc16", "Fujitsu Micro F2MC-16")

// memory accessors
#define read_8(addr)            m_program->read_byte(addr)
#define read_16(addr)           m_program->read_word(addr)
#define write_8(addr, data)     m_program->write_byte(addr, data)
#define write_16(addr, data)    m_program->write_word(addr, data)

#define read_8_vector(addr)     m_program->read_byte(addr)

std::unique_ptr<util::disasm_interface> f2mc16_device::create_disassembler()
{
	return std::make_unique<f2mc16_disassembler>();
}

f2mc16_device::f2mc16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_program(nullptr)
{
}

f2mc16_device::f2mc16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: f2mc16_device(mconfig, F2MC16, tag, owner, clock)
{
}

device_memory_interface::space_config_vector f2mc16_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_program_config) };
}

void f2mc16_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	set_icountptr(m_icount);

	state_add(F2MC16_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).callimport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).callimport().noshow();
	state_add(F2MC16_ACC, "AL", m_acc);
}

void f2mc16_device::device_reset()
{
	m_pc = (read_8_vector(0xffffde) << 16) | (read_8_vector(0xffffdd) << 8) | read_8_vector(0xffffdc);
	printf("RESET: PC=%x\n", m_pc);
}

void f2mc16_device::execute_run()
{
	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void f2mc16_device::execute_set_input(int inputnum, int state)
{
	// TODO
}
