// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail
/***************************************************************

 Pro Baseball Skill Tryout (JPN Ver.)
 (c) 1985 Data East

 Driver by Pierpaolo Prazzoli and Bryan McPhail

 TODO:
 - Fix sprite position in cocktail mode

=================================================================
Debug cheats:

*tryout
Pitching stage (3)
$201 remaining ball counter
$208 strikes count
(note: put a wpset 20x,1,w and modify the value AFTER that the
       game modifies it)

****************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "includes/tryout.h"

WRITE8_MEMBER(tryout_state::nmi_ack_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE );
}

WRITE8_MEMBER(tryout_state::sound_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

/*this is actually irq/nmi mask, polls only four values at start up (81->01->81->01) and then
  stays on this state.*/
WRITE8_MEMBER(tryout_state::sound_irq_ack_w)
{
//  m_audiocpu->set_input_line(0, CLEAR_LINE);
}

void tryout_state::machine_start()
{
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x2000);
}

WRITE8_MEMBER(tryout_state::bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x01);
}

static ADDRESS_MAP_START( main_cpu, AS_PROGRAM, 8, tryout_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1000, 0x17ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0xbfff) AM_ROM
	AM_RANGE(0xc800, 0xc87f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xcc00, 0xcc7f) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0xd000, 0xd7ff) AM_READWRITE(vram_r, vram_w)
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("DSW")
	AM_RANGE(0xe001, 0xe001) AM_READ_PORT("P1")
	AM_RANGE(0xe002, 0xe002) AM_READ_PORT("P2")
	AM_RANGE(0xe003, 0xe003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xe301, 0xe301) AM_WRITE(flipscreen_w)
	AM_RANGE(0xe302, 0xe302) AM_WRITE(bankswitch_w)
	AM_RANGE(0xe401, 0xe401) AM_WRITE(vram_bankswitch_w)
	AM_RANGE(0xe402, 0xe404) AM_WRITEONLY AM_SHARE("gfx_control")
	AM_RANGE(0xe414, 0xe414) AM_WRITE(sound_w)
	AM_RANGE(0xe417, 0xe417) AM_WRITE(nmi_ack_w)
	AM_RANGE(0xfff0, 0xffff) AM_ROM AM_REGION("maincpu", 0xbff0) /* reset vectors */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu, AS_PROGRAM, 8, tryout_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x4000, 0x4001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(sound_irq_ack_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

INPUT_CHANGED_MEMBER(tryout_state::coin_inserted)
{
	if (oldval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

static INPUT_PORTS_START( tryout )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tryout_state, coin_inserted, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tryout_state, coin_inserted, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,2),
	2,  /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 3, 2, 1, 0, RGN_FRAC(1,2)+3, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout vramlayout =
{
	16, 16,
	128,
	3,
	{ 0x0000 * 8, 0x2000 * 8, 0x4000 * 8 },
	{ 7, 6, 5, 4, 128+7, 128+6, 128+5, 128+4, 256+7, 256+6, 256+5, 256+4, 384+7, 384+6, 384+5, 384+4  },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
		7*8, 6*8,5*8,4*8,3*8,2*8,1*8,0*8 },
	64*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	32*8
};

static GFXDECODE_START( tryout )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 4 )
	GFXDECODE_ENTRY( 0x0000,   0, vramlayout,   0, 4 )
GFXDECODE_END

static MACHINE_CONFIG_START( tryout, tryout_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 2000000)     /* ? */
	MCFG_CPU_PROGRAM_MAP(main_cpu)

	MCFG_CPU_ADD("audiocpu", M6502, 1500000)    /* ? */
	MCFG_CPU_PROGRAM_MAP(sound_cpu)
	MCFG_CPU_PERIODIC_INT_DRIVER(tryout_state, nmi_line_pulse, 1000) /* controls BGM tempo, 1000 is an hand-tuned value to match a side-by-side video */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(tryout_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tryout)
	MCFG_PALETTE_ADD("palette", 0x20)
	MCFG_PALETTE_INIT_OWNER(tryout_state, tryout)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( tryout )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "ch10-1.bin",   0x04000, 0x4000, CRC(d046231b) SHA1(145f9e9b0707824f7ae6d1587754b28c17907807) )
	ROM_LOAD( "ch11.bin",     0x08000, 0x4000, CRC(4d00b6f0) SHA1(cc1e700b8547672d7dd1d262c6181a5c321fbf72) )
	ROM_LOAD( "ch12.bin",     0x10000, 0x4000, CRC(bcd221be) SHA1(69869de8b5d56a97e2cd15fa275527aa767f1e44) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ch00-1.bin",   0x0c000, 0x4000, CRC(8b33d968) SHA1(cf44529e5577d09978b87dc2bbe1415babbf36a0) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ch13.bin",     0x00000, 0x4000, CRC(a9619c58) SHA1(92528b1c4afc95394ac8cad5b37f23da0c6a5310) )

	ROM_REGION( 0x24000, "gfx2", 0 )
	ROM_LOAD( "ch09.bin",     0x00000, 0x4000, CRC(9c5e275b) SHA1(83b29996573d85c73bb4b63086c7a624fad19bde) )
	ROM_LOAD( "ch08.bin",     0x04000, 0x4000, CRC(88396abb) SHA1(2865a265ddfb91c2ad2770da5e0d84a544f3c419) )
	ROM_LOAD( "ch07.bin",     0x08000, 0x4000, CRC(901b5f5e) SHA1(f749b5ec0c51c66655798e8a37c887870370991e) )
	ROM_LOAD( "ch06.bin",     0x0c000, 0x4000, CRC(d937e326) SHA1(5870a82b02438f2fdae089f6d1b8e9ce13d213a6) )
	ROM_LOAD( "ch05.bin",     0x10000, 0x4000, CRC(27f0e7be) SHA1(5fa2bd666d012addfb836d009f962f89e4a00b2d) )
	ROM_LOAD( "ch04.bin",     0x14000, 0x4000, CRC(019e0b75) SHA1(4bfd7cd6c28ec6dfaf8e9bf009716e92759f06c2) )
	ROM_LOAD( "ch03.bin",     0x18000, 0x4000, CRC(b87e2464) SHA1(0089c0ff421929345a1d21951789a6374e0019ff) )
	ROM_LOAD( "ch02.bin",     0x1c000, 0x4000, CRC(62369772) SHA1(89f360003e916bee76d74b7e046bf08349726fda) )
	ROM_LOAD( "ch01.bin",     0x20000, 0x4000, CRC(ee6d57b5) SHA1(7dd2f3b962f088fcbc40fcb74c0a56783857fb7b) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ch14.bpr",     0x00000, 0x0020, CRC(8ce19925) SHA1(12f8f6022f1148b6ba1d019a34247452637063a7) )
ROM_END

GAME( 1985, tryout, 0, tryout, tryout, driver_device, 0, ROT90, "Data East Corporation", "Pro Baseball Skill Tryout (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
