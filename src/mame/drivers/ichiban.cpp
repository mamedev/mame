// license:BSD-3-Clause
// copyright-holders:Guru
/***************************************************************************

Ichi Ban Jyan
Excel, 199?

TODO: code is encrypted (data, not opcodes)

PCB Layout
----------

MJ911
|----------------------------------|
|MB3712 DSW-D DSW-C DSW-B DSW-A  SW|
|   M6378                   BATT   |
|VOL               6264         3  |
|    YM2413 MJB                    |
|M                  1           2  |
|A  YM2149  MJG  |-------|         |
|H               |ALTERA |  Z80    |
|J          MJR  |EP1810 |         |
|O               |       |  ALTERA |
|N               |-------|  EP910  |
|G                                 |
|                                  |
|      41464  41464                |
|      41464  41464       18.432MHz|
|----------------------------------|
Notes:
Z80 clock - 6.144MHz [18.432/3]
YM2149 clock - 1.536MHz [18.432/12]
YM2413 clock - 3.072MHz [18.432/6]
VSync - 60.5686Hz
HSync - 15.510kHz

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/2413intf.h"

#define MAIN_CLOCK XTAL_18_432MHz

class ichibanjyan_state : public driver_device
{
public:
	ichibanjyan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

void ichibanjyan_state::video_start()
{
}

UINT32 ichibanjyan_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}


static ADDRESS_MAP_START( ichibanjyan_map, AS_PROGRAM, 8, ichibanjyan_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( ichibanjyan_io, AS_IO, 8, ichibanjyan_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( ichibanjyan )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ STEP8( 0*512, 8 ) },
	{ STEP8( 0*512, 8*8 ) },
	8*8*8
};

static GFXDECODE_START( ichibanjyan )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,     0, 1 )
GFXDECODE_END


void ichibanjyan_state::machine_start()
{
	UINT8 *ROM = memregion("code")->base();

	membank("bank1")->configure_entries(0, 4, ROM, 0x8000);
}

void ichibanjyan_state::machine_reset()
{
}


static MACHINE_CONFIG_START( ichibanjyan, ichibanjyan_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(ichibanjyan_map)
	MCFG_CPU_IO_MAP(ichibanjyan_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(ichibanjyan_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ichibanjyan)

	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 512)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", YM2149, MAIN_CLOCK/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ymsnd", YM2413, MAIN_CLOCK/6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ichiban )
	ROM_REGION( 0x20000, "code", 0 )
	ROM_LOAD( "3.u15", 0, 0x20000, CRC(76240568) SHA1(cf055d1eaae25661a49ec4722a2c7caca862e66a) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "1.u28", 0, 0x20000, CRC(2caa4d3f) SHA1(5e5af164880140b764c097a65388c22ba5ea572b) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.u14", 0, 0x20000, CRC(b4834d8e) SHA1(836ddf7586dc5440faf88f5ec50a32265e9a0ec8) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "mjr.u36", 0x000, 0x200, CRC(31cd7a90) SHA1(1525ad19d748561a52626e4ab13df67d9bedf3b8) )
	ROM_LOAD( "mjg.u37", 0x200, 0x200, CRC(5b3562aa) SHA1(ada60d2a5a5a657d7b209d18a23b685305d9ff7b) )
	ROM_LOAD( "mjb.u38", 0x400, 0x200, CRC(0ef881cb) SHA1(44b61a443d683f5cb2d1b1a4f74d8a8f41021de5) )
ROM_END

GAME( 199?, ichiban,  0,   ichibanjyan,  ichibanjyan, driver_device,  0,       ROT0, "Excel",      "Ichi Ban Jyan", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
