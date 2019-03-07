// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  Funky Jet                               (c) 1992 Data East / Mitchell Corporation
  Sotsugyo Shousho                        (c) 1995 Mitchell Corporation

  But actually a Data East pcb...  Hardware is pretty close to Super Burger
  Time but with a different graphics chip.

  Emulation by Bryan McPhail, mish@tendril.co.uk


Stephh's notes :

0) 'sotsugyo'

  - COIN2 doesn't work due to code at 0x0001f0 :

        0001F0: 1228 004A                move.b  ($4a,A0), D1

    It should be (as code in 0x00019e) :

        0001F0: 3228 004A                move.w  ($4a,A0), D1

  - SERVICE1 is very strange : instead of adding 1 credit, EACH time it is
    pressed, n credits are added depending on the "Coinage A" Dip Switch :

        Coin_A    n

         3C_1C    1
         2C_1C    1
         1C_1C    1
         1C_2C    2
         1C_3C    3
         1C_4C    4
         1C_5C    5
         1C_6C    6

  - When the "Unused" Dip Switch is ON, the palette is modified.


Funky Jet
Data East, 1992

PCB Layout
----------

DE-0372-0
|-------------------------------------------------------------|
|                                               32.200MHz     |
|   3014   2151     JH02.15F            |-----|               |
|                                       | 45  |  PAL          |
|VOL                6264                |-----|               |
|          JH03.14J             |-----|                       |
|                               | 59  |                       |
| M6295    6264     JH01-2.13F  |-----|    PAL                |
|                                          PAL   63           |
|          6264     JH00-2.11F             PAL                |
|                                     6264               PAL  |
|                                                             |
|    6116                             6264                    |
|    6116                                     28MHz |-------| |
|                     |-------|                     |       | |
|J                    |       |               6116  |  52   | |
|A       DSW2(8)      |  74   |               6116  |       | |
|M                    |       |         6116        |-------| |
|M       DSW1(8)      |-------|         6116                  |
|A      |-----|                                     MAT-01.4A |
|       | 146 |                                               |
|       |-----|     MAT-02.2F                       MAT-00.2A |
|        PAL                                                  |
|-------------------------------------------------------------|
Notes:
      68000 clock     - 14.000MHz [28/2]
      YM2151 clock    - 3.58MHz [32.220/9]
      HuC6280 clock   - 8.050MHz [32.200/4]
      Oki M6295 clock - 1.000MHz [28/28], Sample Rate = 1000000/132
      VSync           - 58Hz
      HSync           - 15.68kHz

      Custom ICs -
                   74 (QFP160)
                   52 (QFP128)
                   45 (QFP80) - HuC6280
                   59 (QFP64) - 68000
                   146(QFP100)
                   63 (SOP28)

***************************************************************************/

#include "emu.h"
#include "includes/funkyjet.h"

#include "cpu/m68000/m68000.h"
#include "machine/decocrpt.h"
#include "machine/gen_latch.h"
#include "sound/ym2151.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

/******************************************************************************/

READ16_MEMBER( funkyjet_state::funkyjet_protection_region_0_146_r )
{
//  uint16_t realdat = deco16_146_funkyjet_prot_r(space,offset&0x3ff,mem_mask);

	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,  /* note, same bitswap as fghthist */      10,  9,  8,  7,  6,   5,  4,  3,  2,  1,    0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t data = m_deco146->read_data( deco146_addr, mem_mask, cs );

//  if ((realdat & mem_mask) != (data & mem_mask))
//      printf("returned %04x instead of %04x (real address %08x swapped addr %08x)\n", data, realdat, real_address, deco146_addr);

	return data;
}

WRITE16_MEMBER( funkyjet_state::funkyjet_protection_region_0_146_w )
{
//  deco16_146_funkyjet_prot_w(space,offset&0x3ff,data,mem_mask);

	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14, /* note, same bitswap as fghthist */       10,  9,  8,  7,  6,   5,  4,  3,  2,  1,    0) & 0x7fff;
	uint8_t cs = 0;
	m_deco146->write_data( space, deco146_addr, data, mem_mask, cs );
}


void funkyjet_state::funkyjet_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x120000, 0x1207ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x140000, 0x143fff).ram();
	map(0x160000, 0x1607ff).ram().share("spriteram");
	map(0x180000, 0x183fff).rw(FUNC(funkyjet_state::funkyjet_protection_region_0_146_r), FUNC(funkyjet_state::funkyjet_protection_region_0_146_w)).share("prot16ram"); /* Protection device */ // unlikely to be cs0 region
	map(0x184000, 0x184001).nopw();
	map(0x188000, 0x188001).nopw();
	map(0x300000, 0x30000f).w(m_deco_tilegen, FUNC(deco16ic_device::pf_control_w));
	map(0x320000, 0x321fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x322000, 0x323fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x340000, 0x340bff).ram().share("pf1_rowscroll");
	map(0x342000, 0x342bff).ram().share("pf2_rowscroll");
}

/******************************************************************************/

/* Physical memory map (21 bits) */
void funkyjet_state::sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).noprw(); /* YM2203 - this board doesn't have one */
	map(0x110000, 0x110001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).noprw(); /* This board only has 1 oki chip */
	map(0x140000, 0x140000).r(m_deco146, FUNC(deco146_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}

/******************************************************************************/

static INPUT_PORTS_START( funkyjet )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)   /* Button 3 only in "test mode" */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)   /* Button 3 only in "test mode" */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	/* Dips seem inverted with respect to other Deco games */
	PORT_START("DSW")
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Freeze" )            PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0xc000, "2" )
	PORT_DIPSETTING(      0x4000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( funkyjetj )
	PORT_INCLUDE(funkyjet)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sotsugyo )
	PORT_INCLUDE(funkyjet)

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )     // See notes
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(      0x0400, "1" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x3000, 0x2000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(      0x3000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze" )            PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static GFXDECODE_START( gfx_funkyjet )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  256, 32 )  /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 256, 32 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout,   0, 16 )  /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

void funkyjet_state::funkyjet(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(28'000'000)/2); /* 28 MHz crystal */
	m_maincpu->set_addrmap(AS_PROGRAM, &funkyjet_state::funkyjet_map);
	m_maincpu->set_vblank_int("screen", FUNC(funkyjet_state::irq6_line_hold));

	H6280(config, m_audiocpu, XTAL(32'220'000)/4); /* Custom chip 45, Audio section crystal is 32.220 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &funkyjet_state::sound_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "lspeaker", 0); // internal sound unused
	m_audiocpu->add_route(ALL_OUTPUTS, "rspeaker", 0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(funkyjet_state::screen_update));
	screen.set_palette("palette");

	DECO146PROT(config, m_deco146, 0);
	m_deco146->port_a_cb().set_ioport("INPUTS");
	m_deco146->port_b_cb().set_ioport("SYSTEM");
	m_deco146->port_c_cb().set_ioport("DSW");
	m_deco146->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);
	m_deco146->set_interface_scramble_interleave();

	GFXDECODE(config, "gfxdecode", "palette", gfx_funkyjet);
	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 1024);

	DECO16IC(config, m_deco_tilegen, 0);
	m_deco_tilegen->set_split(0);
	m_deco_tilegen->set_pf1_size(DECO_64x32);
	m_deco_tilegen->set_pf2_size(DECO_64x32);
	m_deco_tilegen->set_pf1_trans_mask(0x0f);
	m_deco_tilegen->set_pf2_trans_mask(0x0f);
	m_deco_tilegen->set_pf1_col_bank(0x00);
	m_deco_tilegen->set_pf2_col_bank(0x10);
	m_deco_tilegen->set_pf1_col_mask(0x0f);
	m_deco_tilegen->set_pf2_col_mask(0x0f);
	m_deco_tilegen->set_pf12_8x8_bank(0);
	m_deco_tilegen->set_pf12_16x16_bank(1);
	m_deco_tilegen->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen, 0);
	m_sprgen->set_gfx_region(2);
	m_sprgen->set_gfxdecode_tag("gfxdecode");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(32'220'000)/9));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 1); // IRQ2
	ymsnd.add_route(0, "lspeaker", 0.45);
	ymsnd.add_route(1, "rspeaker", 0.45);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(28'000'000)/28, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}

/******************************************************************************/

ROM_START( funkyjet )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "jk00-1.12f", 0x00000, 0x40000, CRC(ce61579d) SHA1(fe755b62c822c996d479cafa6fa7ac7724af6560) )
	ROM_LOAD16_BYTE( "jk01-1.13f", 0x00001, 0x40000, CRC(274d04be) SHA1(a14ec81e40504d3c7deb28114b85b9bbb76a51f5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "jk02.16f",    0x00000, 0x10000, CRC(748c0bd8) SHA1(35910e6a4c4f198fb76bde0f5b053e2c66cfa0ff) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mat02", 0x000000, 0x80000, CRC(e4b94c7e) SHA1(7b6ddd0bd388c8d32277fce4b3abb102724bc7d1) ) /* Encrypted chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mat01", 0x000000, 0x80000, CRC(24093a8d) SHA1(71f76ddd8a4b6e05ceb2fff4e20b6edb5e011e79) ) /* sprites */
	ROM_LOAD( "mat00", 0x080000, 0x80000, CRC(fbda0228) SHA1(815d49898d02e699393e370209181f2ca8301949) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "jk03.15h",    0x00000, 0x20000, CRC(69a0eaf7) SHA1(05038e82ee03106625f05082fe9912e16be181ee) )
ROM_END

ROM_START( funkyjeta )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "jk00.12f", 0x00000, 0x40000, CRC(712089c1) SHA1(84167c90303a228107f55596e2ff8b9f111d1bc2) ) /* Unverified revision, could be JK00-0 or JK00-2 */
	ROM_LOAD16_BYTE( "jk01.13f", 0x00001, 0x40000, CRC(be3920d7) SHA1(6627956d148681bc49991c544a09b07271ea4c7f) ) /* Unverified revision, could be JK01-0 or JK01-2 */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "jk02.16f",    0x00000, 0x10000, CRC(748c0bd8) SHA1(35910e6a4c4f198fb76bde0f5b053e2c66cfa0ff) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mat02", 0x000000, 0x80000, CRC(e4b94c7e) SHA1(7b6ddd0bd388c8d32277fce4b3abb102724bc7d1) ) /* Encrypted chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mat01", 0x000000, 0x80000, CRC(24093a8d) SHA1(71f76ddd8a4b6e05ceb2fff4e20b6edb5e011e79) ) /* sprites */
	ROM_LOAD( "mat00", 0x080000, 0x80000, CRC(fbda0228) SHA1(815d49898d02e699393e370209181f2ca8301949) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "jk03.15h",    0x00000, 0x20000, CRC(69a0eaf7) SHA1(05038e82ee03106625f05082fe9912e16be181ee) )
ROM_END

ROM_START( funkyjetj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "jh00-2.11f", 0x00000, 0x40000, CRC(5b98b700) SHA1(604bd04f4031b0a3b53db2fab4a0e160dff6936d) )
	ROM_LOAD16_BYTE( "jh01-2.13f", 0x00001, 0x40000, CRC(21280220) SHA1(b365b6c8aa778e21a14b2813e93b9c9d02e14995) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "jh02.16f",    0x00000, 0x10000, CRC(748c0bd8) SHA1(35910e6a4c4f198fb76bde0f5b053e2c66cfa0ff) ) /* same as jk02.16f from world set */

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mat02", 0x000000, 0x80000, CRC(e4b94c7e) SHA1(7b6ddd0bd388c8d32277fce4b3abb102724bc7d1) ) /* Encrypted chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mat01", 0x000000, 0x80000, CRC(24093a8d) SHA1(71f76ddd8a4b6e05ceb2fff4e20b6edb5e011e79) ) /* sprites */
	ROM_LOAD( "mat00", 0x080000, 0x80000, CRC(fbda0228) SHA1(815d49898d02e699393e370209181f2ca8301949) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "jh03.15h",    0x00000, 0x20000, CRC(69a0eaf7) SHA1(05038e82ee03106625f05082fe9912e16be181ee) ) /* same as jk03.15h from world set */
ROM_END

ROM_START( sotsugyo )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "03.12f", 0x00000, 0x40000, CRC(d175dfd1) SHA1(61c91d5e20b0492e6ac3b19fe9639eb4f169ae77) )
	ROM_LOAD16_BYTE( "04.13f", 0x00001, 0x40000, CRC(2072477c) SHA1(23820a519e4503854e63ab3ad7eec58178c8d822) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "sb020.16f",    0x00000, 0x10000, CRC(baf5ec93) SHA1(82b22a0b565e51cd40733f21fa876dd7064eb604) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "02.2f", 0x000000, 0x80000, CRC(337b1451) SHA1(ab3a4526e683c23b7634ac3304fb073f6ce98e82) ) /* chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "01.4a", 0x000000, 0x80000, CRC(fa10dd54) SHA1(5dfe66df0bbab5eb151bf65f7e767a2325a50b36) ) /* sprites */
	ROM_LOAD( "00.2a", 0x080000, 0x80000, CRC(d35a14ef) SHA1(b8d27766db7e183aee208c690364e4383f3c6882) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "sb030.15h",    0x00000, 0x20000, CRC(1ea43f48) SHA1(74cc8c740f1c7fa94c2cb460ea4ee7aa0c490ed7) )
ROM_END

void funkyjet_state::init_funkyjet()
{
	deco74_decrypt_gfx(machine(), "gfx1");
}

/******************************************************************************/

GAME( 1992, funkyjet,  0,        funkyjet, funkyjet,  funkyjet_state, init_funkyjet, ROT0, "Mitchell", "Funky Jet (World, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, funkyjeta, funkyjet, funkyjet, funkyjet,  funkyjet_state, init_funkyjet, ROT0, "Mitchell", "Funky Jet (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, funkyjetj, funkyjet, funkyjet, funkyjetj, funkyjet_state, init_funkyjet, ROT0, "Mitchell (Data East Corporation license)", "Funky Jet (Japan, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, sotsugyo,  0,        funkyjet, sotsugyo,  funkyjet_state, init_funkyjet, ROT0, "Mitchell (Atlus license)", "Sotsugyo Shousho", MACHINE_SUPPORTS_SAVE )
