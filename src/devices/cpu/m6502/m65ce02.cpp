// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65ce02.c

    6502 with Z register and some more stuff

***************************************************************************/

#include "emu.h"
#include "m65ce02.h"

const device_type M65CE02 = &device_creator<m65ce02_device>;

m65ce02_device::m65ce02_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	m65c02_device(mconfig, M65CE02, "M65CE02", tag, owner, clock, "m65ce02", __FILE__), TMP3(0), Z(0), B(0)
{
}

m65ce02_device::m65ce02_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	m65c02_device(mconfig, type, name, tag, owner, clock, shortname, source), TMP3(0), Z(0), B(0)
{
}

offs_t m65ce02_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
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
		strprintf(str, "%c%c%c%c%c%c%c",
						P & F_N ? 'N' : '.',
						P & F_V ? 'V' : '.',
						P & F_E ? 'E' : '.',
						P & F_D ? 'D' : '.',
						P & F_I ? 'I' : '.',
						P & F_Z ? 'Z' : '.',
						P & F_C ? 'C' : '.');
		break;
	case M65CE02_B:
		strprintf(str, "%02x", B >> 8);
		break;
	}
}

#include "cpu/m6502/m65ce02.inc"
