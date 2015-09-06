// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Success Joe / Ashita no Joe [Wave]

driver by David Haywood and bits from Pierpaolo Prazzoli


Upper board marked: W9011

Sticker on the upper pcb of Success Joe (World) labelled:
M6100506A
SUCCESS JOE
900110149

Two sub-boards:

Upper:

 Program
  ROMS 1 to 8 (ST M27512)
  Standard Motorola MC68000P8
  8.0000 MHz osc.
  PALs W9011A (AMD PALCE16V8H) + W9011B (MMI PAL 16L88CN)

 Sound
  ROM 9 (ST M27256)
  Standard Zilog Z80 (Z0840004PSC)
  Yamaha YM2203C

 GFX?
  Mask ROMs 401, 402 & 403 (Hitachi HN62414 Mask ROMs)

 Note: the pcb has a place for a battery circuit but the components are not soldered.

Lower:

 GFX?
  Mask ROMs 404, 405, 406, 407, 408 & 409 (Hitachi HN62414 Mask ROMs)
  PAL W90120R2 (MMI PAL 16L88CN)
  EPL (Ricoh EPL16P8BP, not dumped)
  13.3330 MHz osc.

Dips:

 Two banks (* = default)
  A
                                    1   2   3   4   5   6   7   8
   Game Style      * Table          ON                      OFF OFF
                   Upright          OFF                     OFF OFF
   Screen Reverse  * Usual              OFF                 OFF OFF
                   Reverse              ON                  OFF OFF
   Test Mode       * Game mode              OFF             OFF OFF
                   Test Mode                ON              OFF OFF
   Demo Sound      * Yes                        OFF         OFF OFF
                   No                           ON          OFF OFF
   Play Fee - Coin * 1 Coin 1 Play                  OFF OFF OFF OFF
                   1 Coin 2 Play                    ON  OFF OFF OFF
                   2 Coin 1 Play                    OFF ON  OFF OFF
                   2 Coin 3 Play                    ON  ON  OFF OFF

  B
                                    1   2   3   4   5   6   7   8
   Difficulty      * Rank B         OFF OFF OFF OFF OFF OFF OFF OFF
                   Rank A           ON  OFF OFF OFF OFF OFF OFF OFF
                   Rank C           OFF ON  OFF OFF OFF OFF OFF OFF
                   Rank D           ON  ON  OFF OFF OFF OFF OFF OFF

   Easy (A) -> Difficult (D)

Game is controled with 4-direction lever and two buttons
Coin B is not used

*************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "includes/ashnojoe.h"

READ16_MEMBER(ashnojoe_state::fake_4a00a_r)
{
	/* If it returns 1 there's no sound. Is it used to sync the game and sound?
	or just a debug enable/disable register? */
	return 0;
}

WRITE16_MEMBER(ashnojoe_state::ashnojoe_soundlatch_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_soundlatch_status = 1;
		soundlatch_byte_w(space, 0, data & 0xff);
	}
}

static ADDRESS_MAP_START( ashnojoe_map, AS_PROGRAM, 16, ashnojoe_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x040000, 0x041fff) AM_RAM_WRITE(ashnojoe_tileram3_w) AM_SHARE("tileram_3")
	AM_RANGE(0x042000, 0x043fff) AM_RAM_WRITE(ashnojoe_tileram4_w) AM_SHARE("tileram_4")
	AM_RANGE(0x044000, 0x044fff) AM_RAM_WRITE(ashnojoe_tileram5_w) AM_SHARE("tileram_5")
	AM_RANGE(0x045000, 0x045fff) AM_RAM_WRITE(ashnojoe_tileram2_w) AM_SHARE("tileram_2")
	AM_RANGE(0x046000, 0x046fff) AM_RAM_WRITE(ashnojoe_tileram6_w) AM_SHARE("tileram_6")
	AM_RANGE(0x047000, 0x047fff) AM_RAM_WRITE(ashnojoe_tileram7_w) AM_SHARE("tileram_7")
	AM_RANGE(0x048000, 0x048fff) AM_RAM_WRITE(ashnojoe_tileram_w) AM_SHARE("tileram")
	AM_RANGE(0x049000, 0x049fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x04a000, 0x04a001) AM_READ_PORT("P1")
	AM_RANGE(0x04a002, 0x04a003) AM_READ_PORT("P2")
	AM_RANGE(0x04a004, 0x04a005) AM_READ_PORT("DSW")
	AM_RANGE(0x04a006, 0x04a007) AM_WRITEONLY AM_SHARE("tilemap_reg")
	AM_RANGE(0x04a008, 0x04a009) AM_WRITE(ashnojoe_soundlatch_w)
	AM_RANGE(0x04a00a, 0x04a00b) AM_READ(fake_4a00a_r)  // ??
	AM_RANGE(0x04a010, 0x04a019) AM_WRITE(joe_tilemaps_xscroll_w)
	AM_RANGE(0x04a020, 0x04a029) AM_WRITE(joe_tilemaps_yscroll_w)
	AM_RANGE(0x04c000, 0x04ffff) AM_RAM
	AM_RANGE(0x080000, 0x0bffff) AM_ROM
ADDRESS_MAP_END


WRITE8_MEMBER(ashnojoe_state::adpcm_w)
{
	m_adpcm_byte = data;
}

READ8_MEMBER(ashnojoe_state::sound_latch_r)
{
	m_soundlatch_status = 0;
	return soundlatch_byte_r(space, 0);
}

READ8_MEMBER(ashnojoe_state::sound_latch_status_r)
{
	return m_soundlatch_status;
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, ashnojoe_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, ashnojoe_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x02, 0x02) AM_WRITE(adpcm_w)
	AM_RANGE(0x04, 0x04) AM_READ(sound_latch_r)
	AM_RANGE(0x06, 0x06) AM_READ(sound_latch_status_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( ashnojoe )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )  // anything else and the controls don't work
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )  // anything else and the controls don't work
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* unused ? */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0400, 0x0000, "Number of controller" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0400, "1" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28,
		32,36,40, 44, 48, 52, 56, 60 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};

static GFXDECODE_START( ashnojoe )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles8x8_layout, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx4", 0, tiles16x16_layout, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx5", 0, tiles16x16_layout, 0, 0x100 )
GFXDECODE_END


WRITE_LINE_MEMBER(ashnojoe_state::ym2203_irq_handler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(ashnojoe_state::ym2203_write_a)
{
	/* This gets called at 8910 startup with 0xff before the 5205 exists, causing a crash */
	if (data == 0xff)
		return;

	m_msm->reset_w(!(data & 0x01));
}

WRITE8_MEMBER(ashnojoe_state::ym2203_write_b)
{
	membank("bank4")->set_entry(data & 0x0f);
}

WRITE_LINE_MEMBER(ashnojoe_state::ashnojoe_vclk_cb)
{
	if (m_msm5205_vclk_toggle == 0)
	{
		m_msm->data_w(m_adpcm_byte >> 4);
	}
	else
	{
		m_msm->data_w(m_adpcm_byte & 0xf);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}

	m_msm5205_vclk_toggle ^= 1;
}

void ashnojoe_state::machine_start()
{
	save_item(NAME(m_adpcm_byte));
	save_item(NAME(m_soundlatch_status));
	save_item(NAME(m_msm5205_vclk_toggle));
}

void ashnojoe_state::machine_reset()
{
	m_adpcm_byte = 0;
	m_soundlatch_status = 0;
	m_msm5205_vclk_toggle = 0;
}


static MACHINE_CONFIG_START( ashnojoe, ashnojoe_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(ashnojoe_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ashnojoe_state,  irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(14*8, 50*8-1, 3*8, 29*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(ashnojoe_state, screen_update_ashnojoe)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ashnojoe)
	MCFG_PALETTE_ADD("palette", 0x1000/2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 4000000)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(ashnojoe_state, ym2203_irq_handler))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(ashnojoe_state, ym2203_write_a))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(ashnojoe_state, ym2203_write_b))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.1)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(ashnojoe_state, ashnojoe_vclk_cb))
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START( scessjoe )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "5.4q", 0x00000, 0x10000, CRC(c805f9e7) SHA1(e1e85701bde496b1fd64211b94bfb0def597ae51) )
	ROM_LOAD16_BYTE( "6.4s", 0x00001, 0x10000, CRC(eda7a537) SHA1(3bb19fbdfb6c8af4e2078958fa445ac1f4434d0d) )
	ROM_LOAD16_WORD_SWAP( "sj201-nw.6m", 0x80000, 0x40000, CRC(5a64ca42) SHA1(660b8bca21ef3c2230adce7cb7e7d1f018714f23) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 32k for Z80 code */
	ROM_LOAD( "9.8q", 0x0000, 0x8000, CRC(8767e212) SHA1(13bf927febedff9d7d164fbf0da7fb3a588c2a94) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "8.5e", 0x00000, 0x10000, CRC(9bcb160e) SHA1(1677048e5ce26562ff7ba36fcc2d0ed5a652b91e) )
	ROM_LOAD( "7.5c", 0x10000, 0x10000, CRC(b250c69d) SHA1(594b1bb94a162b07944a971b7fedddca5c37f2cb) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "4.4e", 0x00000, 0x10000, CRC(aa6336d3) SHA1(43f70cc3223f11d7929dd44b0edf0a31f5fe41c3) )
	ROM_LOAD( "3.4c", 0x10000, 0x10000, CRC(7e2d86b5) SHA1(8b8d1b9240a700e29afc109eddf6e58a0a7666a4) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "2.3m", 0x00000, 0x10000, CRC(c3254938) SHA1(fd57163f740cd4fdecca94cced91314c289741ae) )
	ROM_LOAD( "1.1m", 0x10000, 0x10000, CRC(5d16a6fa) SHA1(2af907b0fcb9ff93340de3301da4b10e945455e5) )

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD16_WORD_SWAP( "sj402-nw.8e", 0x000000, 0x80000, CRC(b6d33d06) SHA1(688ccf467a5112ec522811894e2626ab5f155903) )
	ROM_LOAD16_WORD_SWAP( "sj403-nw.7e", 0x080000, 0x80000, CRC(07143f56) SHA1(1b953c8826d3993a486eed6b9d94d37145fd2e79) )

	ROM_REGION( 0x300000, "gfx5", 0 )
	ROM_LOAD16_WORD_SWAP( "sj404-nw.7a", 0x000000, 0x80000, CRC(8f134128) SHA1(026a6076d54cd5f1d06b29c51031cb79a6b2c11d) )
	ROM_LOAD16_WORD_SWAP( "sj405-nw.7c", 0x080000, 0x80000, CRC(6fd81699) SHA1(8a4f9e47dd39b4b0213c3682da2221ca53bba658) )
	ROM_LOAD16_WORD_SWAP( "sj406-nw.7d", 0x100000, 0x80000, CRC(634e33e6) SHA1(1d6a72a4ca80cd1c1fd6ce9359c304b45091cdfe) )
	ROM_LOAD16_WORD_SWAP( "sj407-nw.7f", 0x180000, 0x80000, CRC(5c66ff06) SHA1(9923ba00679e1b47b5da63c1a13e0f8dd4c78bb5) )
	ROM_LOAD16_WORD_SWAP( "sj408-nw.7g", 0x200000, 0x80000, CRC(6a3b1ea1) SHA1(e39a6e52d930f291bf237cf9db3d4b3d2fad53e0) )
	ROM_LOAD16_WORD_SWAP( "sj409-nw.7j", 0x280000, 0x80000, CRC(d8764213) SHA1(89eadefb956863216c8e3d0380394aba35e8c856) )

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_LOAD( "sj401-nw.10r", 0x00000, 0x80000, CRC(25dfab59) SHA1(7d50159204ba05323a2442778f35192e66117dda) )
ROM_END

ROM_START( ashnojoe )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "5.bin", 0x00000, 0x10000, CRC(c61e1569) SHA1(422c18f5810539b5a9e3a9bd4e3b4d70bde8d1d5) )
	ROM_LOAD16_BYTE( "6.bin", 0x00001, 0x10000, CRC(c0a16338) SHA1(fb127b9d38f2c9807b6e23ff71935fc8a22a2e8f) )
	ROM_LOAD16_WORD_SWAP( "sj201-nw.6m", 0x80000, 0x40000, CRC(5a64ca42) SHA1(660b8bca21ef3c2230adce7cb7e7d1f018714f23) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 32k for Z80 code */
	ROM_LOAD( "9.8q", 0x0000, 0x8000, CRC(8767e212) SHA1(13bf927febedff9d7d164fbf0da7fb3a588c2a94) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "8.5e",  0x00000, 0x10000, CRC(9bcb160e) SHA1(1677048e5ce26562ff7ba36fcc2d0ed5a652b91e) )
	ROM_LOAD( "7.bin", 0x10000, 0x10000, CRC(7e1efc42) SHA1(e3c282072fdaa0b98c2a1bf25fd02c680d9ca4d7) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "4.4e", 0x00000, 0x10000, CRC(aa6336d3) SHA1(43f70cc3223f11d7929dd44b0edf0a31f5fe41c3) )
	ROM_LOAD( "3.4c", 0x10000, 0x10000, CRC(7e2d86b5) SHA1(8b8d1b9240a700e29afc109eddf6e58a0a7666a4) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "2.3m",  0x00000, 0x10000, CRC(c3254938) SHA1(fd57163f740cd4fdecca94cced91314c289741ae) )
	ROM_LOAD( "1.bin", 0x10000, 0x10000, CRC(1bf585f0) SHA1(4003941636e7fded95e880109c3c9dd1d8f28b07) )

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD16_WORD_SWAP( "sj402-nw.8e", 0x000000, 0x80000, CRC(b6d33d06) SHA1(688ccf467a5112ec522811894e2626ab5f155903) )
	ROM_LOAD16_WORD_SWAP( "sj403-nw.7e", 0x080000, 0x80000, CRC(07143f56) SHA1(1b953c8826d3993a486eed6b9d94d37145fd2e79) )

	ROM_REGION( 0x300000, "gfx5", 0 )
	ROM_LOAD16_WORD_SWAP( "sj404-nw.7a", 0x000000, 0x80000, CRC(8f134128) SHA1(026a6076d54cd5f1d06b29c51031cb79a6b2c11d) )
	ROM_LOAD16_WORD_SWAP( "sj405-nw.7c", 0x080000, 0x80000, CRC(6fd81699) SHA1(8a4f9e47dd39b4b0213c3682da2221ca53bba658) )
	ROM_LOAD16_WORD_SWAP( "sj406-nw.7d", 0x100000, 0x80000, CRC(634e33e6) SHA1(1d6a72a4ca80cd1c1fd6ce9359c304b45091cdfe) )
	ROM_LOAD16_WORD_SWAP( "sj407-nw.7f", 0x180000, 0x80000, CRC(5c66ff06) SHA1(9923ba00679e1b47b5da63c1a13e0f8dd4c78bb5) )
	ROM_LOAD16_WORD_SWAP( "sj408-nw.7g", 0x200000, 0x80000, CRC(6a3b1ea1) SHA1(e39a6e52d930f291bf237cf9db3d4b3d2fad53e0) )
	ROM_LOAD16_WORD_SWAP( "sj409-nw.7j", 0x280000, 0x80000, CRC(d8764213) SHA1(89eadefb956863216c8e3d0380394aba35e8c856) )

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_LOAD( "sj401-nw.10r", 0x00000, 0x80000, CRC(25dfab59) SHA1(7d50159204ba05323a2442778f35192e66117dda) )
ROM_END

DRIVER_INIT_MEMBER(ashnojoe_state,ashnojoe)
{
	UINT8 *ROM = memregion("adpcm")->base();
	membank("bank4")->configure_entries(0, 16, &ROM[0x00000], 0x8000);

	membank("bank4")->set_entry(0);
}

GAME( 1990, scessjoe, 0,        ashnojoe, ashnojoe, ashnojoe_state, ashnojoe, ROT0, "Taito Corporation / Wave", "Success Joe (World)",   MACHINE_SUPPORTS_SAVE )
GAME( 1990, ashnojoe, scessjoe, ashnojoe, ashnojoe, ashnojoe_state, ashnojoe, ROT0, "Taito Corporation / Wave", "Ashita no Joe (Japan)", MACHINE_SUPPORTS_SAVE )
