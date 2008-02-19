/***************************************************************************
  Munch Mobile
  (C) 1982 SNK

  2 Z80s
  2 AY-8910s
  15 MHz crystal

  Known Issues:
    - sprite priority problems
    - it's unclear if mirroring the videoram chunks is correct behavior
    - several unmapped registers
    - sustained sounds (when there should be silence)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"


extern UINT8 *mnchmobl_vreg;
extern UINT8 *mnchmobl_status_vram;
extern UINT8 *mnchmobl_sprite_xpos;
extern UINT8 *mnchmobl_sprite_attr;
extern UINT8 *mnchmobl_sprite_tile;

PALETTE_INIT( mnchmobl );
VIDEO_START( mnchmobl );
WRITE8_HANDLER( mnchmobl_palette_bank_w );
WRITE8_HANDLER( mnchmobl_flipscreen_w );
READ8_HANDLER( mnchmobl_sprite_xpos_r );
WRITE8_HANDLER( mnchmobl_sprite_xpos_w );
READ8_HANDLER( mnchmobl_sprite_attr_r );
WRITE8_HANDLER( mnchmobl_sprite_attr_w );
READ8_HANDLER( mnchmobl_sprite_tile_r );
WRITE8_HANDLER( mnchmobl_sprite_tile_w );
VIDEO_UPDATE( mnchmobl );


/***************************************************************************/

static int mnchmobl_nmi_enable = 0;

static WRITE8_HANDLER( mnchmobl_nmi_enable_w )
{
	mnchmobl_nmi_enable = data;
}

static INTERRUPT_GEN( mnchmobl_interrupt )
{
	static int which;
	which = !which;
	if( which ) cpunum_set_input_line(machine, 0, 0, HOLD_LINE);
	else if( mnchmobl_nmi_enable ) cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( mnchmobl_soundlatch_w )
{
	soundlatch_w( offset, data );
	cpunum_set_input_line(Machine, 1, 0, HOLD_LINE );
}

static WRITE8_HANDLER( sound_nmi_ack_w )
{
	cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, CLEAR_LINE);
}

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_READ(MRA8_RAM) /* working RAM */
	AM_RANGE(0xa000, 0xa3ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa400, 0xa7ff) AM_READ(mnchmobl_sprite_xpos_r) /* mirrored */
	AM_RANGE(0xa800, 0xabff) AM_READ(MRA8_RAM)
	AM_RANGE(0xac00, 0xafff) AM_READ(mnchmobl_sprite_tile_r) /* mirrored */
	AM_RANGE(0xb000, 0xb3ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xb400, 0xb7ff) AM_READ(mnchmobl_sprite_attr_r) /* mirrored */
	AM_RANGE(0xb800, 0xb8ff) AM_MIRROR(0x0100) AM_READ(MRA8_RAM)
	AM_RANGE(0xbe02, 0xbe02) AM_READ(input_port_3_r) /* DSW1 */
	AM_RANGE(0xbe03, 0xbe03) AM_READ(input_port_4_r) /* DSW2 */
	AM_RANGE(0xbf01, 0xbf01) AM_READ(input_port_0_r) /* coin, start */
	AM_RANGE(0xbf02, 0xbf02) AM_READ(input_port_1_r) /* P1 controls */
	AM_RANGE(0xbf03, 0xbf03) AM_READ(input_port_2_r) /* P2 controls */
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
 	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(MWA8_RAM) /* working RAM */
	AM_RANGE(0xa000, 0xa3ff) AM_WRITE(MWA8_RAM) AM_BASE(&mnchmobl_sprite_xpos)
	AM_RANGE(0xa400, 0xa7ff) AM_WRITE(mnchmobl_sprite_xpos_w)
	AM_RANGE(0xa800, 0xabff) AM_WRITE(MWA8_RAM) AM_BASE(&mnchmobl_sprite_tile)
	AM_RANGE(0xac00, 0xafff) AM_WRITE(mnchmobl_sprite_tile_w)
	AM_RANGE(0xb000, 0xb3ff) AM_WRITE(MWA8_RAM) AM_BASE(&mnchmobl_sprite_attr)
	AM_RANGE(0xb400, 0xb7ff) AM_WRITE(mnchmobl_sprite_attr_w)
	AM_RANGE(0xb800, 0xb8ff) AM_MIRROR(0x0100) AM_WRITE(MWA8_RAM) AM_BASE(&videoram)
	AM_RANGE(0xba00, 0xbbff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xbc00, 0xbc7f) AM_WRITE(MWA8_RAM) AM_BASE(&mnchmobl_status_vram)
	AM_RANGE(0xbe00, 0xbe00) AM_WRITE(mnchmobl_soundlatch_w)
	AM_RANGE(0xbe01, 0xbe01) AM_WRITE(mnchmobl_palette_bank_w)
	AM_RANGE(0xbe11, 0xbe11) AM_WRITE(MWA8_RAM) /* ? */
	AM_RANGE(0xbe21, 0xbe21) AM_WRITE(MWA8_RAM) /* ? */
	AM_RANGE(0xbe31, 0xbe31) AM_WRITE(MWA8_RAM) /* ? */
	AM_RANGE(0xbe41, 0xbe41) AM_WRITE(mnchmobl_flipscreen_w)
	AM_RANGE(0xbe61, 0xbe61) AM_WRITE(mnchmobl_nmi_enable_w) /* ENI 1-10C */
	AM_RANGE(0xbf00, 0xbf07) AM_WRITE(MWA8_RAM) AM_BASE(&mnchmobl_vreg) /* MY0 1-8C */
ADDRESS_MAP_END

static ADDRESS_MAP_START( readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x2000, 0x2000) AM_READ(soundlatch_r)
	AM_RANGE(0xe000, 0xe7ff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x4000) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x5000, 0x5000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(MWA8_NOP) /* ? */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(MWA8_NOP) /* ? */
	AM_RANGE(0xc000, 0xc000) AM_WRITE(sound_nmi_ack_w)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

static INPUT_PORTS_START( mnchmobl )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) /* service */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START /* P1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START /* P2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* DSW1 0xbe02 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x1e, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(    0x12, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x1a, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "No Bonus" )
	PORT_DIPSETTING(    0x20, "70000" )
	PORT_DIPSETTING(    0x40, "60000" )
	PORT_DIPSETTING(    0x60, "50000" )
	PORT_DIPSETTING(    0x80, "40000" )
	PORT_DIPSETTING(    0xa0, "30000" )
	PORT_DIPSETTING(    0xc0, "20000" )
	PORT_DIPSETTING(    0xe0, "10000" )

	PORT_START	/* DSW2 0xbe03 */
	PORT_DIPNAME( 0x03, 0x00, "Second Bonus Life" )
	PORT_DIPSETTING(    0x00, "No Bonus?" )
	PORT_DIPSETTING(    0x01, "100000?" )
	PORT_DIPSETTING(    0x02, "40000?" )
	PORT_DIPSETTING(    0x03, "30000?" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	8,8,
	256,
	4,
	{ 0, 8, 256*128,256*128+8 },
	{ 7,6,5,4,3,2,1,0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout tile_layout =
{
	8,8,
	0x100,
	4,
	{ 8,12,0,4 },
	{ 0,0,1,1,2,2,3,3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout sprite_layout1 =
{
	32,32,
	128,
	3,
	{ 0x4000*8,0x2000*8,0 },
	{
		7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0,
		0x8000+7,0x8000+7,0x8000+6,0x8000+6,0x8000+5,0x8000+5,0x8000+4,0x8000+4,
		0x8000+3,0x8000+3,0x8000+2,0x8000+2,0x8000+1,0x8000+1,0x8000+0,0x8000+0
	},
	{
		 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		 8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8,
		16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,
		24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8
	},
	256
};

static const gfx_layout sprite_layout2 =
{
	32,32,
	128,
	3,
	{ 0,0,0 },
	{
		7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0,
		0x8000+7,0x8000+7,0x8000+6,0x8000+6,0x8000+5,0x8000+5,0x8000+4,0x8000+4,
		0x8000+3,0x8000+3,0x8000+2,0x8000+2,0x8000+1,0x8000+1,0x8000+0,0x8000+0
	},
	{
		 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		 8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8,
		16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,
		24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8
	},
	256
};

static GFXDECODE_START( munchmo )
	GFXDECODE_ENTRY( REGION_GFX1, 0,      char_layout,      0,  4 )	/* colors   0- 63 */
	GFXDECODE_ENTRY( REGION_GFX2, 0x1000, tile_layout,     64,  4 )	/* colors  64-127 */
	GFXDECODE_ENTRY( REGION_GFX3, 0,      sprite_layout1, 128, 16 )	/* colors 128-255 */
	GFXDECODE_ENTRY( REGION_GFX4, 0,      sprite_layout2, 128, 16 )	/* colors 128-255 */
GFXDECODE_END

static MACHINE_DRIVER_START( munchmo )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 3750000) /* ? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(mnchmobl_interrupt,2)

	MDRV_CPU_ADD(Z80, 3750000) /* ? */
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(readmem_sound,writemem_sound)
	MDRV_CPU_VBLANK_INT(nmi_line_assert,1)

	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256+32+32, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255+32+32,0, 255-16)
	MDRV_GFXDECODE(munchmo)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(mnchmobl)
	MDRV_VIDEO_START(mnchmobl)
	MDRV_VIDEO_UPDATE(mnchmobl)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


ROM_START( joyfulr )
	ROM_REGION( 0x10000, REGION_CPU1, 0 ) /* 64k for CPUA */
	ROM_LOAD( "m1j.10e", 0x0000, 0x2000, CRC(1fe86e25) SHA1(e13abc20741dfd8a260f354efda3b3a25c820343) )
	ROM_LOAD( "m2j.10d", 0x2000, 0x2000, CRC(b144b9a6) SHA1(efed5fd6ba941b2baa7c8a17fe7323172c8fb17c) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "mu.2j",	 0x0000, 0x2000, CRC(420adbd4) SHA1(3da18cda97ca604dc074b50c4f36287e0679224a) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "s1.10a",	 0x0000, 0x1000, CRC(c0bcc301) SHA1(b8961e7bbced4dfe9c72f839ea9b89d3f2e629b2) )	/* characters */
	ROM_LOAD( "s2.10b",	 0x1000, 0x1000, CRC(96aa11ca) SHA1(84438d6b27d520e95b8706c91c5c20de1785604c) )

	ROM_REGION( 0x2000, REGION_GFX2, 0 )
	ROM_LOAD( "b1.2c",	 0x0000, 0x1000, CRC(8ce3a403) SHA1(eec5813076c31bb8534f7d1f83f2a397e552ed69) )	/* tile layout */
	ROM_LOAD( "b2.2b",	 0x1000, 0x1000, CRC(0df28913) SHA1(485700d3b7f2bfcb970e8f9edb7d18ed9a708bd2) )	/* 4x8 tiles */

	ROM_REGION( 0x6000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "f1j.1g",	 0x0000, 0x2000, CRC(93c3c17e) SHA1(902f458c4efe74187a58a3c1ecd146e343657977) )	/* sprites */
	ROM_LOAD( "f2j.3g",	 0x2000, 0x2000, CRC(b3fb5bd2) SHA1(51ff8b0bec092c9404944d6069c4493049604cb8) )
	ROM_LOAD( "f3j.5g",	 0x4000, 0x2000, CRC(772a7527) SHA1(fe561d5323472e79051614a374e92aab17636055) )

	ROM_REGION( 0x2000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "h",		 0x0000, 0x2000, CRC(332584de) SHA1(9ef75a77e6cc298a315d80b7f2d24414827c7063) )	/* monochrome sprites */

	ROM_REGION( 0x0100, REGION_PROMS, 0 )
	ROM_LOAD( "a2001.clr", 0x0000, 0x0100, CRC(1b16b907) SHA1(fc362174af128827b0b8119fdc1b5569598c087a) ) /* color prom */
ROM_END

ROM_START( mnchmobl )
	ROM_REGION( 0x10000, REGION_CPU1, 0 ) /* 64k for CPUA */
	ROM_LOAD( "m1.10e",	 0x0000, 0x2000, CRC(a4bebc6a) SHA1(7c13b2b87168dee3c1b8e931487a56d0a528386e) )
	ROM_LOAD( "m2.10d",	 0x2000, 0x2000, CRC(f502d466) SHA1(4da5a32b3903fb7fbef38fc385408b9390b5f57f) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "mu.2j",	 0x0000, 0x2000, CRC(420adbd4) SHA1(3da18cda97ca604dc074b50c4f36287e0679224a) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "s1.10a",	 0x0000, 0x1000, CRC(c0bcc301) SHA1(b8961e7bbced4dfe9c72f839ea9b89d3f2e629b2) )	/* characters */
	ROM_LOAD( "s2.10b",	 0x1000, 0x1000, CRC(96aa11ca) SHA1(84438d6b27d520e95b8706c91c5c20de1785604c) )

	ROM_REGION( 0x2000, REGION_GFX2, 0 )
	ROM_LOAD( "b1.2c",	 0x0000, 0x1000, CRC(8ce3a403) SHA1(eec5813076c31bb8534f7d1f83f2a397e552ed69) )	/* tile layout */
	ROM_LOAD( "b2.2b",	 0x1000, 0x1000, CRC(0df28913) SHA1(485700d3b7f2bfcb970e8f9edb7d18ed9a708bd2) )	/* 4x8 tiles */

	ROM_REGION( 0x6000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "f1.1g",	 0x0000, 0x2000, CRC(b75411d4) SHA1(d058a6c219676f8ba4e498215f5716c630bb1d20) )	/* sprites */
	ROM_LOAD( "f2.3g",	 0x2000, 0x2000, CRC(539a43ba) SHA1(a7b30c41d9fdb420ec8f0c6441432c1b2b69c4be) )
	ROM_LOAD( "f3.5g",	 0x4000, 0x2000, CRC(ec996706) SHA1(e71e99061ce83068b0ec60ae97759a9d78c7cdf9) )

	ROM_REGION( 0x2000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "h",		 0x0000, 0x2000, CRC(332584de) SHA1(9ef75a77e6cc298a315d80b7f2d24414827c7063) )	/* monochrome sprites */

	ROM_REGION( 0x0100, REGION_PROMS, 0 )
	ROM_LOAD( "a2001.clr", 0x0000, 0x0100, CRC(1b16b907) SHA1(fc362174af128827b0b8119fdc1b5569598c087a) ) /* color prom */
ROM_END


GAME( 1983, joyfulr,  0,       munchmo, mnchmobl, 0, ROT270, "SNK", "Joyful Road (Japan)", GAME_IMPERFECT_SOUND | GAME_NO_COCKTAIL )
GAME( 1983, mnchmobl, joyfulr, munchmo, mnchmobl, 0, ROT270, "SNK (Centuri license)", "Munch Mobile (US)", GAME_IMPERFECT_SOUND | GAME_NO_COCKTAIL )
