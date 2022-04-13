// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"
#include "gunsense.h"

#include "cpu/upd78k/upd78k2.h"

// device type definition
DEFINE_DEVICE_TYPE(SEGA_GUNSENSE, sega_gunsense_board_device, "gunsense", "Sega 838-13143 Gun Sense Board")

sega_gunsense_board_device::sega_gunsense_board_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGA_GUNSENSE, tag, owner, clock)
{
}

void sega_gunsense_board_device::device_start()
{
}


void sega_gunsense_board_device::mem_map(address_map &map)
{
	map(0x00000, 0x0fcff).rom().region("gunmcu", 0);
}


void sega_gunsense_board_device::device_add_mconfig(machine_config &config)
{
	upd78213_device &mcu(UPD78213(config, "gunmcu", 12'000'000)); // clock unknown
	mcu.set_addrmap(AS_PROGRAM, &sega_gunsense_board_device::mem_map);
}

/*
 Gun board info:
 "838-13143-nn IC BD GUN SENSE xxx" board, D78213 MCU based, was used in big number of Model2 / Model3 / NAOMI / Chihiro / Lindbergh games.
 known firmwares:
 EPR-20006A  oldest revision?
 EPR-21262 - older revision, used in: 04 HOD (House of the Dead 2), 06 JPT (Jurassic Park The Lost World).
 TG12      - newer revision, used in: 08 SPY, 09 SPY UR, 11 SPY UR EXTRA - Confidential Mission and later games, backward compatible with older games as well.
*/

ROM_START(gunsense)
	ROM_REGION( 0x10000, "gunmcu", 0 )
	ROM_LOAD( "epr-20006a.ic2",0x00000, 0x10000, CRC(45f310dc) SHA1(b7cf40a1671dc351b607d8d6bba0d51ea128eb75) )
	ROM_LOAD( "epr-21262.ic2", 0x00000, 0x10000, CRC(c9adf9b6) SHA1(fc2a331430ef2f009f653b242220599c824cd1d2) )
	ROM_LOAD( "tg12.ic2",      0x00000, 0x10000, CRC(2c9600b1) SHA1(91813a43851c48d400fde41b1198dabf55bade2d) )
ROM_END

const tiny_rom_entry *sega_gunsense_board_device::device_rom_region() const
{
	return ROM_NAME(gunsense);
}
