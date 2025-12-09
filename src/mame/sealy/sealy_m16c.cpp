// license:BSD-3-Clause
// copyright-holders: Luca Elia

/***************************************************************************

Crazy Dou Di Zhu
Sealy, 2004?

Skeleton driver

PCB Layout:

SEALY 2004.9
|-------------------------|
|     3                   |
|                        1|
|J       13.0MHz   ACTEL  |
|A          HY-02  A54SX16|
|M   M6295                |
|M                        |
|A  BATT                  |
| SW     M30624   6264   2|
|-------------------------|
Notes:
      M30624 - 16-bit microcontroller with 20k RAM and 256k ROM
               chip will be protected and likely contain the main program.
      HY-02  - unknown DIP8 chip (maybe some PIC?)

***************************************************************************/

#include "emu.h"

#include "cpu/h8/h83048.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sealy_m16c_state : public driver_device
{
public:
	sealy_m16c_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")
	{ }

	void sealy(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

	void palette_init(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


void sealy_m16c_state::palette_init(palette_device &palette) const
{
//  for (int i = 0; i < 32768; i++)
//      palette.set_pen_color(i, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}

uint32_t sealy_m16c_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	return 0;
}


void sealy_m16c_state::program_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom();
}


static INPUT_PORTS_START( sealy )
// no DIP switches
INPUT_PORTS_END


static const gfx_layout gfxlayout_8x8x16 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 2) },
	{ STEP8(0, 8*2) },
	{ STEP8(0, 8*8*2) },
	8*8*16
};


static GFXDECODE_START( gfx_sealy )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout_8x8x16, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_8x8x16, 0, 1 )
GFXDECODE_END


void sealy_m16c_state::sealy(machine_config &config)
{
	// basic machine hardware
	H83044(config, m_maincpu, 13.5_MHz_XTAL); // wrong CPU, but we only have a M16C disassembler ATM. XTAL not readable, but marked 13.5M on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &sealy_m16c_state::program_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(sealy_m16c_state::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_sealy);
	PALETTE(config, m_palette, FUNC(sealy_m16c_state::palette_init), 32768);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 13.5_MHz_XTAL / 13, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // divider and pin 7 not verified
}


ROM_START( crzyddz )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "crzyddz_m30624.mcu", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "1", 0x000000, 0x200000, CRC(d202a278) SHA1(3ae75d6942527e58a56a703e40de22e70535b332) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "2", 0x000000, 0x200000, CRC(c1382873) SHA1(7e506ee013e2c97f8d4f88cf33871a27fd034841) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "3", 0x00000, 0x80000, CRC(cb626168) SHA1(652b20e92c82de480e3cd41c2e3c984fcb0c120a) )

	ROM_REGION( 0x4280, "pic", 0 )
	ROM_LOAD( "hy-02", 0x0000, 0x4280, NO_DUMP )
ROM_END

// 三打哈 (Sān Dǎ Hā)
// PCB is extremely similar to the one documented at the top, but has a 62256 instead of the 6264
ROM_START( sandaha )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sandaha_m30624.mcu", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "1.u1", 0x000000, 0x200000, CRC(bb92334f) SHA1(f8530f9519ba28fe696fa3ddab463413d30b37a7) ) // 27C160

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "2.u2", 0x000000, 0x200000, CRC(c3f1711b) SHA1(2adeaca9785489a8ebbfc931050ff5f1447c9972) ) // 27C160

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "3", 0x00000, 0x80000, CRC(e611f748) SHA1(3bc5361fe192f61f35df32a6e3e6d68b8404bf27) )

	ROM_REGION( 0x4280, "pic", 0 )
	ROM_LOAD( "hy-01", 0x0000, 0x4280, NO_DUMP )
ROM_END

// 疯狂斗地主 (Fēngkuáng Dòu Dìzhǔ)
// This could be very similar to crzyddz above (GFX ROMs have few differences)
// PCB is extremely similar to the one documented at the top, but has a 62256 instead of the 6264 and a Altera MAX EPM3256AOC208-10 instead of the A54SX16
ROM_START( fkddz )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "fkddz_m30624.mcu", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "1.u1", 0x000000, 0x200000, CRC(c70937b2) SHA1(62308c59a3176c796bc64be585b6382614bbe0d4) ) // 27C160

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "2.u2", 0x000000, 0x200000, CRC(13b77a6d) SHA1(df12653d7b9bdd2409994b548d085c31f24e68c6) ) // 27C160

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "3", 0x00000, 0x80000, CRC(cb626168) SHA1(652b20e92c82de480e3cd41c2e3c984fcb0c120a) )

	ROM_REGION( 0x4280, "pic", 0 )
	ROM_LOAD( "pic", 0x0000, 0x4280, NO_DUMP ) // scratched / unreadable
ROM_END

// 疯斗加强版 (Fēngdòu Jiāqiáng Bǎn)
// PCB is extremely similar to the one documented at the top, but has a 62256 instead of the 6264
ROM_START( fdjqb )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "fdjqb_m30624.mcu", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "1.u1", 0x000000, 0x200000, CRC(c70937b2) SHA1(62308c59a3176c796bc64be585b6382614bbe0d4) ) // 27C160, same as fkddz

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "2.u2", 0x000000, 0x200000, CRC(13b77a6d) SHA1(df12653d7b9bdd2409994b548d085c31f24e68c6) ) // 27C160, same as fkddz

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "3.u3", 0x00000, 0x80000, CRC(cb626168) SHA1(652b20e92c82de480e3cd41c2e3c984fcb0c120a) ) // 27C040, same as crzyddz

	ROM_REGION( 0x4280, "pic", 0 )
	ROM_LOAD( "key-02", 0x0000, 0x4280, NO_DUMP )
ROM_END

// 杜爆四方 (Gāngbào Sìfāng)
// Has 2x 62256 instead of the 6264, 14.31818 XTAL instead of 13.5, Altera MAX EPM7256SRC208-12 instead of the A54SX16, no HY-02
ROM_START( gbsf )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "gbsf_m30624.mcu", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "03084586-4.u20", 0x000000, 0x200000, CRC(d410152b) SHA1(be51b55130b09dd1b0b56d175a33837775eb0d90) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "03084586-4.u21", 0x000000, 0x200000, CRC(338eac71) SHA1(aee6c6fb166c529bf179594ebfbe06c11bdb3f4d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "3", 0x00000, 0x40000, CRC(67291416) SHA1(7ddf33ad780e547b335de94d0090f08dd230aaa1) )
ROM_END
} // anonymous namespace


// years were taken from GFX ROMs
GAME( 2004,  crzyddz,  0, sealy, sealy, sealy_m16c_state, empty_init, ROT0, "Sealy", "Crazy Dou Di Zhu",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004,  sandaha,  0, sealy, sealy, sealy_m16c_state, empty_init, ROT0, "Sealy", "San Da Ha",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004,  fkddz,    0, sealy, sealy, sealy_m16c_state, empty_init, ROT0, "Sealy", "Fengkuang Dou Dizhu",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004,  fdjqb,    0, sealy, sealy, sealy_m16c_state, empty_init, ROT0, "Sealy", "Fengdou Jiaqiang Ban", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2003,  gbsf,     0, sealy, sealy, sealy_m16c_state, empty_init, ROT0, "Sealy", "Gangbao Sifang",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
