/***************************************************************************

    Last Bank skeleton driver

    Uses a TC0091LVC, a variant of the one used on Taito L HW

    TODO:
    - somebody should port CPU core contents in a shared file;

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"

class lastbank_state : public driver_device
{
public:
	lastbank_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};

static VIDEO_START( lastbank )
{

}

static SCREEN_UPDATE( lastbank )
{
	return 0;
}

static ADDRESS_MAP_START( lastbank_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM /* TODO: ROM banks! */

	AM_RANGE(0x8000, 0x9fff) AM_RAM
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

static ADDRESS_MAP_START( lastbank_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

ADDRESS_MAP_END


static INPUT_PORTS_START( lastbank )

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

static const gfx_layout char_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

static GFXDECODE_START( lastbank )
	GFXDECODE_ENTRY( "gfx1", 0, bg2_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, sp2_layout, 0, 16 )
	GFXDECODE_ENTRY( "maincpu",           0, char_layout,  0, 16 )  // Ram-based
GFXDECODE_END

#define MASTER_CLOCK XTAL_14_31818MHz

static MACHINE_CONFIG_START( lastbank, lastbank_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MASTER_CLOCK/4) //!!! TC0091LVC !!!
	MCFG_CPU_PROGRAM_MAP(lastbank_map)
	MCFG_CPU_IO_MAP(lastbank_io)

//  MCFG_CPU_ADD("audiocpu",Z80,MASTER_CLOCK/4)

	//MCFG_MACHINE_START(lastbank)
	//MCFG_MACHINE_RESET(lastbank)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE(lastbank)


	MCFG_GFXDECODE( lastbank )
	MCFG_PALETTE_LENGTH(16)

	MCFG_VIDEO_START(lastbank)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	// es8712
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( lastbank )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "3.u9", 0x00000, 0x40000, CRC(f430e1f0) SHA1(dd5b697f5c2250d98911f4c7d3e7d4cc16b0b40f) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "8.u48", 0x00000, 0x10000, CRC(3a7bfe10) SHA1(7dc543e11d3c0b9872fcc622339ade25383a1eb3) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "5.u10", 0x00000, 0x20000, CRC(51f3c5a7) SHA1(73d4c8817fe96d75be32c43e816e93c52b5d2b27) )

	ROM_REGION( 0x200000, "essnd", 0 ) /* Samples */
	ROM_LOAD( "6.u55", 0x00000, 0x40000, CRC(9e78e234) SHA1(031f93e4bc338d0257fa673da7ce656bb1cda5fb) )
	ROM_LOAD( "7.u60", 0x40000, 0x80000, CRC(41be7146) SHA1(00f1c0d5809efccf888e27518a2a5876c4b633d8) )
ROM_END

GAME( 1994, lastbank,  0,   lastbank, lastbank,  0, ROT0, "Excellent Systems", "Last Bank", GAME_IS_SKELETON )
