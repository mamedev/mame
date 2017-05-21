// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65ce02.c

    6502 with Z register and some more stuff

***************************************************************************/

#include "emu.h"
#include "m65ce02.h"

DEFINE_DEVICE_TYPE(M65CE02, m65ce02_device, "m65ce02", "M65CE02")

m65ce02_device::m65ce02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m65ce02_device(mconfig, M65CE02, tag, owner, clock)
{
}

m65ce02_device::m65ce02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m65c02_device(mconfig, type, tag, owner, clock), TMP3(0), Z(0), B(0)
{
}

offs_t m65ce02_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	return disassemble_generic(stream, pc, oprom, opram, options, disasm_entries);
}

void m65ce02_device::init()
{
	m65c02_device::init();
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
	if(direct_disabled)
		mintf = new mi_default_nd;
	else
		mintf = new mi_default_normal;

	init();
}

void m65ce02_device::device_reset()
{
	m65c02_device::device_reset();
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

void m65ce02_device::state_export(const device_state_entry &entry)
{
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
