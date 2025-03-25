// license:BSD-3-Clause
// copyright-holders:

/***************************************************************************************

Skeleton driver for "Paracaidista" (from Video Game / Electrogame).

There are two versions of the game: The original from 1979 (undumped), and a
recreation from 2023 done by the same original programmer (Javier Valero, who
later founded Gaelco) for the same hardware.

Six PCBs connected by a small custom backplane.

More info and schematics for the original 1979 version:
   https://www.recreativas.org/el-paracaidista-404-videogame-electrogame

More info, schematics (minor differences from the original hardware), and source
code for the 2023 version:
   https://www.recreativas.org/paracaidista-version-2023-08-15294-videogame-electrogame

****************************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"

#include "machine/i8155.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class paracaidista_state : public driver_device
{
public:
	paracaidista_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void paracaidista(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// devices
	required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START(paracaidista)
INPUT_PORTS_END

uint32_t paracaidista_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static GFXDECODE_START( gfx_paracaidista )
GFXDECODE_END

void paracaidista_state::paracaidista(machine_config &config)
{
	I8085A(config, m_maincpu, 6'553'600);

	I8155(config, "m8155", 6'553'600);

	// Video hardware (probably wrong values)
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(paracaidista_state::screen_update));
	screen.set_size(320, 200);
	screen.set_visarea(0, 320-1, 0, 200-1);
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(8);

	// Sound hardware
	SPEAKER(config, "mono").front_center();
}

// Recreation from 2023
ROM_START( paraca )
	ROM_REGION( 0x2400, "maincpu", 0 )
	ROM_LOAD( "rom1.a3", 0x0000, 0x0400, CRC(5e052f4c) SHA1(4de01c21c3771eaded13b4c5a215ab44650c3133) )
	ROM_LOAD( "rom2.a2", 0x0400, 0x0400, CRC(09f80f7e) SHA1(94f9c7cf4bb55055cb1580e44865acd99000fe25) )
	ROM_LOAD( "rom3.a1", 0x0800, 0x0400, CRC(ea6b27be) SHA1(c4fbe92509faff33bfd7d5169ed21108a019464f) )
	ROM_LOAD( "rom4.b3", 0x0C00, 0x0400, CRC(0d9d9f44) SHA1(06d3e2090e38dcb64bb279c3c128835b05c4e405) )
	ROM_LOAD( "rom5.b2", 0x1000, 0x0400, CRC(33579a33) SHA1(68c7442132d6a816b53ee615b05408683178a3d3) )
	ROM_LOAD( "rom6.b1", 0x1400, 0x0400, CRC(d7719b47) SHA1(a2f3d079aa8919defdaea77450e5defb6c721669) )
	ROM_LOAD( "rom7.c3", 0x1800, 0x0400, CRC(95fa4573) SHA1(761bb8d0c2bcab5b3b3804dc24584e971e05c700) )
	ROM_LOAD( "rom8.c2", 0x1c00, 0x0400, CRC(b7e96680) SHA1(13bdc30c6b083ff08ce1228cb458bedb2060d2f6) )
	ROM_LOAD( "rom9.c1", 0x2000, 0x0400, CRC(eafe8b9e) SHA1(ab3c2e564f02b4c997cb1b58fcc1e978e77ee9db) )
	// ROMs 10, 11, and 12 unused
ROM_END


} // Anonymous namespace

//    YEAR  NAME    PARENT  MACHINE       INPUT         CLASS               INIT        ROT   COMPANY                     FULLNAME                     FLAGS
GAME( 2023, paraca, 0,      paracaidista, paracaidista, paracaidista_state, empty_init, ROT0, "Video Game / Electrogame", "Paracaidista (recreation)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
