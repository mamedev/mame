/***************************************************************************

  IPM Invader (M10 m10 hardware)
  Sky Chuter By IREM
  Space Beam (M15 m15 hardware)
  Green Beret (?M15 ?m15 hardware)

  (c) 12/2/1998 Lee Taylor

Notes:
- Colors are close to screen shots for IPM Invader. The other games have not
  been verified.
- The bitmap strips in IPM Invader might be slightly misplaced

TODO:
- Dip switches

Head On
-------
Irem, 1979? / 1980?

PCB Layout
----------

    M-15L
   |---------------------------------------------------------------------------------|
   |                                                                                 |
   | DSW(8)  74175   74175   7400  74LS08   74121   M53214     |-------|      E1.9A  |
   |                                                           | 6502  |             |
   |          7432   74175   7404    7427    7442  74LS241     |-------|             |
   |                                                                          E2.9B  |
   |                                                                                 |
   |        74LS74    7432  74161   74161    7442  74LS241  74LS367  74LS367         |
 |-|                                                                          E3.9C  |
 |          M53214 74LS367   7442    7486    8216     2114  74LS157  74LS367         |
 |                                                                                   |
 |4         M53214 74LS367  74161    7486    8216     2114  74LS157     2111  E4.9D  |
 |4                                                                                  |
 |W         M53214 74LS367  74161    7486    8216     2114  74LS157     2111         |
 |A                                                                           E5.9F  |
 |Y         M53214 74LS367  74161    7486    8216    74166     2114  74LS157         |
 |                         11.73MHz                                                  |
 |            7400    7432  7404    74161    8216    74166     2114  74LS157  E6.9G  |
 |-|                                                                                 |
   |   VR3 VR2 VR1    7432  7404     7400  *74173     7400  74LS139  74LS157         |
   |                                       *74S04                                    |
   |                                                                                 |
   |---------------------------------------------------------------------------------|
Notes:
      All IC's are listed
      All ROMs type 2708 (1K x8)

      6502 clock: 1.173MHz
               *: These 2 IC's piggybacked. 74S04 on top
         VR1/2/3: 5K potentiometers
            2114: 1K x4 SRAM
            2111: 256bytes x4 SRAM
            8216: 256bytes x1 SRAM

Sound PCB
---------

M-15S
|---------------------------|
|                           |
|  NE555  NE555             |
|                           |
|  NE555  NE555             |
|               LM3900   VR1|
|                           |
|  c1815x9               VR2|
|                           |
|               LM3900   VR3|
|                           |
|                        VR4|
|                           |
|                        VR5|
|    TA7222                 |
|---------------------------|
Notes:
      PCB contains lots of resistors, capacitors, transistors etc.

      VR1/2/3/4/5: Potentiometers for volume of each sound
            C1815: Transistor (x9)
           TA7222: Power Amp

***************************************************************************/

#include "driver.h"

extern UINT8* iremm15_chargen;

VIDEO_UPDATE( skychut );
VIDEO_UPDATE( iremm15 );
WRITE8_HANDLER( skychut_colorram_w );
WRITE8_HANDLER( skychut_ctrl_w );

static UINT8 *memory;


static PALETTE_INIT( skychut )
{
	int i;

	palette_set_color(machine,0,MAKE_RGB(0xff,0xff,0xff));
	palette_set_color(machine,1,MAKE_RGB(0xff,0xff,0x00));
	palette_set_color(machine,2,MAKE_RGB(0xff,0x00,0xff));
	palette_set_color(machine,3,MAKE_RGB(0xff,0x00,0x00));
	palette_set_color(machine,4,MAKE_RGB(0x00,0xff,0xff));
	palette_set_color(machine,5,MAKE_RGB(0x00,0xff,0x00));
	palette_set_color(machine,6,MAKE_RGB(0x00,0x00,0xff));
	palette_set_color(machine,7,MAKE_RGB(0x00,0x00,0x00));

	for (i = 0;i < 8;i++)
	{
		colortable[2*i+0] = 7;
		colortable[2*i+1] = i;
	}
}


static ADDRESS_MAP_START( skychut_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x02ff) AM_READ(MRA8_RAM) /* scratch ram */
	AM_RANGE(0x1000, 0x2fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x4800, 0x4bff) AM_READ(MRA8_RAM) /* Foreground colour  */
	AM_RANGE(0x5000, 0x53ff) AM_READ(MRA8_RAM) /* BKgrnd colour ??? */
	AM_RANGE(0xa200, 0xa200) AM_READ(input_port_1_r)
	AM_RANGE(0xa300, 0xa300) AM_READ(input_port_0_r)
/*  AM_RANGE(0xa700, 0xa700) AM_READ(input_port_3_r)*/
	AM_RANGE(0xfc00, 0xffff) AM_READ(MRA8_ROM)	/* for the reset / interrupt vectors */
ADDRESS_MAP_END


static ADDRESS_MAP_START( skychut_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x02ff) AM_WRITE(MWA8_RAM) AM_BASE(&memory)
	AM_RANGE(0x1000, 0x2fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(skychut_colorram_w) AM_BASE(&colorram) /* foreground colour  */
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(MWA8_RAM) AM_BASE(&iremm15_chargen) /* background ????? */
//  AM_RANGE(0xa100, 0xa1ff) AM_WRITE(MWA8_RAM) /* Sound writes????? */
	AM_RANGE(0xa400, 0xa400) AM_WRITE(skychut_ctrl_w)	/* line at bottom of screen?, sound, flip screen */
	AM_RANGE(0xfc00, 0xffff) AM_WRITE(MWA8_ROM)	/* for the reset / interrupt vectors */
ADDRESS_MAP_END

static ADDRESS_MAP_START( greenberet_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x02ff) AM_READ(MRA8_RAM) /* scratch ram */
	AM_RANGE(0x1000, 0x33ff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x4800, 0x4bff) AM_READ(MRA8_RAM) /* Foreground colour  */
	AM_RANGE(0x5000, 0x57ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_3_r)
	AM_RANGE(0xa200, 0xa200) AM_READ(input_port_1_r)
	AM_RANGE(0xa300, 0xa300) AM_READ(input_port_0_r)
	AM_RANGE(0xfc00, 0xffff) AM_READ(MRA8_ROM)	/* for the reset / interrupt vectors */
ADDRESS_MAP_END

static ADDRESS_MAP_START( greenberet_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x02ff) AM_WRITE(MWA8_RAM) AM_BASE(&memory)
	AM_RANGE(0x1000, 0x33ff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(skychut_colorram_w) AM_BASE(&colorram) /* foreground colour  */
	AM_RANGE(0x5000, 0x57ff) AM_WRITE(MWA8_RAM) AM_BASE(&iremm15_chargen)
	AM_RANGE(0xa100, 0xa1ff) AM_WRITE(MWA8_RAM) /* Sound writes????? */
	AM_RANGE(0xa400, 0xa400) AM_WRITE(MWA8_NOP)	/* sound, flip screen */
	AM_RANGE(0xfc00, 0xffff) AM_WRITE(MWA8_ROM)	/* for the reset / interrupt vectors */
ADDRESS_MAP_END


static INTERRUPT_GEN( skychut_interrupt )
{
	if (readinputport(2) & 1)	/* Left Coin */
        cpunum_set_input_line(0, INPUT_LINE_NMI, PULSE_LINE);
    else
    	cpunum_set_input_line(0, 0, HOLD_LINE);
}

static INPUT_PORTS_START( skychut )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL

	PORT_START	/* IN1 */
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING (  0x00, "3" )
	PORT_DIPSETTING (  0x01, "4" )
	PORT_DIPSETTING (  0x02, "5" )

	PORT_START	/* FAKE */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
INPUT_PORTS_END

static INPUT_PORTS_START( spacebeam )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START	/* IN1 */
	PORT_DIPNAME(0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING (  0x00, "2" )
	PORT_DIPSETTING (  0x01, "3" )
	PORT_DIPSETTING (  0x02, "4" )
	PORT_DIPSETTING (  0x03, "5" )
	PORT_DIPNAME(0x08, 0x00, "Replay" )
	PORT_DIPSETTING (  0x00, "30000" )
	PORT_DIPSETTING (  0x08, DEF_STR( None ) )
	PORT_DIPNAME(0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING (  0x00, "Testmode" )
	PORT_DIPSETTING (  0x10, "1 Coin 1 Play" )
	PORT_DIPSETTING (  0x20, "1 Coin 2 Plays" )

	PORT_START	/* FAKE */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START	/* IN3 */
	PORT_BIT( 0x03, 0, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( headoni )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	/* IN1 */
	PORT_DIPNAME(0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING (  0x00, "2" )
	PORT_DIPSETTING (  0x01, "3" )
	PORT_DIPSETTING (  0x02, "4" )
	PORT_DIPSETTING (  0x03, "5" )
	PORT_DIPNAME(0x08, 0x00, "Replay" )
	PORT_DIPSETTING (  0x00, "30000" )
	PORT_DIPSETTING (  0x08, DEF_STR( None ) )
	PORT_DIPNAME(0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING (  0x00, "Testmode" )
	PORT_DIPSETTING (  0x10, "1 Coin 1 Play" )
	PORT_DIPSETTING (  0x20, "1 Coin 2 Plays" )

	PORT_START	/* FAKE */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START	/* IN3 */
	PORT_BIT( 0x03, 0, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	1,	/* 1 bits per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( skychut )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout, 0, 8 )
GFXDECODE_END


static MACHINE_DRIVER_START( skychut )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6502,20000000/8)
	MDRV_CPU_PROGRAM_MAP(skychut_readmem,skychut_writemem)
	MDRV_CPU_VBLANK_INT(skychut_interrupt,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(skychut)
	MDRV_PALETTE_LENGTH(8)
	MDRV_COLORTABLE_LENGTH(2*8)

	MDRV_PALETTE_INIT(skychut)
	MDRV_VIDEO_START(generic)
	MDRV_VIDEO_UPDATE(skychut)

	/* sound hardware */
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( greenberet )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M6502,11730000/10)
	MDRV_CPU_PROGRAM_MAP(greenberet_readmem,greenberet_writemem)
	MDRV_CPU_VBLANK_INT(skychut_interrupt,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_LENGTH(8)
	MDRV_COLORTABLE_LENGTH(2*8)

	MDRV_PALETTE_INIT(skychut)
	MDRV_VIDEO_START(generic)
	MDRV_VIDEO_UPDATE(iremm15)

	/* sound hardware */
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( headoni )
	MDRV_IMPORT_FROM(greenberet)
	MDRV_CPU_REPLACE("main", M6502,11730000/16)
MACHINE_DRIVER_END




/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( andromed )//Jumps to an unmapped sub-routine at $2fc9
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "am1",  0x1000, 0x0400, CRC(53df0152) SHA1(d27113740094d219b0e05a930d8daa4c22129183) )
	ROM_LOAD( "am2",  0x1400, 0x0400, CRC(dab64957) SHA1(77ced520f8e78bb08ddab4213646cf55d834e63e) )
	ROM_LOAD( "am3",  0x1800, 0x0400, CRC(f983f35c) SHA1(1bfee6cf7d18b56594831f2efa7dcc53b47d7e30) )
	ROM_LOAD( "am4",  0x1c00, 0x0400, CRC(09f20717) SHA1(c54c9b7d16b40a7ab49eac255906b43b03939d2b) )
	ROM_RELOAD(       0xfc00, 0x0400 )	/* for the reset and interrupt vectors */
	ROM_LOAD( "am5",  0x2000, 0x0400, CRC(518a3b88) SHA1(5e20c905c2190b381a105327e112fcc0a127bb2f) )
	ROM_LOAD( "am6",  0x2400, 0x0400, CRC(ce3d5fff) SHA1(c34178aca9ffb8b2dd468d9e3369a985f52daf9a) )
	ROM_LOAD( "am7",  0x2800, 0x0400, CRC(30d3366f) SHA1(aa73bba194fa6d1f3909f8df517a0bff07583ea9) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "am9",  0x0000, 0x0400, CRC(a1c8f4db) SHA1(bedf5d7126c7e9b91ad595188c69aa2c043c71e8) )
	ROM_LOAD( "am10", 0x0400, 0x0400, CRC(be2de8f3) SHA1(7eb3d1eb88b4481b0dcb7d001207f516a5db32b3) )
ROM_END

ROM_START( ipminvad )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "b1r",  0x1000, 0x0400, CRC(f9a7eb9b) SHA1(93ac65d3ac725d3e4c2fb769816ee808ab609911) )
	ROM_LOAD( "b2r",  0x1400, 0x0400, CRC(af11c1aa) SHA1(6a74fcc7cb1627b1c427a77da89b69ccf3175800) )
	ROM_LOAD( "b3r",  0x1800, 0x0400, CRC(ed49e481) SHA1(8771a34f432e6d88acc5f7529f16c980a77485db) )
	ROM_LOAD( "b4r",  0x1c00, 0x0400, CRC(6d5db95b) SHA1(135500fc17524e8608c3bcfe26321144aa0afb91) )
	ROM_RELOAD(       0xfc00, 0x0400 )	/* for the reset and interrupt vectors */
	ROM_LOAD( "b5r",  0x2000, 0x0400, CRC(eabba7aa) SHA1(75e47eacd429f48f0a3a4539e5ecb4b1ea7281b1) )
	ROM_LOAD( "b6r",  0x2400, 0x0400, CRC(3d0e7fa6) SHA1(14903bfc9506cb8e37807fb397be79f5eab99e3b) )
	ROM_LOAD( "b7r",  0x2800, 0x0400, CRC(cf04864f) SHA1(6fe3ce208334321b63ada779fed69ec7cf4051ad) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "b9r",  0x0000, 0x0400, CRC(56942cab) SHA1(ba13a856477fc6cf7fd36996e47a3724f862f888) )
	ROM_LOAD( "b10r", 0x0400, 0x0400, CRC(be4b8585) SHA1(0154eae62585e154cf20edcf4599bda8bd333aa9) )
ROM_END

ROM_START( skychut )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "sc1d",  0x1000, 0x0400, CRC(30b5ded1) SHA1(3a8b4fa344522404661b062808a2ea1d5858fdd0) )
	ROM_LOAD( "sc2d",  0x1400, 0x0400, CRC(fd1f4b9e) SHA1(e5606979abe1fa4cc9eae0c4f61516769db35c39) )
	ROM_LOAD( "sc3d",  0x1800, 0x0400, CRC(67ed201e) SHA1(589b1efdc1bbccff296f6420e2b320cd54b4ac8e) )
	ROM_LOAD( "sc4d",  0x1c00, 0x0400, CRC(9b23a679) SHA1(a101f9b0fdde927a43e53e9b7d0dffb9dcca9e16) )
	ROM_RELOAD(        0xfc00, 0x0400 )	/* for the reset and interrupt vectors */
	ROM_LOAD( "sc5a",  0x2000, 0x0400, CRC(51d975e6) SHA1(7d345025ef28c8a81f599cde445eeb336c368fce) )
	ROM_LOAD( "sc6e",  0x2400, 0x0400, CRC(617f302f) SHA1(4277ef97279eb63fc68b6c40f8545b31abaab474) )
	ROM_LOAD( "sc7",   0x2800, 0x0400, CRC(dd4c8e1a) SHA1(b5a141d8ac256ba6522308e5f194bfaf5c75fa5b) )
	ROM_LOAD( "sc8d",  0x2c00, 0x0400, CRC(aca8b798) SHA1(d9048d060314d8f20ab1967fee846d35c22ac693) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "sc9d",  0x0000, 0x0400, CRC(2101029e) SHA1(34cddf076d3d860aa03043db14837f42449aefe7) )
	ROM_LOAD( "sc10d", 0x0400, 0x0400, CRC(2f81c70c) SHA1(504935c89a4158a067cbf1dcdb27f7421678915d) )
ROM_END

ROM_START( spacbeam )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "m1b", 0x1000, 0x0400, CRC(5a1c3e0b) SHA1(1c9c58359d74b14ce96934fcc6acefbdfaf1e1be) )
	ROM_LOAD( "m2b", 0x1400, 0x0400, CRC(a02bd9d7) SHA1(d25dfa66b422bdbb29b1922007c84f1947fe9be1) )
	ROM_LOAD( "m3b", 0x1800, 0x0400, CRC(78040843) SHA1(0b8a3ab09dff951aa527649f82b8877cf01126c1) )
	ROM_LOAD( "m4b", 0x1c00, 0x0400, CRC(74705a44) SHA1(8fa9d22a58f08086bf2d89e3d92eca097cdd2cbf) )
	ROM_RELOAD(      0xfc00, 0x0400 )	/* for the reset and interrupt vectors */
	ROM_LOAD( "m5b", 0x2000, 0x0400, CRC(afdf1242) SHA1(e26a8e91edb3d8ba96b3d225813760f42238b003) )
	ROM_LOAD( "m6b", 0x2400, 0x0400, CRC(12afb0c2) SHA1(bf6ed90cf4815f0fb41d435954d4c346a55098f5) )
ROM_END

ROM_START( headoni )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "e1.9a", 0x1000, 0x0400, CRC(05da5265) SHA1(17e0c9261978770325a0befdcdd8a1b07ed39df0) )
	ROM_LOAD( "e2.9b", 0x1400, 0x0400, CRC(dada26a8) SHA1(1368ade1c0c57d33d15594370cf1edf95fc44fd1) )
	ROM_LOAD( "e3.9c", 0x1800, 0x0400, CRC(61ff24f5) SHA1(0e68aedd01b765fb2af76f914b3d287ecf30f716) )
	ROM_LOAD( "e4.9d", 0x1c00, 0x0400, CRC(ce4c5a67) SHA1(8db493d43f311a29127405aad7693bc08b570b14) )
	ROM_RELOAD(      0xfc00, 0x0400 )	/* for the reset and interrupt vectors */
	ROM_LOAD( "e5.9f", 0x2000, 0x0400, CRC(b5232439) SHA1(39b8fb4bbd00a73b9a2b68bc3e88fb45d3f62d7c) )
	ROM_LOAD( "e6.9g", 0x2400, 0x0400, CRC(99acd1a6) SHA1(799382c1b079aad3034a1cc738dc06954978a0ac) )
ROM_END

ROM_START( greenber )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "gb1", 0x1000, 0x0400, CRC(018ff672) SHA1(54d082a332831afc28b47704a5656da0a8a902fa) ) // ok
	ROM_LOAD( "gb2", 0x1400, 0x0400, CRC(ea8f2267) SHA1(ad5bb38a80fbc7c70c8fa6f41086a7ade81655bc) ) // ok
	ROM_LOAD( "gb3", 0x1800, 0x0400, CRC(8f337920) SHA1(ac3d76eb368645ba23f5823b39c04fae49d481e1) ) // ok
	ROM_LOAD( "gb4", 0x1c00, 0x0400, CRC(7eeac4eb) SHA1(c668ad45ebc4aca558371539031efc4ec3990e44) ) // ok
	ROM_RELOAD(      0xfc00, 0x0400 )	/* for the reset and interrupt vectors */
	ROM_LOAD( "gb5", 0x2000, 0x0400, CRC(b2f8e69a) SHA1(44295e58da890a8c4aba6fe90defe9c578c95592) )
	ROM_LOAD( "gb6", 0x2400, 0x0400, CRC(50ea8bd3) SHA1(a816c5fcc603b28c2ae59f217871a7e85fb794e1) )
	ROM_LOAD( "gb7", 0x2800, 0x0400, NO_DUMP ) // 2be8 entry
	ROM_LOAD( "gb8", 0x2c00, 0x0400, CRC(34700b31) SHA1(c148e2475eaaa0e9d1e2412eea359a7ba744e563) )
	ROM_LOAD( "gb9", 0x3000, 0x0400, CRC(c27b9ba3) SHA1(a2f4f0c4b61eb03bba13ae5d25dc01009a4f86ee) ) // ok ?
ROM_END

GAME( 1979, andromed, 0, skychut,    skychut,   0, ROT270, "Irem", "Andromeda (Japan?)", GAME_NO_COCKTAIL | GAME_NO_SOUND | GAME_IMPERFECT_COLORS | GAME_NOT_WORKING )
GAME( 1979?,ipminvad, 0, skychut,    skychut,   0, ROT270, "Irem", "IPM Invader", GAME_NO_COCKTAIL | GAME_NO_SOUND | GAME_IMPERFECT_COLORS )
GAME( 1980, skychut,  0, skychut,    skychut,   0, ROT270, "Irem", "Sky Chuter", GAME_NO_COCKTAIL | GAME_NO_SOUND | GAME_IMPERFECT_COLORS )
GAME( 1979, spacbeam, 0, greenberet, spacebeam, 0, ROT270, "Irem", "Space Beam", GAME_NO_COCKTAIL | GAME_NO_SOUND | GAME_IMPERFECT_COLORS )
GAME( 1979?,headoni,  0, headoni,    headoni,   0, ROT270, "Irem", "Head On (Irem, M-15 Hardware)", GAME_NO_COCKTAIL | GAME_NO_SOUND | GAME_IMPERFECT_COLORS )
GAME( 1980, greenber, 0, greenberet, spacebeam, 0, ROT270, "Irem", "Green Beret (Irem)", GAME_NO_COCKTAIL | GAME_NO_SOUND | GAME_IMPERFECT_COLORS | GAME_NOT_WORKING )
