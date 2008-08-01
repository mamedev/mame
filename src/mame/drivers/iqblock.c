/***************************************************************************

IQ Block   (c) 1992 IGS

Driver by Nicola Salmoria and Ernesto Corvi

TODO:
- Who generates IRQ and NMI? How many should there be per frame?

- Sound chip is a UM3567. Is this compatible to something already in MAME? yes, YM2413

- Coin 2 doesn't work? DIP switch setting?

- Protection:
  I can see it reading things like the R register here and there, so it might
  be cycle-dependant or something.

  'Crash 1' checks I was able to see:
  PC = $52FA
  PC = $507F

  'Crash 2' checks I was able to see:
  PC = $54E6

Stephh's notes :

  - Coin 2 as well as buttons 2 to 4 for each player are only read in "test mode".
    Same issue for Dip Siwtches 0-7 and 1-2 to 1-6.
    Some other games on the same hardware might use them.
  - Dip Switch 0 is stored at 0xf0ac and Dip Switch 1 is stored at 0xf0ad.
    However they are both read back at the same time with "ld   hl,($F0AC)" instructions.
  - Dip Switches 0-0 and 0-1 are read via code at 0x9470.
    This routine is called when you made a "line" after the routine that checks the score
    for awarding extra help and/or changing background.
    Data is coming from 4 possible tables (depending on them) which seem to be 0x84 bytes wide.
    Table 0 offset is 0xeaf7.
    IMO, this has something to do with difficulty but there is no confirmation about that !
  - Dip Switch 1-0 is read only once after the P.O.S.T. via code at 0xa200.
    It changes (or not) the contents of 0xf0db.w which can get these 2 possible values
    at start : 0x47a3 (when OFF) or 0x428e (when ON) which seem to be tables.
    If you set a WP to 0xf0db, you'll notice that it's called more often in the "demo mode"
    when the Dip Switch is ON, so, as it implies writes to outport 0x50b0, I think it has
    something to do with "Demo Sounds".
    I can't tell however if setting the Dip Switch to OFF means "Demo Sounds" OFF or ON !

Grndtour:
 - Title should flash 3X slowly. In MAME it flashes too fast, or strangely??


***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "machine/8255ppi.h"
#include "iqblock.h"
#include "sound/2413intf.h"

static UINT8 *rambase;

static WRITE8_HANDLER( iqblock_prot_w )
{
    rambase[0xe26] = data;
    rambase[0xe27] = data;
    rambase[0xe1c] = data;
}

static WRITE8_HANDLER( grndtour_prot_w )
{
	rambase[0xe39] = data;
    rambase[0xe3a] = data;
    rambase[0xe2f] = data;

}


static INTERRUPT_GEN( iqblock_interrupt )
{
	if (cpu_getiloops() & 1)
		cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);	/* ???? */
	else
		cpunum_set_input_line(machine, 0, 0, ASSERT_LINE);			/* ???? */
}

static WRITE8_HANDLER( iqblock_irqack_w )
{
	cpunum_set_input_line(machine, 0, 0, CLEAR_LINE);
}

static READ8_HANDLER( extrarom_r )
{
	return memory_region(machine, "user1")[offset];
}


static WRITE8_HANDLER( port_C_w )
{
	/* bit 4 unknown; it is pulsed at the end of every NMI */

	/* bit 5 seems to be 0 during screen redraw */
	iqblock_videoenable = data & 0x20;

	/* bit 6 is coin counter */
	coin_counter_w(0,data & 0x40);

	/* bit 7 could be a second coin counter, but coin 2 doesn't seem to work... */
}

static const ppi8255_interface ppi8255_intf =
{
	input_port_0_r,			/* Port A read */
	input_port_1_r,			/* Port B read */
	input_port_2_r,			/* Port C read */
	NULL,					/* Port A write */
	NULL,					/* Port B write */
	port_C_w				/* Port C write */
};


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_BASE(&rambase)
ADDRESS_MAP_END


static ADDRESS_MAP_START( main_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x2000, 0x23ff) AM_WRITE(paletteram_xBBBBBGGGGGRRRRR_split1_w)
	AM_RANGE(0x2800, 0x2bff) AM_WRITE(paletteram_xBBBBBGGGGGRRRRR_split2_w)
	AM_RANGE(0x6000, 0x603f) AM_WRITE(iqblock_fgscroll_w)
	AM_RANGE(0x6800, 0x69ff) AM_WRITE(iqblock_fgvideoram_w)	/* initialized up to 6fff... bug or larger tilemap? */
	AM_RANGE(0x7000, 0x7fff) AM_WRITE(iqblock_bgvideoram_w)
	AM_RANGE(0x5080, 0x5083) AM_DEVWRITE(PPI8255, "ppi8255", ppi8255_w)
	AM_RANGE(0x5080, 0x5083) AM_DEVREAD(PPI8255, "ppi8255", ppi8255_r)
	AM_RANGE(0x5090, 0x5090) AM_READ(input_port_3_r)
	AM_RANGE(0x50a0, 0x50a0) AM_READ(input_port_4_r)
	AM_RANGE(0x50b0, 0x50b0) AM_WRITE(YM2413_register_port_0_w) // UM3567_register_port_0_w
	AM_RANGE(0x50b1, 0x50b1) AM_WRITE(YM2413_data_port_0_w) // UM3567_data_port_0_w
	AM_RANGE(0x50c0, 0x50c0) AM_WRITE(iqblock_irqack_w)
	AM_RANGE(0x7000, 0x7fff) AM_READ(iqblock_bgvideoram_r)
	AM_RANGE(0x8000, 0xffff) AM_READ(extrarom_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( iqblock )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )				// "test mode" only

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )					// "test mode" only
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL	// "test mode" only

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )				// "test mode" only
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 )				// "test mode" only
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL	// "test mode" only
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL	// "test mode" only
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_DIPNAME( 0x03, 0x03, "Unknown SW 0-0&1" )	// Difficulty ? Read notes above
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x0c, "Helps" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START
	PORT_DIPNAME( 0x01, 0x00, "Demo Sounds?" )	// To be confirmed ! Read notes above
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END



static const gfx_layout tilelayout1 =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ 8, 0, RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0, RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout tilelayout2 =
{
	8,32,
	RGN_FRAC(1,2),
	4,
	{ 8, 0, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0 },
	{	0, 1, 2, 3, 4, 5, 6, 7 },
	{	0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16 },
	32*16
};

static const gfx_layout tilelayout3 =
{
	8,32,
	RGN_FRAC(1,3),
	6,
	{ 8, 0, RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0, RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0 },
	{	0, 1, 2, 3, 4, 5, 6, 7 },
	{	0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16 },
	32*16
};

static GFXDECODE_START( iqblock )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout1, 0, 16 )	/* only odd color codes are used */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout2, 0,  4 )	/* only color codes 0 and 3 used */
GFXDECODE_END

static GFXDECODE_START( cabaret )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout1, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout3, 0, 16 )
GFXDECODE_END


static MACHINE_DRIVER_START( iqblock )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80,12000000/2)	/* 6 MHz */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_IO_MAP(main_portmap,0)
	MDRV_CPU_VBLANK_INT_HACK(iqblock_interrupt,16)

	MDRV_DEVICE_ADD( "ppi8255", PPI8255 )
	MDRV_DEVICE_CONFIG( ppi8255_intf )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(iqblock)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(iqblock)
	MDRV_VIDEO_UPDATE(iqblock)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cabaret )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z180,12000000/2)	/* 6 MHz , appears to use Z180 instructions */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_IO_MAP(main_portmap,0)
	MDRV_CPU_VBLANK_INT_HACK(iqblock_interrupt,16)

	MDRV_DEVICE_ADD( "ppi8255", PPI8255 )
	MDRV_DEVICE_CONFIG( ppi8255_intf )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(cabaret)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(iqblock)
	MDRV_VIDEO_UPDATE(iqblock)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( iqblock )
	ROM_REGION( 0x20000, "main", 0 )	/* 64k for code + 64K for extra RAM */
	ROM_LOAD( "u7.v5",        0x0000, 0x10000, CRC(811f306e) SHA1(d0aef80f1624002d05721276358f26a3ef69a3f6) )

	ROM_REGION( 0x8000, "user1", 0 )
	ROM_LOAD( "u8.6",         0x0000, 0x8000, CRC(2651bc27) SHA1(53e1d6ffd78c8a612863b29b0f8734e740d563c7) )	/* background maps, read by the CPU */

	ROM_REGION( 0x60000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "u28.1",        0x00000, 0x20000, CRC(ec4b64b4) SHA1(000e9df0c0b5fcde5ead218dfcdc156bc4be909d) )
	ROM_LOAD( "u27.2",        0x20000, 0x20000, CRC(74aa3de3) SHA1(16757c24765d22026793a0c53d3f24c106951a18) )
	ROM_LOAD( "u26.3",        0x40000, 0x20000, CRC(2896331b) SHA1(51eba9f9f653a11cb96c461ab495d943d34cedc6) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "u25.4",        0x0000, 0x4000, CRC(8fc222af) SHA1(ac1fb5e6caec391a76e3af51e133aecc65cd5aed) )
	ROM_LOAD( "u24.5",        0x4000, 0x4000, CRC(61050e1e) SHA1(1f7185b2a5a2e237120276c95344744b146b4bf6) )
ROM_END

ROM_START( grndtour )
	ROM_REGION( 0x20000, "main", 0 )	/* 64k for code + 64K for extra RAM */
	ROM_LOAD( "grand7.u7",        0x0000, 0x10000, CRC(95cac31e) SHA1(47bbcce6981ea3d38e0aa49ccd3762a4529f3c96) )

	ROM_REGION( 0x8000, "user1", 0 )
	ROM_LOAD( "grand6.u8",         0x0000, 0x8000, CRC(4c634b86) SHA1(c36df147187bc526f2348bc2f4d4c4e35bb45f38) )	/* background maps, read by the CPU */

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "grand1.u28",        0x00000, 0x40000, CRC(de85c664) SHA1(3a4b0cac88a0fea1c80541fe49c799e3550bedee) )
	ROM_LOAD( "grand2.u27",        0x40000, 0x40000, CRC(8456204e) SHA1(b604d501f360670f57b937ad96af64c1c2038ef7) )
	ROM_LOAD( "grand3.u26",        0x80000, 0x40000, CRC(77632917) SHA1(d91eadec2e0fb3082299362d18814b8ec4c5e068) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "grand4.u25",        0x0000, 0x4000, CRC(48d09746) SHA1(64669f572b9a98b078ee1ea0b614c117e5dfbec9) )
	ROM_LOAD( "grand5.u24",        0x4000, 0x4000, CRC(f896efb2) SHA1(8dc8546e363b4ff80983e3b8e2a19ebb7ff30c7b) )
ROM_END

ROM_START( cabaret )
	ROM_REGION( 0x20000, "main", 0 )	/* 64k for code + 64K for extra RAM */
	ROM_LOAD( "cg-8v204.u97",  0x0000, 0x10000, CRC(44cebf77) SHA1(e3f4e4abf41388f0eed50cf9a0fd0b14aa2f8b93) )

	ROM_REGION( 0x8000, "user1", 0 )
	ROM_LOAD( "cg-7.u98",  0x0000, 0x8000, CRC(b93ae6f8) SHA1(accb87045c278d5d79fff65bb763aa6e8025a945) )	/* background maps, read by the CPU */

	ROM_REGION( 0x60000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "cg-4.u43",  0x00000, 0x20000, CRC(e509f50a) SHA1(7e68ca54642c92cdb348d5cf9466065938d0e027) )
	ROM_LOAD( "cg-5.u44",  0x20000, 0x20000, CRC(e2cbf489) SHA1(3a15ed7efd5696656e6d55b54ec0ff779bdb0d98) )
	ROM_LOAD( "cg-6.u45",  0x40000, 0x20000, CRC(4f2fced7) SHA1(b954856ffdc97fbc99fd3ec087376fbf466d2d5a) )

	ROM_REGION( 0xc000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "cg-1.u40",  0x0000, 0x4000, CRC(7dee8b1f) SHA1(80dbdf6aab9b02cc000956b7894023552428e6a1) )
	ROM_LOAD( "cg-2.u41",  0x4000, 0x4000, CRC(ce8dea39) SHA1(b30d1678a7b98cd821d2ce7383a83cb7c9f31b5f) )
	ROM_LOAD( "cg-3.u42",  0x8000, 0x4000, CRC(7e1f821f) SHA1(b709d49f9d1890fe3b8ca7f90affc0017a0ad95e) )
ROM_END

static DRIVER_INIT( iqblock )
{
	UINT8 *rom = memory_region(machine, "main");
	int i;

	/* decrypt the program ROM */
	for (i = 0;i < 0xf000;i++)
	{
		if ((i & 0x0282) != 0x0282) rom[i] ^= 0x01;
		if ((i & 0x0940) == 0x0940) rom[i] ^= 0x02;
		if ((i & 0x0090) == 0x0010) rom[i] ^= 0x20;
	}

	/* initialize pointers for I/O mapped RAM */
	paletteram         = rom + 0x12000;
	paletteram_2       = rom + 0x12800;
	iqblock_fgvideoram = rom + 0x16800;
	iqblock_bgvideoram = rom + 0x17000;
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfe26, 0xfe26, 0, 0, iqblock_prot_w);
	iqblock_video_type=1;
}

static DRIVER_INIT( grndtour )
{
	UINT8 *rom = memory_region(machine, "main");
	int i;

	/* decrypt the program ROM */
	for (i = 0;i < 0xf000;i++)
	{
		if ((i & 0x0282) != 0x0282) rom[i] ^= 0x01;
		if ((i & 0x0940) == 0x0940) rom[i] ^= 0x02;
		if ((i & 0x0060) == 0x0040) rom[i] ^= 0x20;
	}

	/* initialize pointers for I/O mapped RAM */
	paletteram         = rom + 0x12000;
	paletteram_2       = rom + 0x12800;
	iqblock_fgvideoram = rom + 0x16800;
	iqblock_bgvideoram = rom + 0x17000;
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfe39, 0xfe39, 0, 0, grndtour_prot_w);
	iqblock_video_type=0;
}


static DRIVER_INIT( cabaret )
{
	UINT8 *rom = memory_region(machine, "main");
	int i;

	/* decrypt the program ROM */
	for (i = 0;i < 0xf000;i++)
	{
		if ((i & 0xb206) == 0xa002) rom[i] ^= 0x01;	// could be (i & 0x3206) == 0x2002
	}

	/* initialize pointers for I/O mapped RAM */
	paletteram         = rom + 0x12000;
	paletteram_2       = rom + 0x12800;
	iqblock_fgvideoram = rom + 0x16800;
	iqblock_bgvideoram = rom + 0x17000;
	iqblock_video_type=0;
}


GAME( 1993, iqblock,  0, iqblock,  iqblock, iqblock,  ROT0, "IGS", "IQ-Block", 0 )
GAME( 1993, grndtour, 0, iqblock,  iqblock, grndtour, ROT0, "IGS", "Grand Tour", 0 )

GAME( 19??, cabaret,  0, cabaret,  iqblock, cabaret,  ROT0, "IGS", "Cabaret", GAME_NOT_WORKING | GAME_NO_SOUND )
