// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
// thanks-to:Jason Nelson
/*****************************************************************************

   MOLE ATTACK by Yachiyo Electronics Co.,LTD. 1982

   Known Clones:
   "Holey Moley", from tai (Thomas Automatics, Inc.)

   Known Issues:
   - some dips not mapped
   - protection isn't fully understood, but game seems to be ok.
   - cpu speeds unverified

   Buttons are laid out as follows:
   7   8   9
   4   5   6
   1   2   3

   Working RAM notes:
   0x2e0                    number of credits
   0x2F1                    coin up trigger
   0x2F2                    round counter
   0x2F3                    flag value
   0x2FD                    hammer aim for attract mode
   0x2E1-E2                 high score
   0x2ED-EE                 score
   0x301-309                presence and height of mole in each hole, from bottom left
   0x30a
   0x32E-336                if a hammer is above a mole. (Not the same as collision)
   0x337                    dip switch related
   0x338                    dip switch related
   0x340                    hammer control: manual=0; auto=1
   0x34C                    round point 10s.
   0x34D                    which bonus round pattern to use for moles.
   0x349                    button pressed (0..8 / 0xff)
   0x350                    number of players
   0x351                    irq-related
   0x361
   0x362
   0x363
   0x364
   0x366                    mirrors tile bank
   0x36B                    controls which player is playing. (1 == player 2);
   0x3DC                    affects mole popmessage
   0x3E5                    round point/passing point control?
   0x3E7                    round point/passing point control?

******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>


namespace {

class mole_state : public driver_device
{
public:
	mole_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tileram(*this, "tileram", 0x400*2, ENDIANNESS_LITTLE)
	{ }

	void mole(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	memory_share_creator<uint16_t> m_tileram;

	/* video-related */
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_tile_bank = 0;

	void tileram_w(offs_t offset, uint8_t data);
	void tilebank_w(uint8_t data);
	void irqack_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	uint8_t protection_r(offs_t offset);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

TILE_GET_INFO_MEMBER(mole_state::get_bg_tile_info)
{
	uint16_t const code = m_tileram[tile_index];

	tileinfo.set(BIT(code, 9), code & 0x1ff, 0, 0);
}

void mole_state::video_start()
{
	std::fill_n(&m_tileram[0], m_tileram.length(), 0);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mole_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 40, 25);

	save_item(NAME(m_tile_bank));
}

void mole_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data | (m_tile_bank << 8);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void mole_state::tilebank_w(uint8_t data)
{
	m_tile_bank = data;
}

void mole_state::irqack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void mole_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(BIT(data, 0));
}

uint32_t mole_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

uint8_t mole_state::protection_r(offs_t offset)
{
	/*  Following are all known examples of Mole Attack
	**  code reading from the protection circuitry:
	**
	**  5b09:
	**  ram[0x0361] = (ram[0x885+ram[0x8a5])&ram[0x886]
	**  ram[0x0363] = ram[0x886]
	**
	**  53c7:
	**  ram[0xe0] = ram[0x800]+ram[0x802]+ram[0x804]
	**  ram[0xea] = ram[0x828]
	**
	**  ram[0xe2] = (ram[0x806]&ram[0x826])|ram[0x820]
	**  ram[0xe3] = ram[0x826]
	**
	**  ram[0x361] = (ram[0x8cd]&ram[0x8ad])|ram[0x8ce]
	**  ram[0x362] = ram[0x8ae] = 0x32
	**
	**  ram[0x363] = ram[0x809]+ram[0x829]+ram[0x828]
	**  ram[0x364] = ram[0x808]
	*/

	switch (offset)
	{
	case 0x08: return 0xb0; /* random mole placement */
	case 0x26:
		if (m_maincpu->pc() == 0x53d5)
		{
			return 0x06; /* bonus round */
		}
		else
		{ // pc == 0x5159, 0x5160
			return 0xc6; /* game start */
		}
	case 0x86: return 0x91; /* game over */
	case 0xae: return 0x32; /* coinage */
	}

	/*  The above are critical protection reads.
	**  It isn't clear what effect (if any) the
	**  remaining reads have; for now we simply
	**  return 0x00
	*/

	return 0x00;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void mole_state::main_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x0800, 0x08ff).r(FUNC(mole_state::protection_r));
	map(0x0800, 0x0800).nopw(); // ???
	map(0x0820, 0x0820).nopw(); // ???
	map(0x5000, 0x7fff).mirror(0x8000).rom();
	map(0x8000, 0x83ff).w(FUNC(mole_state::tileram_w)).nopr();
	map(0x8400, 0x8400).w(FUNC(mole_state::tilebank_w));
	map(0x8c00, 0x8c01).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x8c40, 0x8c40).nopw(); // ???
	map(0x8c80, 0x8c80).nopw(); // ???
	map(0x8c81, 0x8c81).nopw(); // ???
	map(0x8d00, 0x8d00).portr("DSW").w(FUNC(mole_state::irqack_w));
	map(0x8d40, 0x8d40).portr("IN0");
	map(0x8d80, 0x8d80).portr("IN1");
	map(0x8dc0, 0x8dc0).portr("IN2").w(FUNC(mole_state::flipscreen_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mole )
	PORT_START("DSW")   /* 0x8d00 */
	PORT_DIPNAME( 0x01, 0x00, "Round Points" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x04, "B" )
	PORT_DIPSETTING(    0x08, "C" )
	PORT_DIPSETTING(    0x0c, "D" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x10, "B" )
	PORT_DIPSETTING(    0x20, "C" )
	PORT_DIPSETTING(    0x30, "D" )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN0")   /* 0x8d40 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Pad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Pad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Pad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Pad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("P1 Pad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("P1 Pad 8") PORT_CODE(KEYCODE_8_PAD)

	PORT_START("IN1")   /* 0x8d80 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("P1 Pad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Pad 1") PORT_CODE(KEYCODE_Q) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Pad 2") PORT_CODE(KEYCODE_W) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Pad 3") PORT_CODE(KEYCODE_E) PORT_COCKTAIL
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN2")   /* 0x8dc0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("P2 Pad 8") PORT_CODE(KEYCODE_X) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("P2 Pad 7") PORT_CODE(KEYCODE_Z) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Pad 4") PORT_CODE(KEYCODE_A) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("P2 Pad 9") PORT_CODE(KEYCODE_C) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Pad 6") PORT_CODE(KEYCODE_D) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Pad 5") PORT_CODE(KEYCODE_S) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tile_layout =
{
	8,8,    /* character size */
	512,    /* number of characters */
	3,      /* number of bitplanes */
	{ 0x0000*8, 0x1000*8, 0x2000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( gfx_mole )
	GFXDECODE_ENTRY( "gfx", 0x0000, tile_layout, 0x00, 1 )
	GFXDECODE_ENTRY( "gfx", 0x3000, tile_layout, 0x00, 1 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mole_state::mole(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 2'000'000); // ???
	m_maincpu->set_addrmap(AS_PROGRAM, &mole_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(mole_state::irq0_line_assert));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(40*8, 25*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 25*8-1);
	screen.set_screen_update(FUNC(mole_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mole);
	PALETTE(config, "palette", palette_device::RBG_3BIT);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, "aysnd", 2000000).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mole ) // ALL ROMS ARE 2732
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for 6502 code
	ROM_LOAD( "m3a.5h", 0x5000, 0x1000, CRC(5fbbdfef) SHA1(8129e90a05b3ca50f47f7610eec51c16c8609590) )
	ROM_LOAD( "m2a.7h", 0x6000, 0x1000, CRC(f2a90642) SHA1(da6887725d70924fc4b9cca83172276976f5020c) )
	ROM_LOAD( "m1a.8h", 0x7000, 0x1000, CRC(cff0119a) SHA1(48fc81b8c68e977680e7b8baf1193f0e7e0cd013) )

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "mea.4a", 0x0000, 0x1000, CRC(49d89116) SHA1(aa4cde07e10624072e50ba5bd209acf93092cf78) )
	ROM_LOAD( "mca.6a", 0x1000, 0x1000, CRC(04e90300) SHA1(c908a3a651e50428eedc2974160cdbf2ed946abc) )
	ROM_LOAD( "maa.9a", 0x2000, 0x1000, CRC(6ce9442b) SHA1(c08bf0911f1dfd4a3f9452efcbb3fd3688c4bf8c) )
	ROM_LOAD( "mfa.3a", 0x3000, 0x1000, CRC(0d0c7d13) SHA1(8a6d371571391f2b54ffa65b77e4e83fd607d2c9) )
	ROM_LOAD( "mda.5a", 0x4000, 0x1000, CRC(41ae1842) SHA1(afc169c3245b0946ef81e65d0b755d498ee71667) )
	ROM_LOAD( "mba.8a", 0x5000, 0x1000, CRC(50c43fc9) SHA1(af478f3d89cd6c87f32dcdda7fabce25738c340b) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, mole, 0, mole, mole, mole_state, empty_init, ROT0, "Yachiyo Electronics", "Mole Attack", MACHINE_SUPPORTS_SAVE )
