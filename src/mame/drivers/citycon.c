/***************************************************************************


2008-07
Dip locations added from dip listing at crazykong.com

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/2203intf.h"


extern UINT8 *citycon_videoram;
extern UINT8 *citycon_scroll;
extern UINT8 *citycon_linecolor;
WRITE8_HANDLER( citycon_videoram_w );
WRITE8_HANDLER( citycon_linecolor_w );
WRITE8_HANDLER( citycon_background_w );

VIDEO_UPDATE( citycon );
VIDEO_START( citycon );


static READ8_HANDLER( citycon_in_r )
{
	return input_port_read(space->machine, flip_screen_get(space->machine) ? "P2" : "P1");
}



static ADDRESS_MAP_START( citycon_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_RAM_WRITE(citycon_videoram_w) AM_BASE(&citycon_videoram)
	AM_RANGE(0x2000, 0x20ff) AM_WRITE(citycon_linecolor_w) AM_BASE(&citycon_linecolor)
	AM_RANGE(0x2800, 0x28ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x3000, 0x3000) AM_READWRITE(citycon_in_r, citycon_background_w)	/* player 1 & 2 inputs multiplexed */
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("DSW1") AM_WRITE(soundlatch_w)
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("DSW2") AM_WRITE(soundlatch2_w)
	AM_RANGE(0x3004, 0x3005) AM_WRITE(SMH_RAM) AM_BASE(&citycon_scroll)
	AM_RANGE(0x3007, 0x3007) AM_READ(watchdog_reset_r)	/* ? */
	AM_RANGE(0x3800, 0x3cff) AM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE(&paletteram)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x4000, 0x4001) AM_DEVWRITE("ay", ay8910_address_data_w)
//  AM_RANGE(0x4002, 0x4002) AM_DEVREAD("ay", ay8910_r)  /* ?? */
	AM_RANGE(0x6000, 0x6001) AM_DEVREADWRITE("ym", ym2203_r, ym2203_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( citycon )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	/* the coin input must stay low for exactly 2 frames to be consistently recognized. */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW2:7" )
	/* According to manual this is Flip Screen setting */
//  PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip Screen ) ) PORT_DIPLOCATION("SW2:8")
//  PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	5,
	{ 16, 12, 8, 4, 0 },
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3 },
	{ 0*24, 1*24, 2*24, 3*24, 4*24, 5*24, 6*24, 7*24 },
	24*8
};

static const gfx_layout tilelayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	4,	/* 4 bits per pixel */
	{ 4, 0, 0xc000*8+4, 0xc000*8+0 },
	{ 0, 1, 2, 3, 256*8*8+0, 256*8*8+1, 256*8*8+2, 256*8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	8,16,	/* 8*16 sprites */
	128,	/* 128 sprites */
	4,	/* 4 bits per pixel */
	{ 4, 0, 0x2000*8+4, 0x2000*8+0 },
	{ 0, 1, 2, 3, 128*16*8+0, 128*16*8+1, 128*16*8+2, 128*16*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
            8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8	/* every sprite takes 16 consecutive bytes */
};


static GFXDECODE_START( citycon )
//  GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 512, 32 ) /* colors 512-639 */
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 640, 32 )	/* colors 512-639 */
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout, 0, 16 )	/* colors 0-255 */
	GFXDECODE_ENTRY( "gfx2", 0x01000, spritelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, tilelayout, 256, 16 )	/* colors 256-511 */
	GFXDECODE_ENTRY( "gfx3", 0x01000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x02000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x03000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x04000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x05000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x06000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x07000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x08000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x09000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x0a000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x0b000, tilelayout, 256, 16 )
GFXDECODE_END



static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_MEMORY_HANDLER("audiocpu", PROGRAM, soundlatch_r),
		DEVCB_MEMORY_HANDLER("audiocpu", PROGRAM, soundlatch2_r),
		DEVCB_NULL,
		DEVCB_NULL
	},
	NULL
};



static MACHINE_DRIVER_START( citycon )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, 2048000)        /* 2.048 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(citycon_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", M6809, 640000)       /* 0.640 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(citycon)
	MDRV_PALETTE_LENGTH(640+1024)	/* 640 real palette + 1024 virtual palette */

	MDRV_VIDEO_START(citycon)
	MDRV_VIDEO_UPDATE(citycon)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 1250000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("ym", YM2203, 1250000)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.40)
	MDRV_SOUND_ROUTE(1, "mono", 0.40)
	MDRV_SOUND_ROUTE(2, "mono", 0.40)
	MDRV_SOUND_ROUTE(3, "mono", 0.20)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( citycon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c10",          0x4000, 0x4000, CRC(ae88b53c) SHA1(dd12310bd9c9b93462446e8e0a1c853506bf3aa1) )
	ROM_LOAD( "c11",          0x8000, 0x8000, CRC(139eb1aa) SHA1(c570e8ca1499f7ea61938e78c32c1cc3050ca2b7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c1",           0x8000, 0x8000, CRC(1fad7589) SHA1(2e626bbbab8cffe11ee7de3e56aa1871c29d5fa9) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "c4",           0x00000, 0x2000, CRC(a6b32fc6) SHA1(d99d5a527440e9a91525c1084b95b213e3b760ec) )	/* Characters */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "c12",          0x00000, 0x2000, CRC(08eaaccd) SHA1(a970381e3ba22bcdea6df2d31cd8a10c4b2bc413) )	/* Sprites    */
	ROM_LOAD( "c13",          0x02000, 0x2000, CRC(1819aafb) SHA1(8a5ffcd8866e09c5568879257384767d61796111) )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "c9",           0x00000, 0x8000, CRC(8aeb47e6) SHA1(bb09dbe6b37e1bd02abf3024ac4d954c8f0e70f2) )	/* Background tiles */
	ROM_LOAD( "c8",           0x08000, 0x4000, CRC(0d7a1eeb) SHA1(60b8d4124ce857a248d3c41fdb050f11be58549f) )
	ROM_LOAD( "c6",           0x0c000, 0x8000, CRC(2246fe9d) SHA1(f7f8708d499bcbd1a583e1092b54425ad1105f94) )
	ROM_LOAD( "c7",           0x14000, 0x4000, CRC(e8b97de9) SHA1(f4d1b7075f47ab4522c36281b97eaa02fe383814) )

	ROM_REGION( 0xe000, "gfx4", 0 )	/* background tilemaps */
	ROM_LOAD( "c2",           0x0000, 0x8000, CRC(f2da4f23) SHA1(5ea1a51c3ac283796f7eafb6719d88356767340d) )	/* background maps */
	ROM_LOAD( "c3",           0x8000, 0x4000, CRC(7ef3ac1b) SHA1(8a0497c4e4733f9c50d576f632210b82497a5e1c) )
	ROM_LOAD( "c5",           0xc000, 0x2000, CRC(c03d8b1b) SHA1(641c1eba334d36ea64b9293a20320b31c7c88858) )	/* color codes for the background */
ROM_END

ROM_START( citycona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c10",          0x4000, 0x4000, CRC(ae88b53c) SHA1(dd12310bd9c9b93462446e8e0a1c853506bf3aa1) )
	ROM_LOAD( "c11b",         0x8000, 0x8000, CRC(d64af468) SHA1(5bb3541af3ce632e8eca313231205713d72fb9dc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c1",           0x8000, 0x8000, CRC(1fad7589) SHA1(2e626bbbab8cffe11ee7de3e56aa1871c29d5fa9) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "c4",           0x00000, 0x2000, CRC(a6b32fc6) SHA1(d99d5a527440e9a91525c1084b95b213e3b760ec) )	/* Characters */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "c12",          0x00000, 0x2000, CRC(08eaaccd) SHA1(a970381e3ba22bcdea6df2d31cd8a10c4b2bc413) )	/* Sprites    */
	ROM_LOAD( "c13",          0x02000, 0x2000, CRC(1819aafb) SHA1(8a5ffcd8866e09c5568879257384767d61796111) )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "c9",           0x00000, 0x8000, CRC(8aeb47e6) SHA1(bb09dbe6b37e1bd02abf3024ac4d954c8f0e70f2) )	/* Background tiles */
	ROM_LOAD( "c8",           0x08000, 0x4000, CRC(0d7a1eeb) SHA1(60b8d4124ce857a248d3c41fdb050f11be58549f) )
	ROM_LOAD( "c6",           0x0c000, 0x8000, CRC(2246fe9d) SHA1(f7f8708d499bcbd1a583e1092b54425ad1105f94) )
	ROM_LOAD( "c7",           0x14000, 0x4000, CRC(e8b97de9) SHA1(f4d1b7075f47ab4522c36281b97eaa02fe383814) )

	ROM_REGION( 0xe000, "gfx4", 0 )	/* background tilemaps */
	ROM_LOAD( "c2",           0x0000, 0x8000, CRC(f2da4f23) SHA1(5ea1a51c3ac283796f7eafb6719d88356767340d) )	/* background maps */
	ROM_LOAD( "c3",           0x8000, 0x4000, CRC(7ef3ac1b) SHA1(8a0497c4e4733f9c50d576f632210b82497a5e1c) )
	ROM_LOAD( "c5",           0xc000, 0x2000, CRC(c03d8b1b) SHA1(641c1eba334d36ea64b9293a20320b31c7c88858) )	/* color codes for the background */
ROM_END

ROM_START( cruisin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cr10",         0x4000, 0x4000, CRC(cc7c52f3) SHA1(69d76f146fb1dac62c6def3a4269012b3880f03b) )
	ROM_LOAD( "cr11",         0x8000, 0x8000, CRC(5422f276) SHA1(d384fc4f853fe79b73e939a8fc7b7af780659c5e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c1",           0x8000, 0x8000, CRC(1fad7589) SHA1(2e626bbbab8cffe11ee7de3e56aa1871c29d5fa9) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "cr4",          0x00000, 0x2000, CRC(8cd0308e) SHA1(7303b9e074bda557d64b39e04cef0f965a756be6) )	/* Characters */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "c12",          0x00000, 0x2000, CRC(08eaaccd) SHA1(a970381e3ba22bcdea6df2d31cd8a10c4b2bc413) )	/* Sprites    */
	ROM_LOAD( "c13",          0x02000, 0x2000, CRC(1819aafb) SHA1(8a5ffcd8866e09c5568879257384767d61796111) )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "c9",           0x00000, 0x8000, CRC(8aeb47e6) SHA1(bb09dbe6b37e1bd02abf3024ac4d954c8f0e70f2) )	/* Background tiles */
	ROM_LOAD( "c8",           0x08000, 0x4000, CRC(0d7a1eeb) SHA1(60b8d4124ce857a248d3c41fdb050f11be58549f) )
	ROM_LOAD( "c6",           0x0c000, 0x8000, CRC(2246fe9d) SHA1(f7f8708d499bcbd1a583e1092b54425ad1105f94) )
	ROM_LOAD( "c7",           0x14000, 0x4000, CRC(e8b97de9) SHA1(f4d1b7075f47ab4522c36281b97eaa02fe383814) )

	ROM_REGION( 0xe000, "gfx4", 0 )	/* background tilemaps */
	ROM_LOAD( "c2",           0x0000, 0x8000, CRC(f2da4f23) SHA1(5ea1a51c3ac283796f7eafb6719d88356767340d) )	/* background maps */
	ROM_LOAD( "c3",           0x8000, 0x4000, CRC(7ef3ac1b) SHA1(8a0497c4e4733f9c50d576f632210b82497a5e1c) )
	ROM_LOAD( "c5",           0xc000, 0x2000, CRC(c03d8b1b) SHA1(641c1eba334d36ea64b9293a20320b31c7c88858) )	/* color codes for the background */
ROM_END



static DRIVER_INIT( citycon )
{
	UINT8 *rom = memory_region(machine, "gfx1");
	int i;


	/*
      City Connection controls the text color code for each _scanline_, not
      for each character as happens in most games. To handle that conveniently,
      I convert the 2bpp char data into 5bpp, and create a virtual palette so
      characters can still be drawn in one pass.
      */
	for (i = 0x0fff;i >= 0;i--)
	{
		int mask;

		rom[3*i] = rom[i];
		rom[3*i+1] = 0;
		rom[3*i+2] = 0;
		mask = rom[i] | (rom[i] << 4) | (rom[i] >> 4);
		if (i & 0x01) rom[3*i+1] |= mask & 0xf0;
		if (i & 0x02) rom[3*i+1] |= mask & 0x0f;
		if (i & 0x04) rom[3*i+2] |= mask & 0xf0;
	}
}



GAME( 1985, citycon,  0,       citycon, citycon, citycon, ROT0, "Jaleco", "City Connection (set 1)", 0 )
GAME( 1985, citycona, citycon, citycon, citycon, citycon, ROT0, "Jaleco", "City Connection (set 2)", 0 )
GAME( 1985, cruisin,  citycon, citycon, citycon, citycon, ROT0, "Jaleco (Kitkorp license)", "Cruisin", 0 )
