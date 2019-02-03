// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Gotcha  (c) 1997 Dongsung

driver by Nicola Salmoria

TODO:
- Find out what the "Explane Type" dip switch actually does.
- Should use the artwork system to show the lamp state: there are 12 lamps, one
  for every button, and they are used a lot during the game (see gotcha_lamps_w).
- Unknown writes to 0x30000c. It changes for some levels, it's probably
  gfx related but since everything seems fine I've no idea what it might do.
- Unknown sound writes at C00F; also, there's an NMI handler that would
  read from C00F.
- Sound samples were getting chopped; I fixed this by changing sound/adpcm.cpp to
  disregard requests to play new samples until the previous one is finished*.

Gotcha pcb: 97,7,29 PARA VER 3.0 but it is the same as ppchamp

Pasha Pasha Champ Mini Game Festival
Dongsung, 1997

PCB Layout
----------
97,7,29 PARA VER 2.0
|------------------------------------------------|
|HA13001  CA5102     U53    14.31818MHz          |
|           CY5001   U54                         |
|VOL  6MHz 6116      U55  6116               6116|
| 1MHz     UZ02      U56                         |
|          Z80                          GAL      |
|J  AD-65  UZ11                                  |
|A                6116                  PAL      |
|M                                               |
|M  DIPSW2        6116      PAL                  |
|A                                               |
|   DIPSW1  6116                |--------|       |
|           6116  PAL           |ALTERA  |       |
|                               |MAX     |       |
|     UCN5801      6        PAL |EPM7128 |   U41A|
|     UCN5801 PAL  8  62256     |--------|U42A   |
|  LAMP       PAL  0  U3                         |
|PBSW         PAL  0  62256  6264            U41B|
|                  0  U2     6264         U42B   |
|------------------------------------------------|
Notes:
      68000 clock - 14.31818MHz
      M6295 clock - 1.000MHz, sample rate = 1000000/165
      YM2151 clock- 3.579545MHz [14.31818/4]
      Z80 clock   - 6.000MHz
      VSync       - 55Hz
      HSync       - 14.5kHz
      LAMP        - Player 1, 2 & 3 lamp driver connector for buttons Start, Blue, Green and Red
                    14 pin connector, 4 for each player plus 2 for 12V
      PBSW        - Player 1, 2 & 3 button connector for buttons Start, Blue, Green and Red
                    15 pin connector, 4 for each player plus 3 for ground

***************************************************************************/

#include "emu.h"
#include "includes/gotcha.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ym2151.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "gotcha.lh"


WRITE16_MEMBER(gotcha_state::gotcha_lamps_w)
{
	for (int p = 0; p < 3; p++)
	{
		m_lamp_r[p] = BIT(data, 4 * p);
		m_lamp_g[p] = BIT(data, 4 * p + 1);
		m_lamp_b[p] = BIT(data, 4 * p + 2);
		m_lamp_s[p] = BIT(data, 4 * p + 3);
	}
}

WRITE8_MEMBER(gotcha_state::gotcha_oki_bank_w)
{
	m_oki->set_rom_bank(!BIT(data, 0));
}


void gotcha_state::gotcha_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100001, 0x100001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x100002, 0x100003).w(FUNC(gotcha_state::gotcha_lamps_w));
	map(0x100004, 0x100004).w(FUNC(gotcha_state::gotcha_oki_bank_w));
	map(0x120000, 0x12ffff).ram();
	map(0x140000, 0x1405ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x160000, 0x1607ff).ram().share("spriteram");
	map(0x180000, 0x180001).portr("INPUTS");
	map(0x180002, 0x180003).portr("SYSTEM");
	map(0x180004, 0x180005).portr("DSW");
	map(0x300000, 0x300001).w(FUNC(gotcha_state::gotcha_gfxbank_select_w));
	map(0x300002, 0x300009).w(FUNC(gotcha_state::gotcha_scroll_w));
//  { 0x30000c, 0x30000d,
	map(0x30000e, 0x30000f).w(FUNC(gotcha_state::gotcha_gfxbank_w));
	map(0x320000, 0x320fff).w(FUNC(gotcha_state::gotcha_fgvideoram_w)).share("fgvideoram");
	map(0x322000, 0x322fff).w(FUNC(gotcha_state::gotcha_bgvideoram_w)).share("bgvideoram");
}


void gotcha_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xc002, 0xc002).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write)).mirror(1);
	map(0xc006, 0xc006).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xd000, 0xd7ff).ram();
}



static INPUT_PORTS_START( gotcha )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0007, "1" )
	PORT_DIPSETTING(      0x0006, "2" )
	PORT_DIPSETTING(      0x0005, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPSETTING(      0x0003, "5" )
	PORT_DIPSETTING(      0x0002, "6" )
	PORT_DIPSETTING(      0x0001, "7" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0010, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0030, "1" )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x00c0, 0x0080, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00c0, "1 Coin/99 Credits" )
	PORT_DIPNAME( 0x0100, 0x0100, "Info" )          PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Explane Type" )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Game Selection" )    PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END



static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static GFXDECODE_START( gfx_gotcha )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   0x100, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x000, 16 )
GFXDECODE_END



void gotcha_state::machine_start()
{
	m_lamp_r.resolve();
	m_lamp_g.resolve();
	m_lamp_b.resolve();
	m_lamp_s.resolve();

	save_item(NAME(m_banksel));
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_scroll));
}

void gotcha_state::machine_reset()
{
	int i;

	for (i = 0; i < 4; i++)
	{
		m_gfxbank[i] = 0;
		m_scroll[i] = 0;
	}

	m_banksel = 0;
}

MACHINE_CONFIG_START(gotcha_state::gotcha)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M68000, 14.318181_MHz_XTAL)    /* 14.31818 MHz */
	MCFG_DEVICE_PROGRAM_MAP(gotcha_map)

	MCFG_DEVICE_ADD("audiocpu", Z80, 6_MHz_XTAL)   /* 6 MHz */
	MCFG_DEVICE_PROGRAM_MAP(sound_map)

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(55);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(gotcha_state::screen_update_gotcha));
	screen.set_palette("palette");
	screen.screen_vblank().set_inputline(m_maincpu, M68K_IRQ_6, HOLD_LINE);
	screen.screen_vblank().append_inputline(m_audiocpu, INPUT_LINE_NMI); // ?

	GFXDECODE(config, m_gfxdecode, "palette", gfx_gotcha);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 768);

	DECO_SPRITE(config, m_sprgen, 0);
	m_sprgen->set_gfx_region(1);
	m_sprgen->set_is_bootleg(true);
	m_sprgen->set_offsets(5, -1); // aligned to 2nd instruction screen in attract
	m_sprgen->set_gfxdecode_tag(m_gfxdecode);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 14.318181_MHz_XTAL / 4));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.80);
	ymsnd.add_route(1, "mono", 0.80);

	MCFG_DEVICE_ADD("oki", OKIM6295, 1_MHz_XTAL, okim6295_device::PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gotcha )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gotcha.u3",    0x00000, 0x40000, CRC(5e5d52e0) SHA1(c3e9375350b7931e3c9874a045d7a9d8df5ea691) )
	ROM_LOAD16_BYTE( "gotcha.u2",    0x00001, 0x40000, CRC(3aa8eaff) SHA1(348f2ab43101d51c553ff10f9d18cc499006c965) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gotcha_u.z02", 0x0000, 0x10000, CRC(f4f6e16b) SHA1(a360c571bee7391c66e98e5e111e78ac9732390e) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "gotcha-u.42a", 0x000000, 0x20000, CRC(4ea822f0) SHA1(5b25d4c80138d9a0f3d481fa0c2f3665772bc0c8) )
	ROM_CONTINUE(             0x100000, 0x20000 )
	ROM_CONTINUE(             0x020000, 0x20000 )
	ROM_CONTINUE(             0x120000, 0x20000 )
	ROM_LOAD( "gotcha-u.42b", 0x040000, 0x20000, CRC(6bb529ac) SHA1(d872ec3d13d2bef4f8e0d0a8e72827b5ca87e193) )
	ROM_CONTINUE(             0x140000, 0x20000 )
	ROM_CONTINUE(             0x060000, 0x20000 )
	ROM_CONTINUE(             0x160000, 0x20000 )
	ROM_LOAD( "gotcha-u.41a", 0x080000, 0x20000, CRC(49299b7b) SHA1(85276453b6fce925c7b10c713e35284066df6ebf) )
	ROM_CONTINUE(             0x180000, 0x20000 )
	ROM_CONTINUE(             0x0a0000, 0x20000 )
	ROM_CONTINUE(             0x1a0000, 0x20000 )
	ROM_LOAD( "gotcha-u.41b", 0x0c0000, 0x20000, CRC(c093f04e) SHA1(e731714c9fe9b583a23e162a5513574e63d0f454) )
	ROM_CONTINUE(             0x1c0000, 0x20000 )
	ROM_CONTINUE(             0x0e0000, 0x20000 )
	ROM_CONTINUE(             0x1e0000, 0x20000 )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "gotcha.u56",   0x000000, 0x80000, CRC(85f6a062) SHA1(77d1c9c8394af0c487fa6d657ae740eae940682a) )
	ROM_LOAD( "gotcha.u55",   0x080000, 0x80000, CRC(426b4e48) SHA1(91e79c9fd1f9cf84df8e1d6b67780d1cacd4a0f2) )
	ROM_LOAD( "gotcha.u54",   0x100000, 0x80000, CRC(903e05a4) SHA1(4fb675958f4dc057f8da7edff1f6680482bdc5dd) )
	ROM_LOAD( "gotcha.u53",   0x180000, 0x80000, CRC(3c24d51e) SHA1(8b987db14a56950cc0f77e232e20fcdd89f98f2b) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "gotcha-u.z11", 0x000000, 0x80000, CRC(6111c6ae) SHA1(9170a37eaca56586da2f5e4894816640193c8802) )
ROM_END

ROM_START( ppchamp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u3", 0x00000, 0x40000, CRC(f56c0fc2) SHA1(7158c9f252e48b0605dc98e3f0d3ad9d0b376cc8) )
	ROM_LOAD16_BYTE( "u2", 0x00001, 0x40000, CRC(a941ffdc) SHA1(0667dafd11ba3a79e8c6df61521344c70e287250) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "uz02", 0x00000, 0x10000, CRC(f4f6e16b) SHA1(a360c571bee7391c66e98e5e111e78ac9732390e) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "u42a",         0x000000, 0x20000, CRC(f0b521d1) SHA1(fe44bfa13818eee08d112c2f75e14bfd67bbefbf) )
	ROM_CONTINUE(             0x100000, 0x20000 )
	ROM_CONTINUE(             0x020000, 0x20000 )
	ROM_CONTINUE(             0x120000, 0x20000 )
	ROM_LOAD( "u42b",         0x040000, 0x20000, CRC(1107918e) SHA1(bb508da36814f2954d6a9996b777d095f6e9c243) )
	ROM_CONTINUE(             0x140000, 0x20000 )
	ROM_CONTINUE(             0x060000, 0x20000 )
	ROM_CONTINUE(             0x160000, 0x20000 )
	ROM_LOAD( "u41a",         0x080000, 0x20000, CRC(3f567d33) SHA1(77122c1cdea663922fe570e005bfbb4c779f30da) )
	ROM_CONTINUE(             0x180000, 0x20000 )
	ROM_CONTINUE(             0x0a0000, 0x20000 )
	ROM_CONTINUE(             0x1a0000, 0x20000 )
	ROM_LOAD( "u41b",         0x0c0000, 0x20000, CRC(18a3497e) SHA1(7938f4e723bf4d29de6c9eda807c37d86b7ac78c) )
	ROM_CONTINUE(             0x1c0000, 0x20000 )
	ROM_CONTINUE(             0x0e0000, 0x20000 )
	ROM_CONTINUE(             0x1e0000, 0x20000 )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "u56", 0x000000, 0x80000, CRC(160e46b3) SHA1(e2bec3388d41afb9f1025d66c15fcc6ca4d40703) )
	ROM_LOAD( "u55", 0x080000, 0x80000, CRC(7351b61c) SHA1(2ef3011a7a1ff253f45186e46cfdce5f4ef17322) )
	ROM_LOAD( "u54", 0x100000, 0x80000, CRC(a3d8c5ef) SHA1(f59874844934f3ce76a49e4a9618510537378387) )
	ROM_LOAD( "u53", 0x180000, 0x80000, CRC(10ca65c4) SHA1(66ba3c6e1bda18c5668a609adc60bfe547205e53) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "uz11", 0x00000, 0x80000, CRC(3d96274c) SHA1(c7a670af86194c370bf8fb30afbe027ab78a0227) )
ROM_END

ROM_START( ppchampa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "d8-d15.u8", 0x00000, 0x40000, CRC(e7f8b97a) SHA1(85216fc64d3482a1108bd1bf7792db441be5e999) )
	ROM_LOAD16_BYTE( "d0-d7.u2",  0x00001, 0x40000, CRC(35ee8ad7) SHA1(ce721899b627935703c5b2fc4fa4d107192d3814) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "uz02", 0x00000, 0x10000, CRC(f4f6e16b) SHA1(a360c571bee7391c66e98e5e111e78ac9732390e) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "u42a",         0x000000, 0x20000, CRC(f0b521d1) SHA1(fe44bfa13818eee08d112c2f75e14bfd67bbefbf) )
	ROM_CONTINUE(             0x100000, 0x20000 )
	ROM_CONTINUE(             0x020000, 0x20000 )
	ROM_CONTINUE(             0x120000, 0x20000 )
	ROM_LOAD( "u42b",         0x040000, 0x20000, CRC(1107918e) SHA1(bb508da36814f2954d6a9996b777d095f6e9c243) )
	ROM_CONTINUE(             0x140000, 0x20000 )
	ROM_CONTINUE(             0x060000, 0x20000 )
	ROM_CONTINUE(             0x160000, 0x20000 )
	ROM_LOAD( "u41a",         0x080000, 0x20000, CRC(3f567d33) SHA1(77122c1cdea663922fe570e005bfbb4c779f30da) )
	ROM_CONTINUE(             0x180000, 0x20000 )
	ROM_CONTINUE(             0x0a0000, 0x20000 )
	ROM_CONTINUE(             0x1a0000, 0x20000 )
	ROM_LOAD( "u41b",         0x0c0000, 0x20000, CRC(18a3497e) SHA1(7938f4e723bf4d29de6c9eda807c37d86b7ac78c) )
	ROM_CONTINUE(             0x1c0000, 0x20000 )
	ROM_CONTINUE(             0x0e0000, 0x20000 )
	ROM_CONTINUE(             0x1e0000, 0x20000 )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "u56", 0x000000, 0x80000, CRC(160e46b3) SHA1(e2bec3388d41afb9f1025d66c15fcc6ca4d40703) )
	ROM_LOAD( "u55", 0x080000, 0x80000, CRC(7351b61c) SHA1(2ef3011a7a1ff253f45186e46cfdce5f4ef17322) )
	ROM_LOAD( "u54", 0x100000, 0x80000, CRC(a3d8c5ef) SHA1(f59874844934f3ce76a49e4a9618510537378387) )
	ROM_LOAD( "u53", 0x180000, 0x80000, CRC(10ca65c4) SHA1(66ba3c6e1bda18c5668a609adc60bfe547205e53) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "uz11", 0x00000, 0x80000, CRC(3d96274c) SHA1(c7a670af86194c370bf8fb30afbe027ab78a0227) )
ROM_END

GAMEL( 1997, gotcha,   0,      gotcha, gotcha, gotcha_state, empty_init, ROT0, "Dongsung / Para", "Got-cha Mini Game Festival",                   MACHINE_SUPPORTS_SAVE, layout_gotcha )
GAMEL( 1997, ppchamp,  gotcha, gotcha, gotcha, gotcha_state, empty_init, ROT0, "Dongsung / Para", "Pasha Pasha Champ Mini Game Festival (Korea, set 1)", MACHINE_SUPPORTS_SAVE, layout_gotcha )
GAMEL( 1997, ppchampa, gotcha, gotcha, gotcha, gotcha_state, empty_init, ROT0, "Dongsung / Para", "Pasha Pasha Champ Mini Game Festival (Korea, set 2)", MACHINE_SUPPORTS_SAVE, layout_gotcha )
