/***************************************************************************

 GX545 Scooter Shooter - (c) 1985 Konami

 It uses a mixed hardware based on Jailbreak and Iron Horse

***************************************************************************/

#include "driver.h"
#include "sound/2203intf.h"

extern UINT8 *scotrsht_scroll;

extern WRITE8_HANDLER( scotrsht_videoram_w );
extern WRITE8_HANDLER( scotrsht_colorram_w );
extern WRITE8_HANDLER( scotrsht_charbank_w );
extern WRITE8_HANDLER( scotrsht_palettebank_w );

extern PALETTE_INIT( scotrsht );
extern VIDEO_START( scotrsht );
extern VIDEO_UPDATE( scotrsht );

static int irq_enable;

static WRITE8_HANDLER( ctrl_w )
{
	irq_enable = data & 0x02;
	flip_screen_set(data & 0x08);
}

static INTERRUPT_GEN( scotrsht_interrupt )
{
	if (irq_enable)
		cpunum_set_input_line(machine, 0, 0, HOLD_LINE);
}

static WRITE8_HANDLER( scotrsht_soundlatch_w )
{
	soundlatch_w(machine,0,data);
	cpunum_set_input_line(machine, 1, 0, HOLD_LINE);
}

static ADDRESS_MAP_START( scotrsht_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM_WRITE(scotrsht_colorram_w) AM_BASE(&colorram)
    AM_RANGE(0x0800, 0x0fff) AM_RAM_WRITE(scotrsht_videoram_w) AM_BASE(&videoram)
    AM_RANGE(0x1000, 0x10bf) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size) /* sprites */
	AM_RANGE(0x10c0, 0x1fff) AM_RAM /* work ram */
    AM_RANGE(0x2000, 0x201f) AM_RAM AM_BASE(&scotrsht_scroll) /* scroll registers */
	AM_RANGE(0x2040, 0x2040) AM_WRITE(SMH_NOP)
	AM_RANGE(0x2041, 0x2041) AM_WRITE(SMH_NOP)
    AM_RANGE(0x2042, 0x2042) AM_WRITE(SMH_NOP)  /* it should be -> bit 2 = scroll direction like in jailbrek, but it's not used */
	AM_RANGE(0x2043, 0x2043) AM_WRITE(scotrsht_charbank_w)
    AM_RANGE(0x2044, 0x2044) AM_WRITE(ctrl_w)
	AM_RANGE(0x3000, 0x3000) AM_WRITE(scotrsht_palettebank_w)
	AM_RANGE(0x3100, 0x3100) AM_WRITE(scotrsht_soundlatch_w)
	AM_RANGE(0x3200, 0x3200) AM_WRITE(SMH_NOP) /* it writes 0, 1 */
	AM_RANGE(0x3100, 0x3100) AM_READ(input_port_4_r) /* DSW1 */
	AM_RANGE(0x3200, 0x3200) AM_READ(input_port_5_r) /* DSW2 */
	AM_RANGE(0x3300, 0x3300) AM_READ(input_port_0_r) /* coins, start */
	AM_RANGE(0x3301, 0x3301) AM_READ(input_port_1_r) /* joy1 */
	AM_RANGE(0x3302, 0x3302) AM_READ(input_port_2_r) /* joy2 */
	AM_RANGE(0x3303, 0x3303) AM_READ(input_port_3_r) /* DSW0 */
	AM_RANGE(0x3300, 0x3300) AM_WRITE(watchdog_reset_w) /* watchdog */
    AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( scotrsht_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
    AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( scotrsht_sound_port, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(YM2203_status_port_0_r, YM2203_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM2203_write_port_0_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( scotrsht )
	PORT_START	/* IN0 - $3300 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN1 - $3301 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN2 - $3302 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* DSW0  - $3303 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "Invalid" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "Invalid" )

	PORT_START	/* DSW1  - $3100 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "30K 80K" )
	PORT_DIPSETTING(    0x00, "40K 90K" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, "Difficult" )
	PORT_DIPSETTING(    0x00, "Very Difficult" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW2  - $3200 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,1),	/* 1024 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },	/* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,1),	/* 512 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },	/* the bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( scotrsht )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0, 16*8 ) /* characters */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*16*8, 16*8 ) /* sprites */
GFXDECODE_END

static MACHINE_DRIVER_START( scotrsht )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", M6809, 18432000/6)        /* 3.072 MHz */
	MDRV_CPU_PROGRAM_MAP(scotrsht_map,0)
	MDRV_CPU_VBLANK_INT("main", scotrsht_interrupt)

	MDRV_CPU_ADD("audio", Z80, 18432000/6)        /* 3.072 MHz */
	MDRV_CPU_PROGRAM_MAP(scotrsht_sound_map,0)
	MDRV_CPU_IO_MAP(scotrsht_sound_port,0)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(scotrsht)
	MDRV_PALETTE_LENGTH(16*8*16+16*8*16)

	MDRV_PALETTE_INIT(scotrsht)
	MDRV_VIDEO_START(scotrsht)
	MDRV_VIDEO_UPDATE(scotrsht)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2203, 18432000/6)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( scotrsht )
	ROM_REGION( 0x10000, RGNCLASS_CPU, "main", 0 )
	ROM_LOAD( "gx545_g03_12c.bin", 0x8000, 0x4000, CRC(b808e0d3) SHA1(d42b6979ade705a7522bd0bbc3eaa6d661580902) )
	ROM_CONTINUE(				   0x4000, 0x4000 )
	ROM_LOAD( "gx545_g02_10c.bin", 0xc000, 0x4000, CRC(b22c0586) SHA1(07c21609c6cdfe2b8dd734d21086c5236ff8197b) )

	ROM_REGION( 0x10000, RGNCLASS_CPU, "audio", 0 )	/* 64k for sound code */
	ROM_LOAD( "gx545_g01_8c.bin",  0x0000, 0x4000, CRC(46a7cc65) SHA1(73389fe04ce40da124d630dc3f8e58600d9556fc) )

	ROM_REGION( 0x08000, RGNCLASS_GFX, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "gx545_g05_5f.bin",  0x0000, 0x8000, CRC(856c349c) SHA1(ba45e6d18e56cc7fc49c8fda190ec152ce6bd15c) )	/* characters */

	ROM_REGION( 0x10000, RGNCLASS_GFX, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "gx545_g06_6f.bin",  0x0000, 0x8000, CRC(14ad7601) SHA1(6dfcf2abfa2ea056c948d82d35c55f033f3e4678) )	/* sprites */
	ROM_LOAD( "gx545_h04_4f.bin",  0x8000, 0x8000, CRC(c06c11a3) SHA1(6e89c738498d716fd43d9cc7b71b23438bd3c4b8) )

	ROM_REGION( 0x0500, RGNCLASS_PROMS, "proms", 0 )
	ROM_LOAD( "gx545_6301_1f.bin", 0x0000, 0x0100, CRC(f584586f) SHA1(0576cd0a738737c18143af887efd5ce76cdfc7cb) ) /* red */
	ROM_LOAD( "gx545_6301_2f.bin", 0x0100, 0x0100, CRC(ad464db1) SHA1(24937f2c9143e925c9becb488e11aa6daa807817) ) /* green */
	ROM_LOAD( "gx545_6301_3f.bin", 0x0200, 0x0100, CRC(bd475d23) SHA1(4ae6dfbb5c40a5ff97d7d80d0a441c1dc6dc5705) ) /* blue */
	ROM_LOAD( "gx545_6301_7f.bin", 0x0300, 0x0100, CRC(2b0cd233) SHA1(a2ccf693bf378ce8dd311c4224ad20de59418f88) ) /* char lookup */
	ROM_LOAD( "gx545_6301_8f.bin", 0x0400, 0x0100, CRC(c1c7cf58) SHA1(08452228bf13e43ce4a05806f79e9cd1542416f1) ) /* sprites lookup */
ROM_END

GAME( 1985, scotrsht, 0, scotrsht, scotrsht, 0, ROT90,"Konami", "Scooter Shooter", 0 )
