/***************************************************************************

Mysterious Stones

driver by Nicola Salmoria

Notes:
- The subtitle of the two sets is slightly different:
  "dr john s adventure" vs. "dr kick in adventure".
  The Dr John's is a bug fix. See the routine at 4376/4384 for example. The
  old set thrashes the Y register, the new one saves in on the stack. The
  newer set also resets the sound chips more often.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/ay8910.h"


extern UINT8 *mystston_videoram2;

extern WRITE8_HANDLER( mystston_videoram_w );
extern WRITE8_HANDLER( mystston_videoram2_w );
extern WRITE8_HANDLER( mystston_scroll_w );
extern WRITE8_HANDLER( mystston_control_w );

extern PALETTE_INIT( mystston );
extern VIDEO_START( mystston );
extern VIDEO_UPDATE( mystston );


static int VBLK = 0x80;
static int soundlatch;


static WRITE8_HANDLER( mystston_soundlatch_w )
{
	soundlatch = data;
}

static WRITE8_HANDLER( mystston_soundcontrol_w )
{
	static int last;

	/* bit 5 goes to 8910 #0 BDIR pin  */
	if ((last & 0x20) == 0x20 && (data & 0x20) == 0x00)
	{
		/* bit 4 goes to the 8910 #0 BC1 pin */
		if (last & 0x10)
			AY8910_control_port_0_w(machine,0,soundlatch);
		else
			AY8910_write_port_0_w(machine,0,soundlatch);
	}
	/* bit 7 goes to 8910 #1 BDIR pin  */
	if ((last & 0x80) == 0x80 && (data & 0x80) == 0x00)
	{
		/* bit 6 goes to the 8910 #1 BC1 pin */
		if (last & 0x40)
			AY8910_control_port_1_w(machine,0,soundlatch);
		else
			AY8910_write_port_1_w(machine,0,soundlatch);
	}

	last = data;
}

static READ8_HANDLER( port3_r )
{
	int port = readinputport(3);

	return port | VBLK;
}

static WRITE8_HANDLER( mystston_irq_reset_w )
{
	cpunum_set_input_line(Machine, 0, 0, CLEAR_LINE);
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x077f) AM_READ(MRA8_RAM)
	AM_RANGE(0x0800, 0x0fff) AM_READ(MRA8_RAM)	/* work RAM? */
	AM_RANGE(0x1000, 0x1fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x2000, 0x2000) AM_READ(input_port_0_r)
	AM_RANGE(0x2010, 0x2010) AM_READ(input_port_1_r)
	AM_RANGE(0x2020, 0x2020) AM_READ(input_port_2_r)
	AM_RANGE(0x2030, 0x2030) AM_READ(port3_r)
	AM_RANGE(0x4000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x077f) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x0780, 0x07df) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x07e0, 0x07ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x0800, 0x0fff) AM_WRITE(MWA8_RAM)	/* work RAM? */
	AM_RANGE(0x1000, 0x17ff) AM_WRITE(mystston_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x1800, 0x1bff) AM_WRITE(mystston_videoram2_w) AM_BASE(&mystston_videoram2)
	AM_RANGE(0x1c00, 0x1fff) AM_WRITE(MWA8_RAM)	/* work RAM? This gets copied to videoram */
	AM_RANGE(0x2000, 0x2000) AM_WRITE(mystston_control_w)	/* text color, flip screen & coin counters */
	AM_RANGE(0x2010, 0x2010) AM_WRITE(mystston_irq_reset_w)
	AM_RANGE(0x2020, 0x2020) AM_WRITE(mystston_scroll_w)
	AM_RANGE(0x2030, 0x2030) AM_WRITE(mystston_soundlatch_w)
	AM_RANGE(0x2040, 0x2040) AM_WRITE(mystston_soundcontrol_w)
	AM_RANGE(0x2060, 0x2077) AM_WRITE(paletteram_BBGGGRRR_w) AM_BASE(&paletteram)
	AM_RANGE(0x4000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END


static INPUT_PORTS_START( mystston )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START	/* DSW1 */
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(   0x01, "3" )
	PORT_DIPSETTING(   0x00, "5" )
	PORT_DIPNAME(0x02, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(   0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Hard ) )
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(   0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(   0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(   0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(   0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(   0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )

	PORT_START	/* DSW2 */
	PORT_DIPNAME(0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME(0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(   0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x20, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// VBLANK
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( mystston )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,   3*8, 4 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout, 2*8, 1 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout, 0*8, 2 )
GFXDECODE_END


static INTERRUPT_GEN( mystston_interrupt )
{
	int scanline = 271 - cpu_getiloops();
	static int coin;

	/* Inserting a coin triggers an NMI */
	if ((readinputport(0) & 0xc0) != 0xc0)
	{
		if (coin == 0)
		{
			coin = 1;
			nmi_line_pulse(machine, cpunum);
			return;
		}
	}
	else coin = 0;

	/* VBLK is lowered on scanline 8 */
	if (scanline == 8)
		VBLK = 0;

	/* VBLK is raised on scanline 248 */
	if (scanline == 248)
		VBLK = 0x80;

	/* IMS is triggered every time VLOC line 3 is raised,
       as VLOC counter starts at 16, effectively every 16 scanlines */
	if ((scanline % 16) == 0)
		cpunum_set_input_line(machine, 0, 0, ASSERT_LINE);
}


static MACHINE_DRIVER_START( mystston )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6502, 12000000/8)	// 1.5 MHz
	MDRV_CPU_PROGRAM_MAP(readmem, writemem)
	MDRV_CPU_VBLANK_INT_HACK(mystston_interrupt, 272)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(((12000000.0 / 256.0) / 3.0) / 272.0)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(mystston)
	MDRV_PALETTE_LENGTH(24+32)

	MDRV_PALETTE_INIT(mystston)
	MDRV_VIDEO_START(mystston)
	MDRV_VIDEO_UPDATE(mystston)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END


ROM_START( mystston )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "rom6.bin",     0x4000, 0x2000, CRC(7bd9c6cd) SHA1(4d14edc783ba1a6c01d2fb9ea29ec85b8fec3c3b) )
	ROM_LOAD( "rom5.bin",     0x6000, 0x2000, CRC(a83f04a6) SHA1(d8cdf310511c1fef4fbde80ef2161fda00f965d7) )
	ROM_LOAD( "rom4.bin",     0x8000, 0x2000, CRC(46c73714) SHA1(5b9ac3a35aeeea6a0cd2d838c144925d83b36a7f) )
	ROM_LOAD( "rom3.bin",     0xa000, 0x2000, CRC(34f8b8a3) SHA1(a270f6665a9f76f97ac02201d51fe2817e6e8f22) )
	ROM_LOAD( "rom2.bin",     0xc000, 0x2000, CRC(bfd22cfc) SHA1(137cd61c8b1e997e7e50edd57f1671031d8e3ac5) )
	ROM_LOAD( "rom1.bin",     0xe000, 0x2000, CRC(fb163e38) SHA1(d6f02e90bfd9badd7751bc0a87fdfdd1d0a7e202) )

	ROM_REGION( 0x0c000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ms6",          0x00000, 0x2000, CRC(85c83806) SHA1(cdfed6c224754e8f79b154533b06b7de4a44b4d3) )
	ROM_LOAD( "ms9",          0x02000, 0x2000, CRC(b146c6ab) SHA1(712c0c17780f222be5c8b09185a22e900ab23944) )
	ROM_LOAD( "ms7",          0x04000, 0x2000, CRC(d025f84d) SHA1(eaaaa0bde3db850098d04a0af85993026e503fc5) )
	ROM_LOAD( "ms10",         0x06000, 0x2000, CRC(d85015b5) SHA1(f4afab248dfde354650e59fadd5ab9616b04dac1) )
	ROM_LOAD( "ms8",          0x08000, 0x2000, CRC(53765d89) SHA1(c8bfc311123b076dccae9f7e3b95460bf9fc843d) )
	ROM_LOAD( "ms11",         0x0a000, 0x2000, CRC(919ee527) SHA1(609ee854ab3a4fdbf3404a68a4a657b85250f742) )

	ROM_REGION( 0x0c000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ms12",         0x00000, 0x2000, CRC(72d8331d) SHA1(f0a3bc6c9d9966f169f4721c2453f7ee210f0feb) )
	ROM_LOAD( "ms13",         0x02000, 0x2000, CRC(845a1f9b) SHA1(aa2eabd2a5e89e150b5d2fb3d88f91902e5ebb48) )
	ROM_LOAD( "ms14",         0x04000, 0x2000, CRC(822874b0) SHA1(9376d48045bf67df91d103effd1d08bd8debad26) )
	ROM_LOAD( "ms15",         0x06000, 0x2000, CRC(4594e53c) SHA1(a011a5269a9b0ca7a964181efe8413d5637c34f4) )
	ROM_LOAD( "ms16",         0x08000, 0x2000, CRC(2f470b0f) SHA1(79b50a7d113fed4669361c5f6c60ec96c94344c6) )
	ROM_LOAD( "ms17",         0x0a000, 0x2000, CRC(38966d1b) SHA1(89e3e54d3298cefeb35922d2292e3e7b8e995871) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "ic61",         0x0000, 0x0020, CRC(e802d6cf) SHA1(233ceb9e3a91939e1925766a696bc65ab0dffa50) )
ROM_END

ROM_START( myststno )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "ms0",          0x4000, 0x2000, CRC(6dacc05f) SHA1(43054199901639516205c7ea145462d0abea8fb1) )
	ROM_LOAD( "ms1",          0x6000, 0x2000, CRC(a3546df7) SHA1(89c0349885a9369406a1121cd3db28963b25f2e6) )
	ROM_LOAD( "ms2",          0x8000, 0x2000, CRC(43bc6182) SHA1(dc36c10eee20009922e89d9bfdf6c2f6ffb881ce) )
	ROM_LOAD( "ms3",          0xa000, 0x2000, CRC(9322222b) SHA1(25192ac9e8e66cd2bc21c66c690c57c6b9836f2d) )
	ROM_LOAD( "ms4",          0xc000, 0x2000, CRC(47cefe9b) SHA1(49422b664b1322373a9cd3cb2907f8f5492faf87) )
	ROM_LOAD( "ms5",          0xe000, 0x2000, CRC(b37ae12b) SHA1(55ee1193088145c85adddd377d9e5ee58aca922f) )

	ROM_REGION( 0x0c000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ms6",          0x00000, 0x2000, CRC(85c83806) SHA1(cdfed6c224754e8f79b154533b06b7de4a44b4d3) )
	ROM_LOAD( "ms9",          0x02000, 0x2000, CRC(b146c6ab) SHA1(712c0c17780f222be5c8b09185a22e900ab23944) )
	ROM_LOAD( "ms7",          0x04000, 0x2000, CRC(d025f84d) SHA1(eaaaa0bde3db850098d04a0af85993026e503fc5) )
	ROM_LOAD( "ms10",         0x06000, 0x2000, CRC(d85015b5) SHA1(f4afab248dfde354650e59fadd5ab9616b04dac1) )
	ROM_LOAD( "ms8",          0x08000, 0x2000, CRC(53765d89) SHA1(c8bfc311123b076dccae9f7e3b95460bf9fc843d) )
	ROM_LOAD( "ms11",         0x0a000, 0x2000, CRC(919ee527) SHA1(609ee854ab3a4fdbf3404a68a4a657b85250f742) )

	ROM_REGION( 0x0c000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ms12",         0x00000, 0x2000, CRC(72d8331d) SHA1(f0a3bc6c9d9966f169f4721c2453f7ee210f0feb) )
	ROM_LOAD( "ms13",         0x02000, 0x2000, CRC(845a1f9b) SHA1(aa2eabd2a5e89e150b5d2fb3d88f91902e5ebb48) )
	ROM_LOAD( "ms14",         0x04000, 0x2000, CRC(822874b0) SHA1(9376d48045bf67df91d103effd1d08bd8debad26) )
	ROM_LOAD( "ms15",         0x06000, 0x2000, CRC(4594e53c) SHA1(a011a5269a9b0ca7a964181efe8413d5637c34f4) )
	ROM_LOAD( "ms16",         0x08000, 0x2000, CRC(2f470b0f) SHA1(79b50a7d113fed4669361c5f6c60ec96c94344c6) )
	ROM_LOAD( "ms17",         0x0a000, 0x2000, CRC(38966d1b) SHA1(89e3e54d3298cefeb35922d2292e3e7b8e995871) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "ic61",         0x0000, 0x0020, CRC(e802d6cf) SHA1(233ceb9e3a91939e1925766a696bc65ab0dffa50) )
ROM_END


GAME( 1984, mystston, 0,        mystston, mystston, 0, ROT270, "Technos", "Mysterious Stones - Dr. John's Adventure", 0 )
GAME( 1984, myststno, mystston, mystston, mystston, 0, ROT270, "Technos", "Mysterious Stones - Dr. Kick in Adventure", 0 )
