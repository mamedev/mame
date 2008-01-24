/***************************************************************************

Pinball Action memory map (preliminary)

driver by Nicola Salmoria


0000-9fff ROM
d000-d3ff Video RAM
d400-d7ff Color RAM
d800-dbff Background Video RAM
dc00-dfff Background Color RAM
e000-e07f Sprites
e400-e5ff Palette RAM

read:
e600      IN0
e601      IN1
e602      IN2
e604      DSW1
e605      DSW2
e606      watchdog reset????

write:
e600      interrupt enable
e604      flip screen
e606      bg scroll? not sure
e800      command for the sound CPU


Notes:
- pbactio2 has a ROM for a third Z80, not emulated, function unknown

***************************************************************************/

#include "driver.h"
#include "sound/ay8910.h"
#include "machine/segacrpt.h"


extern UINT8 *pbaction_videoram2,*pbaction_colorram2;

extern WRITE8_HANDLER( pbaction_videoram_w );
extern WRITE8_HANDLER( pbaction_colorram_w );
extern WRITE8_HANDLER( pbaction_videoram2_w );
extern WRITE8_HANDLER( pbaction_colorram2_w );
extern WRITE8_HANDLER( pbaction_flipscreen_w );
extern WRITE8_HANDLER( pbaction_scroll_w );

extern VIDEO_START( pbaction );
extern VIDEO_UPDATE( pbaction );


static WRITE8_HANDLER( pbaction_sh_command_w )
{
	soundlatch_w(offset,data);
	cpunum_set_input_line_and_vector(Machine, 1,0,HOLD_LINE,0x00);
}


static UINT8 *work_ram;


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_ROM)
	AM_RANGE(0xc000, 0xdfff) AM_READ(MRA8_RAM)
	AM_RANGE(0xe000, 0xe07f) AM_READ(MRA8_RAM)
	AM_RANGE(0xe400, 0xe5ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xe600, 0xe600) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0xe601, 0xe601) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0xe602, 0xe602) AM_READ(input_port_2_r)	/* IN2 */
	AM_RANGE(0xe604, 0xe604) AM_READ(input_port_3_r)	/* DSW1 */
	AM_RANGE(0xe605, 0xe605) AM_READ(input_port_4_r)	/* DSW2 */
	AM_RANGE(0xe606, 0xe606) AM_READ(MRA8_NOP)	/* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(MWA8_RAM) AM_BASE(&work_ram)
	AM_RANGE(0xd000, 0xd3ff) AM_WRITE(pbaction_videoram2_w) AM_BASE(&pbaction_videoram2)
	AM_RANGE(0xd400, 0xd7ff) AM_WRITE(pbaction_colorram2_w) AM_BASE(&pbaction_colorram2)
	AM_RANGE(0xd800, 0xdbff) AM_WRITE(pbaction_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xdc00, 0xdfff) AM_WRITE(pbaction_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xe000, 0xe07f) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xe400, 0xe5ff) AM_WRITE(paletteram_xxxxBBBBGGGGRRRR_le_w) AM_BASE(&paletteram)
	AM_RANGE(0xe600, 0xe600) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0xe604, 0xe604) AM_WRITE(pbaction_flipscreen_w)
	AM_RANGE(0xe606, 0xe606) AM_WRITE(pbaction_scroll_w)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(pbaction_sh_command_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x8000, 0x8000) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xffff, 0xffff) AM_WRITE(MWA8_NOP)	/* watchdog? */
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x10, 0x10) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x21, 0x21) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(AY8910_control_port_2_w)
	AM_RANGE(0x31, 0x31) AM_WRITE(AY8910_write_port_2_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( pbaction )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "70K 200K 1000K" )
	PORT_DIPSETTING(    0x00, "70K 200K" )
	PORT_DIPSETTING(    0x04, "100K 300K 1000K" )
	PORT_DIPSETTING(    0x03, "100K 300K" )
	PORT_DIPSETTING(    0x02, "100K" )
	PORT_DIPSETTING(    0x06, "200K 1000K" )
	PORT_DIPSETTING(    0x05, "200K" )
	PORT_DIPSETTING(    0x07, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, "Extra" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x00, "Difficulty (Flippers)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0x00, "Difficulty (Outlanes)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )
INPUT_PORTS_END



static const gfx_layout charlayout1 =
{
	8,8,	/* 8*8 characters */
	1024,	/* 1024 characters */
	3,	/* 3 bits per pixel */
	{ 0, 1024*8*8, 2*1024*8*8 },	/* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },	/* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};
static const gfx_layout charlayout2 =
{
	8,8,	/* 8*8 characters */
	2048,	/* 2048 characters */
	4,	/* 4 bits per pixel */
	{ 0, 2048*8*8, 2*2048*8*8, 3*2048*8*8 },	/* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },	/* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};
static const gfx_layout spritelayout1 =
{
	16,16,	/* 16*16 sprites */
	128,	/* 128 sprites */
	3,	/* 3 bits per pixel */
	{ 0, 256*16*16, 2*256*16*16 },	/* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,	/* pretty straightforward layout */
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8	/* every sprite takes 32 consecutive bytes */
};
static const gfx_layout spritelayout2 =
{
	32,32,	/* 32*32 sprites */
	32,	/* 32 sprites */
	3,	/* 3 bits per pixel */
	{ 0*64*32*32, 1*64*32*32, 2*64*32*32 },	/* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,	/* pretty straightforward layout */
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
			40*8+0, 40*8+1, 40*8+2, 40*8+3, 40*8+4, 40*8+5, 40*8+6, 40*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			80*8, 81*8, 82*8, 83*8, 84*8, 85*8, 86*8, 87*8 },
	128*8	/* every sprite takes 128 consecutive bytes */
};



static GFXDECODE_START( pbaction )
	GFXDECODE_ENTRY( REGION_GFX1, 0x00000, charlayout1,    0, 16 )	/*   0-127 characters */
	GFXDECODE_ENTRY( REGION_GFX2, 0x00000, charlayout2,  128,  8 )	/* 128-255 background */
	GFXDECODE_ENTRY( REGION_GFX3, 0x00000, spritelayout1,  0, 16 )	/*   0-127 normal sprites */
	GFXDECODE_ENTRY( REGION_GFX3, 0x01000, spritelayout2,  0, 16 )	/*   0-127 large sprites */
GFXDECODE_END


static INTERRUPT_GEN( pbaction_interrupt )
{
	cpunum_set_input_line_and_vector(machine, 1, 0, HOLD_LINE, 0x02);	/* the CPU is in Interrupt Mode 2 */
}


static MACHINE_DRIVER_START( pbaction )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 4000000)	/* 4 MHz? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	MDRV_CPU_ADD(Z80, 3072000)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_IO_MAP(0,sound_writeport)
	MDRV_CPU_VBLANK_INT(pbaction_interrupt,2)	/* ??? */
									/* IRQs are caused by the main CPU */
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(pbaction)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(pbaction)
	MDRV_VIDEO_UPDATE(pbaction)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pbaction )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "b-p7.bin",     0x0000, 0x4000, CRC(8d6dcaae) SHA1(c9e605f9d291cb8c7163655ea96c605b7d30365f) )
	ROM_LOAD( "b-n7.bin",     0x4000, 0x4000, CRC(d54d5402) SHA1(a4c3205bfe5fba8bb1ff3ad15941a77c35b44a27) )
	ROM_LOAD( "b-l7.bin",     0x8000, 0x2000, CRC(e7412d68) SHA1(e75731d9bea80e0dc09798dd46e3b947fdb54aaa) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for sound board */
	ROM_LOAD( "a-e3.bin",     0x0000,  0x2000, CRC(0e53a91f) SHA1(df2827197cd55c3685e5ac8b26c20800623cb932) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "a-s6.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "a-s7.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "a-s8.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "a-j5.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "a-j6.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "a-j7.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "a-j8.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "b-c7.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "b-d7.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "b-f7.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END


ROM_START( pbactio2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "pba16.bin",     0x0000, 0x4000, CRC(4a239ebd) SHA1(74e6da0485ac78093b4f09953fa3accb14bc3e43) )
	ROM_LOAD( "pba15.bin",     0x4000, 0x4000, CRC(3afef03a) SHA1(dec714415d2fd00c9021171a48f6c94b40888ae8) )
	ROM_LOAD( "pba14.bin",     0x8000, 0x2000, CRC(c0a98c8a) SHA1(442f37af31db13fd98602dd7f9eeae5529da0f44) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for sound board */
	ROM_LOAD( "pba1.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* 64k for a third Z80 (not emulated) */
	ROM_LOAD( "pba17.bin",    0x0000,  0x4000, CRC(2734ae60) SHA1(4edcdfac1611c49c4f890609efbe8352b8161f8e) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "a-s6.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "a-s7.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "a-s8.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "a-j5.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "a-j6.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "a-j7.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "a-j8.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "b-c7.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "b-d7.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "b-f7.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

ROM_START( pbactio3 )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "14.bin",     0x0000, 0x4000, CRC(f17a62eb) SHA1(8dabfc0ad127c154c0293a65df32d52d57dd9755) )
	ROM_LOAD( "12.bin",     0x4000, 0x4000, CRC(ec3c64c6) SHA1(6130b80606d717f95e219316c2d3fa0a1980ea1d) )
	ROM_LOAD( "13.bin",     0x8000, 0x4000, CRC(c93c851e) SHA1(b41077708fce4ccbcecdeae32af8821ca5322e87) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for sound board */
	ROM_LOAD( "pba1.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "a-s6.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "a-s7.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "a-s8.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "a-j5.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "a-j6.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "a-j7.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "a-j8.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "b-c7.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "b-d7.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "b-f7.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END


static READ8_HANDLER( pbactio3_prot_kludge_r )
{
	/* on startup, the game expect this location to NOT act as RAM */
	if (activecpu_get_pc() == 0xab80)
		return 0;

	return work_ram[0];
}

static DRIVER_INIT( pbactio3 )
{
	int i;
	UINT8 *rom = memory_region(REGION_CPU1);

	/* first of all, do a simple bitswap */
	for (i = 0;i < 0xc000;i++)
	{
		rom[i] = BITSWAP8(rom[i], 7,6,5,4,1,2,3,0);
	}

	/* then do the standard Sega decryption */
	pbaction_decode();

	/* install a protection (?) workaround */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xc000, 0, 0, pbactio3_prot_kludge_r );
}


GAME( 1985, pbaction, 0,        pbaction, pbaction, 0,        ROT90, "Tehkan", "Pinball Action (set 1)", 0 )
GAME( 1985, pbactio2, pbaction, pbaction, pbaction, 0,        ROT90, "Tehkan", "Pinball Action (set 2)", 0 )
GAME( 1985, pbactio3, pbaction, pbaction, pbaction, pbactio3, ROT90, "Tehkan", "Pinball Action (set 3, encrypted)", 0 )
