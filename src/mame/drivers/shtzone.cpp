// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

"Sega Shooting Zone" aka "Sega Sharp Shooter"

This is an SMS with a timer system, official Sega product.  Cabinet has a lightgun, it runs the SMS light gun games.

It has 2 IO controllers, and 1 VDP, so I'm guessing the BIOS just displays to some kind of Segment LED display.

---------------------------------

Shooting Zone by SEGA 1987

834-6294

CPU(s) : D780C (x2)

Xtal : 10.7380 Mhz

RAMS(s) : D4168C (x3)
    : MB8464-12L

Eprom : Epr10894A.20

PAL : 315-5287

Customs IC's :  315-5216 (x2)

        315-5124

GAMES for this system :

Black Belt (mpr10150.ic1)

Shooting Gallery

Gangster Town

Marksman Shooting / Trap Shooting / Safari Hunt (315-5028.ic1 + Mpr10157.ic2)

Fantasy Zone(1)

---------------------------------

Notes:
(1) apparently.... seems a bit odd, because it's not a gun game


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"

class shtzone_state : public driver_device
{
public:
	shtzone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	void shtzone(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_shtzone(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void shtzone_map(address_map &map);
};

void shtzone_state::shtzone_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x5fff).ram();
}


static INPUT_PORTS_START( shtzone )
INPUT_PORTS_END


void shtzone_state::machine_start()
{
}

void shtzone_state::machine_reset()
{
}

void shtzone_state::video_start()
{
}


uint32_t shtzone_state::screen_update_shtzone(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void shtzone_state::shtzone(machine_config &config)
{
	/* basic machine hardware */
	z80_device &timercpu(Z80(config, "timercpu", 10738000/4));
	timercpu.set_addrmap(AS_PROGRAM, &shtzone_state::shtzone_map);

	/* + SMS CPU */


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 256-1);
	screen.set_screen_update(FUNC(shtzone_state::screen_update_shtzone));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100);
}


ROM_START( shtzone )
	ROM_REGION( 0x4000, "timercpu", 0 )
	ROM_LOAD( "epr10894a.20", 0x00000, 0x04000, CRC(ea8901d9) SHA1(43fd8bfc395e3b2e3fbe9645d692a5eb04783d9c) )
ROM_END

GAME( 1987, shtzone, 0, shtzone, shtzone, shtzone_state, empty_init, ROT0, "Sega", "Shooting Zone System BIOS", MACHINE_IS_SKELETON | MACHINE_IS_BIOS_ROOT )
