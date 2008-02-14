/***************************************************************************

  Super Shanghai Dragon's Eye             (c) 1992 Hot-B

  PCB is manufactured by either Hot-B or Taito, but uses Data East custom
  chips.

  HB-PCB-A4
  M6100691A (distributed by Taito)

  CPU  : 68000
  Sound: Z80B YM2203 Y3014 M6295
  OSC  : 28.0000MHz 16.0000MHz

  The original uses a protection chip which isn't fully worked out yet.
  Sound doesn't work.  The bootleg never seems to write to a sound latch
  (Not sure the bootleg even has the same sound hardware as the original).

  Emulation by Bryan McPhail, mish@tendril.co.uk


Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - There is no confirmation yet that the "Demo Sounds" Dip Switch does
    something as I don't see where bit 0 of 0xfec04a is tested 8(

  - The First "Unused" Dip Switch is probably used in other (older ?) versions
    to act as a "Debug Mode" Dip Switch. When it's ON, you have these features :

      * there is an extended "test mode" that also allows you to test the
        BG and Object ROMS via a menu.
      * You can end a level by pressing BUTTON3 from player 2 8)

  - The "Adult Mode" Dip Switch determines if "Shanghai Paradise" is available.
  - The "Quest Mode" Dip Switch determines if "Shanghai Quest" is available.
  - The "Use Mahjong Tiles" Dip Switch only has an effect when playing
    "Shanghai Advanced".

1) 'sshangha'

  - There are writes to 0x100000-0x10000f (code from 0x000964 to 0x000a8c),
    but their effect is unknown.

2) 'sshanghb'

  - There are writes to 0x101000-0x10100f (code from 0x000964 to 0x000a8c),
    but their effect is unknown. Note that the code is the SAME as the one
    in 'sshangha' (only the 0x10?00? addresses are different).

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"

#define SSHANGHA_HACK	0

VIDEO_START( sshangha );
VIDEO_UPDATE( sshangha );

WRITE16_HANDLER( sshangha_pf2_data_w );
WRITE16_HANDLER( sshangha_pf1_data_w );
WRITE16_HANDLER( sshangha_control_0_w );
WRITE16_HANDLER( sshangha_palette_24bit_w );
WRITE16_HANDLER (sshangha_video_w);

extern UINT16 *sshangha_pf1_data;
extern UINT16 *sshangha_pf2_data;
extern UINT16 *sshangha_pf1_rowscroll, *sshangha_pf2_rowscroll;

static UINT16 *sshangha_prot_data;

/******************************************************************************/

static WRITE16_HANDLER( sshangha_protection16_w )
{
	COMBINE_DATA(&sshangha_prot_data[offset]);

	logerror("CPU #0 PC %06x: warning - write unmapped control address %06x %04x\n",activecpu_get_pc(),offset<<1,data);

	if (offset == (0x260 >> 1)) {
		//soundlatch_w(0,data&0xff);
		//cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE);
	}
}

static WRITE16_HANDLER( sshangha_sound_w )
{
	soundlatch_w(0,data&0xff);
	cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE);
}

/* Protection/IO chip 146 */
static READ16_HANDLER( sshangha_protection16_r )
{
	switch (offset)
	{
		case 0x050 >> 1: /* Player 1 & Player 2 joysticks & fire buttons */
			return (readinputport(0) + (readinputport(1) << 8));
		case 0x76a >> 1: /* Credits */
			return readinputport(2);
		case 0x0ac >> 1: /* DIPS */
			return (readinputport(3) + (readinputport(4) << 8));

		// Protection TODO
	}

	logerror("CPU #0 PC %06x: warning - read unmapped control address %06x\n",activecpu_get_pc(),offset<<1);
	return sshangha_prot_data[offset];
}

static READ16_HANDLER( sshanghb_protection16_r )
{
	switch (offset)
	{
		case 0x050 >> 1: /* Player 1 & Player 2 joysticks & fire buttons */
			return (readinputport(0) + (readinputport(1) << 8));
		case 0x76a >> 1: /* Credits */
			return readinputport(2);
		case 0x0ac >> 1: /* DIPS */
			return (readinputport(3) + (readinputport(4) << 8));
	}
	return sshangha_prot_data[offset];
}

/* Probably returns 0xffff when sprite DMA is complete, the game waits on it */
static READ16_HANDLER( deco_71_r )
{
	return 0xffff;
}

/******************************************************************************/

static MACHINE_RESET( sshangha )
{
	/* Such thing is needed as there is no code to turn the screen
       to normal orientation when the game is reset.
       I'm using the value that forces the screen to be in normal
         orientation when entering the "test mode"
         (check the game code from 0x0006b8 to 0x0006f0).
       I can't tell however if this is accurate or not. */
	sshangha_control_0_w(0, 0x10, 0xff00);
}

/******************************************************************************/

static ADDRESS_MAP_START( sshangha_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)

	AM_RANGE(0x084000, 0x0847ff) AM_READ(sshanghb_protection16_r)
	AM_RANGE(0x101000, 0x101001) AM_READ(deco_71_r)//bootleg hack

	AM_RANGE(0x200000, 0x207fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x340000, 0x340fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x350000, 0x350001) AM_READ(deco_71_r)
	AM_RANGE(0x360000, 0x360fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x370000, 0x370001) AM_READ(deco_71_r)
	AM_RANGE(0x380000, 0x383fff) AM_READ(MRA16_RAM)
	AM_RANGE(0xfec000, 0xff3fff) AM_READ(MRA16_RAM)
	AM_RANGE(0xff4000, 0xff47ff) AM_READ(sshangha_protection16_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sshangha_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x10000c, 0x10000d) AM_WRITE(sshangha_sound_w)
	AM_RANGE(0x10100c, 0x10100d) AM_WRITE(sshangha_sound_w)	/* the bootleg writes here */
	AM_RANGE(0x200000, 0x201fff) AM_WRITE(sshangha_pf1_data_w) AM_BASE(&sshangha_pf1_data)
	AM_RANGE(0x202000, 0x203fff) AM_WRITE(sshangha_pf2_data_w) AM_BASE(&sshangha_pf2_data)
	AM_RANGE(0x204000, 0x2047ff) AM_WRITE(MWA16_RAM) AM_BASE(&sshangha_pf1_rowscroll)
	AM_RANGE(0x206000, 0x2067ff) AM_WRITE(MWA16_RAM) AM_BASE(&sshangha_pf2_rowscroll)
	AM_RANGE(0x300000, 0x30000f) AM_WRITE(sshangha_control_0_w)
	AM_RANGE(0x320000, 0x320001) AM_WRITE(sshangha_video_w)
	AM_RANGE(0x320002, 0x320005) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x340000, 0x340fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16)
	AM_RANGE(0x350000, 0x350007) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x360000, 0x360fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16_2)
	AM_RANGE(0x370000, 0x370007) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x380000, 0x383fff) AM_WRITE(sshangha_palette_24bit_w) AM_BASE(&paletteram16)
	AM_RANGE(0x3c0000, 0x3c0fff) AM_WRITE(MWA16_RAM)	/* Sprite ram buffer on bootleg only?? */
	AM_RANGE(0xfec000, 0xff3fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0xff4000, 0xff47ff) AM_WRITE(sshangha_protection16_w) AM_BASE(&sshangha_prot_data)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0xc000, 0xc000) AM_READ(YM2203_status_port_0_r)
	AM_RANGE(0xf800, 0xffff) AM_READ(MRA8_RAM)
//  AM_RANGE(0xf800, 0xf800) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(YM2203_control_port_0_w)
	AM_RANGE(0xc001, 0xc001) AM_WRITE(YM2203_write_port_0_w)
	AM_RANGE(0xf800, 0xffff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( sshangha )
	PORT_START_TAG("IN0")	/* Player 1 controls (0xfec047.b) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	PORT_NAME("P1 Pick Tile")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	PORT_NAME("P1 Cancel")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)	PORT_NAME("P1 Help")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START_TAG("IN1")	/* Player 2 controls (0xfec046.b) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	PORT_NAME("P2 Pick Tile")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	PORT_NAME("P2 Cancel")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)	PORT_NAME("P2 Help")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START_TAG("IN2")	/* Credits */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Dips seem inverted with respect to other Deco games */

	PORT_START_TAG("DSW1")	/* Dip switch bank 1 (0xfec04a.b, inverted bits order) */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )	// To be confirmed
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x10, "Coin Mode" )			// Check code at 0x0010f2
	PORT_DIPSETTING(    0x10, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_EQUALS,0x10)//Mode 1
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_EQUALS,0x10)//Mode 1
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_EQUALS,0x10)//Mode 1
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_EQUALS,0x10)//Mode 1
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_NOTEQUALS,0x10)//Mode 2
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_NOTEQUALS,0x10)//Mode 2
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_NOTEQUALS,0x10)//Mode 2
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_NOTEQUALS,0x10)//Mode 2
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_EQUALS,0x10)//Mode 1
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_EQUALS,0x10)//Mode 1
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_EQUALS,0x10)//Mode 1
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_EQUALS,0x10)//Mode 1
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_NOTEQUALS,0x10)//Mode 2
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_NOTEQUALS,0x10)//Mode 2
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_NOTEQUALS,0x10)//Mode 2
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )PORT_CONDITION("DSW1",0x10,PORTCOND_NOTEQUALS,0x10)//Mode 2

	PORT_START_TAG("DSW2")	/* Dip switch bank 2 (0xfec04b.b, inverted bits order) */
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
#if SSHANGHA_HACK
	PORT_DIPNAME( 0x20, 0x20, "Debug Mode" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
#else
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                   // See notes
#endif
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "Tile Animation" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Use Mahjong Tiles" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Adult Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
  	PORT_DIPNAME( 0x01, 0x01, "Quest Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 chars */
	4096,
	4,		/* 4 bits per pixel  */
	{ 8, 0, 0x100000*8+8,0x100000*8+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 8, 0, 0x100000*8+8, 0x100000*8+0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( sshangha )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,  256, 64 ) /* Characters 8x8 */
	GFXDECODE_ENTRY( REGION_GFX1, 0, tilelayout,  256, 64 ) /* Tiles 16x16 */
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout,    0, 32 ) /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

static void irqhandler(int state)
{
	cpunum_set_input_line(Machine, 1,0,state);
}

static const struct YM2203interface ym2203_interface =
{
	0,0,0,0,irqhandler
};

static MACHINE_DRIVER_START( sshangha )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 28000000/2)
	MDRV_CPU_PROGRAM_MAP(sshangha_readmem,sshangha_writemem)
	MDRV_CPU_VBLANK_INT(irq6_line_hold,1)

	MDRV_CPU_ADD(Z80, 16000000/4)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MDRV_MACHINE_RESET(sshangha)	/* init machine */

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER /*| VIDEO_BUFFERS_SPRITERAM*/)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MDRV_GFXDECODE(sshangha)
	MDRV_PALETTE_LENGTH(4096)

	MDRV_VIDEO_START(sshangha)
	MDRV_VIDEO_UPDATE(sshangha)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2203, 16000000/4)
	MDRV_SOUND_CONFIG(ym2203_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.60)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.60)

	MDRV_SOUND_ADD(OKIM6295, 1023924)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)
MACHINE_DRIVER_END

/******************************************************************************/

ROM_START( sshangha )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ss007-1.u28", 0x00000, 0x20000, CRC(bc466edf) SHA1(b96525b2c879d15b46a7753fa6ebf12a851cd019) )
	ROM_LOAD16_BYTE( "ss006-1.u27", 0x00001, 0x20000, CRC(872a2a2d) SHA1(42d7a01465d5c403354aaf0f2dab8adb9afe61b0) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* Sound CPU */
	ROM_LOAD( "ss008.u82", 0x000000, 0x010000, CRC(04dc3647) SHA1(c06a7e8932c03de5759a9b69da0d761006b49517) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ss001.u8",  0x000000, 0x100000, CRC(ebeca5b7) SHA1(1746e757ad9bbef2aa9028c54f25d4aa4dedf79e) )
	ROM_LOAD( "ss002.u7",  0x100000, 0x100000, CRC(67659f29) SHA1(50944877665b7b848b3f7063892bd39a96a847cf) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ss003.u39", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) ) /* Copy of rom at u47 */
	ROM_LOAD( "ss004.u37", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) ) /* Copy of rom at u46 */

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "ss005.u86", 0x000000, 0x040000, CRC(c53a82ad) SHA1(756e453c8b5ce8e47f93fbda3a9e48bb73e93e2e) )
ROM_END

ROM_START( sshanghb )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "sshanb_2.010", 0x00000, 0x20000, CRC(bc7ed254) SHA1(aeee4b8a8265902bb41575cc143738ecf3aff57d) )
	ROM_LOAD16_BYTE( "sshanb_1.010", 0x00001, 0x20000, CRC(7b049f49) SHA1(2570077c67dbd35053d475a18c3f10813bf914f7) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* Sound CPU */
	ROM_LOAD( "ss008.u82", 0x000000, 0x010000, CRC(04dc3647) SHA1(c06a7e8932c03de5759a9b69da0d761006b49517) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ss001.u8",  0x000000, 0x100000, CRC(ebeca5b7) SHA1(1746e757ad9bbef2aa9028c54f25d4aa4dedf79e) )
	ROM_LOAD( "ss002.u7",  0x100000, 0x100000, CRC(67659f29) SHA1(50944877665b7b848b3f7063892bd39a96a847cf) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ss003.u39", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) ) /* Copy of rom at u47 */
	ROM_LOAD( "ss004.u37", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) ) /* Copy of rom at u46 */

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "ss005.u86", 0x000000, 0x040000, CRC(c53a82ad) SHA1(756e453c8b5ce8e47f93fbda3a9e48bb73e93e2e) )
ROM_END


static DRIVER_INIT( sshangha )
{
#if SSHANGHA_HACK
	/* This is a hack to allow you to use the extra features
         of the first "Unused" Dip Switch (see notes above). */
	UINT16 *RAM = (UINT16 *)memory_region(REGION_CPU1);
	RAM[0x000384/2] = 0x4e71;
	RAM[0x000386/2] = 0x4e71;
	RAM[0x000388/2] = 0x4e71;
	RAM[0x00038a/2] = 0x4e71;
	/* To avoid checksum error (only useful for 'sshangha') */
	RAM[0x000428/2] = 0x4e71;
	RAM[0x00042a/2] = 0x4e71;
#endif
}


GAME( 1992, sshangha, 0,        sshangha, sshangha, sshangha, ROT0, "Hot-B.",  "Super Shanghai Dragon's Eye (Japan)", GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND )
GAME( 1992, sshanghb, sshangha, sshangha, sshangha, sshangha, ROT0, "bootleg", "Super Shanghai Dragon's Eye (World, bootleg)", GAME_NO_SOUND )
