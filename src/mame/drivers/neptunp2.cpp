// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/***************************************************************************

    Neptune's Pearls (c) Unidesa?

    skeleton driver, can't do much without gfx roms anyway.

***************************************************************************/


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

MACHINE_CONFIG_START(neptunp2_state::neptunp2)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu",I80188,20000000) // N80C188-20 AMD
	MCFG_DEVICE_PROGRAM_MAP(neptunp2_map)
	MCFG_DEVICE_IO_MAP(neptunp2_io)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", neptunp2_state, irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(neptunp2_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_PALETTE("palette")

	GFXDECODE(config, "gfxdecode", "palette", gfx_neptunp2);
	MCFG_PALETTE_ADD("palette", 512)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

MACHINE_CONFIG_END

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


GAME( 199?, neptunp2,  0,   neptunp2, neptunp2, neptunp2_state, empty_init, ROT0, "Unidesa?", "Neptune's Pearls 2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
