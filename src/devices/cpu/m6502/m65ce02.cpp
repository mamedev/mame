// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65ce02.cpp

    6502 with Z register and some more stuff

***************************************************************************/

#include "emu.h"
#include "m65ce02.h"
#include "m65ce02d.h"

DEFINE_DEVICE_TYPE(M65CE02, m65ce02_device, "m65ce02", "CSG 65CE02")

m65ce02_device::m65ce02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m65ce02_device(mconfig, M65CE02, tag, owner, clock)
{
}

m65ce02_device::m65ce02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	w65c02_device(mconfig, type, tag, owner, clock), m_TMP3(0), m_Z(0), m_B(0)
{
}

std::unique_ptr<util::disasm_interface> m65ce02_device::create_disassembler()
{
	return std::make_unique<m65ce02_disassembler>();
}

void m65ce02_device::init()
{
	w65c02_device::init();
	state_add(M65CE02_Z, "Z", m_Z);
	state_add(M65CE02_B, "B", m_B).callimport().formatstr("%2s");
	save_item(NAME(m_B));
	save_item(NAME(m_Z));
	save_item(NAME(m_TMP3));
	m_Z = 0x00;
	m_B = 0x0000;
	m_TMP3 = 0x0000;
}

void m65ce02_device::device_start()
{
	m_mintf = std::make_unique<mi_default>();

	init();
}

void m65ce02_device::device_reset()
{
	w65c02_device::device_reset();
	m_Z = 0x00;
	m_B = 0x0000;
}

void m65ce02_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		m_P = m_P | F_B;
		break;
	case M65CE02_B:
		m_B <<= 8;
		break;
	}
}

void m65ce02_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		str = string_format("%c%c%c%c%c%c%c",
						m_P & F_N ? 'N' : '.',
						m_P & F_V ? 'V' : '.',
						m_P & F_E ? 'E' : '.',
						m_P & F_D ? 'D' : '.',
						m_P & F_I ? 'I' : '.',
						m_P & F_Z ? 'Z' : '.',
						m_P & F_C ? 'C' : '.');
		break;
	case M65CE02_B:
		str = string_format("%02x", m_B >> 8);
		break;
	}
}

#include "cpu/m6502/m65ce02.hxx"
