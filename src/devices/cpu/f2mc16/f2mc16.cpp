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
#define read_32(addr)           m_program->read_dword(addr)
#define write_8(addr, data)     m_program->write_byte(addr, data)
#define write_16(addr, data)    m_program->write_word(addr, data)
#define write_32(addr, data)    m_program->write_dword(addr, data)

#define read_8_vector(addr)     m_program->read_byte(addr)
#define read_16_vector(addr)    m_program->read_word(addr)
#define read_32_vector(addr)    m_program->read_dword(addr)

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

	state_add(F2MC16_PCB, "PCB", m_pcb);
	state_add(F2MC16_PC, "PC", m_pc).formatstr("%04X");
	state_add(STATE_GENPC, "GENPC", m_temp).callimport().callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_temp).callimport().callexport().noshow();
	state_add(F2MC16_PS, "PS", m_ps).formatstr("%04X");
	state_add(F2MC16_PCB, "DTB", m_dtb).formatstr("%02X");
	state_add(F2MC16_PCB, "ADB", m_adb).formatstr("%02X");
	state_add(F2MC16_ACC, "AL", m_acc).formatstr("%08X");
	state_add(F2MC16_USB, "USB", m_usb).formatstr("%02X");
	state_add(F2MC16_USP, "USP", m_usp).formatstr("%04X");
	state_add(F2MC16_SSB, "SSB", m_ssb).formatstr("%02X");
	state_add(F2MC16_SSP, "SSP", m_ssp).formatstr("%04X");
	state_add(F2MC16_DPR, "DPR", m_dpr).formatstr("%02X");

	set_icountptr(m_icount);
}

void f2mc16_device::device_reset()
{
	m_usb = m_ssb = 0;
	m_usp = m_ssp = 0;
	m_ps = 0;
	m_acc = 0;
	m_dpr = m_dtb = 0;

	m_pc = read_16_vector(0xffffdc);
	m_pcb = read_8_vector(0xffffde);
}

void f2mc16_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_pc = (m_temp & 0xffff);
			m_pcb = (m_temp >> 16) & 0xff;
			break;
	}
}

void f2mc16_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_temp = m_pc;
			m_temp |= (m_pcb << 16);
			break;
	}
}

void f2mc16_device::execute_run()
{
	debugger_instruction_hook((m_pcb<<16) | m_pc);
	printf("Debug hook: %06x\n", (m_pcb<<16) | m_pc);

	m_icount = 0;
}

void f2mc16_device::execute_set_input(int inputnum, int state)
{
}
