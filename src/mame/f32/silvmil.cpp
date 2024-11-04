// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
  Silver Millennium

  yet another Data East / Tumble Pop derived hardware
  this one seems similar to (but not identical to)
  the f32/crospang.cpp hardware from F2 system
  also very close to gotcha.c, which was also a Para
  board.

  todo: refresh rate
        verify dipswitches - difficulty & unknown dips

OSC on Silver Millennium are 12MHz, 14.31818MHz & 4.096MHz
The above has been verified on two boards.

OSC on World Puzzlove are 12MHz, 14.31818MHz & 4MHz
OSC on Korean Puzzlove are 12MHz, 15MHz & 4MHz
Only one example of each of these is known to exist so far.

Very likely to be 'whatever crystals we had on hand which were close enough for the batch' situations.

*/

#include "emu.h"
#include "decospr.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class silvmil_state : public driver_device
{
public:
	silvmil_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_sprgen(*this, "spritegen")
		, m_soundlatch(*this, "soundlatch")
		, m_bg_videoram(*this, "bg_videoram")
		, m_fg_videoram(*this, "fg_videoram")
		, m_spriteram(*this, "spriteram")
	{ }

	void puzzlovek(machine_config &config);
	void puzzlove(machine_config &config);
	void silvmil(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<decospr_device> m_sprgen;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<u16> m_bg_videoram;
	required_shared_ptr<u16> m_fg_videoram;
	required_shared_ptr<u16> m_spriteram;

	/* video-related */
	tilemap_t   *m_bg_layer = nullptr;
	tilemap_t   *m_fg_layer = nullptr;
	int         m_tilebank[4]{};
	int         m_whichbank = 0;

	void tilebank_w(u16 data);
	void tilebank1_w(u16 data);
	void fg_scrolly_w(u16 data);
	void bg_scrolly_w(u16 data);
	void fg_scrollx_w(u16 data);
	void bg_scrollx_w(u16 data);

	void fg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void soundcmd_w(u8 data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(scan_rows);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

void silvmil_state::tilebank_w(u16 data)
{
	m_tilebank[m_whichbank] = (data >> 8) & 0x1f;

//  printf("%08x tilebank_w %04x (which = %04x)\n",pc, data, m_whichbank);
	m_fg_layer->mark_all_dirty();
	m_bg_layer->mark_all_dirty();
}

void silvmil_state::tilebank1_w(u16 data)
{
	m_whichbank = (data >> 8) & 0x3;
}

void silvmil_state::fg_scrolly_w(u16 data)
{
	m_fg_layer->set_scrolly(0, data + 8);
}

void silvmil_state::bg_scrolly_w(u16 data)
{
	m_bg_layer->set_scrolly(0, data + 8);
}

void silvmil_state::fg_scrollx_w(u16 data)
{
	m_fg_layer->set_scrollx(0, data);
}

void silvmil_state::bg_scrollx_w(u16 data)
{
	m_bg_layer->set_scrollx(0, data + 4);
}

void silvmil_state::fg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

void silvmil_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(silvmil_state::get_bg_tile_info)
{
	int data  = m_bg_videoram[tile_index];
	int tile  = data & 0x3ff;
	int color = (data >> 12) & 0x0f;
	int bank = m_tilebank[(data & 0xc00) >> 10] * 0x400;

	tileinfo.set(0, tile + bank, color + 0x20, 0);
}

TILE_GET_INFO_MEMBER(silvmil_state::get_fg_tile_info)
{
	int data  = m_fg_videoram[tile_index];
	int tile  = data & 0x3ff;
	int color = (data >> 12) & 0x0f;
	int bank = m_tilebank[(data & 0xc00) >> 10] * 0x400;

	tileinfo.set(0, tile + bank, color + 0x10, 0);
}

TILEMAP_MAPPER_MEMBER(silvmil_state::scan_rows)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) << 6);
}

void silvmil_state::video_start()
{
	m_bg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(silvmil_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(silvmil_state::scan_rows)), 16, 16, 64, 32);
	m_fg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(silvmil_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(silvmil_state::scan_rows)), 16, 16, 64, 32);

	m_fg_layer->set_transparent_pen(0);
}

u32 silvmil_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_layer->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_layer->draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}


void silvmil_state::soundcmd_w(u8 data)
{
	m_soundlatch->write(data & 0xff);
	machine().scheduler().perfect_quantum(attotime::from_usec(20));
}

void silvmil_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();

	map(0x100000, 0x100001).w(FUNC(silvmil_state::tilebank1_w));
	map(0x100002, 0x100003).w(FUNC(silvmil_state::fg_scrollx_w));
	map(0x100004, 0x100005).w(FUNC(silvmil_state::fg_scrolly_w));
	map(0x100006, 0x100007).w(FUNC(silvmil_state::bg_scrollx_w));
	map(0x100008, 0x100009).w(FUNC(silvmil_state::bg_scrolly_w));
	map(0x10000e, 0x10000f).w(FUNC(silvmil_state::tilebank_w));

	map(0x120000, 0x120fff).ram().w(FUNC(silvmil_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x122000, 0x122fff).ram().w(FUNC(silvmil_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x200000, 0x2005ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x210000, 0x2107ff).ram().share(m_spriteram);
	map(0x270001, 0x270001).w(FUNC(silvmil_state::soundcmd_w));
	map(0x280000, 0x280001).portr("P1_P2");
	map(0x280002, 0x280003).portr("COIN");
	map(0x280004, 0x280005).portr("DSW");
	map(0x300000, 0x30ffff).ram();
}


static INPUT_PORTS_START( silvmil )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COIN")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") /* According to Service Mode - Need verification!! */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DSW1:3 - Unknown" )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DSW1:4 - Unknown" )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DSW1:5 - Unknown" )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DSW1:6 - Unknown" )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DSW1:7 - Unknown" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DSW1:8 - Unknown" )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSW2:1 - Unknown" )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DSW2:2 - Unknown" )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:3") /* Verified */
	PORT_DIPSETTING(      0x0400, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:4,5,6") /* Verified */
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_2C ) ) /* Works the same as 2C/1C */
	PORT_DIPSETTING(      0x2800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_2C ) ) /* Works like 2C/1C then 1C/1C repeat */
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_2C ) ) /* Works the same as 1C/1C */
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Coin Box" )          PORT_DIPLOCATION("SW2:7") /* Functionally reversed?? */
	PORT_DIPSETTING(      0x4000, "1" ) /* Credits from Coin1 or Coin2 */
	PORT_DIPSETTING(      0x0000, "2" ) /* Doesn't credit up from Coin2 */
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" ) /* Verified */
INPUT_PORTS_END


static INPUT_PORTS_START( puzzlove )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COIN")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") /* NOTE: In the service mode, the game reports the dipswitch banks in reverse. IE: SW1 is actually SW2 & SW2 is actually SW1 */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW1:1" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW1:2" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Very_Easy ) )       /* Initial start time 40 secs */
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )            /* Initial start time 30 secs - Other in game effects?? */
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )            /* Initial start time 30 secs - Other in game effects?? */
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )       /* Initial start time 30 secs - Other in game effects?? */
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_2C ) ) /* Works the same as 2C/1C */
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_2C ) ) /* Works like 2C/1C then 1C/1C repeat */
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_2C ) ) /* Works the same as 1C/1C */
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Coin Box" )      PORT_DIPLOCATION("SW2:6") /* Always displays total credits - No separate Credit display per player */
	PORT_DIPSETTING(      0x2000, "1" ) /* Both players use coins from either Coin 1 or Coin 2 */
	PORT_DIPSETTING(      0x0000, "2" ) /* Players use coins from own coin chute. IE: Player 1 -> Coin 1, Player 2 -> Coin 2 but shows only total credits */
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x4000, "3" )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" ) /* Verified */
INPUT_PORTS_END

static INPUT_PORTS_START( puzzlovek )
	PORT_INCLUDE(puzzlove)

	PORT_MODIFY("DSW")
	/* Korean rev has demo sound */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING( 0x0001, DEF_STR ( Off ) )
	PORT_DIPSETTING( 0x0000, DEF_STR ( On ) )

	/* Korean rev has slightly different difficulty settings */
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Easy ) )       /* Initial start time 35 secs */
	PORT_DIPSETTING(      0x0200, DEF_STR( Very_Easy ) )  /* Initial start time 40 secs - Other in game effects?? */
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )       /* Initial start time 30 secs - Other in game effects?? */
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )  /* Initial start time 30 secs - Other in game effects?? */
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


static GFXDECODE_START( gfx_silvmil )
	GFXDECODE_ENTRY( "tiles", 0, tlayout_alt, 0, 64 )  /* Tiles 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_silvmil_spr )
	GFXDECODE_ENTRY( "sprites", 0, tlayout, 0, 64 )  /* Sprites 16x16 */
GFXDECODE_END


void silvmil_state::machine_start()
{
	save_item(NAME(m_tilebank));
	save_item(NAME(m_whichbank));
}

void silvmil_state::machine_reset()
{
	m_tilebank[0] = 0;
	m_tilebank[1] = 0;
	m_tilebank[2] = 0;
	m_tilebank[3] = 0;
	m_whichbank = 0;
}


void silvmil_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xc002, 0xc002).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)).mirror(1);
	map(0xc006, 0xc006).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xc00f, 0xc00f).nopw(); // ??
	map(0xd000, 0xd7ff).ram();
}


void silvmil_state::silvmil(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); /* Verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &silvmil_state::mem_map);
	m_maincpu->set_vblank_int("screen", FUNC(silvmil_state::irq6_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(4'096'000))); /* Verified */
	audiocpu.set_addrmap(AS_PROGRAM, &silvmil_state::sound_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 40*8-1, 0, 30*8-1);
	screen.set_screen_update(FUNC(silvmil_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x300);
	GFXDECODE(config, m_gfxdecode, "palette", gfx_silvmil);

	DECO_SPRITE(config, m_sprgen, 0, "palette", gfx_silvmil_spr);
	m_sprgen->set_is_bootleg(true);
	m_sprgen->set_offsets(5, 7);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4)); /* Verified */
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.25);

	OKIM6295(config, "oki", XTAL(4'096'000)/4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.25); /* Verified */
}

void silvmil_state::puzzlove(machine_config &config)
{
	silvmil(config);
	subdevice<z80_device>("audiocpu")->set_clock(XTAL(4'000'000)); /* Verified */

	m_sprgen->set_bootleg_type(1);

	subdevice<okim6295_device>("oki")->set_clock(XTAL(4'000'000)/4); /* Verified */
}

void silvmil_state::puzzlovek(machine_config &config)
{
	puzzlove(config);
	subdevice<ym2151_device>("ymsnd")->set_clock(XTAL(15'000'000)/4); /* Verified */
}


ROM_START( silvmil )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "d-13_u3.bin", 0x00000, 0x40000, CRC(46106c59) SHA1(ab965c1ea9ce89a67875530d76ac2bde7fd13e13) )
	ROM_LOAD16_BYTE( "d-14_u2.bin", 0x00001, 0x40000, CRC(65421eca) SHA1(5258df7ceab6427e1b6f93f51896f7f3a651300c) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* z80  */
	ROM_LOAD( "d-11_uz02.bin", 0x00000, 0x10000, CRC(b41289e6) SHA1(edf83206de418757a154d5a085b0b2fd97a77807) )

	ROM_REGION( 0x40000, "oki", 0 ) /* samples */
	ROM_LOAD( "d-12_uz1.bin", 0x00000, 0x40000, CRC(a170d8a9) SHA1(77339382570498f9f6eeb80595bfe72fc853fd68))

	ROM_REGION( 0x400000, "tiles", 0 )
	ROM_LOAD16_BYTE( "d-16_u41.bin",   0x000000, 0x20000, CRC(ff5ea605) SHA1(38e32f391b211ed280dd7f05eb13301cb9ddf57c) )
	ROM_CONTINUE ( 0x200000,0x20000 )
	ROM_CONTINUE ( 0x040000,0x20000 )
	ROM_CONTINUE ( 0x240000,0x20000 )
	ROM_LOAD16_BYTE( "d-15_u42.bin",   0x000001, 0x20000, CRC(5f72d6cc) SHA1(88641a42c0fc6e84ffd009fb40d0b1774e8c0630) )
	ROM_CONTINUE ( 0x200001,0x20000 )
	ROM_CONTINUE ( 0x040001,0x20000 )
	ROM_CONTINUE ( 0x240001,0x20000 )
	ROM_LOAD16_BYTE( "d-20_pat01.bin", 0x080001, 0x20000, CRC(cc4d3b49) SHA1(a2ef0d949443c9133487f83d5879daabca688d66) )
	ROM_CONTINUE ( 0x280001,0x20000 )
	ROM_CONTINUE ( 0x0c0001,0x20000 )
	ROM_CONTINUE ( 0x2c0001,0x20000 )
	ROM_LOAD16_BYTE( "d-17_pat11.bin", 0x080000, 0x20000, CRC(2facb3f1) SHA1(ea380345e1f15451e1ded5de178ec4648c33e2f0) )
	ROM_CONTINUE ( 0x280000,0x20000 )
	ROM_CONTINUE ( 0x0c0000,0x20000 )
	ROM_CONTINUE ( 0x2c0000,0x20000 )
	ROM_LOAD16_BYTE( "d-17_pat02.bin", 0x100001, 0x20000, CRC(addc5261) SHA1(73c5e29a39364c286a110678eb65b85ab1cf97d4) )
	ROM_CONTINUE ( 0x300001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_CONTINUE ( 0x340001,0x20000)
	ROM_LOAD16_BYTE( "d-18_pat12.bin", 0x100000, 0x20000, CRC(a429b237) SHA1(ab709b93c5232f3fdbb60954f93d04bb3828ac76) )
	ROM_CONTINUE ( 0x300000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)
	ROM_CONTINUE ( 0x340000,0x20000)
	ROM_LOAD16_BYTE( "d-18_pat03.bin", 0x180001, 0x20000, CRC(36b9c407) SHA1(291e29a2f5de6db639988e99a010fcac233387ad) )
	ROM_CONTINUE ( 0x380001,0x20000)
	ROM_CONTINUE ( 0x1c0001,0x20000)
	ROM_CONTINUE ( 0x3c0001,0x20000)
	ROM_LOAD16_BYTE( "d-19_pat13.bin", 0x180000, 0x20000, CRC(af2507ce) SHA1(280b3995a6164e4ef834247d637ee46c61bf9950) )
	ROM_CONTINUE ( 0x380000,0x20000)
	ROM_CONTINUE ( 0x1c0000,0x20000)
	ROM_CONTINUE ( 0x3c0000,0x20000)

	ROM_REGION( 0x200000, "sprites", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "d-17_u53.bin", 0x000000, 0x80000, CRC(4d177bda) SHA1(980205dead92830362095dac61d2e99bf62f0f5d) )
	ROM_LOAD16_BYTE( "d-18_u54.bin", 0x000001, 0x80000, CRC(218c4471) SHA1(6a64d7a6c18eb078a5848f4f97aa0c65e74ad3d9) )
	ROM_LOAD16_BYTE( "d-19_u55.bin", 0x100000, 0x80000, CRC(59507521) SHA1(8e0eaf8ecdfcaefccb5657278ecb2fea7489afb3) )
	ROM_LOAD16_BYTE( "d-20_u56.bin", 0x100001, 0x80000, CRC(e67c2c7d) SHA1(cddfd6a3d934e71853af62e3d2bf312618c9b4ff) )
ROM_END

ROM_START( puzzlove )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "3.u3", 0x00000, 0x40000, CRC(826c8472) SHA1(54f1a9fa0431de840d6fce466e09098a8d601660) )
	ROM_LOAD16_BYTE( "4.u2", 0x00001, 0x40000, CRC(64ddc708) SHA1(60b29f8cd5dd654be34452fd197a77abc872e63d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* z80  */
	ROM_LOAD( "1.uz02", 0x00000, 0x20000, CRC(3077e7f3) SHA1(e2bf634a2166e1851486a801e74a7ec0d4599c28) )

	ROM_REGION( 0x40000, "oki", 0 ) /* samples */
	ROM_LOAD( "2.uz11", 0x00000, 0x40000, CRC(4c06ec68) SHA1(3cfca1c98e73c65a45b65d43e012c5529572c057) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "10.u41",   0x000000, 0x20000, CRC(7200f878) SHA1(27c6389f802f6e0af0210e2b01788914c0eb1d04) )
	ROM_CONTINUE ( 0x080000,0x20000 )
	ROM_CONTINUE ( 0x040000,0x20000 )
	ROM_CONTINUE ( 0x0c0000,0x20000 )
	ROM_LOAD16_BYTE( "9.u42",   0x000001, 0x20000, CRC(21b1b297) SHA1(0f589d1c62d0f79b1379e4444b119bdc4cc70cfb) )
	ROM_CONTINUE ( 0x080001,0x20000 )
	ROM_CONTINUE ( 0x040001,0x20000 )
	ROM_CONTINUE ( 0x0c0001,0x20000 )

	ROM_REGION( 0x200000, "sprites", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "5.u53", 0x000000, 0x80000, CRC(8707d5a0) SHA1(05480ac34982a4e4768b7f3fccd2e557ca4b2545) )
	ROM_LOAD16_BYTE( "6.u54", 0x000001, 0x80000, CRC(60a6d614) SHA1(0693c08c51d6b3a05373c9999f01b0b8d23a1c89) )
	ROM_LOAD16_BYTE( "7.u55", 0x100000, 0x80000, CRC(0f2ea5c4) SHA1(4cb46fc6272e3cc14dfdcd7831157433ee7cf247) )
	ROM_LOAD16_BYTE( "8.u56", 0x100001, 0x80000, CRC(037dcd3d) SHA1(fcdf604710518982e0b4acc81a56fa703d0c9407) )
ROM_END

ROM_START( puzzlovek )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "3.u3", 0x00000, 0x40000, CRC(28f403f0) SHA1(5f9fc18f705fe81f0dceaf2d62b6caffb0b0462e) ) // sldh
	ROM_LOAD16_BYTE( "4.u2", 0x00001, 0x40000, CRC(809371b9) SHA1(6b2dc899a4e2cf4fca777f0ec4a08e636099e0d2) ) // sldh

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* z80  */
	ROM_LOAD( "1.uz02", 0x00000, 0x20000, CRC(3077e7f3) SHA1(e2bf634a2166e1851486a801e74a7ec0d4599c28) )

	ROM_REGION( 0x40000, "oki", 0 ) /* samples */
	ROM_LOAD( "2.uz11", 0x00000, 0x40000, CRC(4c06ec68) SHA1(3cfca1c98e73c65a45b65d43e012c5529572c057) )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "10.u41",  0x000000, 0x20000, CRC(3f952c54) SHA1(1f18579ce98305d20ec0f4e216f8170a62b9c68b) ) // sldh
	ROM_CONTINUE ( 0x080000,0x20000 )
	ROM_CONTINUE ( 0x040000,0x20000 )
	ROM_CONTINUE ( 0x0c0000,0x20000 )
	ROM_LOAD16_BYTE( "9.u42",   0x000001, 0x20000, CRC(4f71ee93) SHA1(db0d00fa0f6c9b7bc638dcb50f041fb32827c7c4) ) // sldh
	ROM_CONTINUE ( 0x080001,0x20000 )
	ROM_CONTINUE ( 0x040001,0x20000 )
	ROM_CONTINUE ( 0x0c0001,0x20000 )

	ROM_REGION( 0x200000, "sprites", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "5.u53", 0x000000, 0x80000, CRC(c5732995) SHA1(e7faecb19f4bdb103b782e38463d32b357ea63bc) ) // sldh
	ROM_LOAD16_BYTE( "6.u54", 0x000001, 0x80000, CRC(a3b98fd1) SHA1(2b2b7c9df19882a0565e38504b73f56ea27d71ab) ) // sldh
	ROM_LOAD16_BYTE( "7.u55", 0x100000, 0x80000, CRC(a4c73b48) SHA1(9f26af7b961c96cfd1c45f85f1d6dc4f364e3541) ) // sldh
	ROM_LOAD16_BYTE( "8.u56", 0x100001, 0x80000, CRC(95b5f049) SHA1(1104dac1fbf6a894b7d8294b3f44a0edbf363157) ) // sldh
ROM_END

} // anonymous namespace


GAME( 1995, silvmil,    0,        silvmil,   silvmil,   silvmil_state, empty_init, ROT270, "Para", "Silver Millennium", MACHINE_SUPPORTS_SAVE )
GAME( 1994, puzzlove,   0,        puzzlove,  puzzlove,  silvmil_state, empty_init, ROT0,   "Para", "PuzzLove", MACHINE_SUPPORTS_SAVE )
GAME( 1994, puzzlovek,  puzzlove, puzzlovek, puzzlovek, silvmil_state, empty_init, ROT0,   "Para", "PuzzLove (Korea)", MACHINE_SUPPORTS_SAVE )
