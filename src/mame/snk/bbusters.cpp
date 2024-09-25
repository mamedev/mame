// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Beast Busters           A9003   (c) 1989 SNK Corporation

    Beast Busters is a large dedicated (non-jamma) triple machine gun game,
    the gun positions values are read in an interrupt routine that must be
    called for each position (X and Y for 3 guns, so at least 6 times a
    frame).

    ----------------------------------------------------------------------------------------

    Stephh's notes (based on the games M68000 code and some tests) :

    1) 'bbusters'

    - Country/version is stored at 0x003954.w and the possible values are
    0x0000, 0x0004 and 0x0008 (0x000c being the same as 0x0008), 0x0008
    being the value stored in ROM in the current set.
    I haven't noticed any major effect (copyright/logo, language, coinage),
    the only thing I found is that the text relative to number of coins
    needed is different (but it's a lie as coinage is NOT modified).
    So my guess is that if other versions exist, part of the code (or at
    least data in it) will have to be different.
    Anyway, here is my guess to determine the versions (by using some infos
    from 'mechatt' :

        Value   Country
        0x0000    Japan
        0x0004    US?
        0x0008    World?   (value stored in the current set)
        0x000c    World?   (same as 0x0008 - impossible choice ?)

    - Coin buttons act differently depending on the "Coin Slots" Dip Switch :

    * "Coin Slots" Dip Switch set to "Common" :

        . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
        . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
        . COIN3    : NO EFFECT !
        . COIN4    : NO EFFECT !
        . COIN5    : NO EFFECT !
        . COIN6    : NO EFFECT !
        . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

    * "Coin Slots" Dip Switch set to "Individual" :

        . COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
        . COIN2    : adds coin(s)/credit(s) for player 1 depending on "Coin B" Dip Switch
        . COIN3    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
        . COIN4    : adds coin(s)/credit(s) for player 2 depending on "Coin B" Dip Switch
        . COIN5    : adds coin(s)/credit(s) for player 3 depending on "Coin A" Dip Switch
        . COIN6    : adds coin(s)/credit(s) for player 3 depending on "Coin B" Dip Switch
        . SERVICE1 : adds coin(s)/credit(s) for all players depending on "Coin A" Dip Switch

    Note that I had to map COIN5 and COIN6 to SERVICE2 and SERVICE3 to be
    able to use the default parametrable keys. Let me know if there is a
    another (better ?) way to do so.

    ----------------------------------------------------------------------------------------

    RansAckeR's notes:

    bbusters:

    If you only calibrate the P1 gun or do not hit the correct spots for all guns
    you will get either garbage or a black screen when rebooting.
    According to the manual this happens when the eprom contains invalid gun aim data.

    If you calibrate the guns correctly the game runs as expected:
    1) Using P1 controls fire at the indicated spots.
    2) Using P2 controls fire at the indicated spots.
    3) Using P3 controls fire at the indicated spots.

    The locations of the shots fired in attract mode are defined by a table
    starting at $65000. The value taken from there is combined with data from
    the gun calibration to calculate the final position of the shots.
    Unexpected calibration values will therefore cause the game to show the
    shots in weird positions (see MT07333).

    The EEPROM data starts with the 16 bit calibration values for all six axes
    in the order: Minimum axis 0, middle axis 0, maximum axis 0 (repeat for
    the other 5 axes).

    ----------------------------------------------------------------------------------------

    Beast Busters Region code works as follows

    ROM[0x003954/2] = data * 4;

    Country/Version :

     - 0x0000 : Japan?
     - 0x0004 : US?
     - 0x0008 : World?    (default)
     - 0x000c : World?    (same as 0x0008)

***************************************************************************/

#include "emu.h"

// src/mame
#include "snk_bbusters_spr.h"
// src/devices
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/upd7004.h"
#include "sound/ymopn.h"
#include "video/bufsprite.h"
// src/emu
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class bbusters_state : public driver_device
{
public:
	bbusters_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_sprites(*this, "sprites%u", 1U),
		m_spriteram(*this, "spriteram%u", 1U),
		m_soundlatch(*this, "soundlatch%u", 1U),
		m_tx_videoram(*this, "tx_videoram"),
		m_pf_data(*this, "pf%u_data", 1U),
		m_pf_scroll_data(*this, "pf%u_scroll_data", 1U),
		m_gun_recoil(*this, "Player%u_Gun_Recoil", 1U),
		m_eprom_data(*this, "eeprom")
	{ }

	void bbusters(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<snk_bbusters_spr_device, 2> m_sprites;
	required_device_array<buffered_spriteram16_device, 2> m_spriteram;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;

	required_shared_ptr<uint16_t> m_tx_videoram;
	required_shared_ptr_array<uint16_t, 2> m_pf_data;
	required_shared_ptr_array<uint16_t, 2> m_pf_scroll_data;

	output_finder<3> m_gun_recoil;
	required_shared_ptr<uint16_t> m_eprom_data;

	tilemap_t *m_fix_tilemap = nullptr;
	tilemap_t *m_pf_tilemap[2]{};

	TILE_GET_INFO_MEMBER(get_tile_info);
	template <int Layer, int Gfx> TILE_GET_INFO_MEMBER(get_pf_tile_info);

	void sound_cpu_w(uint8_t data);
	void video_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Layer> void pf_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void coin_counter_w(uint8_t data);

	void bbusters_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;

	uint16_t eprom_r(offs_t offset);
	void three_gun_output_w(uint16_t data);

	void mixlow(bitmap_ind16 &bitmap, bitmap_ind16 &srcbitmap, const rectangle &cliprect);
	void mix(bitmap_ind16 &bitmap, bitmap_ind16 &srcbitmap, const rectangle &cliprect);

	template <typename Proc>
	void mix_sprites(bitmap_ind16 &bitmap, bitmap_ind16 &srcbitmap, const rectangle &cliprect, Proc MIX);

	bitmap_ind16 m_bitmap_sprites[2];
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

void bbusters_state::machine_start()
{
	m_gun_recoil.resolve();
}

void bbusters_state::sound_cpu_w(uint8_t data)
{
	m_soundlatch[0]->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

// EPROM is byte wide, top half of word _must_ be 0xff
uint16_t bbusters_state::eprom_r(offs_t offset)
{
	return (m_eprom_data[offset] & 0xff) | 0xff00;
}

void bbusters_state::three_gun_output_w(uint16_t data)
{
	for (int i = 0; i < 3; i++)
		m_gun_recoil[i] = BIT(data, i);
}

template <int Layer>
void bbusters_state::pf_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pf_data[Layer][offset]);
	m_pf_tilemap[Layer]->mark_tile_dirty(offset);
}

void bbusters_state::coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}


TILE_GET_INFO_MEMBER(bbusters_state::get_tile_info)
{
	uint16_t tile = m_tx_videoram[tile_index];

	tileinfo.set(0, tile&0xfff, tile>>12, 0);
}

template <int Layer, int Gfx>
TILE_GET_INFO_MEMBER(bbusters_state::get_pf_tile_info)
{
	uint16_t tile = m_pf_data[Layer][tile_index];

	tileinfo.set(Gfx, tile&0xfff, tile>>12, 0);
}

void bbusters_state::video_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_fix_tilemap->mark_tile_dirty(offset);
}

/******************************************************************************/


void bbusters_state::video_start()
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bbusters_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fix_tilemap->set_transparent_pen(15);

	m_pf_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&bbusters_state::get_pf_tile_info<0, 1>))), TILEMAP_SCAN_COLS, 16, 16, 128, 32);
	m_pf_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&bbusters_state::get_pf_tile_info<1, 2>))), TILEMAP_SCAN_COLS, 16, 16, 128, 32);

	m_pf_tilemap[0]->set_transparent_pen(15);

	for (int i = 0; i < 2; i++)
	{
		m_screen->register_screen_bitmap(m_bitmap_sprites[i]);
		m_bitmap_sprites[i].fill(0xffff);
	}

}

/******************************************************************************/

/******************************************************************************/

template <typename Proc>
void bbusters_state::mix_sprites(bitmap_ind16 &bitmap, bitmap_ind16 &srcbitmap, const rectangle &cliprect, Proc MIX)
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

uint32_t bbusters_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bitmap_sprites[0].fill(0xffff);
	m_bitmap_sprites[1].fill(0xffff);
	m_sprites[1]->draw_sprites(m_bitmap_sprites[1], cliprect);
	m_sprites[0]->draw_sprites(m_bitmap_sprites[0], cliprect);

	m_pf_tilemap[0]->set_scrollx(0, m_pf_scroll_data[0][0]);
	m_pf_tilemap[0]->set_scrolly(0, m_pf_scroll_data[0][1]);
	m_pf_tilemap[1]->set_scrollx(0, m_pf_scroll_data[1][0]);
	m_pf_tilemap[1]->set_scrolly(0, m_pf_scroll_data[1][1]);

	m_pf_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	// Palettes 0xc-0xf confirmed to be behind tilemap on Beast Busters for 2nd sprite chip (elevator stage)
	mix_sprites(bitmap, m_bitmap_sprites[1], cliprect, [](uint16_t srcdat, uint16_t x, uint16_t *dstbuf) { if ((srcdat & 0xc0) == 0xc0) dstbuf[x] = srcdat + 512; } );
	m_pf_tilemap[0]->draw(screen, bitmap, cliprect, 0, 2);
	mix_sprites(bitmap, m_bitmap_sprites[1], cliprect, [](uint16_t srcdat, uint16_t x, uint16_t *dstbuf) { if ((srcdat & 0xc0) != 0xc0) dstbuf[x] = srcdat + 512; } );
	mix_sprites(bitmap, m_bitmap_sprites[0], cliprect, [](uint16_t srcdat, uint16_t x, uint16_t *dstbuf) { dstbuf[x] = srcdat + 256; } );

	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/*******************************************************************************/

void bbusters_state::bbusters_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x08ffff).ram().share("ram");
	map(0x090000, 0x090fff).ram().w(FUNC(bbusters_state::video_w)).share("tx_videoram");
	map(0x0a0000, 0x0a0fff).ram().share("spriteram1");
	map(0x0a1000, 0x0a7fff).ram();     /* service mode */
	map(0x0a8000, 0x0a8fff).ram().share("spriteram2");
	map(0x0a9000, 0x0affff).ram();     /* service mode */
	map(0x0b0000, 0x0b1fff).ram().w(FUNC(bbusters_state::pf_w<0>)).share("pf1_data");
	map(0x0b2000, 0x0b3fff).ram().w(FUNC(bbusters_state::pf_w<1>)).share("pf2_data");
	map(0x0b4000, 0x0b5fff).ram();     /* service mode */
	map(0x0b8000, 0x0b8003).writeonly().share("pf1_scroll_data");
	map(0x0b8008, 0x0b800b).writeonly().share("pf2_scroll_data");
	map(0x0d0000, 0x0d0fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x0e0000, 0x0e0001).portr("COINS");  /* Coins */
	map(0x0e0002, 0x0e0003).portr("IN0");    /* Player 1 & 2 */
	map(0x0e0004, 0x0e0005).portr("IN1");    /* Player 3 */
	map(0x0e0008, 0x0e0009).portr("DSW1");   /* Dip 1 */
	map(0x0e000a, 0x0e000b).portr("DSW2");   /* Dip 2 */
	map(0x0e0019, 0x0e0019).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));
	map(0x0e8000, 0x0e8003).rw("adc", FUNC(upd7004_device::read), FUNC(upd7004_device::write)).umask16(0x00ff);
	map(0x0f0000, 0x0f0001).w(FUNC(bbusters_state::coin_counter_w));
	map(0x0f0008, 0x0f0009).w(FUNC(bbusters_state::three_gun_output_w));
	map(0x0f0019, 0x0f0019).w(FUNC(bbusters_state::sound_cpu_w));
	map(0x0f8000, 0x0f80ff).r(FUNC(bbusters_state::eprom_r)).writeonly().share("eeprom"); /* Eeprom */
}

/*******************************************************************************/

void bbusters_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch[0], FUNC(generic_latch_8_device::read)).w(m_soundlatch[1], FUNC(generic_latch_8_device::write));
}

void bbusters_state::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xc0, 0xc1).nopw(); /* -> Main CPU */
}

/******************************************************************************/

static INPUT_PORTS_START( bbusters )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire")    // "Fire"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Grenade") // "Grenade"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")    // "Fire"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Grenade") // "Grenade"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Fire")    // "Fire"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Grenade") // "Grenade"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )       // See notes
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x06, 0x06, "Magazine / Grenade" )        PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x04, "5 / 2" )
	PORT_DIPSETTING(    0x06, "7 / 3" )
	PORT_DIPSETTING(    0x02, "9 / 4" )
	PORT_DIPSETTING(    0x00, "12 / 5" )
	/* Manual (from a different revision/region?) says:
	                    SW1:4   SW1:5   SW1:6
	1C_1C 1 To continue OFF     OFF     OFF
	2C_1C 1 To continue ON      OFF     OFF
	1C_2C 1 To continue OFF     ON      OFF
	2C_1C 2 To continue ON      ON      OFF
	3C_1C 1 To continue OFF     OFF     ON
	3C_1C 2 To continue ON      OFF     ON
	4C_3C 1 To continue OFF     ON      ON
	Free Play Mode      OFF     OFF     OFF

	SW1:7 Unused
	SW1:8 Blood color: ON=green OFF=red */
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )                PORT_DIPLOCATION("SW1:8") // See notes
	PORT_DIPSETTING(    0x80, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Mode" )                 PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x0c, "Demo Sounds On" )
	PORT_DIPSETTING(    0x04, "Infinite Energy (Cheat)")
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )            /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )            /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )            /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("GUNY1")
	PORT_BIT(0x3ff, 0x1a6, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x0e6, 0x272) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX1")
	PORT_BIT(0x3ff, 0x23a, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x136, 0x36a) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNY2")
	PORT_BIT(0x3ff, 0x1f6, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x146, 0x2aa) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNX2")
	PORT_BIT(0x3ff, 0x1de, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x10e, 0x2e2) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY3")
	PORT_BIT(0x3ff, 0x21e, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x16e, 0x2f6) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("GUNX3")
	PORT_BIT(0x3ff, 0x212, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x14e, 0x33e) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)
INPUT_PORTS_END

/******************************************************************************/

static GFXDECODE_START( gfx_bbusters )
	GFXDECODE_ENTRY( "tx_tiles", 0, gfx_8x8x4_packed_msb,                      0, 16 )
	GFXDECODE_ENTRY( "gfx4", 0,     gfx_8x8x4_col_2x2_group_packed_msb,      768, 16 )
	GFXDECODE_ENTRY( "gfx5", 0,     gfx_8x8x4_col_2x2_group_packed_msb, 1024+256, 16 )
GFXDECODE_END

/******************************************************************************/

void bbusters_state::bbusters(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbusters_state::bbusters_map);
	m_maincpu->set_vblank_int("screen", FUNC(bbusters_state::irq6_line_hold));

	Z80(config, m_audiocpu, 4000000); // Accurate
	m_audiocpu->set_addrmap(AS_PROGRAM, &bbusters_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &bbusters_state::sound_portmap);

	NVRAM(config, "eeprom", nvram_device::DEFAULT_ALL_1); // actually 28C04 parallel EEPROM

	upd7004_device &adc(UPD7004(config, "adc", 8_MHz_XTAL / 2));
	adc.eoc_ff_callback().set_inputline(m_maincpu, 2);
	adc.in_callback<0>().set_ioport("GUNY1");
	adc.in_callback<1>().set_ioport("GUNX1");
	adc.in_callback<2>().set_ioport("GUNY2");
	adc.in_callback<3>().set_ioport("GUNX2");
	adc.in_callback<4>().set_ioport("GUNY3");
	adc.in_callback<5>().set_ioport("GUNX3");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(bbusters_state::screen_update));
	screen.screen_vblank().set(m_spriteram[0], FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.screen_vblank().append(m_spriteram[1], FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_bbusters);
	PALETTE(config, "palette").set_format(palette_device::RGBx_444, 2048);

	SNK_BBUSTERS_SPR(config, m_sprites[0], 0);
	m_sprites[0]->set_scaletable_tag("sprites1:scale_table");
	m_sprites[0]->set_palette("palette");
	m_sprites[0]->set_spriteram_tag("spriteram1");

	SNK_BBUSTERS_SPR(config, m_sprites[1], 0);
	m_sprites[1]->set_scaletable_tag("sprites2:scale_table");
	m_sprites[1]->set_palette("palette");
	m_sprites[1]->set_spriteram_tag("spriteram2");

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);
	BUFFERED_SPRITERAM16(config, m_spriteram[1]);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "lspeaker", 1.0);
	ymsnd.add_route(0, "rspeaker", 1.0);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

/******************************************************************************/

ROM_START( bbusters )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb-3.k10",   0x000000, 0x20000, CRC(04da1820) SHA1(0b6e06adf9c181d7aef28f781efbdd2c225fe81e) )
	ROM_LOAD16_BYTE( "bb-5.k12",   0x000001, 0x20000, CRC(777e0611) SHA1(b7ac0c6ea3738d560a5be75aed286821de918808) )
	ROM_LOAD16_BYTE( "bb-2.k8",    0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",   0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "tx_tiles", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "sprites1", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD16_WORD_SWAP( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD16_WORD_SWAP( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD16_WORD_SWAP( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD16_WORD_SWAP( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD16_WORD_SWAP( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD16_WORD_SWAP( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "sprites1:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x10000, "sprites2:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "bb-pcmb.l3",  0x000000, 0x80000, CRC(c8d5dd53) SHA1(0f7e94532cc14852ca12c1b792e5479667af899e) )
ROM_END

ROM_START( bbustersu )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb-ver3-u3.k10", 0x000000, 0x20000, CRC(c80ec3bc) SHA1(81cccc920c6dc58ccd20fb38bfede717f534986f) )
	ROM_LOAD16_BYTE( "bb-ver3-u5.k12", 0x000001, 0x20000, CRC(5ded86d1) SHA1(de2ce91b85a1d74e60a7093211c1a7d3c27c1d72) )
	ROM_LOAD16_BYTE( "bb-2.k8",        0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",       0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "tx_tiles", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "sprites1", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD16_WORD_SWAP( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD16_WORD_SWAP( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD16_WORD_SWAP( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD16_WORD_SWAP( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD16_WORD_SWAP( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD16_WORD_SWAP( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "sprites1:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x10000, "sprites2:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )
ROM_END

ROM_START( bbustersua )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb-ver2-u3.k10", 0x000000, 0x20000, CRC(6930088b) SHA1(265f0b584d81b6fdcda5c3a2e0bd15d56443bb35) )
	ROM_LOAD16_BYTE( "bb-ver2-u5.k12", 0x000001, 0x20000, CRC(cfdb2c6c) SHA1(54a837dc84b74d12e931f607f3dc9ee06a7e4d31) )
	ROM_LOAD16_BYTE( "bb-2.k8",        0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",       0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "tx_tiles", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "sprites1", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD16_WORD_SWAP( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD16_WORD_SWAP( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD16_WORD_SWAP( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD16_WORD_SWAP( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD16_WORD_SWAP( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD16_WORD_SWAP( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "sprites1:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x10000, "sprites2:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )
ROM_END

ROM_START( bbustersj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb3_ver2_j3.k10", 0x000000, 0x20000, CRC(6a1cd941) SHA1(d29775703f30b0a440e5e960006c0d33bb09992c) ) /* red "J3" stamped on program labels - 3 Player version */
	ROM_LOAD16_BYTE( "bb5_ver2_j3.k12", 0x000001, 0x20000, CRC(7b180752) SHA1(7ae98e3eb81b19a9208e8dae1cdd64796021d034) )
	ROM_LOAD16_BYTE( "bb-2.k8",    0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",   0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "tx_tiles", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "sprites1", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD16_WORD_SWAP( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD16_WORD_SWAP( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD16_WORD_SWAP( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD16_WORD_SWAP( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD16_WORD_SWAP( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD16_WORD_SWAP( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "sprites1:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x10000, "sprites2:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "bb-pcmb.l3",  0x000000, 0x80000, CRC(c8d5dd53) SHA1(0f7e94532cc14852ca12c1b792e5479667af899e) )
ROM_END

ROM_START( bbustersja )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb3_ver2_j2.k10", 0x000000, 0x20000, CRC(605eb62f) SHA1(b13afd561731ad9115c5b997b8a7a79a57557612) ) /* red "J2" stamped on program labels - 2 Player version */
	ROM_LOAD16_BYTE( "bb5_ver2_j2.k12", 0x000001, 0x20000, CRC(9deea26f) SHA1(c5436db0c55da9b0c5e0e053f59a1e17ee4690a6) )
	ROM_LOAD16_BYTE( "bb-2.k8",    0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",   0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "tx_tiles", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "sprites1", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD16_WORD_SWAP( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD16_WORD_SWAP( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD16_WORD_SWAP( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD16_WORD_SWAP( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD16_WORD_SWAP( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD16_WORD_SWAP( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD16_WORD_SWAP( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "sprites1:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x10000, "sprites2:scale_table", 0 ) /* Zoom table - same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "bb-pcmb.l3",  0x000000, 0x80000, CRC(c8d5dd53) SHA1(0f7e94532cc14852ca12c1b792e5479667af899e) )
ROM_END

} // anonymous namespace

/******************************************************************************/

GAME( 1989, bbusters,   0,        bbusters, bbusters, bbusters_state, empty_init, ROT0, "SNK", "Beast Busters (World)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1989, bbustersu,  bbusters, bbusters, bbusters, bbusters_state, empty_init, ROT0, "SNK", "Beast Busters (US, Version 3)",              MACHINE_SUPPORTS_SAVE )
GAME( 1989, bbustersua, bbusters, bbusters, bbusters, bbusters_state, empty_init, ROT0, "SNK", "Beast Busters (US, Version 2)",              MACHINE_SUPPORTS_SAVE )
GAME( 1989, bbustersj,  bbusters, bbusters, bbusters, bbusters_state, empty_init, ROT0, "SNK", "Beast Busters (Japan, Version 2, 3 Player)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, bbustersja, bbusters, bbusters, bbusters, bbusters_state, empty_init, ROT0, "SNK", "Beast Busters (Japan, Version 2, 2 Player)", MACHINE_SUPPORTS_SAVE )
