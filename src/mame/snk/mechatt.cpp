// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Mechanized Attack       A8002   (c) 1989 SNK Corporation

    Compared to Beast Busters (A9003), Mechanized Attack (A8002) is an
    earlier design, it only has one sprite chip, no eeprom, and only 2
    machine guns, but the tilemaps are twice the size.

    ----------------------------------------------------------------------------------------

    Stephh's notes (based on the games M68000 code and some tests) :

    2) 'mechatt'

        - Country/version is stored at 0x06a000.w and the possible values are :

            Value   Country
            0x0000    Japan
            0x1111    World    (value stored in the current set)
            0x2222    US
            0x3333    Asia?    (it looks like Japanese text but some "symbols" are missing)

    2a) Japan version

        - All texts are in Japanese.
        - "(c) 1989 (Corp) S-N-K".
        - "Coin Slots" Dip Switch has no effect.
        - "Coin A" and "Coin B" Dip Switches are the same as in the World version.
        - Coin buttons effect :

        * "Coin Slots" are ALWAYS considered as "Common" :

            . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
            . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
            . COIN3    : NO EFFECT !
            . COIN4    : NO EFFECT !
            . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

    2b) World version

        - All texts are in English.
        - "(c) 1989 SNK Corporation".
        - Coin buttons effect :

    * "Coin Slots" Dip Switch set to "Common" :

        . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
        . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
        . COIN3    : NO EFFECT !
        . COIN4    : NO EFFECT !
        . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

    * "Coin Slots" Dip Switch set to "Individual" :

        . COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
        . COIN2    : adds coin(s)/credit(s) for player 1 depending on "Coin B" Dip Switch
        . COIN3    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
        . COIN4    : adds coin(s)/credit(s) for player 2 depending on "Coin B" Dip Switch
        . SERVICE1 : adds coin(s)/credit(s) for all players depending on "Coin A" Dip Switch

    2c) US version

        - All texts are in English.
        - "(c) 1989 SNK Corp. of America".
        - Additional FBI logo as first screen as well as small FBI notice at the bottom left
        of the screens until a coin is inserted.
        - "Coin Slots" Dip Switch has no effect.
        - "Coin A" Dip Switch is different from the World version :

            World      US
            4C_1C    "Free Play"
            3C_1C    special (see below)
            2C_1C    "2 Coins to Start, 1 to Continue"
            1C_1C    1C_1C

        It's a bit hard to explain the "special" coinage, so here are some infos :

            * when you insert a coin before starting a game, you are awarded 2 credits
            if credits == 0, else you are awarded 1 credit
            * when you insert a coin to continue, you are ALWAYS awarded 1 credit

        - "Coin B" Dip Switch has no effect.

        - Coin buttons effect :

        * "Coin Slots" are ALWAYS considered as "Individual" :

            . COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
            . COIN2    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
            . COIN3    : NO EFFECT !
            . COIN4    : NO EFFECT !
            . SERVICE1 : adds coin(s)/credit(s) for all players depending on "Coin A" Dip Switch

    2d) Asia? version

        - All texts are in Japanese ? (to be confirmed)
        - "(c) 1989 SNK Corporation".
        - "Coin Slots" Dip Switch has no effect.
        - "Coin A" and "Coin B" Dip Switches are the same as in the World version.
        - Coin buttons effect :

        * "Coin Slots" are ALWAYS considered as "Common" :

            . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
            . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
            . COIN3    : NO EFFECT !
            . COIN4    : NO EFFECT !
            . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

    ----------------------------------------------------------------------------------------

    HIGHWAYMAN's notes:

    after adding the mechanized attack u.s. roms I suspect that there is more than just a few bytes changed ;-)

    ----------------------------------------------------------------------------------------

    Mech Attack Region code works as follows

    ROM[0x06a000/2] = (data << 12) | (data << 8) | (data << 4) | (data << 0);

    Country :
    - 0x0000 : Japan
    - 0x1111 : World (default)
    - 0x2222 : US
    - 0x3333 : Asia?

***************************************************************************/

#include "emu.h"

// src/mame
#include "snk_bbusters_spr.h"
// src/devices
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "video/bufsprite.h"
// src/emu
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class mechatt_state : public driver_device
{
public:
	mechatt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_sprites(*this, "sprites1"),
		m_spriteram(*this, "spriteram1"),
		m_soundlatch(*this, "soundlatch%u", 1U),
		m_tx_videoram(*this, "tx_videoram"),
		m_pf_data(*this, "pf%u_data", 1U),
		m_pf_scroll_data(*this, "pf%u_scroll_data", 1U),
		m_gun_io(*this, { "GUNX1", "GUNY1", "GUNX2", "GUNY2" }),
		m_gun_recoil(*this, "Player%u_Gun_Recoil", 1U)
	{ }

	void mechatt(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<snk_bbusters_spr_device> m_sprites;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_shared_ptr<uint16_t> m_tx_videoram;
	required_shared_ptr_array<uint16_t, 2> m_pf_data;
	required_shared_ptr_array<uint16_t, 2> m_pf_scroll_data;
	required_ioport_array<4> m_gun_io;
	output_finder<2> m_gun_recoil;

	tilemap_t *m_fix_tilemap = nullptr;
	tilemap_t *m_pf_tilemap[2]{};

	TILE_GET_INFO_MEMBER(get_tile_info);
	template <int Layer, int Gfx> TILE_GET_INFO_MEMBER(get_pf_tile_info);

	void sound_cpu_w(uint8_t data);
	void video_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Layer> void pf_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void coin_counter_w(uint8_t data);

	void mechatt_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sounda_portmap(address_map &map) ATTR_COLD;

	void two_gun_output_w(uint16_t data);
	uint16_t mechatt_gun_r(offs_t offset);

	template <typename Proc>
	void mix_sprites(bitmap_ind16 &bitmap, bitmap_ind16 &srcbitmap, const rectangle &cliprect, Proc MIX);

	bitmap_ind16 m_bitmap_sprites;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/******************************************************************************/

void mechatt_state::machine_start()
{
	m_gun_recoil.resolve();
}

void mechatt_state::sound_cpu_w(uint8_t data)
{
	m_soundlatch[0]->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

template<int Layer>
void mechatt_state::pf_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pf_data[Layer][offset]);
	m_pf_tilemap[Layer]->mark_tile_dirty(offset);
}

void mechatt_state::coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}


TILE_GET_INFO_MEMBER(mechatt_state::get_tile_info)
{
	uint16_t tile = m_tx_videoram[tile_index];
	tileinfo.set(0, tile&0xfff, tile>>12, 0);
}

template <int Layer, int Gfx>
TILE_GET_INFO_MEMBER(mechatt_state::get_pf_tile_info)
{
	uint16_t tile = m_pf_data[Layer][tile_index];
	tileinfo.set(Gfx, tile&0xfff, tile>>12, 0);
}

void mechatt_state::video_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_fix_tilemap->mark_tile_dirty(offset);
}

/******************************************************************************/

void mechatt_state::video_start()
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mechatt_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fix_tilemap->set_transparent_pen(15);

	m_pf_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&mechatt_state::get_pf_tile_info<0, 1>))), TILEMAP_SCAN_COLS, 16, 16, 256, 32);
	m_pf_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&mechatt_state::get_pf_tile_info<1, 2>))), TILEMAP_SCAN_COLS, 16, 16, 256, 32);

	m_pf_tilemap[0]->set_transparent_pen(15);

	m_screen->register_screen_bitmap(m_bitmap_sprites);
	m_bitmap_sprites.fill(0xffff);
}

template <typename Proc>
void mechatt_state::mix_sprites(bitmap_ind16 &bitmap, bitmap_ind16 &srcbitmap, const rectangle &cliprect, Proc MIX)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t *srcbuf = &srcbitmap.pix(y);
		uint16_t *dstbuf = &bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t srcdat = srcbuf[x];
			if ((srcdat & 0xf) != 0xf)
				MIX(srcdat, x, dstbuf);
		}
	}
}

uint32_t mechatt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bitmap_sprites.fill(0xffff);
	m_sprites->draw_sprites(m_bitmap_sprites, cliprect);

	m_pf_tilemap[0]->set_scrollx(0, m_pf_scroll_data[0][0]);
	m_pf_tilemap[0]->set_scrolly(0, m_pf_scroll_data[0][1]);
	m_pf_tilemap[1]->set_scrollx(0, m_pf_scroll_data[1][0]);
	m_pf_tilemap[1]->set_scrolly(0, m_pf_scroll_data[1][1]);

	m_pf_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	// during level 3, plane sprites must rise from the underground, obscured by the fg tilemap
	mix_sprites(bitmap, m_bitmap_sprites, cliprect, [](uint16_t srcdat, uint16_t x, uint16_t* dstbuf) { if ((srcdat & 0xc0) == 0xc0) dstbuf[x] = srcdat + 256; } );
	m_pf_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	mix_sprites(bitmap, m_bitmap_sprites, cliprect, [](uint16_t srcdat, uint16_t x, uint16_t* dstbuf) { if ((srcdat & 0xc0) != 0xc0) dstbuf[x] = srcdat + 256; } );

	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/*******************************************************************************/

void mechatt_state::two_gun_output_w(uint16_t data)
{
	for (int i = 0; i < 2; i++)
		m_gun_recoil[i] = BIT(data, i);
}

uint16_t mechatt_state::mechatt_gun_r(offs_t offset)
{
	int x = m_gun_io[offset ? 2 : 0]->read();
	int y = m_gun_io[offset ? 3 : 1]->read();

	// TODO - does the hardware really clamp like this?
	x += 0x18;
	if (x > 0xff) x = 0xff;
	if (y > 0xef) y = 0xef;

	return x | (y << 8);
}

void mechatt_state::mechatt_map(address_map &map)
{
	map(0x000000, 0x06ffff).rom();
	map(0x070000, 0x07ffff).ram().share("ram");
	map(0x090000, 0x090fff).ram().w(FUNC(mechatt_state::video_w)).share("tx_videoram");
	map(0x0a0000, 0x0a0fff).ram().share("spriteram1");
	map(0x0a1000, 0x0a7fff).nopw();
	map(0x0b0000, 0x0b3fff).ram().w(FUNC(mechatt_state::pf_w<0>)).share("pf1_data");
	map(0x0b8000, 0x0b8003).writeonly().share("pf1_scroll_data");
	map(0x0c0000, 0x0c3fff).ram().w(FUNC(mechatt_state::pf_w<1>)).share("pf2_data");
	map(0x0c8000, 0x0c8003).writeonly().share("pf2_scroll_data");
	map(0x0d0000, 0x0d07ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x0e0000, 0x0e0001).portr("IN0");
	map(0x0e0002, 0x0e0003).portr("DSW1");
	map(0x0e0004, 0x0e0007).r(FUNC(mechatt_state::mechatt_gun_r));
	map(0x0e4000, 0x0e4001).w(FUNC(mechatt_state::coin_counter_w));
	map(0x0e4002, 0x0e4003).w(FUNC(mechatt_state::two_gun_output_w));
	map(0x0e8001, 0x0e8001).r(m_soundlatch[1], FUNC(generic_latch_8_device::read)).w(FUNC(mechatt_state::sound_cpu_w));
}

/******************************************************************************/

void mechatt_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch[0], FUNC(generic_latch_8_device::read)).w(m_soundlatch[1], FUNC(generic_latch_8_device::write));
}

void mechatt_state::sounda_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym2608_device::read), FUNC(ym2608_device::write));
	map(0xc0, 0xc1).nopw(); // -> Main CPU
}

/******************************************************************************/

static INPUT_PORTS_START( mechatt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )     // See notes
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire")  // "Fire"
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Grenade")   // "Grenade"
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")  // "Fire"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Grenade")   // "Grenade"
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Slots" )                PORT_DIPLOCATION("SW1:1") // Listed as "Unused" (manual from different revision/region?), See notes
	PORT_DIPSETTING(      0x0001, "Common" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Magazine / Grenade" )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, "5 / 2" )
	PORT_DIPSETTING(      0x000c, "6 / 3" )
	PORT_DIPSETTING(      0x0004, "7 / 4" )
	PORT_DIPSETTING(      0x0000, "8 / 5" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:5,6") // See notes
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:7,8") // Listed as "Unused" (manual from different revision/region?), See notes
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Game Mode" )                 PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0c00, "Demo Sounds On" )
	PORT_DIPSETTING(      0x0400, "Infinite Energy (Cheat)")
	PORT_DIPSETTING(      0x0000, "Freeze" )
	PORT_DIPUNUSED_DIPLOC(0x1000, 0x1000, "SW2:5" )         /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC(0x2000, 0x2000, "SW2:6" )         /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC(0x4000, 0x4000, "SW2:7" )         /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("GUNX1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("GUNY1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("GUNY2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( mechattj )
	PORT_INCLUDE( mechatt )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW1:1" )
INPUT_PORTS_END

static INPUT_PORTS_START( mechattu )
	PORT_INCLUDE( mechatt )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0010, "1 Coin/2 Credits first, then 1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0020, "2 Coins/1 Credit first, then 1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x00c0, 0x00c0, "SW1:7,8" )
INPUT_PORTS_END

static INPUT_PORTS_START( mechattu1 )
	PORT_INCLUDE( mechattu )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("GUNX2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("GUNY2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/******************************************************************************/

static GFXDECODE_START( gfx_mechatt )
	GFXDECODE_ENTRY( "tx_tiles", 0, gfx_8x8x4_packed_msb,                 0, 16 )
	GFXDECODE_ENTRY( "gfx3",     0, gfx_8x8x4_col_2x2_group_packed_msb, 512, 16 )
	GFXDECODE_ENTRY( "gfx4",     0, gfx_8x8x4_col_2x2_group_packed_msb, 768, 16 )
GFXDECODE_END


/******************************************************************************/

void mechatt_state::mechatt(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mechatt_state::mechatt_map);
	m_maincpu->set_vblank_int("screen", FUNC(mechatt_state::irq4_line_hold));

	Z80(config, m_audiocpu, 4000000); /* Accurate */
	m_audiocpu->set_addrmap(AS_PROGRAM, &mechatt_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &mechatt_state::sounda_portmap);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(mechatt_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mechatt);
	PALETTE(config, "palette").set_format(palette_device::RGBx_444, 1024);

	BUFFERED_SPRITERAM16(config, m_spriteram);

	SNK_BBUSTERS_SPR(config, m_sprites, 0);
	m_sprites->set_scaletable_tag("sprites1:scale_table");
	m_sprites->set_palette("palette");
	m_sprites->set_spriteram_tag("spriteram1");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	ym2608_device &ymsnd(YM2608(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "lspeaker", 0.15);
	ymsnd.add_route(0, "rspeaker", 0.15);
	ymsnd.add_route(1, "lspeaker", 0.80);
	ymsnd.add_route(2, "rspeaker", 0.80);
}

/******************************************************************************/

ROM_START( mechatt )
	ROM_REGION( 0x80000, "maincpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD16_BYTE( "ma5-e.n12", 0x000000, 0x20000, CRC(9bbb852a) SHA1(34b696bf79cf53cac1c384a3143c0f3f243a71f3) )
	ROM_LOAD16_BYTE( "ma4.l12",   0x000001, 0x20000, CRC(0d414918) SHA1(0d51b893d37ba124b983beebb691e65bdc52d300) )
	ROM_LOAD16_BYTE( "ma7.n13",   0x040000, 0x20000, CRC(61d85e1b) SHA1(46234d48ac21c481a5e70c6a654a341ebdd4cd3a) )
	ROM_LOAD16_BYTE( "ma6-f.l13", 0x040001, 0x20000, CRC(4055fe8d) SHA1(b4d8bd5f73805ce1c332eff657dddbb88ff45b37) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_3.e13", 0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "tx_tiles", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "ma_1.l2", 0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "sprites1", 0 ) // Located on the A8002-2 board
	ROM_LOAD16_WORD_SWAP( "mao89p13.bin",  0x000000, 0x80000, CRC(8bcb16cf) SHA1(409ee1944188d9ce39adce29b1df029b560dd5b0) )
	ROM_LOAD16_WORD_SWAP( "ma189p15.bin",  0x080000, 0x80000, CRC(b84d9658) SHA1(448adecb0067d8f5b219ec2f94a8dec84187a554) )
	ROM_LOAD16_WORD_SWAP( "ma289p17.bin",  0x100000, 0x80000, CRC(6cbe08ac) SHA1(8f81f6e92b84ab6867452011d52f3e7689c62a1a) )
	ROM_LOAD16_WORD_SWAP( "ma389m15.bin",  0x180000, 0x80000, CRC(34d4585e) SHA1(38d9fd5d775e4b3c8b8b487a6ba9b8bdcb3274b0) )

	ROM_REGION( 0x80000, "gfx3", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mab189a2.bin",  0x000000, 0x80000, CRC(e1c8b4d0) SHA1(2f8a1839cca892f8380c7cffe7a12e615d38fd55) )

	ROM_REGION( 0x80000, "gfx4", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mab289c2.bin",  0x000000, 0x80000, CRC(14f97ceb) SHA1(a22033532ea616dc3a3db8b66ad6ccc6172ed7cc) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_2.d10", 0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "sprites1:scale_table", 0 ) // Zoom table - Located on the A8002-2 board
	ROM_LOAD( "ma_8.f10", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma_9.f12", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) // identical to ma_8.f10
ROM_END

ROM_START( mechattj ) // Uses EPROMs on official SNK A8002-5 & A8002-6 sub boards instead of MaskROMs
	ROM_REGION( 0x80000, "maincpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD16_BYTE( "ma5j.n12", 0x000000, 0x20000, CRC(e6bb5952) SHA1(3b01eccc20d99fd33ff8e303afa902abb66e1036) )
	ROM_LOAD16_BYTE( "ma4j.l12", 0x000001, 0x20000, CRC(c78baa62) SHA1(c3554698fbc94e3625269c5cb1fc664364f3fb3f) )
	ROM_LOAD16_BYTE( "ma7j.n13", 0x040000, 0x20000, CRC(12a68fc2) SHA1(c935788723d8ea3bfe99244b8c7b2aff85579912) )
	ROM_LOAD16_BYTE( "ma6j.l13", 0x040001, 0x20000, CRC(332b2f54) SHA1(c768f5437a20ea406523d3de9e1ea807b39e1622) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_3.e13", 0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "tx_tiles", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "ma_1.l2", 0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "sprites1", 0 ) // Located on the A8002-6 sub board
	ROM_LOAD16_BYTE( "s_9.a1",  0x000001, 0x20000, CRC(6e8e194c) SHA1(02bbd573a322a3f7f8e92ccceebffdd598b5489e) ) // these 4 == mao89p13.bin
	ROM_LOAD16_BYTE( "s_1.b1",  0x000000, 0x20000, CRC(fd9161ed) SHA1(b3e2434dd9cb1cafe1022774b863b5f1a008a9d2) )
	ROM_LOAD16_BYTE( "s_10.a2", 0x040001, 0x20000, CRC(fad6a1ab) SHA1(5347b4493c8004dc8cedc0b37aba494f203142b8) )
	ROM_LOAD16_BYTE( "s_2.b2",  0x040000, 0x20000, CRC(549056f0) SHA1(f515aa98ab25f3735dbfdefcb8d55ba3b2075b70) )
	ROM_LOAD16_BYTE( "s_11.a3", 0x080001, 0x20000, CRC(3887a382) SHA1(b40861fc1414b2fa299772e76a78cb8dc00b71b7) ) // these 4 == ma189p15.bin
	ROM_LOAD16_BYTE( "s_3.b3",  0x080000, 0x20000, CRC(cb99f565) SHA1(9ed1b21f4a33b9a614bca38610378857560cdaba) )
	ROM_LOAD16_BYTE( "s_12.a4", 0x0c0001, 0x20000, CRC(63417b49) SHA1(786249fa7e8770de5b5882debdc2913d58e9170e) )
	ROM_LOAD16_BYTE( "s_4.b4",  0x0c0000, 0x20000, CRC(d739d48a) SHA1(04d2ecea72b6e651b815865946c9c9cfae4e5c4d) )
	ROM_LOAD16_BYTE( "s_13.a5", 0x100001, 0x20000, CRC(eccd47b6) SHA1(6b9c63fee97a7568114f227a89a1effd6b04806a) ) // these 4 == ma289p17.bin
	ROM_LOAD16_BYTE( "s_5.b5",  0x100000, 0x20000, CRC(e15244da) SHA1(ebf3072565c53d0098d373b5093ba6918c4eddae) )
	ROM_LOAD16_BYTE( "s_14.a6", 0x140001, 0x20000, CRC(bbbf0461) SHA1(c5299ab1d45f685a5d160492247cf1303ef6937a) )
	ROM_LOAD16_BYTE( "s_6.b6",  0x140000, 0x20000, CRC(4ee89f75) SHA1(bda0e9095da2d424faac341fd934000a621796eb) )
	ROM_LOAD16_BYTE( "s_15.a7", 0x180001, 0x20000, CRC(cde29bad) SHA1(24c1b43c6d717eaaf7c01ec7de89837947334224) ) // these 4 == ma389m15.bin
	ROM_LOAD16_BYTE( "s_7.b7",  0x180000, 0x20000, CRC(065ed221) SHA1(c03ca5b4d1198939a57b5fccf6a79d70afe1faaf) )
	ROM_LOAD16_BYTE( "s_16.a8", 0x1c0001, 0x20000, CRC(70f28040) SHA1(91012728953563fcc576725337e6ba7e1b49d1ba) )
	ROM_LOAD16_BYTE( "s_8.b8",  0x1c0000, 0x20000, CRC(a6f8574f) SHA1(87c041669b2eaec495ae10a6f45b6668accb92bf) )

	ROM_REGION( 0x80000, "gfx3", 0 ) // these 4 == mab189a2.bin - Located on the A8002-5 sub board
	ROM_LOAD( "s_21.b3", 0x000000, 0x20000, CRC(701a0072) SHA1(b03b6fa18e0cfcd5c7c541025fa2d3632d2f8387) )
	ROM_LOAD( "s_22.b4", 0x020000, 0x20000, CRC(34e6225c) SHA1(f6335084f4f4c7a4b6528e6ad74962b88f81e3bc) )
	ROM_LOAD( "s_23.b5", 0x040000, 0x20000, CRC(9a7399d3) SHA1(04e0327b0da75f621b51e1831cbdc4537082e32b) )
	ROM_LOAD( "s_24.b6", 0x060000, 0x20000, CRC(f097459d) SHA1(466364677f048519eb2894ddecf76f5c52f6afe9) )

	ROM_REGION( 0x80000, "gfx4", 0 ) // these 4 == mab289c2.bin - Located on the A8002-5 sub board
	ROM_LOAD( "s_17.a3", 0x000000, 0x20000, CRC(cc47c4a3) SHA1(140f53b671b4eaed6fcc516c4018f07a6d7c2290) )
	ROM_LOAD( "s_18.a4", 0x020000, 0x20000, CRC(a04377e8) SHA1(841c6c3073b137f6a5c875db32039186c014f785) )
	ROM_LOAD( "s_19.a5", 0x040000, 0x20000, CRC(b07f5289) SHA1(8817bd225edf9b0fa439b220617f925365e39253) )
	ROM_LOAD( "s_20.a6", 0x060000, 0x20000, CRC(a9bb4fa9) SHA1(ccede784671a864667b92a8101d686c26c78d76f) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_2.d10", 0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "sprites1:scale_table", 0 ) // Zoom table - Located on the A8002-2 board
	ROM_LOAD( "ma_8.f10", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma_9.f12", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) // identical to ma_8.f10
ROM_END

ROM_START( mechattu )
	ROM_REGION( 0x80000, "maincpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD16_BYTE( "ma5u.n12", 0x000000, 0x20000, CRC(485ea606) SHA1(0c499f08d7c6d861ba7c50a8f577823613a7923c) )
	ROM_LOAD16_BYTE( "ma4u.l12", 0x000001, 0x20000, CRC(09fa31ec) SHA1(008abb2e09f83614c277471e534f20cba3e354d7) )
	ROM_LOAD16_BYTE( "ma7u.n13", 0x040000, 0x20000, CRC(f45b2c70) SHA1(65523d202d378bab890f1f7bffdde152dd246d4a) )
	ROM_LOAD16_BYTE( "ma6u.l13", 0x040001, 0x20000, CRC(d5d68ce6) SHA1(16057d882781015f6d1c7bb659e0812a8459c3f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_3.e13", 0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "tx_tiles", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "ma_1.l2", 0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "sprites1", 0 ) // Located on the A8002-2 board
	ROM_LOAD16_WORD_SWAP( "mao89p13.bin",  0x000000, 0x80000, CRC(8bcb16cf) SHA1(409ee1944188d9ce39adce29b1df029b560dd5b0) )
	ROM_LOAD16_WORD_SWAP( "ma189p15.bin",  0x080000, 0x80000, CRC(b84d9658) SHA1(448adecb0067d8f5b219ec2f94a8dec84187a554) )
	ROM_LOAD16_WORD_SWAP( "ma289p17.bin",  0x100000, 0x80000, CRC(6cbe08ac) SHA1(8f81f6e92b84ab6867452011d52f3e7689c62a1a) )
	ROM_LOAD16_WORD_SWAP( "ma389m15.bin",  0x180000, 0x80000, CRC(34d4585e) SHA1(38d9fd5d775e4b3c8b8b487a6ba9b8bdcb3274b0) )

	ROM_REGION( 0x80000, "gfx3", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mab189a2.bin",  0x000000, 0x80000, CRC(e1c8b4d0) SHA1(2f8a1839cca892f8380c7cffe7a12e615d38fd55) )

	ROM_REGION( 0x80000, "gfx4", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mab289c2.bin",  0x000000, 0x80000, CRC(14f97ceb) SHA1(a22033532ea616dc3a3db8b66ad6ccc6172ed7cc) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_2.d10", 0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "sprites1:scale_table", 0 ) // Zoom table - Located on the A8002-2 board
	ROM_LOAD( "ma_8.f10", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma_9.f12", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) // identical to ma_8.f10
ROM_END

/* does Ver1 on the roms mean it's a revision, the first version, or used because it's the single player version? */
ROM_START( mechattu1 ) // Uses EPROMs on official SNK A8002-5 & A8002-6 sub boards instead of MaskROMs
	ROM_REGION( 0x80000, "maincpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD16_BYTE( "ma_ver1_5u.n12", 0x000000, 0x20000, CRC(dcd2e971) SHA1(e292b251c429b6990e97233e86360e5d43f573f2) )
	ROM_LOAD16_BYTE( "ma_ver1_4u.l12", 0x000001, 0x20000, CRC(69c8a85b) SHA1(07c6d395772a5e096e3ac42c5248eadccc146ad1) )
	ROM_LOAD16_BYTE( "ma7u.n13",       0x040000, 0x20000, CRC(f45b2c70) SHA1(65523d202d378bab890f1f7bffdde152dd246d4a) )
	ROM_LOAD16_BYTE( "ma6u.l13",       0x040001, 0x20000, CRC(d5d68ce6) SHA1(16057d882781015f6d1c7bb659e0812a8459c3f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_3.e13", 0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "tx_tiles", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "ma_1.l2", 0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "sprites1", 0 ) // Located on the A8002-6 sub board
	ROM_LOAD16_BYTE( "s_9.a1",  0x000001, 0x20000, CRC(6e8e194c) SHA1(02bbd573a322a3f7f8e92ccceebffdd598b5489e) ) // these 4 == mao89p13.bin
	ROM_LOAD16_BYTE( "s_1.b1",  0x000000, 0x20000, CRC(fd9161ed) SHA1(b3e2434dd9cb1cafe1022774b863b5f1a008a9d2) )
	ROM_LOAD16_BYTE( "s_10.a2", 0x040001, 0x20000, CRC(fad6a1ab) SHA1(5347b4493c8004dc8cedc0b37aba494f203142b8) )
	ROM_LOAD16_BYTE( "s_2.b2",  0x040000, 0x20000, CRC(549056f0) SHA1(f515aa98ab25f3735dbfdefcb8d55ba3b2075b70) )
	ROM_LOAD16_BYTE( "s_11.a3", 0x080001, 0x20000, CRC(3887a382) SHA1(b40861fc1414b2fa299772e76a78cb8dc00b71b7) ) // these 4 == ma189p15.bin
	ROM_LOAD16_BYTE( "s_3.b3",  0x080000, 0x20000, CRC(cb99f565) SHA1(9ed1b21f4a33b9a614bca38610378857560cdaba) )
	ROM_LOAD16_BYTE( "s_12.a4", 0x0c0001, 0x20000, CRC(63417b49) SHA1(786249fa7e8770de5b5882debdc2913d58e9170e) )
	ROM_LOAD16_BYTE( "s_4.b4",  0x0c0000, 0x20000, CRC(d739d48a) SHA1(04d2ecea72b6e651b815865946c9c9cfae4e5c4d) )
	ROM_LOAD16_BYTE( "s_13.a5", 0x100001, 0x20000, CRC(eccd47b6) SHA1(6b9c63fee97a7568114f227a89a1effd6b04806a) ) // these 4 == ma289p17.bin
	ROM_LOAD16_BYTE( "s_5.b5",  0x100000, 0x20000, CRC(e15244da) SHA1(ebf3072565c53d0098d373b5093ba6918c4eddae) )
	ROM_LOAD16_BYTE( "s_14.a6", 0x140001, 0x20000, CRC(bbbf0461) SHA1(c5299ab1d45f685a5d160492247cf1303ef6937a) )
	ROM_LOAD16_BYTE( "s_6.b6",  0x140000, 0x20000, CRC(4ee89f75) SHA1(bda0e9095da2d424faac341fd934000a621796eb) )
	ROM_LOAD16_BYTE( "s_15.a7", 0x180001, 0x20000, CRC(cde29bad) SHA1(24c1b43c6d717eaaf7c01ec7de89837947334224) ) // these 4 == ma389m15.bin
	ROM_LOAD16_BYTE( "s_7.b7",  0x180000, 0x20000, CRC(065ed221) SHA1(c03ca5b4d1198939a57b5fccf6a79d70afe1faaf) )
	ROM_LOAD16_BYTE( "s_16.a8", 0x1c0001, 0x20000, CRC(70f28040) SHA1(91012728953563fcc576725337e6ba7e1b49d1ba) )
	ROM_LOAD16_BYTE( "s_8.b8",  0x1c0000, 0x20000, CRC(a6f8574f) SHA1(87c041669b2eaec495ae10a6f45b6668accb92bf) )

	ROM_REGION( 0x80000, "gfx3", 0 ) // these 4 == mab189a2.bin - Located on the A8002-5 sub board
	ROM_LOAD( "s_21.b3", 0x000000, 0x20000, CRC(701a0072) SHA1(b03b6fa18e0cfcd5c7c541025fa2d3632d2f8387) )
	ROM_LOAD( "s_22.b4", 0x020000, 0x20000, CRC(34e6225c) SHA1(f6335084f4f4c7a4b6528e6ad74962b88f81e3bc) )
	ROM_LOAD( "s_23.b5", 0x040000, 0x20000, CRC(9a7399d3) SHA1(04e0327b0da75f621b51e1831cbdc4537082e32b) )
	ROM_LOAD( "s_24.b6", 0x060000, 0x20000, CRC(f097459d) SHA1(466364677f048519eb2894ddecf76f5c52f6afe9) )

	ROM_REGION( 0x80000, "gfx4", 0 ) // these 4 == mab289c2.bin - Located on the A8002-5 sub board
	ROM_LOAD( "s_17.a3", 0x000000, 0x20000, CRC(cc47c4a3) SHA1(140f53b671b4eaed6fcc516c4018f07a6d7c2290) )
	ROM_LOAD( "s_18.a4", 0x020000, 0x20000, CRC(a04377e8) SHA1(841c6c3073b137f6a5c875db32039186c014f785) )
	ROM_LOAD( "s_19.a5", 0x040000, 0x20000, CRC(b07f5289) SHA1(8817bd225edf9b0fa439b220617f925365e39253) )
	ROM_LOAD( "s_20.a6", 0x060000, 0x20000, CRC(a9bb4fa9) SHA1(ccede784671a864667b92a8101d686c26c78d76f) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_2.d10", 0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "sprites1:scale_table", 0 ) // Zoom table - Located on the A8002-2 board
	ROM_LOAD( "ma_8.f10", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma_9.f12", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) // identical to ma_8.f10
ROM_END

} // anonymous namespace


/******************************************************************************/

GAME( 1989, mechatt,    0,        mechatt,  mechatt,  mechatt_state,  empty_init, ROT0, "SNK", "Mechanized Attack (World)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1989, mechattj,   mechatt,  mechatt,  mechattj, mechatt_state,  empty_init, ROT0, "SNK", "Mechanized Attack (Japan)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1989, mechattu,   mechatt,  mechatt,  mechattu, mechatt_state,  empty_init, ROT0, "SNK", "Mechanized Attack (US)",                           MACHINE_SUPPORTS_SAVE )
GAME( 1989, mechattu1,  mechatt,  mechatt,  mechattu1,mechatt_state,  empty_init, ROT0, "SNK", "Mechanized Attack (US, Version 1, Single Player)", MACHINE_SUPPORTS_SAVE )
