// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Fujitsu Micro F2MC-16 series
    Emulation by R. Belmont

    From 50,000 feet these chips look a lot like a 65C816 with no index
    registers.  As you get closer, you can see the banking includes some
    concepts from 8086 segmentation, and the interrupt handling is 68000-like.

    There are two main branches: F and L.  They appear to be compatible with
    each other as far as their extentions to the base ISA not conflicting.

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
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_ps).callimport().formatstr("%7s").noshow();
	state_add(F2MC16_DTB, "DTB", m_dtb).formatstr("%02X");
	state_add(F2MC16_ADB, "ADB", m_adb).formatstr("%02X");
	state_add(F2MC16_ACC, "AL", m_acc).formatstr("%08X");
	state_add(F2MC16_USB, "USB", m_usb).formatstr("%02X");
	state_add(F2MC16_USP, "USP", m_usp).formatstr("%04X");
	state_add(F2MC16_SSB, "SSB", m_ssb).formatstr("%02X");
	state_add(F2MC16_SSP, "SSP", m_ssp).formatstr("%04X");
	state_add(F2MC16_DPR, "DPR", m_dpr).formatstr("%02X");
	state_add(F2MC16_RW0, "RW0", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW1, "RW1", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW2, "RW2", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW3, "RW3", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW4, "RW4", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW5, "RW5", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW6, "RW6", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW7, "RW7", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RL0, "RL0", m_temp).callimport().callexport().formatstr("%08X");
	state_add(F2MC16_RL1, "RL1", m_temp).callimport().callexport().formatstr("%08X");
	state_add(F2MC16_RL2, "RL2", m_temp).callimport().callexport().formatstr("%08X");
	state_add(F2MC16_RL3, "RL3", m_temp).callimport().callexport().formatstr("%08X");
	state_add(F2MC16_R0, "R0", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R1, "R1", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R2, "R2", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R3, "R3", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R4, "R4", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R5, "R5", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R6, "R6", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R7, "R7", m_temp).callimport().callexport().formatstr("%02X");

	set_icountptr(m_icount);
}

void f2mc16_device::device_reset()
{
	m_usb = m_ssb = 0;
	m_usp = m_ssp = 0;
	m_ps &= 0x009f; // clear I and S flags
	m_ps |= F_S;    // set system stack, interrupts disabled, registers at 0x180
	m_acc = 0;
	m_dpr = 0x01;
	m_dtb = 0;

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

		case F2MC16_RW0:  write_16(m_temp, 0x180 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW1:  write_16(m_temp, 0x182 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW2:  write_16(m_temp, 0x184 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW3:  write_16(m_temp, 0x186 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW4:  write_16(m_temp, 0x188 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW5:  write_16(m_temp, 0x18a + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW6:  write_16(m_temp, 0x18c + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW7:  write_16(m_temp, 0x18e + (((m_ps>>8)&0x1f)*0x10)); break;

		case F2MC16_RL0:  write_32(m_temp, 0x180 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RL1:  write_32(m_temp, 0x184 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RL2:  write_32(m_temp, 0x188 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RL3:  write_32(m_temp, 0x18c + (((m_ps>>8)&0x1f)*0x10)); break;

		case F2MC16_R0:  write_8(m_temp, 0x188 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R1:  write_8(m_temp, 0x189 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R2:  write_8(m_temp, 0x18a + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R3:  write_8(m_temp, 0x18b + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R4:  write_8(m_temp, 0x18c + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R5:  write_8(m_temp, 0x18d + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R6:  write_8(m_temp, 0x18e + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R7:  write_8(m_temp, 0x18f + (((m_ps>>8)&0x1f)*0x10)); break;

		case STATE_GENFLAGS:
			break;
	}
}

void f2mc16_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case F2MC16_RW0:  m_temp = read_16(0x180 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW1:  m_temp = read_16(0x182 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW2:  m_temp = read_16(0x184 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW3:  m_temp = read_16(0x186 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW4:  m_temp = read_16(0x188 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW5:  m_temp = read_16(0x18a + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW6:  m_temp = read_16(0x18c + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RW7:  m_temp = read_16(0x18e + (((m_ps>>8)&0x1f)*0x10)); break;

		case F2MC16_RL0:  m_temp = read_32(0x180 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RL1:  m_temp = read_32(0x184 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RL2:  m_temp = read_32(0x188 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_RL3:  m_temp = read_32(0x18c + (((m_ps>>8)&0x1f)*0x10)); break;

		case F2MC16_R0:  m_temp = read_8(0x188 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R1:  m_temp = read_8(0x189 + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R2:  m_temp = read_8(0x18a + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R3:  m_temp = read_8(0x18b + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R4:  m_temp = read_8(0x18c + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R5:  m_temp = read_8(0x18d + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R6:  m_temp = read_8(0x18e + (((m_ps>>8)&0x1f)*0x10)); break;
		case F2MC16_R7:  m_temp = read_8(0x18f + (((m_ps>>8)&0x1f)*0x10)); break;

		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_temp = m_pc;
			m_temp |= (m_pcb << 16);
			break;
	}
}

void f2mc16_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
		switch(entry.index()) {
		case STATE_GENFLAGS:
		case F2MC16_PS:
				str = string_format("%c%c%c%c%c%c%c",
												m_ps & F_I ? 'I' : '.',
												m_ps & F_S ? 'S' : '.',
												m_ps & F_T ? 'T' : '.',
												m_ps & F_N ? 'N' : '.',
												m_ps & F_Z ? 'Z' : '.',
												m_ps & F_V ? 'V' : '.',
												m_ps & F_C ? 'C' : '.');
				break;
		}
}

void f2mc16_device::execute_run()
{
	debugger_instruction_hook((m_pcb<<16) | m_pc);

	m_icount = 0;
}

void f2mc16_device::execute_set_input(int inputnum, int state)
{
}
