// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*
World Cup 90 ( Tecmo ) driver
-----------------------------

Ernesto Corvi
(ernesto@imagina.com)

TODO:
- Dip switches mapping is not complete. ( Anyone has the manual handy? )
- Hook up trackball controls in twcup90t.

CPU #1 : Handles background & foreground tiles, controllers, dipswitches.
CPU #2 : Handles sprites and palette
CPU #3 : Audio.

Memory Layout:

CPU #1
0000-8000 ROM
8000-9000 RAM
a000-a800 Color Ram for background #1 tiles
a800-b000 Video Ram for background #1 tiles
c000-c800 Color Ram for background #2 tiles
c800-d000 Video Ram for background #2 tiles
e000-e800 Color Ram for foreground tiles
e800-f000 Video Ram for foreground tiles
f800-fc00 Common Ram with CPU #2
fc00-fc00 Stick 1 input port
fc02-fc02 Stick 2 input port
fc05-fc05 Start buttons and Coins input port
fc06-fc06 Dip Switch A
fc07-fc07 Dip Switch B

CPU #2
0000-c000 ROM
c000-d000 RAM
d000-d800 RAM Sprite Ram
e000-e800 RAM Palette Ram
f800-fc00 Common Ram with CPU #1

CPU #3
0000-0xc000 ROM
???????????


To enter into input test mode:
-keep pressed one of the start buttons during P.O.S.T.(all sets but twcup90t).
-keep pressed both start buttons during P.O.S.T. until the cross hatch test fade out(in twcup90t).
Press one of the start buttons to exit.


*/

#include "emu.h"

#include "tecmo_spr.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class wc90_state : public driver_device
{
public:
	wc90_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_scrollxlo(*this, "scrollxlo%u", 0U),
		m_scrollxhi(*this, "scrollxhi%u", 0U),
		m_scrollylo(*this, "scrollylo%u", 0U),
		m_scrollyhi(*this, "scrollyhi%u", 0U),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_subbank(*this, "subbank")
	{ }

	void wc90t(machine_config &config);
	void wc90(machine_config &config);
	void pac90(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;

	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr_array<uint8_t, 3> m_scrollxlo;
	required_shared_ptr_array<uint8_t, 3> m_scrollxhi;
	required_shared_ptr_array<uint8_t, 3> m_scrollylo;
	required_shared_ptr_array<uint8_t, 3> m_scrollyhi;
	required_shared_ptr<uint8_t> m_spriteram;

	required_memory_bank m_mainbank;
	required_memory_bank m_subbank;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	void bankswitch_w(uint8_t data);
	void sub_bankswitch_w(uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);
	void txvideoram_w(offs_t offset, uint8_t data);

	uint32_t pri_cb(uint8_t pri);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(track_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(track_get_fg_tile_info);

	DECLARE_VIDEO_START(wc90t);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sound_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(wc90_state::get_bg_tile_info)
{
	int attr = m_bgvideoram[tile_index];
	int tile = m_bgvideoram[tile_index + 0x800] +
					256 * ((attr & 3) + ((attr >> 1) & 4));
	tileinfo.set(2,
			tile,
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90_state::get_fg_tile_info)
{
	int attr = m_fgvideoram[tile_index];
	int tile = m_fgvideoram[tile_index + 0x800] +
					256 * ((attr & 3) + ((attr >> 1) & 4));
	tileinfo.set(1,
			tile,
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90_state::get_tx_tile_info)
{
	tileinfo.set(0,
			m_txvideoram[tile_index + 0x800] + ((m_txvideoram[tile_index] & 0x07) << 8),
			m_txvideoram[tile_index] >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90_state::track_get_bg_tile_info)
{
	int attr = m_bgvideoram[tile_index];
	int tile = m_bgvideoram[tile_index + 0x800] +
					256 * (attr & 7);
	tileinfo.set(2,
			tile,
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90_state::track_get_fg_tile_info)
{
	int attr = m_fgvideoram[tile_index];
	int tile = m_fgvideoram[tile_index + 0x800] +
					256 * (attr & 7);
	tileinfo.set(1,
			tile,
			attr >> 4,
			0);
}


/***************************************************************************

  Callbacks for the sprite priority

***************************************************************************/

uint32_t wc90_state::pri_cb(uint8_t pri)
{
	switch (pri)
	{
		case 0: return 0;
		case 1: return GFX_PMASK_4;
		case 2: return GFX_PMASK_4 | GFX_PMASK_2;
		case 3: return -1;// GFX_PMASK_4 | GFX_PMASK_2 | GFX_PMASK_1; // unused
		default: return -1;
	}
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void wc90_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wc90_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wc90_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wc90_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(wc90_state,wc90t)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wc90_state::track_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wc90_state::track_get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wc90_state::get_tx_tile_info)),       TILEMAP_SCAN_ROWS,  8, 8, 64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void wc90_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void wc90_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void wc90_state::txvideoram_w(offs_t offset, uint8_t data)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t wc90_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	m_bg_tilemap->set_scrollx(0, m_scrollxlo[2][0] + 256 * m_scrollxhi[2][0]);
	m_bg_tilemap->set_scrolly(0, m_scrollylo[2][0] + 256 * m_scrollyhi[2][0]);
	m_fg_tilemap->set_scrollx(0, m_scrollxlo[1][0] + 256 * m_scrollxhi[1][0]);
	m_fg_tilemap->set_scrolly(0, m_scrollylo[1][0] + 256 * m_scrollyhi[1][0]);
	m_tx_tilemap->set_scrollx(0, m_scrollxlo[0][0] + 256 * m_scrollxhi[0][0]);
	m_tx_tilemap->set_scrolly(0, m_scrollylo[0][0] + 256 * m_scrollyhi[0][0]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);
	m_sprgen->draw_wc90_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());
	return 0;
}


void wc90_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data >> 3);
}

void wc90_state::sub_bankswitch_w(uint8_t data)
{
	m_subbank->set_entry(data >> 3);
}

void wc90_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();     // Main RAM
	map(0xa000, 0xafff).ram().w(FUNC(wc90_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xb000, 0xbfff).ram();
	map(0xc000, 0xcfff).ram().w(FUNC(wc90_state::bgvideoram_w)).share(m_bgvideoram);
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xefff).ram().w(FUNC(wc90_state::txvideoram_w)).share(m_txvideoram);
	map(0xf000, 0xf7ff).bankr(m_mainbank);
	map(0xf800, 0xfbff).ram().share("main_sub");
	map(0xfc00, 0xfc00).portr("P1");
	map(0xfc02, 0xfc02).portr("P2");
	map(0xfc05, 0xfc05).portr("SYSTEM");
	map(0xfc06, 0xfc06).portr("DSW1");
	map(0xfc07, 0xfc07).portr("DSW2");
	map(0xfc02, 0xfc02).writeonly().share(m_scrollylo[0]);
	map(0xfc03, 0xfc03).writeonly().share(m_scrollyhi[0]);
	map(0xfc06, 0xfc06).writeonly().share(m_scrollxlo[0]);
	map(0xfc07, 0xfc07).writeonly().share(m_scrollxhi[0]);
	map(0xfc22, 0xfc22).writeonly().share(m_scrollylo[1]);
	map(0xfc23, 0xfc23).writeonly().share(m_scrollyhi[1]);
	map(0xfc26, 0xfc26).writeonly().share(m_scrollxlo[1]);
	map(0xfc27, 0xfc27).writeonly().share(m_scrollxhi[1]);
	map(0xfc42, 0xfc42).writeonly().share(m_scrollylo[2]);
	map(0xfc43, 0xfc43).writeonly().share(m_scrollyhi[2]);
	map(0xfc46, 0xfc46).writeonly().share(m_scrollxlo[2]);
	map(0xfc47, 0xfc47).writeonly().share(m_scrollxhi[2]);
	map(0xfcc0, 0xfcc0).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xfcd0, 0xfcd0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xfce0, 0xfce0).w(FUNC(wc90_state::bankswitch_w));
}

void wc90_state::sub_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().share(m_spriteram);
	map(0xd800, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf000, 0xf7ff).bankr(m_subbank);
	map(0xf800, 0xfbff).ram().share("main_sub");
	map(0xfc00, 0xfc00).w(FUNC(wc90_state::sub_bankswitch_w));
	map(0xfc01, 0xfc01).w("watchdog", FUNC(watchdog_timer_device::reset_w));
}

void wc90_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf803).rw("ymsnd", FUNC(ym2608_device::read), FUNC(ym2608_device::write));
	map(0xfc00, 0xfc00).noprw(); // IRQ acknowledge? (data read and immediately written back)
	map(0xfc10, 0xfc10).r("soundlatch", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( wc90 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Count Down" )            PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "1 Count - 1 Second" )
	PORT_DIPSETTING(    0x00, "1 Count - 56/60 Second" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "1 Player Game Time" )    PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x01, "1:00" )
	PORT_DIPSETTING(    0x02, "1:30" )
	PORT_DIPSETTING(    0x03, "2:00" )
	PORT_DIPSETTING(    0x00, "2:30" )
	PORT_DIPNAME( 0x1c, 0x1c, "2 Players Game Time" )   PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x0c, "1:00" )
	PORT_DIPSETTING(    0x14, "1:30" )
	PORT_DIPSETTING(    0x04, "2:00" )
	PORT_DIPSETTING(    0x18, "2:30" )
	PORT_DIPSETTING(    0x1c, "3:00" )
	PORT_DIPSETTING(    0x08, "3:30" )
	PORT_DIPSETTING(    0x10, "4:00" )
	PORT_DIPSETTING(    0x00, "5:00" )

	// the following 3 switches are listed as "don't touch"
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:2" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )  // ON by default
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END



static INPUT_PORTS_START( pac90 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Ghost Names" )           PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Alternate ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )


	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END


static GFXDECODE_START( gfx_wc90 )
	GFXDECODE_ENTRY( "chars",    0, gfx_8x8x4_packed_msb,               1*16*16, 16*16 )
	GFXDECODE_ENTRY( "tiles1_2", 0, gfx_8x8x4_row_2x2_group_packed_msb, 2*16*16, 16*16 )
	GFXDECODE_ENTRY( "tiles3_4", 0, gfx_8x8x4_row_2x2_group_packed_msb, 3*16*16, 16*16 )
GFXDECODE_END

static GFXDECODE_START( gfx_wc90_spr )
	GFXDECODE_ENTRY( "sprites",  0, gfx_8x8x4_packed_msb, 0*16*16, 16*16 )
GFXDECODE_END


void wc90_state::machine_start()
{
	m_mainbank->configure_entries(0, 32, memregion("maincpu")->base() + 0x10000, 0x800);
	m_subbank->configure_entries(0, 32, memregion("sub")->base() + 0x10000, 0x800);
}


void wc90_state::wc90(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(8'000'000));     // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &wc90_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(wc90_state::irq0_line_hold));

	z80_device &sub(Z80(config, "sub", XTAL(8'000'000)));     // verified on PCB
	sub.set_addrmap(AS_PROGRAM, &wc90_state::sub_map);
	sub.set_vblank_int("screen", FUNC(wc90_state::irq0_line_hold));

	Z80(config, m_audiocpu, XTAL(8'000'000)/2);  // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &wc90_state::sound_map);
	// NMIs are triggered by the main CPU

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.17);         // verified on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(wc90_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wc90);
	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024).set_endianness(ENDIANNESS_BIG);

	TECMO_SPRITE(config, m_sprgen, 0, m_palette, gfx_wc90_spr);
	m_sprgen->set_pri_callback(FUNC(wc90_state::pri_cb));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2608_device &ymsnd(YM2608(config, "ymsnd", XTAL(8'000'000)));  // verified on PCB
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);
}

void wc90_state::wc90t(machine_config &config)
{
	wc90(config);
	MCFG_VIDEO_START_OVERRIDE(wc90_state, wc90t)
}

void wc90_state::pac90(machine_config &config)
{
	wc90(config);
	m_sprgen->set_yoffset(16); // sprites need shifting, why?
}


ROM_START( twcup90 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic87_01.bin",  0x00000, 0x08000, CRC(4a1affbc) SHA1(bc531e97ca31c66fdac194e2d79d5c6ba1300556) )  // c000-ffff is not used
	ROM_LOAD( "ic95_02.bin",  0x10000, 0x10000, CRC(847d439c) SHA1(eade31050da9e84feb4406e327d050a7496871b7) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "ic67_04.bin",  0x00000, 0x10000, CRC(dc6eaf00) SHA1(d53924070a59eee35dc0e6465702e4f04e61a073) )  // c000-ffff is not used
	ROM_LOAD( "ic56_03.bin",  0x10000, 0x10000, CRC(1ac02b3b) SHA1(4f8dc049d404072150342f3c2df04789a73ce244) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic54_05.bin",  0x00000, 0x10000, CRC(27c348b3) SHA1(cf19ff4ae4f323ae3e5a905249b7af8ae342202a) )

	ROM_REGION( 0x010000, "chars", 0 )
	ROM_LOAD( "ic85_07v.bin", 0x00000, 0x10000, CRC(c5219426) SHA1(95e21fcd7de7d418ec287ae7087f6244c6bce5a8) )

	ROM_REGION( 0x040000, "tiles1_2", 0 )
	ROM_LOAD( "ic86_08v.bin", 0x00000, 0x20000, CRC(8fa1a1ff) SHA1(ce624617ac8c8b54e41294cf5dca7a09c91f53ba) )
	ROM_LOAD( "ic90_09v.bin", 0x20000, 0x20000, CRC(99f8841c) SHA1(1969b4d78ca00924a7550826e1c4f4fa0588ef02) )

	ROM_REGION( 0x040000, "tiles3_4", 0 )
	ROM_LOAD( "ic87_10v.bin", 0x00000, 0x20000, CRC(8232093d) SHA1(59bf9c9a858b47326cf0c64b1ee6ac727a15a20b) )
	ROM_LOAD( "ic91_11v.bin", 0x20000, 0x20000, CRC(188d3789) SHA1(35654a99a20735bae09b32f74255f8132dee9af2) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ic50_12v.bin", 0x00000, 0x20000, CRC(da1fe922) SHA1(5184053c2b7dd2bf1cd2e9f783686f2c0db7e47b) )
	ROM_LOAD16_BYTE( "ic54_13v.bin", 0x40000, 0x20000, CRC(9ad03c2c) SHA1(1c1947f9b51a58002e9992fc7c0c1a1c59b4d740) )
	ROM_LOAD16_BYTE( "ic60_14v.bin", 0x00001, 0x20000, CRC(499dfb1b) SHA1(ac67985d36fea18c82a4ea00019d9e6e4bcb5d0d) )
	ROM_LOAD16_BYTE( "ic65_15v.bin", 0x40001, 0x20000, CRC(d8ea5c81) SHA1(ccb3f7d565b1c1b8e874a2df91cda40dde2962ed) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "ic82_06.bin",  0x00000, 0x20000, CRC(2fd692ed) SHA1(0273dc39181504320bec0187d074b2f86c821508) )
ROM_END

ROM_START( twcup90a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "wc90-1.bin",   0x00000, 0x08000, CRC(d1804e1a) SHA1(eec7374f4d23c89843f38fffff436635adb43b63) )  // c000-ffff is not used
	ROM_LOAD( "ic95_02.bin",  0x10000, 0x10000, CRC(847d439c) SHA1(eade31050da9e84feb4406e327d050a7496871b7) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "ic67_04.bin",  0x00000, 0x10000, CRC(dc6eaf00) SHA1(d53924070a59eee35dc0e6465702e4f04e61a073) )  // c000-ffff is not used
	ROM_LOAD( "ic56_03.bin",  0x10000, 0x10000, CRC(1ac02b3b) SHA1(4f8dc049d404072150342f3c2df04789a73ce244) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic54_05.bin",  0x00000, 0x10000, CRC(27c348b3) SHA1(cf19ff4ae4f323ae3e5a905249b7af8ae342202a) )

	ROM_REGION( 0x010000, "chars", 0 )
	ROM_LOAD( "ic85_07v.bin", 0x00000, 0x10000, CRC(c5219426) SHA1(95e21fcd7de7d418ec287ae7087f6244c6bce5a8) )

	ROM_REGION( 0x040000, "tiles1_2", 0 )
	ROM_LOAD( "ic86_08v.bin", 0x00000, 0x20000, CRC(8fa1a1ff) SHA1(ce624617ac8c8b54e41294cf5dca7a09c91f53ba) )
	ROM_LOAD( "ic90_09v.bin", 0x20000, 0x20000, CRC(99f8841c) SHA1(1969b4d78ca00924a7550826e1c4f4fa0588ef02) )

	ROM_REGION( 0x040000, "tiles3_4", 0 )
	ROM_LOAD( "ic87_10v.bin", 0x00000, 0x20000, CRC(8232093d) SHA1(59bf9c9a858b47326cf0c64b1ee6ac727a15a20b) )
	ROM_LOAD( "ic91_11v.bin", 0x20000, 0x20000, CRC(188d3789) SHA1(35654a99a20735bae09b32f74255f8132dee9af2) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ic50_12v.bin", 0x00000, 0x20000, CRC(da1fe922) SHA1(5184053c2b7dd2bf1cd2e9f783686f2c0db7e47b) )
	ROM_LOAD16_BYTE( "ic54_13v.bin", 0x40000, 0x20000, CRC(9ad03c2c) SHA1(1c1947f9b51a58002e9992fc7c0c1a1c59b4d740) )
	ROM_LOAD16_BYTE( "ic60_14v.bin", 0x00001, 0x20000, CRC(499dfb1b) SHA1(ac67985d36fea18c82a4ea00019d9e6e4bcb5d0d) )
	ROM_LOAD16_BYTE( "ic65_15v.bin", 0x40001, 0x20000, CRC(d8ea5c81) SHA1(ccb3f7d565b1c1b8e874a2df91cda40dde2962ed) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "ic82_06.bin",  0x00000, 0x20000, CRC(2fd692ed) SHA1(0273dc39181504320bec0187d074b2f86c821508) )
ROM_END

ROM_START( twcup90b )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic87-1b.bin",  0x00000, 0x08000, CRC(d024a971) SHA1(856c6ab7abc1cd6db42703f70930b84e3da69db0) )  // c000-ffff is not used
	ROM_LOAD( "ic95_02.bin",  0x10000, 0x10000, CRC(847d439c) SHA1(eade31050da9e84feb4406e327d050a7496871b7) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "ic67_04.bin",  0x00000, 0x10000, CRC(dc6eaf00) SHA1(d53924070a59eee35dc0e6465702e4f04e61a073) )  // c000-ffff is not used
	ROM_LOAD( "ic56_03.bin",  0x10000, 0x10000, CRC(1ac02b3b) SHA1(4f8dc049d404072150342f3c2df04789a73ce244) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic54_05.bin",  0x00000, 0x10000, CRC(27c348b3) SHA1(cf19ff4ae4f323ae3e5a905249b7af8ae342202a) )

	ROM_REGION( 0x010000, "chars", 0 )
	ROM_LOAD( "ic85_07v.bin", 0x00000, 0x10000, CRC(c5219426) SHA1(95e21fcd7de7d418ec287ae7087f6244c6bce5a8) )

	ROM_REGION( 0x040000, "tiles1_2", 0 )
	ROM_LOAD( "ic86_08v.bin", 0x00000, 0x20000, CRC(8fa1a1ff) SHA1(ce624617ac8c8b54e41294cf5dca7a09c91f53ba) )
	ROM_LOAD( "ic90_09v.bin", 0x20000, 0x20000, CRC(99f8841c) SHA1(1969b4d78ca00924a7550826e1c4f4fa0588ef02) )

	ROM_REGION( 0x040000, "tiles3_4", 0 )
	ROM_LOAD( "ic87_10v.bin", 0x00000, 0x20000, CRC(8232093d) SHA1(59bf9c9a858b47326cf0c64b1ee6ac727a15a20b) )
	ROM_LOAD( "ic91_11v.bin", 0x20000, 0x20000, CRC(188d3789) SHA1(35654a99a20735bae09b32f74255f8132dee9af2) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ic50_12v.bin", 0x00000, 0x20000, CRC(da1fe922) SHA1(5184053c2b7dd2bf1cd2e9f783686f2c0db7e47b) )
	ROM_LOAD16_BYTE( "ic54_13v.bin", 0x40000, 0x20000, CRC(9ad03c2c) SHA1(1c1947f9b51a58002e9992fc7c0c1a1c59b4d740) )
	ROM_LOAD16_BYTE( "ic60_14v.bin", 0x00001, 0x20000, CRC(499dfb1b) SHA1(ac67985d36fea18c82a4ea00019d9e6e4bcb5d0d) )
	ROM_LOAD16_BYTE( "ic65_15v.bin", 0x40001, 0x20000, CRC(d8ea5c81) SHA1(ccb3f7d565b1c1b8e874a2df91cda40dde2962ed) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "ic82_06.bin",  0x00000, 0x20000, CRC(2fd692ed) SHA1(0273dc39181504320bec0187d074b2f86c821508) )
ROM_END

ROM_START( twcup90c ) // 2 PCB set: 6303 A and 6303 B. ic87_01 is very similar to the one in the twcup90a set.
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic87_01.bin",  0x00000, 0x08000, CRC(f588bb33) SHA1(46e90f145befd50be5ce0ffc05b00a034318a330) )  // sldh, c000-ffff is not used
	ROM_LOAD( "ic95_02.bin",  0x10000, 0x10000, CRC(847d439c) SHA1(eade31050da9e84feb4406e327d050a7496871b7) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "ic67_04.bin",  0x00000, 0x10000, CRC(dc6eaf00) SHA1(d53924070a59eee35dc0e6465702e4f04e61a073) )  // c000-ffff is not used
	ROM_LOAD( "ic56_03.bin",  0x10000, 0x10000, CRC(1ac02b3b) SHA1(4f8dc049d404072150342f3c2df04789a73ce244) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic54_05.bin",  0x00000, 0x10000, CRC(27c348b3) SHA1(cf19ff4ae4f323ae3e5a905249b7af8ae342202a) )

	ROM_REGION( 0x010000, "chars", 0 )
	ROM_LOAD( "ic85_07v.bin", 0x00000, 0x10000, CRC(c5219426) SHA1(95e21fcd7de7d418ec287ae7087f6244c6bce5a8) )

	ROM_REGION( 0x040000, "tiles1_2", 0 )
	ROM_LOAD( "ic86_08v.bin", 0x00000, 0x20000, CRC(8fa1a1ff) SHA1(ce624617ac8c8b54e41294cf5dca7a09c91f53ba) )
	ROM_LOAD( "ic90_09v.bin", 0x20000, 0x20000, CRC(99f8841c) SHA1(1969b4d78ca00924a7550826e1c4f4fa0588ef02) )

	ROM_REGION( 0x040000, "tiles3_4", 0 )
	ROM_LOAD( "ic87_10v.bin", 0x00000, 0x20000, CRC(8232093d) SHA1(59bf9c9a858b47326cf0c64b1ee6ac727a15a20b) )
	ROM_LOAD( "ic91_11v.bin", 0x20000, 0x20000, CRC(188d3789) SHA1(35654a99a20735bae09b32f74255f8132dee9af2) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ic50_12v.bin", 0x00000, 0x20000, CRC(da1fe922) SHA1(5184053c2b7dd2bf1cd2e9f783686f2c0db7e47b) )
	ROM_LOAD16_BYTE( "ic54_13v.bin", 0x40000, 0x20000, CRC(9ad03c2c) SHA1(1c1947f9b51a58002e9992fc7c0c1a1c59b4d740) )
	ROM_LOAD16_BYTE( "ic60_14v.bin", 0x00001, 0x20000, CRC(499dfb1b) SHA1(ac67985d36fea18c82a4ea00019d9e6e4bcb5d0d) )
	ROM_LOAD16_BYTE( "ic65_15v.bin", 0x40001, 0x20000, CRC(d8ea5c81) SHA1(ccb3f7d565b1c1b8e874a2df91cda40dde2962ed) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "ic82_06.bin",  0x00000, 0x20000, CRC(2fd692ed) SHA1(0273dc39181504320bec0187d074b2f86c821508) )
ROM_END

ROM_START( twcup90t )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "wc90a-1.bin",  0x00000, 0x08000, CRC(b6f51a68) SHA1(e0263dee35bf99cb4288a1df825bbbca17c85d36) )  // c000-ffff is not used
	ROM_LOAD( "wc90a-2.bin",  0x10000, 0x10000, CRC(c50f2a98) SHA1(0fbeabadebfa75515d5e35bfcc565ecfa4d6e693) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "ic67_04.bin",  0x00000, 0x10000, CRC(dc6eaf00) SHA1(d53924070a59eee35dc0e6465702e4f04e61a073) )  // c000-ffff is not used
	ROM_LOAD( "wc90a-3.bin",  0x10000, 0x10000, CRC(8c7a9542) SHA1(a06a7cd40d41692c4cc2a35d9c69b944c5baf163) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic54_05.bin",  0x00000, 0x10000, CRC(27c348b3) SHA1(cf19ff4ae4f323ae3e5a905249b7af8ae342202a) )

	ROM_REGION( 0x010000, "chars", 0 )
	ROM_LOAD( "ic85_07v.bin", 0x00000, 0x10000, CRC(c5219426) SHA1(95e21fcd7de7d418ec287ae7087f6244c6bce5a8) )

	ROM_REGION( 0x040000, "tiles1_2", 0 )
	ROM_LOAD( "ic86_08v.bin", 0x00000, 0x20000, CRC(8fa1a1ff) SHA1(ce624617ac8c8b54e41294cf5dca7a09c91f53ba) )
	ROM_LOAD( "ic90_09v.bin", 0x20000, 0x20000, CRC(99f8841c) SHA1(1969b4d78ca00924a7550826e1c4f4fa0588ef02) )

	ROM_REGION( 0x040000, "tiles3_4", 0 )
	ROM_LOAD( "ic87_10v.bin", 0x00000, 0x20000, CRC(8232093d) SHA1(59bf9c9a858b47326cf0c64b1ee6ac727a15a20b) )
	ROM_LOAD( "ic91_11v.bin", 0x20000, 0x20000, CRC(188d3789) SHA1(35654a99a20735bae09b32f74255f8132dee9af2) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ic50_12v.bin", 0x00000, 0x20000, CRC(da1fe922) SHA1(5184053c2b7dd2bf1cd2e9f783686f2c0db7e47b) )
	ROM_LOAD16_BYTE( "ic54_13v.bin", 0x40000, 0x20000, CRC(9ad03c2c) SHA1(1c1947f9b51a58002e9992fc7c0c1a1c59b4d740) )
	ROM_LOAD16_BYTE( "ic60_14v.bin", 0x00001, 0x20000, CRC(499dfb1b) SHA1(ac67985d36fea18c82a4ea00019d9e6e4bcb5d0d) )
	ROM_LOAD16_BYTE( "ic65_15v.bin", 0x40001, 0x20000, CRC(d8ea5c81) SHA1(ccb3f7d565b1c1b8e874a2df91cda40dde2962ed) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "ic82_06.bin",  0x00000, 0x20000, CRC(2fd692ed) SHA1(0273dc39181504320bec0187d074b2f86c821508) )
ROM_END

ROM_START( pac90 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rom1.ic87",  0x00000, 0x08000, CRC(8af34306) SHA1(1a98adca74f46da36e3648d37bfcb56a328a031e) )

	ROM_REGION( 0x20000, "sub", ROMREGION_ERASE00 )
	ROM_LOAD( "rom2.ic67",  0x00000, 0x10000, CRC(bc9bfdf2) SHA1(869e4012e5c577e501143cbfd75cce8cef919c86) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom3.ic54",  0x00000, 0x10000, CRC(1c4d17fd) SHA1(5abebf867de452cc3e85331e91b9110c26a8b050) )

	ROM_REGION( 0x010000, "chars", 0 )
	ROM_LOAD( "char.ic85", 0x00000, 0x10000, CRC(70941a50) SHA1(283583743c21774d0097dc935ae7bc7009b5b633) )
	// char.ic85      CRC32 0b906dae   SHA1 0d14d6a7bbe0b8772143afb4c6c94c62313e4b9c <-- An alternate version...

	ROM_REGION( 0x040000, "tiles1_2", ROMREGION_ERASE00 )
	//ROM_LOAD( "ic86_08v.bin", 0x00000, 0x20000, CRC(8fa1a1ff) SHA1(ce624617ac8c8b54e41294cf5dca7a09c91f53ba) )
	//ROM_LOAD( "ic90_09v.bin", 0x20000, 0x20000, CRC(99f8841c) SHA1(1969b4d78ca00924a7550826e1c4f4fa0588ef02) )

	ROM_REGION( 0x040000, "tiles3_4", ROMREGION_ERASE00 )
	//ROM_LOAD( "ic87_10v.bin", 0x00000, 0x20000, CRC(8232093d) SHA1(59bf9c9a858b47326cf0c64b1ee6ac727a15a20b) )
	//ROM_LOAD( "ic91_11v.bin", 0x20000, 0x20000, CRC(188d3789) SHA1(35654a99a20735bae09b32f74255f8132dee9af2) )

	ROM_REGION( 0x080000, "sprites", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sprite1.ic50", 0x00000, 0x10000, CRC(190852ea) SHA1(fad7eb3aa53d03917173dd5a040655cfd329db32) )
	ROM_LOAD16_BYTE( "sprite2.ic60", 0x00001, 0x10000, CRC(33effbea) SHA1(dbf6b735f3c8bacb695caf5d15ac8b7961bffc74) )

	ROM_REGION( 0x20000, "ymsnd", ROMREGION_ERASE00 )
	ROM_LOAD( "voice.ic82",  0x00000, 0x10000, CRC(abc61f3d) SHA1(c6f123d16a26c4d77c635617dd97bb4b906c463a) )
ROM_END

} // anonymous namespace


GAME( 1989, twcup90,  0,       wc90,  wc90,  wc90_state, empty_init, ROT0,  "Tecmo", "Tecmo World Cup '90 (World set 1)",     MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90a, twcup90, wc90,  wc90,  wc90_state, empty_init, ROT0,  "Tecmo", "Tecmo World Cup '90 (Europe set 1)",    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90b, twcup90, wc90,  wc90,  wc90_state, empty_init, ROT0,  "Tecmo", "Tecmo World Cup '90 (Europe set 2)",    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90c, twcup90, wc90,  wc90,  wc90_state, empty_init, ROT0,  "Tecmo", "Tecmo World Cup '90 (Europe set 3)",    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90t, twcup90, wc90t, wc90,  wc90_state, empty_init, ROT0,  "Tecmo", "Tecmo World Cup '90 (trackball set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 199?, pac90, puckman, pac90, pac90, wc90_state, empty_init, ROT90, "bootleg (Macro)", "Pac-Man (bootleg on World Cup '90 hardware)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // made by Mike Coates etc.
