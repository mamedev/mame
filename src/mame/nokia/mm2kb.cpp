// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 keyboard emulation

*********************************************************************/

#include "emu.h"
#include "mm2kb.h"

DEFINE_DEVICE_TYPE(NOKIA_MM2_KBD, mm2_keyboard_device, "nokia_mm2_kbd", "MikroMikko 2 Keyboard")

ROM_START( mm2_keyboard )
	ROM_REGION( 0x400, "keyboard", 0 )
	ROM_LOAD( "keyboard", 0x000, 0x400, NO_DUMP )
ROM_END

const tiny_rom_entry *mm2_keyboard_device::device_rom_region() const
{
	return ROM_NAME( mm2_keyboard );
}

void mm2_keyboard_device::device_add_mconfig(machine_config &config)
{
}

INPUT_PORTS_START( mm2_keyboard )
INPUT_PORTS_END

ioport_constructor mm2_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mm2_keyboard );
}

mm2_keyboard_device::mm2_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NOKIA_MM2_KBD, tag, owner, clock),
	m_write_txd(*this)
{
}

void mm2_keyboard_device::device_start()
{
}

void mm2_keyboard_device::device_reset()
{
	m_write_txd(1);
}
