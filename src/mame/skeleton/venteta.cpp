// license:BSD-3-Clause
// copyright-holders:

/*
Venteta by Rania (?)

MegaRani SYSTEM Version 2001

Main components:

ATMEGA103 MCU (undumped internal ROM)
PSD813F1 (not dumped)
M48T08-100PC1
EPM7064LC68-10
24 MHz XTAL
4x 6264
ISD2564P audio chip
1 reset switch
*/

#include "emu.h"

#include "cpu/avr8/avr8.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class venteta_state : public driver_device
{
public:
	venteta_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void venteta(machine_config &config);

private:
	required_device<atmega168_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t venteta_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void venteta_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}


static INPUT_PORTS_START(venteta)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static GFXDECODE_START( gfx_venteta ) // TODO
	GFXDECODE_ENTRY( "tiles1", 0, gfx_8x8x8_raw,  0, 16 ) // not correct
	GFXDECODE_ENTRY( "tiles2", 0, gfx_8x8x8_raw,  0, 16 ) // not correct
GFXDECODE_END


void venteta_state::venteta(machine_config &config)
{
	ATMEGA168(config, m_maincpu, 24_MHz_XTAL); // TODO: actually ATMEGA103
	m_maincpu->set_addrmap(AS_PROGRAM, &venteta_state::program_map);
	m_maincpu->set_eeprom_tag("eeprom");

	// TODO: everything
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(venteta_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_venteta);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();
}


ROM_START( venteta )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "internal_rom.ic22", 0x00000, 0x20000, NO_DUMP ) // size not verified

	ROM_REGION( 0x20000, "psd813f1", ROMREGION_ERASE00 )
	ROM_LOAD( "psd813.ic10", 0x00000, 0x20000, NO_DUMP ) // size not verified

	ROM_REGION( 0x1000, "eeprom", ROMREGION_ERASE00 )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "27c801.ic1", 0x000000, 0x100000, CRC(2f7a62e7) SHA1(684c6a83f4ed8ce35042826d327627cf493b7654) ) // 1xxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "27c801.ic2", 0x100000, 0x100000, CRC(654f70fe) SHA1(0d8dfe5196fee48224f847cd0c7e17c2cd0ecf45) ) // 1xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "27c801.ic11", 0x000000, 0x100000, CRC(c1ad4498) SHA1(a5f9a3bc0af37d43bb8b38cc01c927ca18a4ccb2) ) // 1xxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "27c801.ic12", 0x100000, 0x100000, CRC(25fe1d01) SHA1(530f80ed5de836af1876d6a7aec70a0ec3f81d47) ) // 1xxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 200?, venteta, 0, venteta, venteta, venteta_state, empty_init, ROT0, "Rania", "Venteta", MACHINE_IS_SKELETON )
