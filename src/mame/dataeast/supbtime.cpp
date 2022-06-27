// license: BSD-3-Clause
// copyright-holders: Bryan McPhail, David Haywood, Dirk Best
/***************************************************************************

  Super Burger Time     © 1990 Data East Corporation
  China Town            © 1991 Data East Corporation
  Tumblepop             © 1991 Data East Corporation

  These games all run on the DE-0343 board.

  Sound:  Ym2151, Oki adpcm - NOTE!  The sound program writes to the address
of a YM2203 and a 2nd Oki chip but the board does _not_ have them.  The sound
program is simply the 'generic' Data East sound program unmodified for this cut
down hardware (it doesn't write any good sound data btw, mostly zeros).

  Super Burgertime has a few bugs:

  Some sprites clip at the edges of the screen.
  Some burgers (from crushing an enemy) appear with wrong colour.
  Colour cycle on title screen doesn't work first time around.

  These are NOT driver bugs!  They all exist in the original game.

Stephh's notes (based on the games M68000 code and some tests) :

1) 'tumblep*' and 'jumpkids'

  - I don't understand the interest of the "Remove Monsters" Dip Switch :
    as I haven't found a way to "end" a level, I guess that it was used to
    test the backgrounds and the "platforms".

  - The "Edit Levels" Dip Switch allows you to add/delete monsters and
    change their position.

    Notes (for 'tumblep', 'tumblepj', 'tumblep2') :
      * "worlds" and levels are 0-based (00-09 & 00-09) :

          World      Name
            0      America
            1      Brazil
            2      Asia
            3      Soviet
            4      Europe
            5      Egypt
            6      Australia
            7      Antartica
            8      Stratosphere
            9      Space

      * As levels x-9 and 9-x are only constitued of a "big boss", you can't
        edit them !
      * All data is stored within the range 0x02b8c8-0x02d2c9, but it should be
        extended to 0x02ebeb (and perhaps 0x02ffff). TO BE CONFIRMED !
      * Once your levels are ready, turn the Dip Switch OFF and reset the game.
      * Of course, there is no possibility to save the levels when you exit
        MAME, nor the way to reload the default ones 8(

  Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "supbtime.h"
#include "emupal.h"

#define TUMBLEP_HACK 0


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void supbtime_state::supbtime_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x104000, 0x11ffff).nopw(); // Nothing there
	map(0x120000, 0x1207ff).ram().share("spriteram");
	map(0x120800, 0x13ffff).nopw(); // Nothing there
	map(0x140000, 0x1407ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x180001).portr("INPUTS");
	map(0x180002, 0x180003).portr("DSW");
	map(0x180008, 0x180009).portr("SYSTEM");
	map(0x18000a, 0x18000b).r(FUNC(supbtime_state::vblank_ack_r));
	map(0x18000a, 0x18000d).nopw(); // ?
	map(0x1a0001, 0x1a0001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x300000, 0x30000f).rw(m_deco_tilegen, FUNC(deco16ic_device::pf_control_r), FUNC(deco16ic_device::pf_control_w));
	map(0x320000, 0x321fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x322000, 0x323fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x340000, 0x3407ff).ram().share("pf1_rowscroll");
	map(0x342000, 0x3427ff).ram().share("pf2_rowscroll");
}

void supbtime_state::chinatwn_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100001, 0x100001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x120000, 0x1207ff).ram().share("spriteram");
	map(0x140000, 0x1407ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x180001).portr("INPUTS");
	map(0x180002, 0x180003).portr("DSW");
	map(0x180008, 0x180009).portr("SYSTEM");
	map(0x18000a, 0x18000b).r(FUNC(supbtime_state::vblank_ack_r));
	map(0x18000a, 0x18000d).nopw(); // ?
	map(0x1a0000, 0x1a3fff).ram();
	map(0x300000, 0x30000f).rw(m_deco_tilegen, FUNC(deco16ic_device::pf_control_r), FUNC(deco16ic_device::pf_control_w));
	map(0x320000, 0x321fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x322000, 0x323fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x340000, 0x3407ff).ram().share("pf1_rowscroll"); // unused
	map(0x342000, 0x3427ff).ram().share("pf2_rowscroll"); // unused
}

void supbtime_state::tumblep_map(address_map &map)
{
#if TUMBLEP_HACK
	map(0x000000, 0x07ffff).writeonly();   // To write levels modifications
#endif
	map(0x000000, 0x07ffff).rom();
	map(0x100001, 0x100001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x120000, 0x123fff).ram();
	map(0x140000, 0x1407ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x180001).portr("INPUTS");
	map(0x180002, 0x180003).portr("DSW");
	map(0x180008, 0x180009).portr("SYSTEM");
	map(0x18000a, 0x18000b).r(FUNC(supbtime_state::vblank_ack_r));
	map(0x18000a, 0x18000d).nopw(); // ?
	map(0x1a0000, 0x1a07ff).ram().share("spriteram");
	map(0x300000, 0x30000f).w(m_deco_tilegen, FUNC(deco16ic_device::pf_control_w));
	map(0x320000, 0x320fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x322000, 0x322fff).rw(m_deco_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x340000, 0x3407ff).writeonly().share("pf1_rowscroll"); // unused
	map(0x342000, 0x3427ff).writeonly().share("pf2_rowscroll"); // unused
}

// Physical memory map (21 bits)
void supbtime_state::sound_map(address_map &map)
{
	map.global_mask(0x1fffff);
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).noprw(); // YM2203 - this board doesn't have one
	map(0x110000, 0x110001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).noprw(); // This board only has 1 oki chip
	map(0x140000, 0x140001).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x1f0000, 0x1f1fff).ram();
}


//**************************************************************************
//  VIDEO
//**************************************************************************

WRITE_LINE_MEMBER( supbtime_state::vblank_w )
{
	if (state)
		m_maincpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
}

uint16_t supbtime_state::vblank_ack_r()
{
	m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	return 0xffff;
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( supbtime )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )  // Button 3 - unused
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )  // Button 3 - unused
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )

	// inverted with respect to other Deco games
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Cabinet ) )        PORT_DIPLOCATION("DSW1:8") // No effect?
	PORT_DIPSETTING(      0x0001, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )         PORT_DIPLOCATION("DSW1:6,5,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )         PORT_DIPLOCATION("DSW1:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )         PORT_DIPLOCATION("DSW2:6") // Listed as "Don't Change"
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )         PORT_DIPLOCATION("DSW2:5") // Listed as "Don't Change"
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )     PORT_DIPLOCATION("DSW2:4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )          PORT_DIPLOCATION("DSW2:2,1")
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( chinatwn )
	PORT_INCLUDE(supbtime)

	// inverted with respect to other Deco games
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unused ) )         PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )        PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )        PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Time" )                    PORT_DIPLOCATION("DSW2:4,3")
	PORT_DIPSETTING(      0x0000, "1500" )
	PORT_DIPSETTING(      0x8000, "2000" )
	PORT_DIPSETTING(      0xc000, "2500" )
	PORT_DIPSETTING(      0x4000, "3000" )
INPUT_PORTS_END

static INPUT_PORTS_START( tumblep )
	PORT_INCLUDE(supbtime)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Start Price" )             PORT_DIPLOCATION("DSW1:8") // Listed as "Don't Change"
	PORT_DIPSETTING(      0x0001, "1 Coin" )
	PORT_DIPSETTING(      0x0000, "2 Coins" )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0400, 0x0400, "Edit Levels" )             PORT_DIPLOCATION("DSW2:6") // Listed as "Don't Change"
#else
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )         PORT_DIPLOCATION("DSW2:6") // Listed as "Don't Change"
#endif
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0800, 0x0800, "Remove Monsters" )         PORT_DIPLOCATION("DSW2:5") // Listed as "Don't Change"
#else
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )         PORT_DIPLOCATION("DSW2:5") // Listed as "Don't Change"
#endif
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


//**************************************************************************
//  GFXDECODE LAYOUTS
//**************************************************************************

static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(8*2*16,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	32*16
};

static GFXDECODE_START( gfx_supbtime )
	GFXDECODE_ENTRY( "tiles",   0, tile_8x8_layout,   256, 32 ) // 8x8
	GFXDECODE_ENTRY( "tiles",   0, tile_16x16_layout, 256, 32 ) // 16x16
	GFXDECODE_ENTRY( "sprites", 0, tile_16x16_layout,   0, 16 ) // 16x16
GFXDECODE_END


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void supbtime_state::supbtime(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(21'477'272) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &supbtime_state::supbtime_map);

	H6280(config, m_audiocpu, XTAL(32'220'000) / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &supbtime_state::sound_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(28'000'000) / 4, 442, 0, 320, 274, 8, 248);
	screen.screen_vblank().set(FUNC(supbtime_state::vblank_w));
	screen.set_screen_update(FUNC(supbtime_state::screen_update_supbtime));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_supbtime);
	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 1024);

	DECO16IC(config, m_deco_tilegen, 0);
	m_deco_tilegen->set_pf1_size(DECO_64x32);
	m_deco_tilegen->set_pf2_size(DECO_64x32);
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

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(32'220'000) / 9));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 1);    /* IRQ2 */
	ymsnd.add_route(0, "mono", 0.45);
	ymsnd.add_route(1, "mono", 0.45);

	OKIM6295(config, "oki", XTAL(21'477'272) / 20, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // clock frequency & pin 7 not verified
}

void supbtime_state::chinatwn(machine_config &config)
{
	supbtime(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &supbtime_state::chinatwn_map);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(supbtime_state::screen_update_chinatwn));
}

void supbtime_state::tumblep(machine_config &config)
{
	supbtime(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &supbtime_state::tumblep_map);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(supbtime_state::screen_update_tumblep));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( supbtime )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("gk03", 0x00000, 0x20000, CRC(aeaeed61) SHA1(4bceb4475a642a36406395f1e84b16fa137f67a5))
	ROM_LOAD16_BYTE("gk04", 0x00001, 0x20000, CRC(2bc5a4eb) SHA1(721ec73c32af8b998babb6d7c9e526ced0c2389b))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD("gc06.bin", 0x00000, 0x10000, CRC(e0e6c0f4) SHA1(5a8b29752c58ea76d9c7961c5b0d8c94f35037af))

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD("mae02.bin", 0x00000, 0x80000, CRC(a715cca0) SHA1(0539bba39c60324d85599ac69ff78bb215deb511))

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD("mae00.bin", 0x80000, 0x80000, CRC(30043094) SHA1(5302cfd9bdaf90c4901fda75407379c4ce1cbdec))
	ROM_LOAD("mae01.bin", 0x00000, 0x80000, CRC(434af3fb) SHA1(1cfd30d14f03554e826576d6d32ce424f0df3748))

	ROM_REGION( 0x20000, "oki", 0 )
	ROM_LOAD("gc05.bin", 0x00000, 0x20000, CRC(2f2246ff) SHA1(3fcceb6f5aa5f33187bcf4c59d88327f396fa80d))

	ROM_REGION( 0x618, "pals", 0 )
	ROM_LOAD("tg0.a11", 0x000, 0x104, CRC(ac6aa74b) SHA1(73c673243caa829d5f948bca529523e45e8dd64f)) // PAL16R6
	ROM_LOAD("tg1.b15", 0x104, 0x104, CRC(819c4522) SHA1(cb59c42f265c9a7184c34fc6bbccee95b36d5c48)) // PAL16R4
	ROM_LOAD("tg2.c12", 0x208, 0x104, CRC(88f6d299) SHA1(3a0a22a10e0ada659355a74c3c8dc1b9dce8db77)) // PAL16L8
	ROM_LOAD("tg3.c13", 0x30c, 0x104, CRC(3d5f0e97) SHA1(8794fd1205a149fd04dc2337ac70ce5ecb073de9)) // PAL16L8
	ROM_LOAD("tg4.c14", 0x410, 0x104, CRC(e9ee3a67) SHA1(5299f44f1141fcd57b0559b91ec7adb51b36c5c4)) // PAL16L8
	ROM_LOAD("tg5.j1",  0x514, 0x104, CRC(21d02af7) SHA1(4b221a478cb3381e9551de770df7c491c5e59c90)) // PAL16L8
ROM_END

ROM_START( supbtimea ) // this set has no backgrounds ingame for most stages, but has been verifeid as good on multiple PCBs, design choice
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("3.11f", 0x00000, 0x20000, CRC(98b5f263) SHA1(ee4b0d2fcdc95aba0e78d066bd6c4d553a902848))
	ROM_LOAD16_BYTE("4.12f", 0x00001, 0x20000, CRC(937e68b9) SHA1(4779e150518b9014c2154f33d38767c6a7447334))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD("gc06.bin", 0x00000, 0x10000, CRC(e0e6c0f4) SHA1(5a8b29752c58ea76d9c7961c5b0d8c94f35037af))

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD("mae02.bin", 0x00000, 0x80000, CRC(a715cca0) SHA1(0539bba39c60324d85599ac69ff78bb215deb511))

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD("mae00.bin", 0x80000, 0x80000, CRC(30043094) SHA1(5302cfd9bdaf90c4901fda75407379c4ce1cbdec))
	ROM_LOAD("mae01.bin", 0x00000, 0x80000, CRC(434af3fb) SHA1(1cfd30d14f03554e826576d6d32ce424f0df3748))

	ROM_REGION( 0x20000, "oki", 0 )
	ROM_LOAD("gc05.bin", 0x00000, 0x20000, CRC(2f2246ff) SHA1(3fcceb6f5aa5f33187bcf4c59d88327f396fa80d))
ROM_END

ROM_START( supbtimej )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("gc03.bin", 0x00000, 0x20000, CRC(b5621f6a) SHA1(2dfd361e81dc4805bc248cc293d94131162df2d2))
	ROM_LOAD16_BYTE("gc04.bin", 0x00001, 0x20000, CRC(551b2a0c) SHA1(8a6dde2d64029b8e7f7c9b88bd05633b69417dc1))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD("gc06.bin", 0x00000, 0x10000, CRC(e0e6c0f4) SHA1(5a8b29752c58ea76d9c7961c5b0d8c94f35037af))

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD("mae02.bin", 0x000000, 0x80000, CRC(a715cca0) SHA1(0539bba39c60324d85599ac69ff78bb215deb511))

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD("mae00.bin", 0x80000, 0x80000, CRC(30043094) SHA1(5302cfd9bdaf90c4901fda75407379c4ce1cbdec))
	ROM_LOAD("mae01.bin", 0x00000, 0x80000, CRC(434af3fb) SHA1(1cfd30d14f03554e826576d6d32ce424f0df3748))

	ROM_REGION( 0x20000, "oki", 0 )
	ROM_LOAD("gc05.bin",    0x00000, 0x20000, CRC(2f2246ff) SHA1(3fcceb6f5aa5f33187bcf4c59d88327f396fa80d))
ROM_END

ROM_START( chinatwn )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("gv_00-.f11", 0x00000, 0x20000, CRC(2ea7ea5d) SHA1(3d0eb63f3af00bcf10ba7416dd26b366578006bf))
	ROM_LOAD16_BYTE("gv_01-.f13", 0x00001, 0x20000, CRC(bcab03c7) SHA1(cd6c1ad26a0867482565a0544ea1870012cabf34))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD("gv_02-.f16", 0x00000, 0x10000, CRC(95151d84) SHA1(9f49e49f966c3fc460773b187a110073eb595880))

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD("mak-02.h2", 0x00000, 0x80000, CRC(745b2c50) SHA1(557ac71da170a04caaab393dc43e46858ef8dd70))

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD("mak-00.a2", 0x080000, 0x80000, CRC(18e8cc1b) SHA1(afa79557222a94de7d9fde526ca45796f74fb3b2))
	ROM_LOAD("mak-01.a4", 0x000000, 0x80000, CRC(d88ebda8) SHA1(ec6eab95f3ca8ee946151c46c6570b0b0c508ffc))

	ROM_REGION( 0x20000, "oki", 0 )
	ROM_LOAD("gv_03-.j14", 0x00000, 0x20000, CRC(948faf92) SHA1(2538c7d4fa7fe0bfdd5dccece8ee82e911cee63f))
ROM_END

ROM_START( tumblep )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE("hl00-1.f12", 0x00000, 0x40000, CRC(fd697c1b) SHA1(1a3dee4c7383f2bc2d73037e80f8f5d8297e7433))
	ROM_LOAD16_BYTE("hl01-1.f13", 0x00001, 0x40000, CRC(d5a62a3f) SHA1(7249563993fa8e1f19ddae51306d4a576b5cb206))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD("hl02-.f16", 0x00000, 0x10000, CRC(a5cab888) SHA1(622f6adb01e31b8f3adbaed2b9900b54c5922c57))

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD("map-02.rom", 0x00000, 0x80000, CRC(dfceaa26) SHA1(83e391ff39efda71e5fa368ac68ba7d6134bac21))  // encrypted

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD("map-01.rom", 0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398))
	ROM_LOAD("map-00.rom", 0x80000, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e))

	ROM_REGION( 0x20000, "oki", 0 )
	ROM_LOAD("hl03-.j15", 0x00000, 0x20000, CRC(01b81da0) SHA1(914802f3206dc59a720af9d57eb2285bc8ba822b))
ROM_END

ROM_START( tumblepj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE("hk00-1.f12", 0x00000, 0x40000, CRC(2d3e4d3d) SHA1(0acc8b93bd49395904dff11c582bdbaccdbd3eef))
	ROM_LOAD16_BYTE("hk01-1.f13", 0x00001, 0x40000, CRC(56912a00) SHA1(0545f6bff2a0aa2f36adda0f9d73b165387abc3a))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD("hl02-.f16", 0x00000, 0x10000, CRC(a5cab888) SHA1(622f6adb01e31b8f3adbaed2b9900b54c5922c57))

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD("map-02.rom", 0x00000, 0x80000, CRC(dfceaa26) SHA1(83e391ff39efda71e5fa368ac68ba7d6134bac21))  // encrypted

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD("map-01.rom", 0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398))
	ROM_LOAD("map-00.rom", 0x80000, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e))

	ROM_REGION( 0x20000, "oki", 0 )
	ROM_LOAD("hl03-.j15", 0x00000, 0x20000, CRC(01b81da0) SHA1(914802f3206dc59a720af9d57eb2285bc8ba822b))
ROM_END


//**************************************************************************
//  MACHINE
//**************************************************************************

void supbtime_state::init_tumblep()
{
	deco56_decrypt_gfx(machine(), "tiles");

#if TUMBLEP_HACK
	uint16_t *RAM = (uint16_t *)memregion("maincpu")->base();
	RAM[(offset + 0)/2] = 0x0240;
	RAM[(offset + 2)/2] = 0xffff;   // andi.w  #$f3ff, D0
#endif
}

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT    MACHINE   INPUT     CLASS           INIT          ROT   COMPANY                  FULLNAME                            FLAGS
GAME( 1990, supbtime,  0,        supbtime, supbtime, supbtime_state, empty_init,   ROT0, "Data East Corporation", "Super Burger Time (World, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, supbtimea, supbtime, supbtime, supbtime, supbtime_state, empty_init,   ROT0, "Data East Corporation", "Super Burger Time (World, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, supbtimej, supbtime, supbtime, supbtime, supbtime_state, empty_init,   ROT0, "Data East Corporation", "Super Burger Time (Japan)",        MACHINE_SUPPORTS_SAVE )
GAME( 1991, chinatwn,  0,        chinatwn, chinatwn, supbtime_state, empty_init,   ROT0, "Data East Corporation", "China Town (Japan)",               MACHINE_SUPPORTS_SAVE )
GAME( 1991, tumblep,   0,        tumblep,  tumblep,  supbtime_state, init_tumblep, ROT0, "Data East Corporation", "Tumble Pop (World)",               MACHINE_SUPPORTS_SAVE )
GAME( 1991, tumblepj,  tumblep,  tumblep,  tumblep,  supbtime_state, init_tumblep, ROT0, "Data East Corporation", "Tumble Pop (Japan)",               MACHINE_SUPPORTS_SAVE )
