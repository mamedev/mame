/*
Jumping Cross - (c) 1984 SNK
driver by Tomasz Slanina

Based on marvin's maze driver

Todo:
- problems with sprites - strange movement in attract mode,
- $c800-$d7ff - unknown read (related with ^^^ )
- verify dipswitches
- verify colors (is palette banking correct ?)
- unused tileset (almost identical to txt layer tiles ,  few (3?) chars are different)
- cocktail mdoe/screen flipping

Could be bad dump ('final' romset is made of two sets marked as 'bad' )

*/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "snk.h"
#include "sound/ay8910.h"
#include "sound/namco.h"

UINT8 *jcr_textram;

WRITE8_HANDLER( snkwave_w );

READ8_HANDLER( jcross_background_ram_r );
WRITE8_HANDLER( jcross_background_ram_w );

READ8_HANDLER( jcross_text_ram_r );
WRITE8_HANDLER( jcross_text_ram_w );

extern VIDEO_START( jcross );
extern VIDEO_UPDATE( jcross );
extern int jcross_vregs[5];
WRITE8_HANDLER( jcross_palettebank_w );

static int sound_cpu_busy=0;

#ifdef UNUSED_FUNCTION
UINT8 *jcr_sharedram;
static READ8_HANDLER(sharedram_r){	return jcr_sharedram[offset];}
static WRITE8_HANDLER(sharedram_w){	jcr_sharedram[offset]=data;}
#endif

static const struct namco_interface snkwave_interface =
{
	1,
	-1
};

static WRITE8_HANDLER( sound_command_w )
{
	sound_cpu_busy = 0x20;
	soundlatch_w(machine, 0, data);
	cpunum_set_input_line(machine, 2, INPUT_LINE_NMI, PULSE_LINE);
}

static READ8_HANDLER( sound_command_r )
{
	sound_cpu_busy = 0;
	return(soundlatch_r(machine,0));
}

static READ8_HANDLER( sound_nmi_ack_r )
{
	cpunum_set_input_line(Machine, 2, INPUT_LINE_NMI, CLEAR_LINE);
	return 0;
}

static READ8_HANDLER( jcross_port_0_r )
{
	return(input_port_0_r(machine,0) | sound_cpu_busy);
}

static WRITE8_HANDLER(jcross_vregs0_w){jcross_vregs[0]=data;}
static WRITE8_HANDLER(jcross_vregs1_w){jcross_vregs[1]=data;}
static WRITE8_HANDLER(jcross_vregs2_w){jcross_vregs[2]=data;}
static WRITE8_HANDLER(jcross_vregs3_w){jcross_vregs[3]=data;}
static WRITE8_HANDLER(jcross_vregs4_w){jcross_vregs[4]=data;}


static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_BASE(&namco_wavedata)
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(sound_command_r)
	AM_RANGE(0xc000, 0xc000) AM_READ(sound_nmi_ack_r)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0xe002, 0xe007) AM_WRITE(snkwave_w)
	AM_RANGE(0xe008, 0xe008) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0xe009, 0xe009) AM_WRITE(AY8910_write_port_1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_READNOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpuA_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_READ(jcross_port_0_r)
	AM_RANGE(0xa100, 0xa100) AM_READ(input_port_1_r)
	AM_RANGE(0xa200, 0xa200) AM_READ(input_port_2_r)
	AM_RANGE(0xa300, 0xa300) AM_WRITE(sound_command_w)
	AM_RANGE(0xa400, 0xa400) AM_READ(input_port_3_r)
	AM_RANGE(0xa500, 0xa500) AM_READ(input_port_4_r)
	AM_RANGE(0xa600, 0xa600) AM_WRITE(jcross_palettebank_w)
	AM_RANGE(0xa700, 0xa700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xd300, 0xd300) AM_WRITE(jcross_vregs0_w)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(jcross_vregs1_w)
	AM_RANGE(0xd500, 0xd500) AM_WRITE(jcross_vregs2_w)
	AM_RANGE(0xd600, 0xd600) AM_WRITE(jcross_vregs3_w)
	AM_RANGE(0xd700, 0xd700) AM_WRITE(jcross_vregs4_w)
 	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE(1) AM_BASE(&spriteram)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(MRA8_RAM, jcross_background_ram_w) AM_SHARE(2) AM_BASE(&videoram)
	AM_RANGE(0xf000, 0xf3ff) AM_READWRITE(MRA8_RAM, jcross_text_ram_w) AM_BASE(&jcr_textram)
	AM_RANGE(0xf400, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpuB_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa700, 0xa700) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
  	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xc800, 0xd7ff) AM_RAM AM_SHARE(2) /* unknown ??? */
ADDRESS_MAP_END


static INPUT_PORTS_START( jcross )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* sound CPU status */


	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )


	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Unknown SW 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )
	/* PORT_DIPSETTING(    0x10,  )  ???? 'insert more coin'*/
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0xc0, "20000 60000" )
	PORT_DIPSETTING(    0x80, "40000 90000" )
	PORT_DIPSETTING(    0x40, "50000 120000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Bonus Life Occurence" ) /* not verified */
	PORT_DIPSETTING(    0x01, "1st, 2nd, then every 2nd" )
	PORT_DIPSETTING(    0x00, "1st and 2nd only" )
	PORT_DIPNAME( 0x06, 0x04, "scrolling speed" )
	PORT_DIPSETTING(    0x06, "Slow" )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x18, 0x10, "Game mode" )
	PORT_DIPSETTING(    0x18, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x10, "Demo Sounds On" )
	PORT_DIPSETTING(    0x08, "Infinite Lives (Cheat)" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "BG Collisions" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END



/***************************************************************************
**
**  Graphics Layout
**
***************************************************************************/

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ 0,0x2000*8,0x4000*8 },
	{
		7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8
	},
	{
		0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16
	},
	256
};

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	256
};

static GFXDECODE_START( jcross )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tile_layout,	0x080, 8  )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tile_layout,	0x110, 1  )
	GFXDECODE_ENTRY( REGION_GFX3, 0, tile_layout,	0x100, 1  )
	GFXDECODE_ENTRY( REGION_GFX4, 0, sprite_layout,	0x000, 16 ) /* sprites */
GFXDECODE_END


/***************************************************************************
**
**  Machine Driver
**
***************************************************************************/

static MACHINE_DRIVER_START( jcross )

	MDRV_CPU_ADD(Z80, 3360000)
	MDRV_CPU_PROGRAM_MAP(cpuA_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80, 3360000)
	MDRV_CPU_PROGRAM_MAP(cpuB_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80, 4000000)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_IO_MAP(sound_portmap,0)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, 244)

	MDRV_INTERLEAVE(100)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(61)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 255,0, 223)

	MDRV_GFXDECODE(jcross)
	MDRV_PALETTE_LENGTH((16+2)*16)
	MDRV_VIDEO_START(jcross)
	MDRV_VIDEO_UPDATE(jcross)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MDRV_SOUND_ADD(NAMCO, 24000)
	MDRV_SOUND_CONFIG(snkwave_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.08)
MACHINE_DRIVER_END


ROM_START( jcross )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "jcrossa0.10b",  0x0000, 0x2000, CRC(0e79bbcd) SHA1(7088a8effd30080529b797991e24e9807bf90475) )
	ROM_LOAD( "jcrossa1.12b",  0x2000, 0x2000, CRC(999b2bcc) SHA1(e5d13c9c11a82cedee15777341e6424639ecf2f5) )
	ROM_LOAD( "jcrossa2.13b",  0x4000, 0x2000, CRC(ac89e49c) SHA1(9b9a0eec8ad341ce7af58bffe55f10bec696af62) )
	ROM_LOAD( "jcrossa3.14b",  0x6000, 0x2000, CRC(4fd7848d) SHA1(870aea0b8e027616814df87afd24418fd140f736) )
	ROM_LOAD( "jcrossa4.15b",  0x8000, 0x2000, CRC(8500575d) SHA1(b8751b86508de484f2eb8a6702c63a47ec882036) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "jcrossb0.15a",  0x0000, 0x2000, CRC(77ed51e7) SHA1(56b457846f71f442da6f99889231d4b71d5fcb6c) )
	ROM_LOAD( "jcrossb1.14a",  0x2000, 0x2000, CRC(23cf0f70) SHA1(f258e899f332a026eeb0db92330fd60c478218af) )
	ROM_LOAD( "jcrossb2.13a",  0x4000, 0x2000, CRC(5bed3118) SHA1(f105ca55223a4bfbc8e2d61c365c76cf2153254c) )
	ROM_LOAD( "jcrossb3.12a",  0x6000, 0x2000, CRC(cd75dc95) SHA1(ef03d2b0f66f30fad5132e7b6aee9ec978650b53) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )
	ROM_LOAD( "jcrosss0.f1",   0x0000, 0x2000, CRC(9ae8ea93) SHA1(1d824302305a41bf5c354c36e2e11981d1aa5ea4) )
	ROM_LOAD( "jcrosss1.h2",   0x2000, 0x2000, CRC(83785601) SHA1(cd3d484ef5464090c4b543b1edbbedcc52b15071) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "jcrosss.d2",    0x0000, 0x2000, CRC(3ebb5beb) SHA1(de0a1f0fdb5b08b76dab9fa64d9ae3047c4ff84b) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "jcrossb1.a2",   0x0000, 0x2000, CRC(ea3dfbc9) SHA1(eee56acd1c9dbc6c3ecdee4ffe860273e65cc09b) )

	ROM_REGION( 0x2000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "jcrossb4.10a",  0x0000, 0x2000, CRC(08ad93fe) SHA1(04baf2d9735b0d794b114abeced5a6b899958ce7) )

	ROM_REGION( 0x6000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "jcrossf2.j2",    0x0000, 0x2000, CRC(42a12b9d) SHA1(9f2bdb1f84f444442282cf0fc1f7b3c7f9a9bf48) )
	ROM_LOAD( "jcrossf1.k2",    0x2000, 0x2000, CRC(70d219bf) SHA1(9ff9f88221edd141e8204ac810434b4290db7cff) )
	ROM_LOAD( "jcrossf0.l2",  0x4000, 0x2000, CRC(4532509b) SHA1(c99f87e2b06b94d815e6099bccb2aee0edf8c98d) )

	ROM_REGION( 0x0c00, REGION_PROMS, 0 )
	ROM_LOAD( "jcrossp2.j7",  0x000, 0x400, CRC(b72a96a5) SHA1(20d40e4b6a2652e61dc3ad0c4afaec04e3c7cf74) )
	ROM_LOAD( "jcrossp1.j8",  0x400, 0x400, CRC(35650448) SHA1(17e4a661ff304c093bb0253efceaf4e9b2498924) )
	ROM_LOAD( "jcrossp0.j9",  0x800, 0x400, CRC(99f54d48) SHA1(9bd20eaa9706d28eaca9f5e195204d89e302272f) )
ROM_END

GAME( 1984, jcross, 0, jcross, jcross, 0, ROT270,   "SNK", "Jumping Cross",GAME_NO_COCKTAIL|GAME_IMPERFECT_GRAPHICS)


