/***************************************************************************

    Dream Fruit skeleton driver

    Uses a TC0091LVC, a variant of the one used on Taito L HW

    TODO:
    - somebody should port CPU core contents in a shared file;

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"

class dfruit_state : public driver_device
{
public:
	dfruit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};

static VIDEO_START( dfruit )
{

}

static SCREEN_UPDATE( dfruit )
{
	return 0;
}

static ADDRESS_MAP_START( dfruit_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM /* TODO: ROM banks! */

	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xa004, 0xa005) AM_DEVREADWRITE("opn",ym2203_r,ym2203_w)
	AM_RANGE(0xa008, 0xa008) AM_READNOP //watchdog
	/* TODO: RAM banks! */
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xfdff) AM_RAM

	//AM_RANGE(0xfe00, 0xfe03) AM_READWRITE(taitol_bankc_r, taitol_bankc_w)
	//AM_RANGE(0xfe04, 0xfe04) AM_READWRITE(taitol_control_r, taitol_control_w)

	//AM_RANGE(0xff00, 0xff02) AM_READWRITE(irq_adr_r, irq_adr_w)
	//AM_RANGE(0xff03, 0xff03) AM_READWRITE(irq_enable_r, irq_enable_w)
	//AM_RANGE(0xff04, 0xff07) AM_READWRITE(rambankswitch_r, rambankswitch_w)
	//AM_RANGE(0xff08, 0xff08) AM_READWRITE(rombankswitch_r, rombankswitch_w)

ADDRESS_MAP_END

static ADDRESS_MAP_START( dfruit_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

ADDRESS_MAP_END


static INPUT_PORTS_START( dfruit )

INPUT_PORTS_END

static const gfx_layout bg2_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

#define O 8*8*4
#define O2 2*O
static const gfx_layout sp2_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16, O+3, O+2, O+1, O+0, O+19, O+18, O+17, O+16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, O2+0*32, O2+1*32, O2+2*32, O2+3*32, O2+4*32, O2+5*32, O2+6*32, O2+7*32 },
	8*8*4*4
};
#undef O
#undef O2

#if 0
static const gfx_layout char_layout =
{
	8, 8,
	1024,
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};
#endif

static GFXDECODE_START( dfruit )
	GFXDECODE_ENTRY( "gfx1", 0, bg2_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, sp2_layout, 0, 16 )
	//GFXDECODE_ENTRY( NULL,           0, char_layout,  0, 16 )  // Ram-based
GFXDECODE_END

#define MASTER_CLOCK XTAL_14MHz

static MACHINE_CONFIG_START( dfruit, dfruit_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MASTER_CLOCK/2) //!!! TC0091LVC !!!
	MCFG_CPU_PROGRAM_MAP(dfruit_map)
	MCFG_CPU_IO_MAP(dfruit_io)

	//MCFG_MACHINE_START(4enraya)
	//MCFG_MACHINE_RESET(4enraya)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE(dfruit)


	MCFG_GFXDECODE( dfruit )
	MCFG_PALETTE_LENGTH(16)

	MCFG_VIDEO_START(dfruit)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("opn", YM2203, MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( dfruit )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "n-3800ii_ver.1.20.ic2", 0x00000, 0x40000, CRC(4e7c3700) SHA1(17bc731a91460d8f67c2b2b6e038641d57cf93be) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c2.ic10", 0x00000, 0x80000, CRC(d869ab24) SHA1(382e874a846855a7f6f8811625aaa30d9dfa1ce2) )
ROM_END

GAME( 199?, dfruit,  0,   dfruit, dfruit,  0, ROT0, "<unknown>", "Dream Fruit", GAME_IS_SKELETON )
