/****************************************************************************

    This hardware is derived from Arkanoid's hardware.  The 1986 date found in
    the roms probably comes from Arkanoid.  This is a Columns style game and
    the original Columns wasn't released until 1990 and I find it hard to
    believe that this would pre-date Columns.

    HEXA

    driver by Howie Cohen

    Memory map (prelim)
    0000 7fff ROM
    8000 bfff bank switch rom space??
    c000 c7ff RAM
    e000 e7ff video ram
    e800-efff unused RAM

    read:
    d001      AY8910 read
    f000      ???????

    write:
    d000      AY8910 control
    d001      AY8910 write
    d008      bit0/1 = flip screen x/y
              bit 4 = ROM bank??
              bit 5 = char bank
              other bits????????
    d010      watchdog reset, or IRQ acknowledge, or both
    f000      ????????

*************************************************************************

    main hardware consists of.....

    sub board with Z80 x2, 2 ROMs and a scratched 18 pin chip (probably a PIC)

    main board has....
    12MHz xtal
    ay3-8910
    8 position DSW x1
    ROMs x4
    6116 SRAM x3
    82S123 PROMs x3

*************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/hexa.h"


static ADDRESS_MAP_START( hexa_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd001, 0xd001) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0xd000, 0xd001) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xd008, 0xd008) AM_WRITE(hexa_d008_w)
	AM_RANGE(0xd010, 0xd010) AM_WRITE(watchdog_reset_w)	/* or IRQ acknowledge, or both */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(hexa_videoram_w) AM_BASE_SIZE_MEMBER(hexa_state, videoram, videoram_size)
ADDRESS_MAP_END



static INPUT_PORTS_START( hexa )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, "Naughty Pics" )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Difficulty?" )  PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "Easy?" )
	PORT_DIPSETTING(    0x20, "Medium?" )
	PORT_DIPSETTING(    0x10, "Hard?" )
	PORT_DIPSETTING(    0x00, "Hardest?" )
	PORT_DIPNAME( 0x40, 0x40, "Pobys" )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8 by 8 */
	4096,   /* 4096 characters */
	3,		/* 3 bits per pixel */
	{ 2*4096*8*8, 4096*8*8, 0 },	/* plane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },		/* x bit */
	{ 8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7 }, 	/* y bit */
	8*8
};



static GFXDECODE_START( hexa )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,  0 , 32 )
GFXDECODE_END



static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("INPUTS"),
	DEVCB_INPUT_PORT("DSW"),
	DEVCB_NULL,
	DEVCB_NULL
};



static MACHINE_START( hexa )
{
	hexa_state *state = (hexa_state *)machine->driver_data;

	state_save_register_global(machine, state->charbank);
}

static MACHINE_RESET( hexa )
{
	hexa_state *state = (hexa_state *)machine->driver_data;

	state->charbank = 0;
}

static MACHINE_DRIVER_START( hexa )

	/* driver data */
	MDRV_DRIVER_DATA(hexa_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)	/* Imported from arkanoid - correct? */
	MDRV_CPU_PROGRAM_MAP(hexa_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(hexa)
	MDRV_MACHINE_RESET(hexa)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(hexa)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(RRRR_GGGG_BBBB)
	MDRV_VIDEO_START(hexa)
	MDRV_VIDEO_UPDATE(hexa)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, XTAL_12MHz/4/2)	/* Imported from arkanoid - correct? */
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( hexa )
	ROM_REGION( 0x18000, "maincpu", 0 )		/* 64k for code + 32k for banked ROM */
	ROM_LOAD( "hexa.20",      0x00000, 0x8000, CRC(98b00586) SHA1(3591a3b0486d720f0aaa9f0bf4be352cd0ffcbc7) )
	ROM_LOAD( "hexa.21",      0x10000, 0x8000, CRC(3d5d006c) SHA1(ad4eadab82024b122182eacb5a322cfd6e476a70) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "hexa.17",      0x00000, 0x8000, CRC(f6911dd6) SHA1(b12ea27ecddd60820a32d4346afab0cc9d06fa57) )
	ROM_LOAD( "hexa.18",      0x08000, 0x8000, CRC(6e3d95d2) SHA1(6399b7b5d088ceda08fdea9cf650f6b405f038e7) )
	ROM_LOAD( "hexa.19",      0x10000, 0x8000, CRC(ffe97a31) SHA1(f16b5d2b9ace09bcbbfe3dfb73db7fa377d1af7f) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "hexa.001",     0x0000, 0x0100, CRC(88a055b4) SHA1(eee86a7930d0a251f3e5c2134532cd1dede2026c) )
	ROM_LOAD( "hexa.003",     0x0100, 0x0100, CRC(3e9d4932) SHA1(9a336dba7134400312985b9902c77b4141105853) )
	ROM_LOAD( "hexa.002",     0x0200, 0x0100, CRC(ff15366c) SHA1(7feaf1c768bfe76432fb80991585e13d95960b34) )
ROM_END



static DRIVER_INIT( hexa )
{
	UINT8 *RAM = memory_region(machine, "maincpu");
#if 0


	/* Hexa is not protected or anything, but it keeps writing 0x3f to register */
	/* 0x07 of the AY8910, to read the input ports. This causes clicks in the */
	/* music since the output channels are continuously disabled and reenabled. */
	/* To avoid that, we just NOP out the 0x3f write. */

	RAM[0x0124] = 0x00;
	RAM[0x0125] = 0x00;
	RAM[0x0126] = 0x00;
#endif

	memory_configure_bank(machine, "bank1", 0, 2, &RAM[0x10000], 0x4000);
}


GAME( 199?, hexa, 0, hexa, hexa, hexa, ROT0, "D. R. Korea", "Hexa", GAME_SUPPORTS_SAVE )
