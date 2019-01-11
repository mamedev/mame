// license:BSD-3-Clause
// copyright-holders:Luca Elia
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
      HY-02  - unknown DIP8 chip (maybe EEPROM?)

***************************************************************************/

#include "emu.h"
#include "cpu/h8/h83048.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// 13.0 MHz? PCB is labeled with 13.5M
#define MAIN_CLOCK  13000000

class sealy_state : public driver_device
{
public:
	sealy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")
	{ }

	void sealy(machine_config &config);

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

	// screen updates
	void sealy_palette(palette_device &palette) const;
	uint32_t screen_update_sealy(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void sealy_map(address_map &map);
};


void sealy_state::sealy_palette(palette_device &palette) const
{
//  for (int i = 0; i < 32768; i++)
//      palette.set_pen_color(i, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}

uint32_t sealy_state::screen_update_sealy(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	return 0;
}


void sealy_state::sealy_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom();
}


static INPUT_PORTS_START( sealy )
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


MACHINE_CONFIG_START(sealy_state::sealy)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", H83044, MAIN_CLOCK) /* wrong CPU, but we have not a M16C core ATM */
	MCFG_DEVICE_PROGRAM_MAP(sealy_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(sealy_state, screen_update_sealy)

	GFXDECODE(config, "gfxdecode", m_palette, gfx_sealy);
	PALETTE(config, m_palette, FUNC(sealy_state::sealy_palette), 32768);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("oki", OKIM6295, MAIN_CLOCK/13, okim6295_device::PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( crzyddz )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "crzyddz_m30624.mcu", 0x00000, 0x40000, NO_DUMP )
	ROM_FILL(                       0x00000, 0x40000, 0x00 )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "1", 0x000000, 0x200000, CRC(d202a278) SHA1(3ae75d6942527e58a56a703e40de22e70535b332) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "2", 0x000000, 0x200000, CRC(c1382873) SHA1(7e506ee013e2c97f8d4f88cf33871a27fd034841) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "3", 0x00000, 0x80000, CRC(cb626168) SHA1(652b20e92c82de480e3cd41c2e3c984fcb0c120a) )
ROM_END


GAME( 2004?, crzyddz,  0, sealy, sealy, sealy_state, empty_init, ROT0, "Sealy", "Crazy Dou Di Zhu", MACHINE_IS_SKELETON )
