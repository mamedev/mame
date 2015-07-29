// license:???
// copyright-holders:Paul Leaman
/***************************************************************************

  Speed Rumbler

  Driver provided by Paul Leaman

  M6809 for game, Z80 and YM-2203 for sound.

2009/06 - DIP Location and Defaults verified against Speed Rumbler manual.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "includes/srumbler.h"


WRITE8_MEMBER(srumbler_state::bankswitch_w)
{
	/*
	  banking is controlled by two PROMs. 0000-4fff is mapped to the same
	  address (RAM and I/O) for all banks, so we don't handle it here.
	  e000-ffff is all mapped to the same ROMs, however we do handle it
	  here anyway.
	  Note that 5000-8fff can be either ROM or RAM, so we should handle
	  that as well to be 100% accurate.
	 */
	UINT8 *prom1 = memregion("proms")->base() + (data & 0xf0);
	UINT8 *prom2 = memregion("proms")->base() + 0x100 + ((data & 0x0f) << 4);

	for (int i = 0x05;i < 0x10;i++)
	{
		/* bit 2 of prom1 selects ROM or RAM - not supported */
		int bank = ((prom1[i] & 0x03) << 4) | (prom2[i] & 0x0f);

		char bankname[10];
		sprintf(bankname, "%04x", i*0x1000);
		membank(bankname)->set_entry(bank);
	}
}

void srumbler_state::machine_start()
{
	for (int i = 0x05; i < 0x10; i++)
	{
		char bankname[10];
		sprintf(bankname, "%04x", i*0x1000);
		membank(bankname)->configure_entries(0, 64, memregion("user1")->base(), 0x1000);
	}

	/* initialize banked ROM pointers */
	bankswitch_w(m_maincpu->space(AS_PROGRAM), 0, 0);
}

TIMER_DEVICE_CALLBACK_MEMBER(srumbler_state::interrupt)
{
	int scanline = param;

	if (scanline == 248)
		m_maincpu->set_input_line(0,HOLD_LINE);

	if (scanline == 0)
		m_maincpu->set_input_line(M6809_FIRQ_LINE,HOLD_LINE);
}

/*
The "scroll test" routine on the test screen appears to overflow and write
over the control registers (0x4000-0x4080) when it clears the screen.

This doesn't affect anything since it happens to write the correct value
to the page register.

Ignore the warnings about writing to unmapped memory.
*/

static ADDRESS_MAP_START( srumbler_map, AS_PROGRAM, 8, srumbler_state )
	AM_RANGE(0x0000, 0x1dff) AM_RAM  /* RAM (of 1 sort or another) */
	AM_RANGE(0x1e00, 0x1fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x2000, 0x3fff) AM_RAM_WRITE(background_w) AM_SHARE("backgroundram")
	AM_RANGE(0x4008, 0x4008) AM_READ_PORT("SYSTEM") AM_WRITE(bankswitch_w)
	AM_RANGE(0x4009, 0x4009) AM_READ_PORT("P1") AM_WRITE(_4009_w)
	AM_RANGE(0x400a, 0x400a) AM_READ_PORT("P2")
	AM_RANGE(0x400b, 0x400b) AM_READ_PORT("DSW1")
	AM_RANGE(0x400c, 0x400c) AM_READ_PORT("DSW2")
	AM_RANGE(0x400a, 0x400d) AM_WRITE(scroll_w)
	AM_RANGE(0x400e, 0x400e) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x5000, 0x5fff) AM_ROMBANK("5000") AM_WRITE(foreground_w) AM_SHARE("foregroundram") /* Banked ROM */
	AM_RANGE(0x6000, 0x6fff) AM_ROMBANK("6000") /* Banked ROM */
	AM_RANGE(0x6000, 0x6fff) AM_WRITENOP        /* Video RAM 2 ??? (not used) */
	AM_RANGE(0x7000, 0x7fff) AM_ROMBANK("7000") /* Banked ROM */
	AM_RANGE(0x7000, 0x73ff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x8000, 0x8fff) AM_ROMBANK("8000") /* Banked ROM */
	AM_RANGE(0x9000, 0x9fff) AM_ROMBANK("9000") /* Banked ROM */
	AM_RANGE(0xa000, 0xafff) AM_ROMBANK("a000") /* Banked ROM */
	AM_RANGE(0xb000, 0xbfff) AM_ROMBANK("b000") /* Banked ROM */
	AM_RANGE(0xc000, 0xcfff) AM_ROMBANK("c000") /* Banked ROM */
	AM_RANGE(0xd000, 0xdfff) AM_ROMBANK("d000") /* Banked ROM */
	AM_RANGE(0xe000, 0xefff) AM_ROMBANK("e000") /* Banked ROM */
	AM_RANGE(0xf000, 0xffff) AM_ROMBANK("f000") /* Banked ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( srumbler_sound_map, AS_PROGRAM, 8, srumbler_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0xa000, 0xa001) AM_DEVWRITE("ym2", ym2203_device, write)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( srumbler )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )  PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_SERVICE_DIPLOC(  0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k 70k and every 70k" )
	PORT_DIPSETTING(    0x10, "30k 80k and every 80k" )
	PORT_DIPSETTING(    0x08, "20k 80k" )
	PORT_DIPSETTING(    0x00, "30k 80k" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0,8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			2*64+0, 2*64+1, 2*64+2, 2*64+3, 2*64+4, 2*64+5, 2*64+6, 2*64+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};


static GFXDECODE_START( srumbler )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   448, 16 ) /* colors 448 - 511 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   128,  8 ) /* colors 128 - 255 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 256,  8 ) /* colors 256 - 383 */
GFXDECODE_END



static MACHINE_CONFIG_START( srumbler, srumbler_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 1500000)        /* 1.5 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(srumbler_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", srumbler_state, interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, 3000000)        /* 3 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(srumbler_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(srumbler_state, irq0_line_hold, 4*60)


	/* video hardware */
	MCFG_BUFFERED_SPRITERAM8_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(10*8, (64-10)*8-1, 1*8, 31*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(srumbler_state, screen_update)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram8_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", srumbler)

	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 4000000)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 0.30)

	MCFG_SOUND_ADD("ym2", YM2203, 4000000)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 0.30)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( srumbler )
	ROM_REGION( 0x40000, "user1", 0 ) /* Paged ROMs */
	ROM_LOAD( "rc04.14e",   0x00000, 0x08000, CRC(a68ce89c) SHA1(cb5dd8c47c24f9d8ac9a6135c0b7942d16002d25) )
	ROM_LOAD( "rc03.13e",   0x08000, 0x08000, CRC(87bda812) SHA1(f46dcce21d78c8525a2578b73e05b7cd8a2d8745) ) // sldh
	ROM_LOAD( "rc02.12e",   0x10000, 0x08000, CRC(d8609cca) SHA1(893f1f1ac0aef5d31e75228252c14c4b522bff16) ) // sldh
	ROM_LOAD( "rc01.11e",   0x18000, 0x08000, CRC(27ec4776) SHA1(09a53fd6472888664c21f49ab78b2c5d77d2caa1) ) // sldh
	ROM_LOAD( "rc09.14f",   0x20000, 0x08000, CRC(2146101d) SHA1(cacd7a13d67f43a0fc624d1c5e29d9816bd6b1c7) ) // sldh
	ROM_LOAD( "rc08.13f",   0x28000, 0x08000, CRC(838369a6) SHA1(6fdcbe2db488d4d99453b5537cf05ed18112368e) ) // sldh
	ROM_LOAD( "rc07.12f",   0x30000, 0x08000, CRC(de785076) SHA1(bdb104c6c875f5362c0d1ba9a8c5dd450c9c014b) )
	ROM_LOAD( "rc06.11f",   0x38000, 0x08000, CRC(a70f4fd4) SHA1(21be3865b9f7fa265f265a565bab896357d7464f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rc05.2f",    0x00000, 0x08000, CRC(0177cebe) SHA1(0fa94d2057f509a6fe1de210bf513efc82f1ffe7) ) // sldh

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "rc10.6g",    0x00000, 0x04000, CRC(adabe271) SHA1(256d6823dcda404375825103272213e1442c3320) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "rc11.11a",   0x00000, 0x08000, CRC(5fa042ba) SHA1(9e03eaf22286330826501619a7b74181dc42a5fa) ) /* tiles */
	ROM_LOAD( "rc12.13a",   0x08000, 0x08000, CRC(a2db64af) SHA1(35ab93397ee8172813e69edd085b36a5b98ba082) )
	ROM_LOAD( "rc13.14a",   0x10000, 0x08000, CRC(f1df5499) SHA1(b1c47b35c00bc05825353474ad2b33d9669b879e) )
	ROM_LOAD( "rc14.15a",   0x18000, 0x08000, CRC(b22b31b3) SHA1(7aa1a042bccf6a1117c983bb36e88ace7712e867) )
	ROM_LOAD( "rc15.11c",   0x20000, 0x08000, CRC(ca3a3af3) SHA1(bcb43fc66be852acb2f93513d6b7b089b1bdd9fc) )
	ROM_LOAD( "rc16.13c",   0x28000, 0x08000, CRC(c49a4a11) SHA1(4e1432e6d6a7ffc73e695c1db245ea54beee0507) )
	ROM_LOAD( "rc17.14c",   0x30000, 0x08000, CRC(aa80aaab) SHA1(37a8e57e4d8ed8372bc1d7c94cf5a087a01d79ad) )
	ROM_LOAD( "rc18.15c",   0x38000, 0x08000, CRC(ce67868e) SHA1(867d6bc65119fdb7a9788f7d92e6be0326756776) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "rc20.15e",   0x00000, 0x08000, CRC(3924c861) SHA1(e31e0ea50823a910f87eefc969de53f1ad738629) ) /* sprites */
	ROM_LOAD( "rc19.14e",   0x08000, 0x08000, CRC(ff8f9129) SHA1(8402236e297c3b03984a22b727198cc54e0c8117) )
	ROM_LOAD( "rc22.15f",   0x10000, 0x08000, CRC(ab64161c) SHA1(4d8b01ba4c85a732df38db7663bd765a49c671de) )
	ROM_LOAD( "rc21.14f",   0x18000, 0x08000, CRC(fd64bcd1) SHA1(4bb6c0e0027387284de1dc1320887de3231252e9) )
	ROM_LOAD( "rc24.15h",   0x20000, 0x08000, CRC(c972af3e) SHA1(3aaf5fdd07f675bd29a068035f252c0136e7881e) )
	ROM_LOAD( "rc23.14h",   0x28000, 0x08000, CRC(8c9abf57) SHA1(044d5c9904e89b67bd92396a7215b73d96d3965a) )
	ROM_LOAD( "rc26.15j",   0x30000, 0x08000, CRC(d4f1732f) SHA1(d16b6456c73395964c9868078e378e0d5cc48ae7) )
	ROM_LOAD( "rc25.14j",   0x38000, 0x08000, CRC(d2a4ea4f) SHA1(365e534bf56e08b1e727ea7bfdfb537fa274448b) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "63s141.12a", 0x0000, 0x0100, CRC(8421786f) SHA1(7ffe9f3cd081842d9ee38bd67421cb8836e3f7ed) ) /* ROM banking */
	ROM_LOAD( "63s141.13a", 0x0100, 0x0100, CRC(6048583f) SHA1(a0b0f560e7f52978a1bf59417da13cc852617eff) ) /* ROM banking */
	ROM_LOAD( "63s141.8j",  0x0200, 0x0100, CRC(1a89a7ff) SHA1(437160ad5d61a257b7deaf5f5e8b3d4cf56a9663) ) /* priority (not used) */
ROM_END

ROM_START( srumbler2 )
	ROM_REGION( 0x40000, "user1", 0 ) /* Paged ROMs */
	ROM_LOAD( "rc04.14e",   0x00000, 0x08000, CRC(a68ce89c) SHA1(cb5dd8c47c24f9d8ac9a6135c0b7942d16002d25) )
	ROM_LOAD( "rc03.13e",   0x08000, 0x08000, CRC(e82f78d4) SHA1(39cb5d9c18e7635d48aa29221ae99e6a500e2841) ) // sldh
	ROM_LOAD( "rc02.12e",   0x10000, 0x08000, CRC(009a62d8) SHA1(72b52b34186304d70214f56acdb0f3af5bed9503) )
	ROM_LOAD( "rc01.11e",   0x18000, 0x08000, CRC(2ac48d1d) SHA1(9e41cddb8f8f96e55f915ae5c244c123cc4f8c9a) )
	ROM_LOAD( "rc09.14f",   0x20000, 0x08000, CRC(64f23e72) SHA1(2de892f8753df0ac85389328342089bd5cc57f38) )
	ROM_LOAD( "rc08.13f",   0x28000, 0x08000, CRC(74c71007) SHA1(9b7f159dc1e3add85a3aaeb697aa800a37d09f52) ) // sldh
	ROM_LOAD( "rc07.12f",   0x30000, 0x08000, CRC(de785076) SHA1(bdb104c6c875f5362c0d1ba9a8c5dd450c9c014b) )
	ROM_LOAD( "rc06.11f",   0x38000, 0x08000, CRC(a70f4fd4) SHA1(21be3865b9f7fa265f265a565bab896357d7464f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rc05.2f",    0x00000, 0x08000, CRC(ea04fa07) SHA1(e29bfc3ed9e6606206ee41c90aaaeddffa26c1b4) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "rc10.6g",    0x00000, 0x04000, CRC(adabe271) SHA1(256d6823dcda404375825103272213e1442c3320) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "rc11.11a",   0x00000, 0x08000, CRC(5fa042ba) SHA1(9e03eaf22286330826501619a7b74181dc42a5fa) ) /* tiles */
	ROM_LOAD( "rc12.13a",   0x08000, 0x08000, CRC(a2db64af) SHA1(35ab93397ee8172813e69edd085b36a5b98ba082) )
	ROM_LOAD( "rc13.14a",   0x10000, 0x08000, CRC(f1df5499) SHA1(b1c47b35c00bc05825353474ad2b33d9669b879e) )
	ROM_LOAD( "rc14.15a",   0x18000, 0x08000, CRC(b22b31b3) SHA1(7aa1a042bccf6a1117c983bb36e88ace7712e867) )
	ROM_LOAD( "rc15.11c",   0x20000, 0x08000, CRC(ca3a3af3) SHA1(bcb43fc66be852acb2f93513d6b7b089b1bdd9fc) )
	ROM_LOAD( "rc16.13c",   0x28000, 0x08000, CRC(c49a4a11) SHA1(4e1432e6d6a7ffc73e695c1db245ea54beee0507) )
	ROM_LOAD( "rc17.14c",   0x30000, 0x08000, CRC(aa80aaab) SHA1(37a8e57e4d8ed8372bc1d7c94cf5a087a01d79ad) )
	ROM_LOAD( "rc18.15c",   0x38000, 0x08000, CRC(ce67868e) SHA1(867d6bc65119fdb7a9788f7d92e6be0326756776) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "rc20.15e",   0x00000, 0x08000, CRC(3924c861) SHA1(e31e0ea50823a910f87eefc969de53f1ad738629) ) /* sprites */
	ROM_LOAD( "rc19.14e",   0x08000, 0x08000, CRC(ff8f9129) SHA1(8402236e297c3b03984a22b727198cc54e0c8117) )
	ROM_LOAD( "rc22.15f",   0x10000, 0x08000, CRC(ab64161c) SHA1(4d8b01ba4c85a732df38db7663bd765a49c671de) )
	ROM_LOAD( "rc21.14f",   0x18000, 0x08000, CRC(fd64bcd1) SHA1(4bb6c0e0027387284de1dc1320887de3231252e9) )
	ROM_LOAD( "rc24.15h",   0x20000, 0x08000, CRC(c972af3e) SHA1(3aaf5fdd07f675bd29a068035f252c0136e7881e) )
	ROM_LOAD( "rc23.14h",   0x28000, 0x08000, CRC(8c9abf57) SHA1(044d5c9904e89b67bd92396a7215b73d96d3965a) )
	ROM_LOAD( "rc26.15j",   0x30000, 0x08000, CRC(d4f1732f) SHA1(d16b6456c73395964c9868078e378e0d5cc48ae7) )
	ROM_LOAD( "rc25.14j",   0x38000, 0x08000, CRC(d2a4ea4f) SHA1(365e534bf56e08b1e727ea7bfdfb537fa274448b) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "63s141.12a", 0x0000, 0x0100, CRC(8421786f) SHA1(7ffe9f3cd081842d9ee38bd67421cb8836e3f7ed) ) /* ROM banking */
	ROM_LOAD( "63s141.13a", 0x0100, 0x0100, CRC(6048583f) SHA1(a0b0f560e7f52978a1bf59417da13cc852617eff) ) /* ROM banking */
	ROM_LOAD( "63s141.8j",  0x0200, 0x0100, CRC(1a89a7ff) SHA1(437160ad5d61a257b7deaf5f5e8b3d4cf56a9663) ) /* priority (not used) */
ROM_END

ROM_START( srumbler3 )
	ROM_REGION( 0x40000, "user1", 0 ) /* Paged ROMs */
	ROM_LOAD( "rc04.14e",   0x00000, 0x08000, CRC(a68ce89c) SHA1(cb5dd8c47c24f9d8ac9a6135c0b7942d16002d25) )
	ROM_LOAD( "rc03.13e",   0x08000, 0x08000, CRC(0a21992b) SHA1(6096313210ae729b1c2a27a581473b06c60f5611) ) // sldh
	ROM_LOAD( "rc02.12e",   0x10000, 0x08000, CRC(009a62d8) SHA1(72b52b34186304d70214f56acdb0f3af5bed9503) )
	ROM_LOAD( "rc01.11e",   0x18000, 0x08000, CRC(2ac48d1d) SHA1(9e41cddb8f8f96e55f915ae5c244c123cc4f8c9a) )
	ROM_LOAD( "rc09.14f",   0x20000, 0x08000, CRC(64f23e72) SHA1(2de892f8753df0ac85389328342089bd5cc57f38) )
	ROM_LOAD( "rc08.13f",   0x28000, 0x08000, CRC(e361b55c) SHA1(5f3ee4e8e6e855a4334d3599e0ef12bc7bd8c3a4) ) // sldh
	ROM_LOAD( "rc07.12f",   0x30000, 0x08000, CRC(de785076) SHA1(bdb104c6c875f5362c0d1ba9a8c5dd450c9c014b) )
	ROM_LOAD( "rc06.11f",   0x38000, 0x08000, CRC(a70f4fd4) SHA1(21be3865b9f7fa265f265a565bab896357d7464f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rc05.2f",    0x00000, 0x08000, CRC(ea04fa07) SHA1(e29bfc3ed9e6606206ee41c90aaaeddffa26c1b4) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "rc10.6g",    0x00000, 0x04000, CRC(adabe271) SHA1(256d6823dcda404375825103272213e1442c3320) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "rc11.11a",   0x00000, 0x08000, CRC(5fa042ba) SHA1(9e03eaf22286330826501619a7b74181dc42a5fa) ) /* tiles */
	ROM_LOAD( "rc12.13a",   0x08000, 0x08000, CRC(a2db64af) SHA1(35ab93397ee8172813e69edd085b36a5b98ba082) )
	ROM_LOAD( "rc13.14a",   0x10000, 0x08000, CRC(f1df5499) SHA1(b1c47b35c00bc05825353474ad2b33d9669b879e) )
	ROM_LOAD( "rc14.15a",   0x18000, 0x08000, CRC(b22b31b3) SHA1(7aa1a042bccf6a1117c983bb36e88ace7712e867) )
	ROM_LOAD( "rc15.11c",   0x20000, 0x08000, CRC(ca3a3af3) SHA1(bcb43fc66be852acb2f93513d6b7b089b1bdd9fc) )
	ROM_LOAD( "rc16.13c",   0x28000, 0x08000, CRC(c49a4a11) SHA1(4e1432e6d6a7ffc73e695c1db245ea54beee0507) )
	ROM_LOAD( "rc17.14c",   0x30000, 0x08000, CRC(aa80aaab) SHA1(37a8e57e4d8ed8372bc1d7c94cf5a087a01d79ad) )
	ROM_LOAD( "rc18.15c",   0x38000, 0x08000, CRC(ce67868e) SHA1(867d6bc65119fdb7a9788f7d92e6be0326756776) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "rc20.15e",   0x00000, 0x08000, CRC(3924c861) SHA1(e31e0ea50823a910f87eefc969de53f1ad738629) ) /* sprites */
	ROM_LOAD( "rc19.14e",   0x08000, 0x08000, CRC(ff8f9129) SHA1(8402236e297c3b03984a22b727198cc54e0c8117) )
	ROM_LOAD( "rc22.15f",   0x10000, 0x08000, CRC(ab64161c) SHA1(4d8b01ba4c85a732df38db7663bd765a49c671de) )
	ROM_LOAD( "rc21.14f",   0x18000, 0x08000, CRC(fd64bcd1) SHA1(4bb6c0e0027387284de1dc1320887de3231252e9) )
	ROM_LOAD( "rc24.15h",   0x20000, 0x08000, CRC(c972af3e) SHA1(3aaf5fdd07f675bd29a068035f252c0136e7881e) )
	ROM_LOAD( "rc23.14h",   0x28000, 0x08000, CRC(8c9abf57) SHA1(044d5c9904e89b67bd92396a7215b73d96d3965a) )
	ROM_LOAD( "rc26.15j",   0x30000, 0x08000, CRC(d4f1732f) SHA1(d16b6456c73395964c9868078e378e0d5cc48ae7) )
	ROM_LOAD( "rc25.14j",   0x38000, 0x08000, CRC(d2a4ea4f) SHA1(365e534bf56e08b1e727ea7bfdfb537fa274448b) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "63s141.12a", 0x0000, 0x0100, CRC(8421786f) SHA1(7ffe9f3cd081842d9ee38bd67421cb8836e3f7ed) ) /* ROM banking */
	ROM_LOAD( "63s141.13a", 0x0100, 0x0100, CRC(6048583f) SHA1(a0b0f560e7f52978a1bf59417da13cc852617eff) ) /* ROM banking */
	ROM_LOAD( "63s141.8j",  0x0200, 0x0100, CRC(1a89a7ff) SHA1(437160ad5d61a257b7deaf5f5e8b3d4cf56a9663) ) /* priority (not used) */
ROM_END

ROM_START( rushcrsh )
	ROM_REGION( 0x40000, "user1", 0 ) /* Paged ROMs */
	ROM_LOAD( "rc04.14e",   0x00000, 0x08000, CRC(a68ce89c) SHA1(cb5dd8c47c24f9d8ac9a6135c0b7942d16002d25) )
	ROM_LOAD( "rc03.13e",   0x08000, 0x08000, CRC(a49c9be0) SHA1(9aa385063a289e71fef4c2846c8c960a8adafcc0) )
	ROM_LOAD( "rc02.12e",   0x10000, 0x08000, CRC(009a62d8) SHA1(72b52b34186304d70214f56acdb0f3af5bed9503) )
	ROM_LOAD( "rc01.11e",   0x18000, 0x08000, CRC(2ac48d1d) SHA1(9e41cddb8f8f96e55f915ae5c244c123cc4f8c9a) )
	ROM_LOAD( "rc09.14f",   0x20000, 0x08000, CRC(64f23e72) SHA1(2de892f8753df0ac85389328342089bd5cc57f38) )
	ROM_LOAD( "rc08.13f",   0x28000, 0x08000, CRC(2c25874b) SHA1(7862f0af14c508f598a2f05330a61b77b86d624e) )
	ROM_LOAD( "rc07.12f",   0x30000, 0x08000, CRC(de785076) SHA1(bdb104c6c875f5362c0d1ba9a8c5dd450c9c014b) )
	ROM_LOAD( "rc06.11f",   0x38000, 0x08000, CRC(a70f4fd4) SHA1(21be3865b9f7fa265f265a565bab896357d7464f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rc05.2f",    0x00000, 0x08000, CRC(ea04fa07) SHA1(e29bfc3ed9e6606206ee41c90aaaeddffa26c1b4) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "rc10.6g",    0x00000, 0x04000, CRC(0a3c0b0d) SHA1(63f4daaea852c077f0ddd04d4bb4cd6333a8de7c) ) /* characters */ // sldh

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "rc11.11a",   0x00000, 0x08000, CRC(5fa042ba) SHA1(9e03eaf22286330826501619a7b74181dc42a5fa) ) /* tiles */
	ROM_LOAD( "rc12.13a",   0x08000, 0x08000, CRC(a2db64af) SHA1(35ab93397ee8172813e69edd085b36a5b98ba082) )
	ROM_LOAD( "rc13.14a",   0x10000, 0x08000, CRC(f1df5499) SHA1(b1c47b35c00bc05825353474ad2b33d9669b879e) )
	ROM_LOAD( "rc14.15a",   0x18000, 0x08000, CRC(b22b31b3) SHA1(7aa1a042bccf6a1117c983bb36e88ace7712e867) )
	ROM_LOAD( "rc15.11c",   0x20000, 0x08000, CRC(ca3a3af3) SHA1(bcb43fc66be852acb2f93513d6b7b089b1bdd9fc) )
	ROM_LOAD( "rc16.13c",   0x28000, 0x08000, CRC(c49a4a11) SHA1(4e1432e6d6a7ffc73e695c1db245ea54beee0507) )
	ROM_LOAD( "rc17.14c",   0x30000, 0x08000, CRC(aa80aaab) SHA1(37a8e57e4d8ed8372bc1d7c94cf5a087a01d79ad) )
	ROM_LOAD( "rc18.15c",   0x38000, 0x08000, CRC(ce67868e) SHA1(867d6bc65119fdb7a9788f7d92e6be0326756776) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "rc20.15e",   0x00000, 0x08000, CRC(3924c861) SHA1(e31e0ea50823a910f87eefc969de53f1ad738629) ) /* sprites */
	ROM_LOAD( "rc19.14e",   0x08000, 0x08000, CRC(ff8f9129) SHA1(8402236e297c3b03984a22b727198cc54e0c8117) )
	ROM_LOAD( "rc22.15f",   0x10000, 0x08000, CRC(ab64161c) SHA1(4d8b01ba4c85a732df38db7663bd765a49c671de) )
	ROM_LOAD( "rc21.14f",   0x18000, 0x08000, CRC(fd64bcd1) SHA1(4bb6c0e0027387284de1dc1320887de3231252e9) )
	ROM_LOAD( "rc24.15h",   0x20000, 0x08000, CRC(c972af3e) SHA1(3aaf5fdd07f675bd29a068035f252c0136e7881e) )
	ROM_LOAD( "rc23.14h",   0x28000, 0x08000, CRC(8c9abf57) SHA1(044d5c9904e89b67bd92396a7215b73d96d3965a) )
	ROM_LOAD( "rc26.15j",   0x30000, 0x08000, CRC(d4f1732f) SHA1(d16b6456c73395964c9868078e378e0d5cc48ae7) )
	ROM_LOAD( "rc25.14j",   0x38000, 0x08000, CRC(d2a4ea4f) SHA1(365e534bf56e08b1e727ea7bfdfb537fa274448b) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "63s141.12a", 0x0000, 0x0100, CRC(8421786f) SHA1(7ffe9f3cd081842d9ee38bd67421cb8836e3f7ed) ) /* ROM banking */
	ROM_LOAD( "63s141.13a", 0x0100, 0x0100, CRC(6048583f) SHA1(a0b0f560e7f52978a1bf59417da13cc852617eff) ) /* ROM banking */
	ROM_LOAD( "63s141.8j",  0x0200, 0x0100, CRC(1a89a7ff) SHA1(437160ad5d61a257b7deaf5f5e8b3d4cf56a9663) ) /* priority (not used) */
ROM_END



GAME( 1986, srumbler,  0,        srumbler, srumbler, driver_device, 0, ROT270, "Capcom", "The Speed Rumbler (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, srumbler2, srumbler, srumbler, srumbler, driver_device, 0, ROT270, "Capcom", "The Speed Rumbler (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, srumbler3, srumbler, srumbler, srumbler, driver_device, 0, ROT270, "Capcom (Tecfri license)", "The Speed Rumbler (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, rushcrsh,  srumbler, srumbler, srumbler, driver_device, 0, ROT270, "Capcom", "Rush & Crash (Japan)", MACHINE_SUPPORTS_SAVE )
