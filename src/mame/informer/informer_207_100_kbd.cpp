// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Informer 207/100 Keyboard

***************************************************************************/

#include "emu.h"
#include "informer_207_100_kbd.h"


DEFINE_DEVICE_TYPE(INFORMER_207_100_KBD, informer_207_100_kbd_device, "in207100kbd", "Informer 207/100 Keyboard")

informer_207_100_kbd_device::informer_207_100_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, INFORMER_207_100_KBD, tag, owner, clock)
	, m_mcu(*this, "mcu")
{
}

void informer_207_100_kbd_device::device_start()
{
}


static INPUT_PORTS_START(informer_207_100_kbd)
	// TODO
INPUT_PORTS_END

ioport_constructor informer_207_100_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(informer_207_100_kbd);
}

void informer_207_100_kbd_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_mcu, 6'000'000); // XTAL unreadable
}

ROM_START(informer_207_100_kbd)
	ROM_REGION(0x400, "mcu", 0)
	ROM_LOAD("79499-001_version6.bin", 0x000, 0x400, CRC(aad1ed7b) SHA1(f3e5e7d9138abede70e97026f4b2e52e0df6ea31))
ROM_END

const tiny_rom_entry *informer_207_100_kbd_device::device_rom_region() const
{
	return ROM_NAME(informer_207_100_kbd);
}
