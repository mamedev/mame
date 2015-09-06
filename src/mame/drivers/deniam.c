// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Deniam games
driver by Nicola Salmoria

Check archive.org for http://deniam.co.kr

Title            System     Date
---------------- ---------- ----------
GO!GO!           deniam-16b 1995/10/11 UNDUMPED "Go Go Quiz Song Stop"
Logic Pro        deniam-16b 1996/10/20
Karian Cross     deniam-16b 1997/04/17
LOTTERY GAME     deniam-16c 1997/05/21 UNDUMPED "Bogori (Lottery Game)"
Logic Pro 2      deniam-16c 1997/06/20
Propose          deniam-16c 1997/06/21 UNDUMPED

They call the hardware "deniam-16", but it's actually pretty much identical to
Sega System 16.


Notes:

- The logicpr2 OKIM6295 ROM has four banks, but the game seems to only use 0 and 1.
  (the latter two banks are identical/nearly so to the first two?)
- logicpro dip switches might be wrong (using the logicpr2 ones)
- flip screen is not supported but these games don't use it (no flip screen dip
  and no cocktail mode)
- if it's like System 16, the top bit of palette ram should be an additional bit
  for Green. But is it ever not 0?
- Logic Pro 2 sound test menu is bugged on the real pcb. You can't change the sound
  sample and music selected due to an incorrect compare/branch at pc=0x3fd0/0x3fd2
  (for sfx) and pc=0x3ffa/0x3ffc (for bgm). Patching the two compare opcodes with
  the value 0xb041 lets you properly change the sound and bgm in the test mode,
  otherwise they're stuck at 19 and 03 respectively. Verified on real hardware.
- Logic Pro has an unemulated graphical effect: when you insert a coin, the screen
  becomes very slightly darker until the next 'scene change'. This is not emulated
  yet.
- Both Logic Pro and Logic Pro 2 have 4 pixels of garbage (related to fg layer?)
  at the left edge of the screen on the real pcb. Sprites can properly be displayed
  in this area. Mame displays this area as black.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "includes/deniam.h"


WRITE16_MEMBER(deniam_state::sound_command_w)
{
	if (ACCESSING_BITS_8_15)
	{
		soundlatch_byte_w(space,offset, (data >> 8) & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

WRITE8_MEMBER(deniam_state::deniam16b_oki_rom_bank_w)
{
	m_oki->set_bank_base((data & 0x40) ? 0x40000 : 0x00000);
}

WRITE16_MEMBER(deniam_state::deniam16c_oki_rom_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if ((data&0xFE) != 0) popmessage("OKI bank was not 0 or 1! contact MAMEDEV!");
		m_oki->set_bank_base((data & 0x01) ? 0x40000 : 0x00000);
	}
}

WRITE16_MEMBER(deniam_state::deniam_irq_ack_w)
{
	m_maincpu->set_input_line(4, CLEAR_LINE);
}

static ADDRESS_MAP_START( deniam16b_map, AS_PROGRAM, 16, deniam_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM_WRITE(deniam_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x410000, 0x410fff) AM_RAM_WRITE(deniam_textram_w) AM_SHARE("textram")
	AM_RANGE(0x440000, 0x4407ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(deniam_palette_w) AM_SHARE("paletteram")
	AM_RANGE(0xc40000, 0xc40001) AM_WRITE(sound_command_w)
	AM_RANGE(0xc40002, 0xc40003) AM_READWRITE(deniam_coinctrl_r, deniam_coinctrl_w)
	AM_RANGE(0xc40004, 0xc40005) AM_WRITE(deniam_irq_ack_w)
	AM_RANGE(0xc44000, 0xc44001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc44002, 0xc44003) AM_READ_PORT("P1")
	AM_RANGE(0xc44004, 0xc44005) AM_READ_PORT("P2") AM_WRITENOP
	AM_RANGE(0xc44006, 0xc44007) AM_READNOP /* unused? */
	AM_RANGE(0xc4400a, 0xc4400b) AM_READ_PORT("DSW")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, deniam_state )
	AM_RANGE(0x0000, 0xf7ff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, deniam_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("ymsnd", ym3812_device, write)
	AM_RANGE(0x05, 0x05) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x07, 0x07) AM_WRITE(deniam16b_oki_rom_bank_w)
ADDRESS_MAP_END

/* identical to 16b, but handles sound directly */
static ADDRESS_MAP_START( deniam16c_map, AS_PROGRAM, 16, deniam_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM_WRITE(deniam_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x410000, 0x410fff) AM_RAM_WRITE(deniam_textram_w) AM_SHARE("textram")
	AM_RANGE(0x440000, 0x4407ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(deniam_palette_w) AM_SHARE("paletteram")
	AM_RANGE(0xc40000, 0xc40001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0xc40002, 0xc40003) AM_READWRITE(deniam_coinctrl_r, deniam_coinctrl_w)
	AM_RANGE(0xc40004, 0xc40005) AM_WRITE(deniam_irq_ack_w)
	AM_RANGE(0xc40006, 0xc40007) AM_WRITE(deniam16c_oki_rom_bank_w)
	AM_RANGE(0xc44000, 0xc44001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc44002, 0xc44003) AM_READ_PORT("P1")
	AM_RANGE(0xc44004, 0xc44005) AM_READ_PORT("P2")
	AM_RANGE(0xc44006, 0xc44007) AM_READNOP /* read unused? extra input port/dipswitches? */
	AM_RANGE(0xc40008, 0xc4000b) AM_DEVWRITE8("ymsnd", ym3812_device, write, 0xff00)
	AM_RANGE(0xc4400a, 0xc4400b) AM_READ_PORT("DSW") /* probably YM3812 input port */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END



static INPUT_PORTS_START( karianx )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Demo Music" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_SERVICE( 0x80, 0x80 )
INPUT_PORTS_END

static INPUT_PORTS_START( logicpr2 )
	PORT_INCLUDE( karianx )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x18, 0x18, "Play Time" )
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
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


static GFXDECODE_START( deniam )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 128 )    /* colors 0-1023 */
												/* sprites use colors 1024-2047 */
GFXDECODE_END


void deniam_state::machine_start()
{
	save_item(NAME(m_display_enable));
	save_item(NAME(m_coinctrl));

	save_item(NAME(m_bg_scrollx_offs));
	save_item(NAME(m_bg_scrolly_offs));
	save_item(NAME(m_fg_scrollx_offs));
	save_item(NAME(m_fg_scrolly_offs));
	save_item(NAME(m_bg_scrollx_reg));
	save_item(NAME(m_bg_scrolly_reg));
	save_item(NAME(m_fg_scrollx_reg));
	save_item(NAME(m_fg_scrolly_reg));
	save_item(NAME(m_bg_page_reg));
	save_item(NAME(m_fg_page_reg));
	save_item(NAME(m_bg_page));
	save_item(NAME(m_fg_page));
}


void deniam_state::machine_reset()
{
	/* logicpr2 does not reset the bank base on startup, though it probably
	doesn't matter since the coinup sfx (sample borrowed from 'tyrian' on PC)
	exists in both banks; it properly sets the bank as soon as the ufo sfx
	plays or a player character is selected on the character select screen */
	m_oki->set_bank_base(0x00000);
}

static MACHINE_CONFIG_START( deniam16b, deniam_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,XTAL_25MHz/2)    /* 12.5Mhz verified */
	MCFG_CPU_PROGRAM_MAP(deniam16b_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", deniam_state,  irq4_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80,XTAL_25MHz/4)  /* 6.25Mhz verified */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	//MCFG_SCREEN_VISIBLE_AREA(24*8, 64*8-1, 0*8, 28*8-1) // looks better but doesn't match hardware
	MCFG_SCREEN_VISIBLE_AREA(24*8-4, 64*8-5, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(deniam_state, screen_update_deniam)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", deniam)
	MCFG_PALETTE_ADD("palette", 2048)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_25MHz/6) /* "SM64" ym3812 clone; 4.166470 measured, = 4.166666Mhz verified */
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MCFG_OKIM6295_ADD("oki", XTAL_25MHz/24, OKIM6295_PIN7_HIGH) /* 1.041620 measured, = 1.0416666Mhz verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( deniam16c, deniam_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,XTAL_25MHz/2)    /* 12.5Mhz verified */
	MCFG_CPU_PROGRAM_MAP(deniam16c_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", deniam_state,  irq4_line_assert)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	//MCFG_SCREEN_VISIBLE_AREA(24*8, 64*8-1, 0*8, 28*8-1) // looks better but doesn't match hardware
	MCFG_SCREEN_VISIBLE_AREA(24*8-4, 64*8-5, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(deniam_state, screen_update_deniam)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", deniam)
	MCFG_PALETTE_ADD("palette", 2048)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_25MHz/6) /* "SM64" ym3812 clone; 4.166470 measured, = 4.166666Mhz verified) */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MCFG_OKIM6295_ADD("oki", XTAL_25MHz/24, OKIM6295_PIN7_HIGH)  /* 1.041620 measured, = 1.0416666Mhz verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( logicpro )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "logicpro.r4", 0x00000, 0x40000, CRC(c506d484) SHA1(5d662b109e1d2e09556bc4ecbc11bbf5ccb639d3) )
	ROM_LOAD16_BYTE( "logicpro.r3", 0x00001, 0x40000, CRC(d5a4cf62) SHA1(138ea4f1629e453c1a00410eda7086d3633240e3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound */
	ROM_LOAD( "logicpro.r2", 0x0000, 0x10000, CRC(000d624b) SHA1(c0da218ee81d01b3dcef2159bbaaff5d3ddb7619) )

	ROM_REGION( 0x180000, "gfx1", 0 )   /* chars */
	ROM_LOAD( "logicpro.r5", 0x000000, 0x080000, CRC(dedf18c9) SHA1(9725e096427f03ed5fd81584c0aa85a53f9681c9) )
	ROM_LOAD( "logicpro.r6", 0x080000, 0x080000, CRC(3ecbd1c2) SHA1(dd6afacd58eaaa2562e007a92b6667ecc968377d) )
	ROM_LOAD( "logicpro.r7", 0x100000, 0x080000, CRC(47135521) SHA1(ee6a93332190fc966f8e820430d652942f030b00) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites, used at run time */
	ROM_LOAD16_BYTE( "logicpro.r9", 0x000000, 0x080000, CRC(a98bc1d2) SHA1(f4aed07cccca892f3d3a91546b3a98fbe3e66d9c) )
	ROM_LOAD16_BYTE( "logicpro.r8", 0x000001, 0x080000, CRC(1de46298) SHA1(3385a2956d9a427c85554f39c8d85922bbeb1ce1) )

	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "logicpro.r1", 0x0000, 0x080000, CRC(a1fec4d4) SHA1(4390cd18b4a7de2d8cb68270180ea3de42fd2282) )
ROM_END

ROM_START( croquis )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "r4.bin", 0x00000, 0x40000, CRC(03c9055e) SHA1(b1fa8e7a272887decca30eefe73ac782f296f0dd) )
	ROM_LOAD16_BYTE( "r3.bin", 0x00001, 0x40000, CRC(a98ae4f6) SHA1(80fcedb4ee0f35eb2d0b4a248c15f872af2e08f2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound */
	ROM_LOAD( "logicpro.r2", 0x0000, 0x10000, CRC(000d624b) SHA1(c0da218ee81d01b3dcef2159bbaaff5d3ddb7619) )

	ROM_REGION( 0x180000, "gfx1", 0 )   /* chars */
	ROM_LOAD( "logicpro.r5", 0x000000, 0x080000, CRC(dedf18c9) SHA1(9725e096427f03ed5fd81584c0aa85a53f9681c9) )
	ROM_LOAD( "logicpro.r6", 0x080000, 0x080000, CRC(3ecbd1c2) SHA1(dd6afacd58eaaa2562e007a92b6667ecc968377d) )
	ROM_LOAD( "logicpro.r7", 0x100000, 0x080000, CRC(47135521) SHA1(ee6a93332190fc966f8e820430d652942f030b00) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites, used at run time */
	ROM_LOAD16_BYTE( "logicpro.r9", 0x000000, 0x080000, CRC(a98bc1d2) SHA1(f4aed07cccca892f3d3a91546b3a98fbe3e66d9c) )
	ROM_LOAD16_BYTE( "logicpro.r8", 0x000001, 0x080000, CRC(1de46298) SHA1(3385a2956d9a427c85554f39c8d85922bbeb1ce1) )

	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "logicpro.r1", 0x0000, 0x080000, CRC(a1fec4d4) SHA1(4390cd18b4a7de2d8cb68270180ea3de42fd2282) )
ROM_END

ROM_START( karianx )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "even",        0x00000, 0x80000, CRC(fd0ce238) SHA1(4b727366c942c62187d8700666b42a85c059c060) )
	ROM_LOAD16_BYTE( "odd",         0x00001, 0x80000, CRC(be173cdc) SHA1(13230b6129fd1910257624a69a3a4b74696e982e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound */
	ROM_LOAD( "snd",         0x0000, 0x10000, CRC(fedd3375) SHA1(09fb2d5fc91704120f757acf9fa00d149f891a28) )

	ROM_REGION( 0x180000, "gfx1", 0 )   /* chars */
	ROM_LOAD( "bkg1",        0x000000, 0x080000, CRC(5cb8558a) SHA1(9c6024c70a0f0cd529a0e2e853e467ec8d8ab446) )
	ROM_LOAD( "bkg2",        0x080000, 0x080000, CRC(95ff297c) SHA1(28f6c005e73e1680bd8be7ce355fa0d404827105) )
	ROM_LOAD( "bkg3",        0x100000, 0x080000, CRC(6c81f1b2) SHA1(14ef907a9c381b7ef45441d480bb4ccb015e474b) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites, used at run time */
	ROM_LOAD16_BYTE( "obj4",        0x000000, 0x080000, CRC(5f8d75a9) SHA1(0552d046742aeb2fee176887156e73480c75a1bd) )
	ROM_LOAD16_BYTE( "obj1",        0x000001, 0x080000, CRC(967ee97d) SHA1(689f2da67eab86653b846fada39139792cd4aee2) )
	ROM_LOAD16_BYTE( "obj5",        0x100000, 0x080000, CRC(e9fc22f9) SHA1(a1f7f779520346406949500e3224c0c42cbbe026) )
	ROM_LOAD16_BYTE( "obj2",        0x100001, 0x080000, CRC(d39eb04e) SHA1(c59c3e14a506cb04d09cc7eec9962daa242a0b0c) )
	ROM_LOAD16_BYTE( "obj6",        0x200000, 0x080000, CRC(c1ec35a5) SHA1(bf59f4c3de081c8cc398c825fc1f3e8577641f10) )
	ROM_LOAD16_BYTE( "obj3",        0x200001, 0x080000, CRC(6ac1ac87) SHA1(1954e25ac5489a8eca137b86c89c415f1fed360c) )

	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "voi",         0x0000, 0x080000, CRC(c6506a80) SHA1(121229c501bd5678e55c7342619743c773a01a7e) )
ROM_END

ROM_START( logicpr2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lp2-2",       0x00000, 0x80000, CRC(cc1880bf) SHA1(5ea542b63947a570aaf924f7ab739e060e359af8) )
	ROM_LOAD16_BYTE( "lp2-1",       0x00001, 0x80000, CRC(46d5e954) SHA1(7bf5ae19caeecd2123754698276bbc78d68984d9) )

	ROM_REGION( 0x180000, "gfx1", 0 )   /* chars */
	ROM_LOAD( "log2-b01",    0x000000, 0x080000, CRC(fe789e07) SHA1(c3d542564519fd807bc605029f5a2cca571eec9f) )
	ROM_LOAD( "log2-b02",    0x080000, 0x080000, CRC(1e0c51cd) SHA1(c25b3259a173e77785dcee1407ddf191c3efad79) )
	ROM_LOAD( "log2-b03",    0x100000, 0x080000, CRC(916f2928) SHA1(8c73408664dcd3de42cb27fac0d22b87b540bf52) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites, used at run time */
	ROM_LOAD16_WORD_SWAP( "obj",         0x000000, 0x400000, CRC(f221f305) SHA1(aa1d3d86d13e009bfb44cbc6ff4401b811b19f97) )

	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "log2-s01",    0x0000, 0x100000, CRC(2875c435) SHA1(633538d9ac53228ea344605482ac387852c29193) )
ROM_END



GAME( 1996, logicpro, 0,        deniam16b, logicpr2, deniam_state, logicpro, ROT0, "Deniam", "Logic Pro (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, croquis,  logicpro, deniam16b, logicpr2, deniam_state, logicpro, ROT0, "Deniam", "Croquis (Germany)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, karianx,  0,        deniam16b, karianx, deniam_state,  karianx,  ROT0, "Deniam", "Karian Cross (Rev. 1.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, logicpr2, 0,        deniam16c, logicpr2, deniam_state, logicpro, ROT0, "Deniam", "Logic Pro 2 (Japan)", MACHINE_SUPPORTS_SAVE )
