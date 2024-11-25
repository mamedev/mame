// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/*

      F2 System
         "bootleg tumble pop" hardware (like dataeast/tumbleb.cpp)

  Driver by Pierpaolo Prazzoli with some bits by David Haywood


  Cross Pang        (c)1998 F2 System
  Heuk Sun Baek Sa  (c)1997 Oksan / F2 System
  Bestri            (c)1998 F2 System

  No Copyright Notice is displayed for Cross Pang however http://www.f2.co.kr
  at one time did list it as being by F2 System, Released April 1998

  Video seems to be the same as the tumblepop bootleg based hardware
  in dataeast/tumbleb.cpp

  Cross Pang:
    Audio Test isn't correct when a sound is tested, instead musics are right.
    The sample ROM says 'Oksan' (Oksan made Pass, it's unclear how they are
    related to Cross Pang)
  Bestri:
    Bestri includes Heuk San Baek Sa as one of its three sub games.

  2008-08
  Added Service dipswitch and dip locations based on Service Mode.
*/

#include "emu.h"

#include "decospr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_TILEBANK (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_TILEBANK)

#include "logmacro.h"

#define LOGTILEBANK(...) LOGMASKED(LOG_TILEBANK, __VA_ARGS__)


namespace {

class crospang_state : public driver_device
{
public:
	crospang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_fg_videoram(*this, "fg_videoram")
		, m_bg_videoram(*this, "bg_videoram")
		, m_spriteram(*this, "spriteram")
		, m_maincpu(*this, "maincpu")
		, m_sprgen(*this, "spritegen")
		, m_gfxdecode(*this, "gfxdecode")
	{ }

	void crospang(machine_config &config);
	void bestri(machine_config &config);
	void bestria(machine_config &config);
	void pitapat(machine_config &config);
	void pitapata(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u16> m_fg_videoram;
	required_shared_ptr<u16> m_bg_videoram;
	required_shared_ptr<u16> m_spriteram;

	// video-related
	tilemap_t *m_bg_layer = nullptr;
	tilemap_t *m_fg_layer = nullptr;
	u8 m_tilebank[4]{};
	u8 m_tilebankselect = 0U;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<decospr_device> m_sprgen;
	required_device<gfxdecode_device> m_gfxdecode;

	void tilebank_data_w(u16 data);
	void tilebank_select_w(u16 data);
	void bestri_bg_scrolly_w(u16 data);
	void bestri_fg_scrolly_w(u16 data);
	void bestri_fg_scrollx_w(u16 data);
	void bestri_bg_scrollx_w(u16 data);
	void fg_scrolly_w(u16 data);
	void bg_scrolly_w(u16 data);
	void fg_scrollx_w(u16 data);
	void bg_scrollx_w(u16 data);
	void fg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void base_map(address_map &map) ATTR_COLD;
	void bestri_map(address_map &map) ATTR_COLD;
	void bestria_map(address_map &map) ATTR_COLD;
	void crospang_map(address_map &map) ATTR_COLD;
	void pitapat_map(address_map &map) ATTR_COLD;
	void pitapata_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void crospang_state::tilebank_select_w(u16 data)
{
	LOGTILEBANK("tilebank_select_w %04x\n", data);

	m_tilebankselect = (data >> 8) & 3;
}


void crospang_state::tilebank_data_w(u16 data)
{
	LOGTILEBANK("tilebank_data_w %04x\n", data);

	m_tilebank[m_tilebankselect] = data >> 8;

	m_fg_layer->mark_all_dirty();
	m_bg_layer->mark_all_dirty();
}


// Bestri performs some unusual operations on the scroll values before writing them
void crospang_state::bestri_bg_scrolly_w(u16 data)
{
	// addi.w #$1f8, D0
	// eori.w #$154, D0
	int const scroll =  (data & 0x3ff) ^ 0x0155;
	m_bg_layer->set_scrolly(0, -scroll + 7);
}

void crospang_state::bestri_fg_scrolly_w(u16 data)
{
	// addi.w #$1f8, D0
	// eori.w #$aa, D0
	int const scroll = (data & 0x3ff) ^ 0x00ab;
	m_fg_layer->set_scrolly(0, -scroll + 7);
}

void crospang_state::bestri_fg_scrollx_w(u16 data)
{
	// addi.w #$400, D1
	// eori.w #$1e0, D1
	int const scroll =  (data & 0x3ff) ^ 0x1e1;
	m_fg_layer->set_scrollx(0, scroll - 1);
}

void crospang_state::bestri_bg_scrollx_w(u16 data)
{
	// addi.w #$3fc, D1
	// eori.w #$3c0, D1
	int const scroll =  (data & 0x3ff) ^ 0x3c1;
	m_bg_layer->set_scrollx(0, scroll + 3);
}


void crospang_state::fg_scrolly_w(u16 data)
{
	m_fg_layer->set_scrolly(0, data + 8);
}

void crospang_state::bg_scrolly_w(u16 data)
{
	m_bg_layer->set_scrolly(0, data + 8);
}

void crospang_state::fg_scrollx_w(u16 data)
{
	m_fg_layer->set_scrollx(0, data);
}

void crospang_state::bg_scrollx_w(u16 data)
{
	m_bg_layer->set_scrollx(0, data + 4);
}


void crospang_state::fg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

void crospang_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(crospang_state::get_bg_tile_info)
{
	int const data  = m_bg_videoram[tile_index];
	int tile  = data & 0x03ff;
	int const tilebank = (data & 0x0c00) >> 10;
	tile = tile + (m_tilebank[tilebank] << 10);
	int const color = (data >> 12) & 0x0f;

	tileinfo.set(0, tile, color + 0x20, 0);
}

TILE_GET_INFO_MEMBER(crospang_state::get_fg_tile_info)
{
	int const data  = m_fg_videoram[tile_index];
	int tile  = data & 0x03ff;
	int const tilebank = (data & 0x0c00) >> 10;
	tile = tile + (m_tilebank[tilebank] << 10);
	int const color = (data >> 12) & 0x0f;

	tileinfo.set(0, tile, color + 0x10, 0);
}


void crospang_state::video_start()
{
	m_bg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(crospang_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(crospang_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_fg_layer->set_transparent_pen(0);
}

u32 crospang_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_layer->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_layer->draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}


// main COU

void crospang_state::base_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().nopw(); // writes to ROM quite often

	map(0x100000, 0x100001).w(FUNC(crospang_state::tilebank_select_w));
	map(0x10000e, 0x10000f).w(FUNC(crospang_state::tilebank_data_w));

	map(0x120000, 0x1207ff).ram().w(FUNC(crospang_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x122000, 0x1227ff).ram().w(FUNC(crospang_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x200000, 0x2005ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x210000, 0x2107ff).ram().share(m_spriteram);
	map(0x270001, 0x270001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x270004, 0x270007).nopw(); // ??
	map(0x280000, 0x280001).portr("P1_P2");
	map(0x280002, 0x280003).portr("COIN");
	map(0x280004, 0x280005).portr("DSW");
}

// the main RAM and scroll values move around / scrambled between games

void crospang_state::crospang_map(address_map &map)
{
	base_map(map);

	map(0x100002, 0x100003).w(FUNC(crospang_state::fg_scrolly_w));
	map(0x100004, 0x100005).w(FUNC(crospang_state::bg_scrollx_w));
	map(0x100006, 0x100007).w(FUNC(crospang_state::bg_scrolly_w));
	map(0x100008, 0x100009).w(FUNC(crospang_state::fg_scrollx_w));

	map(0x320000, 0x32ffff).ram();
}

void crospang_state::pitapat_map(address_map &map)
{
	base_map(map);

	map(0x100002, 0x100003).w(FUNC(crospang_state::fg_scrolly_w));
	map(0x100004, 0x100005).w(FUNC(crospang_state::bg_scrollx_w));
	map(0x100006, 0x100007).w(FUNC(crospang_state::bg_scrolly_w));
	map(0x100008, 0x100009).w(FUNC(crospang_state::fg_scrollx_w));

	map(0x300000, 0x30ffff).ram();
}

void crospang_state::pitapata_map(address_map &map)
{
	base_map(map);

	map(0x100002, 0x100003).w(FUNC(crospang_state::fg_scrollx_w));
	map(0x100004, 0x100005).w(FUNC(crospang_state::fg_scrolly_w));
	map(0x100006, 0x100007).w(FUNC(crospang_state::bg_scrollx_w));
	map(0x100008, 0x100009).w(FUNC(crospang_state::bg_scrolly_w));

	map(0x300000, 0x30ffff).ram();
}

void crospang_state::bestri_map(address_map &map)
{
	base_map(map);

	map(0x100004, 0x100005).w(FUNC(crospang_state::bestri_fg_scrollx_w));
	map(0x100006, 0x100007).w(FUNC(crospang_state::bestri_fg_scrolly_w));
	map(0x10000a, 0x10000b).w(FUNC(crospang_state::bestri_bg_scrolly_w));
	map(0x10000c, 0x10000d).w(FUNC(crospang_state::bestri_bg_scrollx_w));

	map(0x3a0000, 0x3affff).ram();
}

void crospang_state::bestria_map(address_map &map)
{
	base_map(map);

	map(0x100006, 0x100007).w(FUNC(crospang_state::bestri_fg_scrollx_w));
	map(0x100008, 0x100009).w(FUNC(crospang_state::bestri_fg_scrolly_w));
	map(0x10000a, 0x10000b).w(FUNC(crospang_state::bestri_bg_scrollx_w));
	map(0x10000c, 0x10000d).w(FUNC(crospang_state::bestri_bg_scrolly_w));

	map(0x340000, 0x34ffff).ram();
}

// sound CPU

void crospang_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
}

void crospang_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x02, 0x02).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x06, 0x06).r("soundlatch", FUNC(generic_latch_8_device::read));
}


// verified from M68000 code
static INPUT_PORTS_START( crospang )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COIN")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4")   // to be confirmed
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )         // table at 0x02ee2c
	PORT_DIPSETTING(      0x000c, DEF_STR( Medium ) )       // table at 0x02e88c
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )         // table at 0x02f96c
	PORT_DIPSETTING(      0x0004, DEF_STR( Hardest ) )      // table at 0x02f3cc
	PORT_DIPNAME( 0x0010, 0x0010, "Bonus Power (Points)" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "5k 20k 15k+" )
	PORT_DIPSETTING(      0x0000, "8k 23k 15k+" )
	PORT_DIPNAME( 0x0020, 0x0020, "Number of Powers" )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPNAME( 0x00c0, 0x0040, "Extra Balls per Move" )  PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00c0, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:1,2")   // code at 0x021672 - occurs after level 6
	PORT_DIPSETTING(      0x0300, "6/7" )
	PORT_DIPSETTING(      0x0200, "7/8" )
	PORT_DIPSETTING(      0x0100, "8/9" )
	PORT_DIPSETTING(      0x0000, "9/10" )
	PORT_DIPNAME( 0x0400, 0x0400, "Bonus Power (Bomb)" )    PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "3 Chain Reactions" )
	PORT_DIPSETTING(      0x0000, "4 Chain Reactions" )
	PORT_DIPNAME( 0x1800, 0x1800, "Minimum Balls per Row" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1800, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x0800, "5" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )        // stored at 0x325414.w but not read back
	PORT_SERVICE_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )        // stored at 0x325418.w but not read back
INPUT_PORTS_END

// verified from M68000 code
static INPUT_PORTS_START( heuksun )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COIN")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4")   // stored at 0x324632.w
	PORT_DIPSETTING(      0x000c, DEF_STR( Easy ) )         // 1
	PORT_DIPSETTING(      0x0008, DEF_STR( Medium ) )       // 2
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )         // 3
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )      // 4
	PORT_DIPNAME( 0x0010, 0x0010, "Help Penalty (Heuk Sun)" ) PORT_DIPLOCATION("SW1:5")   // code at 0x01878e and 0x0187f6
	PORT_DIPSETTING(      0x0010, "Constant" )
	PORT_DIPSETTING(      0x0000, "Variable" )              // based on "Difficulty" Dip Switch
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )        // read once during initialisation but not even stored
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW1:7" )       // stored at 0x32463a.w but not read back ?
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	// bits are tested from most to less significant - code at 0x01023e
	PORT_DIPNAME( 0xff00, 0xff00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:1,2,3,4,5,6,7,8") // stored at 0x324662.w but not read back ?
	PORT_DIPSETTING(      0xff00, "0" )
	PORT_DIPSETTING(      0xfe00, "1" )
	PORT_DIPSETTING(      0xfd00, "2" )
	PORT_DIPSETTING(      0xfb00, "3" )
	PORT_DIPSETTING(      0xf700, "4" )
	PORT_DIPSETTING(      0xef00, "5" )
	PORT_DIPSETTING(      0xdf00, "6" )
	PORT_DIPSETTING(      0xbf00, "7" )
	PORT_DIPSETTING(      0x7f00, "8" )
INPUT_PORTS_END

// verified from M68000 code
static INPUT_PORTS_START( bestri )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COIN")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0006, 0x0002, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")   // stored at 0x3a6f78.w
	PORT_DIPSETTING(      0x0018, DEF_STR( Easy ) )         // 1
	PORT_DIPSETTING(      0x0008, DEF_STR( Medium ) )       // 2
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )         // 3
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )      // 4
	PORT_DIPNAME( 0x0020, 0x0020, "Help Penalty (Heuk Sun)" ) PORT_DIPLOCATION("SW1:6")   // code at 0x0b7152 and 0x07b1ba
	PORT_DIPSETTING(      0x0020, "Constant" )
	PORT_DIPSETTING(      0x0000, "Variable" )              // based on "Difficulty" Dip Switch
	PORT_DIPNAME( 0x00c0, 0x00c0, "Girls" )                 PORT_DIPLOCATION("SW1:7,8")   // stored at 0x3a6faa.w
	PORT_DIPSETTING(      0x00c0, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x0040, "No (duplicate 1)" )
	PORT_DIPSETTING(      0x0000, "No (duplicate 2)" )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:1,2,3") // stored at 0x3a6fa6.w but not read back ?
	PORT_DIPSETTING(      0x0700, "0" )
	PORT_DIPSETTING(      0x0300, "1" )
	PORT_DIPSETTING(      0x0500, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0600, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0400, "6" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x1800, 0x1800, "Unknown (Die Break)" )   PORT_DIPLOCATION("SW2:4,5")   // stored at 0x3a6fa8.w
	PORT_DIPSETTING(      0x1800, "0" )
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x1000, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x2000, 0x2000, "Time (Penta)" )          PORT_DIPLOCATION("SW2:6")     // stored at 0x3a6fac.w
	PORT_DIPSETTING(      0x0000, "60" )
	PORT_DIPSETTING(      0x2000, "90" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )        // read once during initialisation but not even stored
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )        // read once during initialisation but not even stored
INPUT_PORTS_END

static INPUT_PORTS_START( pitapat )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COIN")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Boxes to Marvels" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7") // this and the following dip might be difficulty related. By having one or both of them on, most of the times you win the first round without doing anything
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tlayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ STEP8(8*2*16,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static const gfx_layout tlayout_alt =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 8, 0, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0 },
	{ STEP8(0,1), STEP8(8*2*16,1) },
	{ STEP16(0,8*2) },
	64*8
};


static GFXDECODE_START( gfx_crospang )
	GFXDECODE_ENTRY( "tiles",   0, tlayout_alt,   0, 64 )  // 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_crospang_spr )
	GFXDECODE_ENTRY( "sprites", 0, tlayout,       0, 64 )  // 16x16
GFXDECODE_END

void crospang_state::machine_start()
{
	save_item(NAME(m_tilebank));
	save_item(NAME(m_tilebankselect));
}

void crospang_state::machine_reset()
{
	m_tilebank[0] = 0x00;
	m_tilebank[1] = 0x01;
	m_tilebank[2] = 0x02;
	m_tilebank[3] = 0x03;

	m_tilebankselect = 0;
}

void crospang_state::crospang(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(14'318'181) / 2); // 68000P10 @ 7.15909MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &crospang_state::crospang_map);
	m_maincpu->set_vblank_int("screen", FUNC(crospang_state::irq6_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(14'318'181) / 4)); // 3.579545MHz
	audiocpu.set_addrmap(AS_PROGRAM, &crospang_state::sound_map);
	audiocpu.set_addrmap(AS_IO, &crospang_state::sound_io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 40*8-1, 0, 30*8-1);
	screen.set_screen_update(FUNC(crospang_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x300);
	GFXDECODE(config, m_gfxdecode, "palette", gfx_crospang);

	DECO_SPRITE(config, m_sprgen, 0, "palette", gfx_crospang_spr);
	m_sprgen->set_is_bootleg(true);
	m_sprgen->set_offsets(5, 7);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(14'318'181) / 4)); // 3.579545MHz
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(14'318'181) / 16, okim6295_device::PIN7_HIGH)); // 1.789772MHz or 0.894886MHz?? & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void crospang_state::bestri(machine_config &config)
{
	crospang(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &crospang_state::bestri_map);
}

void crospang_state::bestria(machine_config &config)
{
	crospang(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &crospang_state::bestria_map);
}

void crospang_state::pitapat(machine_config &config)
{
	crospang(config);

	// can't be 14'318'181 / 2 as the inputs barely respond and the background graphics glitch badly when the screen fills, doesn't appear to be a vblank bit anywhere to negate this either, P12 rated part
	M68000(config.replace(), m_maincpu, XTAL(14'318'181));
	m_maincpu->set_addrmap(AS_PROGRAM, &crospang_state::pitapat_map);
	m_maincpu->set_vblank_int("screen", FUNC(crospang_state::irq6_line_hold));
}

void crospang_state::pitapata(machine_config &config)
{
	pitapat(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &crospang_state::pitapata_map);
}


ROM_START( crospang ) // Developed April 1998
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "p1.bin", 0x00001, 0x20000, CRC(0bcbbaad) SHA1(807f07be340d7af0aad8d49461b5a7f0221ea3b7) )
	ROM_LOAD16_BYTE( "p2.bin", 0x00000, 0x20000, CRC(0947d204) SHA1(35e7e277c51888a66d305994bf05c3f6bfc3c29e) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "s1.bin", 0x00000, 0x10000, CRC(d61a224c) SHA1(5cd1b2d136ad58ab550c7ba135558d6c8a4cd8f6) )

	ROM_REGION( 0x40000, "oki", 0 ) // samples
	ROM_LOAD( "s2.bin", 0x00000, 0x20000, CRC(9f9ecd22) SHA1(631ffe14018ba39658c435b8ecb23b19a14569ee) ) // sample ROM contains oksan

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "rom1.bin", 0x00000, 0x40000, CRC(905042bb) SHA1(ed5b97e88d24e55f8fcfaaa34251582976cb2527) )
	ROM_LOAD16_BYTE( "rom2.bin", 0x00001, 0x40000, CRC(bc4381e9) SHA1(af0690c253bead3448db5ec8fb258d8284646e89) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "rom3.bin", 0x000000, 0x80000, CRC(cc6e1fce) SHA1(eb5b3ca7222f48916dc6206f987b2669fe7e7c6b) )
	ROM_LOAD16_BYTE( "rom4.bin", 0x000001, 0x80000, CRC(9a91d494) SHA1(1c6280f662f1cf53f7f6defb7e215da75b573fdf) )
	ROM_LOAD16_BYTE( "rom5.bin", 0x100000, 0x80000, CRC(53a34dc5) SHA1(2e5cf8093bf507e81d7447736b7727c3fd20c471) )
	ROM_LOAD16_BYTE( "rom6.bin", 0x100001, 0x80000, CRC(9c633082) SHA1(18b8591b695ee429c9c9855d8cbba6249a4bd809) )
ROM_END


/*
Heuk Sun Baek Sa

+----------------------------------+
|  YM3014 YM3812  M6295 us08       |
|        6116                  uc07|
|      us02  Z80               uc08|
|J  6116                       ud14|
|A  6116                       ud15|
|M    62256 62256 6264         ud16|
|M DSW2 ua02 ua03 6264         ud17|
|A      68000  A1020B              |
|  DSW1                            |
|                                  |
|14.318MHz                         |
+----------------------------------+

Motorola MC68000P10
NEC D780C
Actel A1020B PL84C
YM3812/YM3014 (badged as UA010 & 5A14)
OKI M6295

*/

ROM_START( heuksun )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "ua02.j3", 0x00001, 0x80000, CRC(db2b9c8e) SHA1(aa37e3a056957a12888e2e3112fe78a6bff7d76f) )
	ROM_LOAD16_BYTE( "ua03.j5", 0x00000, 0x80000, CRC(de9f01e8) SHA1(3ee9206e7c3c7bebd7cde6f201c2fa7f9f6553b7) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "us02.r4", 0x00000, 0x10000, CRC(c7cc05fa) SHA1(5fbf479be98f618c63e4c74a250d51279c2f5e3b) )

	ROM_REGION( 0x040000, "oki", 0 ) // samples
	ROM_LOAD( "us08.u7", 0x00000, 0x40000, CRC(ae177589) SHA1(9a1e2b848046f3506ede4f218a9175cc8e984ad8) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "uc08.r11", 0x00001, 0x20000, CRC(242cee69) SHA1(71112ea6aac4db9b923315656f12d2f72173d9cd) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "uc07.t11", 0x00000, 0x20000, CRC(4d1ed885) SHA1(2868394658fac70e31ebd150377d76cfe63a4d5f) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ud14.p11", 0x00000, 0x40000, CRC(4fc2b574) SHA1(f3330d9cc3065b5a96e222300c2ae01e57241632) )
	ROM_LOAD16_BYTE( "ud15.m11", 0x00001, 0x40000, CRC(1d6187a6) SHA1(51f1ac086d67e8b35081ddc14e28b218d3153779) )
	ROM_LOAD16_BYTE( "ud16.l11", 0x80000, 0x40000, CRC(eabec43e) SHA1(fa0a7886ccaf90e9ed59dc283e27f9e8e9aa7d29) )
	ROM_LOAD16_BYTE( "ud17.j11", 0x80001, 0x40000, CRC(c6b04171) SHA1(4d142cad4e0d62764144784634fabeef97d07630) )
ROM_END


/*

Bestri

+----------------------------------+
| YM3014 YM3812  M6295 us08        |
|        us02   6116           uc07|
|         Z80                  uc28|
|       6116                   uc08|
|J      6116                   ud29|
|A    62256 62256 6264         ud14|
|M      ua02 ua03 6264         ud15|
|M DSW2 68000    QL2003        ud16|
|A                   6116      ud17|
|  DSW1              6116          |
|               6116               |
|               6116               |
|    14.31818MHz                   |
+----------------------------------+

Motorola MC68000P10
ZiLOG Z840006PSC (6MHz rated)
QuickLogic QL20003-XPL84C
Yamaha YM3812/YM3014
OKI M6295


ua02.i3 (Odd)
ua03.i5 (even)

Numbers/letters to right of ROM name denotes
  numbers/letters silkscreened under socket

uc07.p12 0
uc28.n12 1
uc08.m12 2
uc29.k12 3
ud14.j12 A
ud15.h12 B
ud16.g12 C
ud17.e12 D

*/

ROM_START( bestri ) // Developed March 1998
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "ua02.i3", 0x00001, 0x80000, CRC(9e94023d) SHA1(61a07eb835d324cb4fe7e3d366dd3907838b2554) )
	ROM_LOAD16_BYTE( "ua03.i5", 0x00000, 0x80000, CRC(08cfa8d8) SHA1(684729887bf2dd2fe22e5bd2e32073169d426e02) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "us02.p3", 0x00000, 0x10000, CRC(c7cc05fa) SHA1(5fbf479be98f618c63e4c74a250d51279c2f5e3b) ) // same as heuksun

	ROM_REGION( 0x040000, "oki", 0 ) // samples
	ROM_LOAD( "us08.q7", 0x00000, 0x40000, CRC(85d8f3de) SHA1(af55678bbe2c187cfee063c6f74cdd568307a7a2) )

	ROM_REGION( 0x200000, "tiles", 0 )

	ROM_LOAD16_BYTE( "uc08.m12", 0x00001, 0x20000, CRC(2fc0c30e) SHA1(0c50efd20340f10961e872b3cd63c36aefed26f0) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "uc07 p12", 0x00000, 0x20000,  CRC(3d299954) SHA1(f3a4d6fd02ed0803758b1ea3fbaccbb4dbb04718) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)
	ROM_LOAD16_BYTE( "uc29.k12", 0x80001, 0x20000, CRC(0260c321) SHA1(0ae7754c0f7de314bd72c51e273f7aaea2bae705) )
	ROM_CONTINUE ( 0x180001,0x20000)
	ROM_CONTINUE ( 0x0c0001,0x20000)
	ROM_CONTINUE ( 0x1c0001,0x20000)
	ROM_LOAD16_BYTE( "uc28.n12", 0x80000, 0x20000, CRC(9938be27) SHA1(1da7861dc44eba6e4ed6a27997428f7652b2f3b5) )
	ROM_CONTINUE ( 0x180000,0x20000)
	ROM_CONTINUE ( 0x0c0000,0x20000)
	ROM_CONTINUE ( 0x1c0000,0x20000)

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ud14.j12", 0x000000, 0x80000, CRC(141c696e) SHA1(3d35a20f7c12a8d8a9f6d351f06fb9df0c673354) )
	ROM_LOAD16_BYTE( "ud15.h12", 0x000001, 0x80000, CRC(7c04adc0) SHA1(9883565d6556ce8ae3da6c91cbf04894e87e6923) )
	ROM_LOAD16_BYTE( "ud16.g12", 0x100000, 0x80000, CRC(3282ea76) SHA1(cc21cac35f47ba299823c2cfe6b4946f8483b821) )
	ROM_LOAD16_BYTE( "ud17.e12", 0x100001, 0x80000, CRC(3a3a3f1a) SHA1(48843140cd63c9387e09b84bd41b13dba35f48ad) )
ROM_END

ROM_START( bestria ) // Developed March 1998
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "o_ua02.i3", 0x00001, 0x80000, CRC(035c86f6) SHA1(d501553ed7fdb462c9c26fff6473cefe71424e26) )
	ROM_LOAD16_BYTE( "e_ua03.i5", 0x00000, 0x80000, CRC(7c53d9f8) SHA1(92dc92471497292d3ba90f3f2fb35f7b4fba240c) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "us02.p3", 0x00000, 0x10000, CRC(c7cc05fa) SHA1(5fbf479be98f618c63e4c74a250d51279c2f5e3b)) // same as heuksun

	ROM_REGION( 0x040000, "oki", 0 ) // samples
	ROM_LOAD( "us08.q7", 0x00000, 0x40000, CRC(85d8f3de) SHA1(af55678bbe2c187cfee063c6f74cdd568307a7a2) )

	ROM_REGION( 0x200000, "tiles", 0 )

	ROM_LOAD16_BYTE( "2_uc08.m12", 0x00001, 0x20000, CRC(23778472) SHA1(00f54aefe52f2f76ab2f2628bf2e860d468e4a02) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "0_uc07.p12", 0x00000, 0x20000, CRC(7aad194c) SHA1(5fc5882886576d939763200e705e1085be60671a) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)
	ROM_LOAD16_BYTE( "3_uc29.k12", 0x80001, 0x20000, CRC(2f5b244f) SHA1(1d9bf3d1dd55a87d52d2d614f46177605e32c6bf) )
	ROM_CONTINUE ( 0x180001,0x20000)
	ROM_CONTINUE ( 0x0c0001,0x20000)
	ROM_CONTINUE ( 0x1c0001,0x20000)
	ROM_LOAD16_BYTE( "1_uc28.n12", 0x80000, 0x20000, CRC(4f737007) SHA1(37f379f3b491da35153ed3d14d8920f94b060643) )
	ROM_CONTINUE ( 0x180000,0x20000)
	ROM_CONTINUE ( 0x0c0000,0x20000)
	ROM_CONTINUE ( 0x1c0000,0x20000)

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "a_ud14.j12", 0x000000, 0x80000, CRC(3502f71b) SHA1(ec012c414ace560ab67d60ce407bd67a4640c217) )
	ROM_LOAD16_BYTE( "b_ud15.h12", 0x000001, 0x80000, CRC(2636b837) SHA1(692987bd8ace452ee40a253437f1e3672f737f98) )
	ROM_LOAD16_BYTE( "c_ud16.g12", 0x100000, 0x80000, CRC(68b0ff81) SHA1(969579c2a29b577b9077e70a03c0ec92997ddcc0) )
	ROM_LOAD16_BYTE( "d_ud17.e12", 0x100001, 0x80000, CRC(60082aed) SHA1(1431afe1a8200bd87520e90051db0ec43207b265) )
ROM_END


/*
Pitapat Puzzle (c) 1997

+----------------------------------+
|  YM3014 YM3812  M6295 us08       |
|        6116                  uc07|
|      us02  Z80               uc08|
|J  6116                       ud14|
|A  6116   GAL                 ud15|
|M    62256 62256 6264         ud16|
|M DSW2 ua02 ua03 6264         ud17|
|A      68000  A1020B              |
|  DSW1              GAL  GAL      |
|                  6116        6116|
|14.318MHz         6116   GAL  6116|
+----------------------------------+

      CPU: Motorola MC68000P12
Sound CPU: NEC D780C
    Sound: YM3812/YM3014, OKI M6295
      OSC: 14.318181MHz
 Graphics: Actel A1020B PL84C
      DSW: 8 position dipswitch x 2

*/

ROM_START( pitapat )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "ua02", 0x00001, 0x40000, CRC(b3d3ac7e) SHA1(7ff894cb6bcb724834de95bdefdb6a6c0ae1d39b) )
	ROM_LOAD16_BYTE( "ua03", 0x00000, 0x40000, CRC(eda85635) SHA1(b6723f5c196c4a531e411fc0d1f2632f514050ac) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "us02", 0x00000, 0x10000, CRC(c7cc05fa) SHA1(5fbf479be98f618c63e4c74a250d51279c2f5e3b) )

	ROM_REGION( 0x40000, "oki", 0 ) // samples
	ROM_LOAD( "us08", 0x00000, 0x40000, CRC(dab99a43) SHA1(32d329f9423ec91eb83ea42ee04de70d92568328) ) // sample ROM contains oksan?

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "uc08", 0x00001, 0x20000, CRC(3f827218) SHA1(38a3f427fad1850776a21a6486251fe33d7af498) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "uc07", 0x00000, 0x20000, CRC(f4a529c1) SHA1(a7cd10e0f57c5495684d82f0471b092599ae4c26) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ud14", 0x000000, 0x40000, CRC(92e23e92) SHA1(4e1b85cef2a55a54ca571bf948809715dd789f30) )
	ROM_LOAD16_BYTE( "ud15", 0x000001, 0x40000, CRC(7d3d6dba) SHA1(d543613fa22407bc8570e9e388c35620850ecd15) )
	ROM_LOAD16_BYTE( "ud16", 0x080000, 0x40000, CRC(5c09dff8) SHA1(412260784e45c6d742e02a285e3adc7361034268) )
	ROM_LOAD16_BYTE( "ud17", 0x080001, 0x40000, CRC(d4c67e2e) SHA1(e684b58333d64f5961983b42f56c61bb0bea2e5c) )
ROM_END

ROM_START( pitapata ) // Main CPU ROMs only differ in the addresses of the scroll registers and this one is 0x00 filled after 0x36c4f up to 0x3ffff while pitapat is 0x00 filled after 0x38a3f up to 0x3ffff
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "ua02", 0x00001, 0x40000, CRC(742652cb) SHA1(9ad426cd95b7ccc4a9394692ac204809ab4f74fd) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "ua03", 0x00000, 0x40000, CRC(936bd573) SHA1(112980271bc55d2e689a05142830415ff6bb9d23) ) // 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80, identical to pitapat
	ROM_LOAD( "us02", 0x00000, 0x10000, CRC(c7cc05fa) SHA1(5fbf479be98f618c63e4c74a250d51279c2f5e3b) )

	ROM_REGION( 0x40000, "oki", 0 ) // samples, seem to be moved around wrt pitapat, but audio CPU ROM's the same
	ROM_LOAD( "us08", 0x00000, 0x40000, CRC(8d8fe72a) SHA1(40d4b1a1eb2fe703e00e63f36b8ae7ae16287fb4) ) // sample ROM contains oksan?

	ROM_REGION( 0x200000, "tiles", 0 ) // 0x3fc80 to 0x3ffff is zero filled while it contains something in pitapat (although it seems gibberish)
	ROM_LOAD16_BYTE( "uc08", 0x000001, 0x20000, CRC(3108a9f2) SHA1(7a5b17b704439cfdc58aabddfc02639992a99354) )
	ROM_CONTINUE(            0x100001, 0x20000 )
	ROM_CONTINUE(            0x040001, 0x20000 )
	ROM_CONTINUE(            0x140001, 0x20000 )
	ROM_LOAD16_BYTE( "uc07", 0x000000, 0x20000, CRC(fa2ff22b) SHA1(afb02ee8598442826b709cac417786aa1bfda009) )
	ROM_CONTINUE(            0x100000, 0x20000 )
	ROM_CONTINUE(            0x040000, 0x20000 )
	ROM_CONTINUE(            0x140000, 0x20000 )

	ROM_REGION( 0x100000, "sprites", 0 ) // identical to pitapat
	ROM_LOAD16_BYTE( "ud14", 0x000000, 0x40000, CRC(92e23e92) SHA1(4e1b85cef2a55a54ca571bf948809715dd789f30) )
	ROM_LOAD16_BYTE( "ud15", 0x000001, 0x40000, CRC(7d3d6dba) SHA1(d543613fa22407bc8570e9e388c35620850ecd15) )
	ROM_LOAD16_BYTE( "ud16", 0x080000, 0x40000, CRC(5c09dff8) SHA1(412260784e45c6d742e02a285e3adc7361034268) )
	ROM_LOAD16_BYTE( "ud17", 0x080001, 0x40000, CRC(d4c67e2e) SHA1(e684b58333d64f5961983b42f56c61bb0bea2e5c) )
ROM_END

} // anonymous namespace


GAME( 1998, crospang, 0,       crospang, crospang, crospang_state, empty_init, ROT0, "F2 System",         "Cross Pang",               MACHINE_SUPPORTS_SAVE )
GAME( 1997, heuksun,  0,       crospang, heuksun,  crospang_state, empty_init, ROT0, "Oksan / F2 System", "Heuk Sun Baek Sa (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, bestri,   0,       bestri,   bestri,   crospang_state, empty_init, ROT0, "F2 System",         "Bestri (Korea, set 1)",    MACHINE_SUPPORTS_SAVE )
GAME( 1998, bestria,  bestri,  bestria,  bestri,   crospang_state, empty_init, ROT0, "F2 System",         "Bestri (Korea, set 2)",    MACHINE_SUPPORTS_SAVE )
GAME( 1997, pitapat,  0,       pitapat,  pitapat,  crospang_state, empty_init, ROT0, "F2 System",         "Pitapat Puzzle (set 1)",   MACHINE_SUPPORTS_SAVE ) // Test Mode calls it 'Puzzle Ball'
GAME( 1997, pitapata, pitapat, pitapata, pitapat,  crospang_state, empty_init, ROT0, "F2 System",         "Pitapat Puzzle (set 2)",   MACHINE_SUPPORTS_SAVE ) // "
