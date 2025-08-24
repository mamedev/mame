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
	w65c02_device(mconfig, type, tag, owner, clock), TMP3(0), Z(0), B(0)
{
}

std::unique_ptr<util::disasm_interface> m65ce02_device::create_disassembler()
{
	return std::make_unique<m65ce02_disassembler>();
}

void m65ce02_device::init()
{
	w65c02_device::init();
	state_add(M65CE02_Z, "Z", Z);
	state_add(M65CE02_B, "B", B).callimport().formatstr("%2s");
	save_item(NAME(B));
	save_item(NAME(Z));
	save_item(NAME(TMP3));
	Z = 0x00;
	B = 0x0000;
	TMP3 = 0x0000;
}

void m65ce02_device::device_start()
{
	mintf = std::make_unique<mi_default>();

	init();
}

void m65ce02_device::device_reset()
{
	w65c02_device::device_reset();
	Z = 0x00;
	B = 0x0000;
}

void m65ce02_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		P = P | F_B;
		break;
	case M65CE02_B:
		B <<= 8;
		break;
	}
}

void m65ce02_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		str = string_format("%c%c%c%c%c%c%c",
						P & F_N ? 'N' : '.',
						P & F_V ? 'V' : '.',
						P & F_E ? 'E' : '.',
						P & F_D ? 'D' : '.',
						P & F_I ? 'I' : '.',
						P & F_Z ? 'Z' : '.',
						P & F_C ? 'C' : '.');
		break;
	case M65CE02_B:
		str = string_format("%02x", B >> 8);
		break;
	}
}

#include "cpu/m6502/m65ce02.hxx"
