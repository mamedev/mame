// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/***************************************************************************

    Neptune's Pearls (c) Unidesa?

    skeleton driver, can't do much without gfx roms anyway.

****************************************************************************

The "960606-5" PCB (found on the "rockroll" set) is used at least on the following games:

Unidesa Cirsa Millenium
Unidesa Cirsa Euro Lucky
Unidesa Cirsa Rock 'n' Roll
Unidesa Cirsa Max Money
Unidesa Cirsa Vikingos
Unidesa Cirsa Mini Joker
Unidesa Cirsa Far West
Unidesa Cirsa Saloon
Unidesa Cirsa Blue Swamp Land
Unidesa Cirsa Vulcano
Unidesa Cirsa Euro Bingo 7 (1000)
Unidesa Cirsa Euro Bingo 7
Unidesa Cirsa Gladiadores
Unidesa Cirsa Nevada
Unidesa Cirsa Monsters Manía
Unidesa Cirsa Mini Guay Plus
Unidesa Cirsa Perla del Caribe
Unidesa Cirsa Super Sevens
Unidesa Cirsa Legend
Unidesa Cirsa Dinopolis
Unidesa Cirsa Megatron
Unidesa Cirsa Megatron Salon
Unidesa Cirsa Extra Cash
Unidesa Cirsa Mini Genio
Unidesa Cirsa Las Llaves del Tesoro
Unidesa Cirsa Secreto de la Pirámide
Unidesa Cirsa Filón
Unidesa Cirsa Multi Points

 CIRSA / UNIDESA 960606-5 CPU BOARD
 _________________________________________________________________
 |        ________                                                |
 |__      |ULN2003|                                   ____        |
 || |__ _  __________________                         X9313       |
 ||P||P||| |OTP 27C8000 or  |   _______    ________               |
 ||1||1||| |27C4001_-_SOUND_|   |OKI   |   |S1 DIPS|              |
 || ||5|P9 __________________   |MSM6376   |_______|  _________   |
 ||_||_|   |    27C8000 or  |   |______|   _________  PAT063/31 (PAL16L8)
 |         |27C4001_-_SOUND_|              |S2 DIPS|              |
 |__                                                              |
 ||P|      __________________  __________________    ____    BATT |
 ||7|      |27C801 or       |  |RAM MS62256-79  |    8583P   3V6  |
 ||_|      |27C4001_________|  |________________|           179mAh|
 |__       __________________  __________________                 |
 ||P|__    |27C801 or       |  |MS628512        |    ____         |
 ||1||P|   |27C4001_________|  |NOT_POPULATED___|   X24C16        |
 ||1||18              _______                                     |
 || |__               |CPLD |    ________    ____________         |
 ||_||P|              |PD18 |    | 75189 |   | CIRSA     |     __ |
 |   |17              |_____|  NOT POPULATED | 38302 or  |     |P||
 |   |_|     XTAL 36.8640MHz     ________    | 38304     |     |2||
 |__                             | 75188 |   |           |     | ||
 ||P|      ___________         NOT POPULATED |___________|     | ||
 ||3|      |CPU       |      ________          ________   ____ |_||
 ||_|      |80C188XL  |      |7407___|         |7406___| LM393    |
 |         |          |      ________          ________           |
 |__       |          |      |74HC14_|         |74HC00_|          |
 ||P|      |__________|      ________          ________           |
 ||8|        ________        |74HC14_|         |74HC14_|          |
 || |        |74HC14_|       ________                             |
 || |      ____________      |74HCT08|       ____________         |
 ||_| __   | CIRSA     |     ________        | CIRSA     |        |
 |    |P|  | 38302 or  |     |74HCT14|       | 38302 or  |        |
 |__  |1|  | 38304     |     ________        | 38304     |        |
 ||P| |3|  | MASTER    |     |74HCT14|       |           |        |
 ||5| __   |___________|                     |___________|        |
 || | |P|                                                         |
 || | |4|    ________        ________           ________          |
 ||_| | |    |ULN2064        |ULN2064           |74LS145 <- NOT POPULATED
 |    | |    ________        ________         __________          |
 |    | |    |ULN2064        |ULN2064         |UDN2580A| <- NOT POPULATED
 |    |_|   __________       ________         __________          |
 |          |___P14___|      |ULN2064         |_ARRAY__| <- NOT POPULATED
 |                          NOT POPULATED                 ______  |
 |_________________________________________________________P19____|

P4, P8, P13, P15, P16 and P19 are unused.

Some service manuals contains the complete PCB schematics (e.g., see the "Manual Técnico Cirsa Vulcano" PDF).

*/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class neptunp2_state : public driver_device
{
public:
	neptunp2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void neptunp2(machine_config &config);

private:
	DECLARE_READ8_MEMBER(test_r);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void neptunp2_io(address_map &map);
	void neptunp2_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void video_start() override;
};


void neptunp2_state::video_start()
{
}

uint32_t neptunp2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ8_MEMBER( neptunp2_state::test_r )
{
	return machine().rand();
}

void neptunp2_state::neptunp2_map(address_map &map)
{
	map(0x00000, 0xbffff).rom();
	map(0xe0000, 0xeffff).ram();

	map(0xd0000, 0xd7fff).ram(); //videoram
	map(0xdb004, 0xdb007).ram();
	map(0xdb00c, 0xdb00f).ram();

	map(0xff806, 0xff806).r(FUNC(neptunp2_state::test_r));
	map(0xff810, 0xff810).r(FUNC(neptunp2_state::test_r));
	map(0xff812, 0xff812).r(FUNC(neptunp2_state::test_r));

	map(0xff980, 0xff980).nopw();

	map(0xffff0, 0xfffff).rom();
}

void neptunp2_state::neptunp2_io(address_map &map)
{
}


static INPUT_PORTS_START( neptunp2 )
INPUT_PORTS_END

#if 0
static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,3),  /* 1024 characters */
	3,  /* 3 bits per pixel */
	{ RGN_FRAC(1,3), RGN_FRAC(2,3), RGN_FRAC(0,3) },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};
#endif

static GFXDECODE_START( gfx_neptunp2 )
//  GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 8 )
GFXDECODE_END

void neptunp2_state::neptunp2(machine_config &config)
{
	/* basic machine hardware */
	I80188(config, m_maincpu, 36.864_MHz_XTAL); // N80C188-20 AMD
	m_maincpu->set_addrmap(AS_PROGRAM, &neptunp2_state::neptunp2_map);
	m_maincpu->set_addrmap(AS_IO, &neptunp2_state::neptunp2_io);
	m_maincpu->set_vblank_int("screen", FUNC(neptunp2_state::irq0_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update(FUNC(neptunp2_state::screen_update));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_neptunp2);
	PALETTE(config, "palette").set_entries(512);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	// OKIM6376(config, "oki", xxx).add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( neptunp2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "u2.bin",   0x000000, 0x100000, CRC(4fbb06d1) SHA1(6490cd3b96b3b61f48fcb843772bd787605ab76f) )

	ROM_REGION( 0x100000, "prg_data", 0 ) //dunno how this maps ...
	ROM_LOAD( "u3.bin",   0x000000, 0x100000, CRC(3c1746e2) SHA1(a7fd59f5397ce1653848e15f16399b537f3a1ea7) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "u14.bin",  0x000000, 0x100000, CRC(a2de1156) SHA1(58b325b720057e8d7105fe3a87ac2c0109afad84) )
	ROM_LOAD( "u15.bin",  0x100000, 0x100000, CRC(8de6d4de) SHA1(121e7507ef57074bc7ad0bf69556f26c84c4e236) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "flash_roms", 0x00000, 0x10000, NO_DUMP )
ROM_END

ROM_START( rockroll ) // PCB serigraphed 'CB1 (CS4)' and '960606-5 CPU'. It was found with most sockets unpopulated. This is mechanical, no GFX but a Samsung VFD.
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "u2",   0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x100000, "prg_data", 0 )
	ROM_LOAD( "u3",   0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "c.rock_n_roll_b-2103_6219_otp_b-82_m27c801.u14",  0x000000, 0x100000, CRC(963d184b) SHA1(8ad8b3215d3fc513dfae27bea2ed2ae9939c0f02) )
	ROM_LOAD( "u15",  0x100000, 0x100000, NO_DUMP ) // it's also possible it wasn't ever populated

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24lc16b.u10",  0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pat_063_tibpal16l8-25cn.bin",  0x000, 0x104, NO_DUMP )
ROM_END

GAME( 199?, neptunp2,  0,   neptunp2, neptunp2, neptunp2_state, empty_init, ROT0, "Unidesa?", "Neptune's Pearls 2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1999, rockroll,  0,   neptunp2, neptunp2, neptunp2_state, empty_init, ROT0, "Unidesa / Cirsa", "Rock 'n' Roll", MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // year taken from parts' manual and sticker on PCB
