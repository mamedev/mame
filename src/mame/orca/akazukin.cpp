// license:BSD-3-Clause
// copyright-holders: Alberto Salso, Ignacio Seki
/*
ORCA OVG-33C + ORCA OVG-46C

TODO:
- Unify '46C to common device implementations;
- Unify sound section, it's ported from sigma/sub.cpp;

Notes:
- Wolves "teleports" left and right when coming from rocks, btanb.

*/


#include "emu.h"

//#include "orca40c.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
//#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class akazukin_state : public driver_device
{
public:
	akazukin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_mainlatch(*this, "mainlatch")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_bgvideoram(*this, "bg%uvideoram", 1U)
		, m_fgvideoram(*this, "fgvideoram")
		, m_fg_vregs(*this, "fg_vregs")
		, m_soundlatch(*this, "soundlatch%u", 0)
		, m_ay(*this, "ay%u", 0)
	{ }

	void akazukin(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<ls259_device> m_mainlatch;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr_array<uint8_t, 2> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_fg_vregs;

	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_device_array<ay8910_device, 2> m_ay;

	uint8_t m_nmi_mask = 0;
	uint8_t m_nmi_sub_mask = 0;

	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	INTERRUPT_GEN_MEMBER(vblank_irq);

	void nmi_sub_mask_w(uint8_t data);
	INTERRUPT_GEN_MEMBER(sub_irq);

	void main_map(address_map &map);
	void main_io(address_map &map);
	void sub_map(address_map &map);
	void sub_io(address_map &map);

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap[2]{};
	uint8_t* m_bg_scroll[2]{};
	uint8_t* m_spriteram[3]{};

	void fgvideoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void bgvideoram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/******************
 *
 * Video section
 *
 *****************/

void akazukin_state::video_start()
{
	m_fg_tilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(akazukin_state::get_fg_tile_info)),  TILEMAP_SCAN_ROWS, 8,8, 32,32);
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(akazukin_state::get_bg_tile_info<0>)), TILEMAP_SCAN_ROWS, 8,8, 32,32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(akazukin_state::get_bg_tile_info<1>)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);

	m_bg_tilemap[0]->set_scroll_cols(32);
	m_bg_tilemap[1]->set_scroll_cols(32);
}


TILE_GET_INFO_MEMBER(akazukin_state::get_fg_tile_info)
{
	const u32 m_fg_codebase = 0x000;
	const u32 m_fg_attrbase = 0x400;
	const u32 m_fg_colbase = 0x800;

	int const code = m_fgvideoram[tile_index + m_fg_codebase] | (m_fgvideoram[tile_index + m_fg_attrbase] << 8);
	int const color = m_fgvideoram[tile_index + m_fg_colbase];
	// TODO: guess, based on the other layers
	int const fxy = (code & 0xc00) >> 10;
	tileinfo.set(0,
			code,
			color & 0x3f,
			TILE_FLIPXY(fxy));
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(akazukin_state::get_bg_tile_info)
{
	const u32 m_bg_codebase = 0x000;
	const u32 m_bg_attrbase = 0x800;
	const u32 m_bg_colbase = 0x400;

	int const code = m_bgvideoram[Which][tile_index + m_bg_codebase] | (m_bgvideoram[Which][tile_index + m_bg_attrbase] << 8);
	int const color = m_bgvideoram[Which][tile_index + m_bg_colbase];
	int fxy = (code & 0xc00) >> 10;
	tileinfo.set(4 - Which,
			code,
			color & 0x3f,
			TILE_FLIPXY(fxy));
}

template <uint8_t Which>
void akazukin_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[Which][offset] = data;
	m_bg_tilemap[Which]->mark_tile_dirty(offset & 0x3ff);
}

void akazukin_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void akazukin_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  for (int offs = 0; offs < 0x40; offs += 2)
	for (int offs = 0x40 - 2; offs >= 0; offs -= 2)
	{
		if (!(offs & 0x10))
			continue;

		const int code = ((m_spriteram[2][offs] & 0xfc) >> 2) + ((m_spriteram[1][offs] & 0x01) << 6)
						+ ((offs & 0x20) << 2);

		const int sx = m_spriteram[2][offs + 1];
		int sy = m_spriteram[0][offs];
		const int color = m_spriteram[0][offs + 1] & 0x3f;
		int flipx = m_spriteram[2][offs] & 0x02;
		int flipy = m_spriteram[2][offs] & 0x01;

		if (flip_screen())
		{
			int temp = flipx;
			flipx = !flipy;
			flipy = !temp;
		}

		if (m_spriteram[1][offs] & 0x08)   // double width
		{
			if (!flip_screen())
				sy = 224 - sy;

			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
					code / 2,
					color,
					flipx, flipy,
					sx, sy, 0);
			// redraw with wraparound y
			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
					code / 2,
					color,
					flipx, flipy,
					sx, sy + 256, 0);

			// redraw with wraparound x
			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
					code / 2,
					color,
					flipx, flipy,
					sx - 256, sy, 0);

			// redraw with wraparound xy
			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
					code / 2,
					color,
					flipx, flipy,
					sx - 256, sy + 256, 0);
		}
		else
		{
			if (!flip_screen())
				sy = 240 - sy;

			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					sx, sy, 0);

			// redraw with wraparound x
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					sx - 256, sy, 0);
		}
	}
}

uint32_t akazukin_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 32; i++)
	{
		m_bg_tilemap[0]->set_scrolly(i, m_bg_scroll[0][i]);
		m_bg_tilemap[1]->set_scrolly(i, m_bg_scroll[1][i]);
	}


	// TODO: copied from orca/vastar.cpp
	// Looks like $ac00 is some kind of '46C mixer control.
	switch (m_fg_vregs[0])
	{
	case 0:
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;

	case 1: // ?? planet probe
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;

	case 2:
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		break;

	case 3:
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
		break;

	case 4: // akazukin title screen
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		break;

	default:
		popmessage("Unimplemented priority %X\n", m_fg_vregs[0]);
		break;
	}

	return 0;
}


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static const gfx_layout spritelayoutdw =
{
	16,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			96*8, 97*8, 98*8, 99*8, 100*8, 101*8, 102*8, 103*8 },
	128*8
};

static GFXDECODE_START( gfx_vastar )
	GFXDECODE_ENTRY( "fgtiles",  0, charlayout,     0, 64 )
	GFXDECODE_ENTRY( "sprites",  0, spritelayout,   0, 64 )
	GFXDECODE_ENTRY( "sprites",  0, spritelayoutdw, 0, 64 )
	GFXDECODE_ENTRY( "bgtiles0", 0, charlayout,     0, 64 )
	GFXDECODE_ENTRY( "bgtiles1", 0, charlayout,     0, 64 )
GFXDECODE_END


/******************
 *
 * Machine section
 *
 *****************/

void akazukin_state::machine_start()
{
	m_spriteram[0] = m_fgvideoram + 0x800;
	m_bg_scroll[0] = m_fgvideoram + 0xbe0;
	m_bg_scroll[1] = m_fgvideoram + 0xbc0;
	m_spriteram[1] = m_fgvideoram + 0x400;
	m_spriteram[2] = m_fgvideoram + 0x000;

	save_item(NAME(m_nmi_mask));
}

void akazukin_state::machine_reset()
{
	// ...
}

WRITE_LINE_MEMBER(akazukin_state::nmi_mask_w)
{
	m_nmi_mask = state;
}

void akazukin_state::nmi_sub_mask_w(uint8_t data)
{
	m_nmi_sub_mask = data & 1;
}


void akazukin_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram().w(FUNC(akazukin_state::bgvideoram_w<1>)).share(m_bgvideoram[1]);
	map(0x9000, 0x9fff).ram().w(FUNC(akazukin_state::bgvideoram_w<0>)).share(m_bgvideoram[0]);
	map(0xa000, 0xabff).ram().w(FUNC(akazukin_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xac00, 0xafff).ram().share(m_fg_vregs);
	map(0xc000, 0xc007).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0xe000, 0xe000).portr("SYSTEM");
	map(0xe800, 0xe800).portr("P1");
	map(0xf000, 0xf000).portr("P2");
	map(0xf800, 0xffff).ram();
}

void akazukin_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).r(m_soundlatch[1], FUNC(generic_latch_8_device::read)).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
}

void akazukin_state::sub_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x5000, 0x5000).w(FUNC(akazukin_state::nmi_sub_mask_w));
}

void akazukin_state::sub_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).r(m_soundlatch[0], FUNC(generic_latch_8_device::read)).w(m_soundlatch[1], FUNC(generic_latch_8_device::write));
	map(0x40, 0x41).w(m_ay[1], FUNC(ay8910_device::address_data_w));
	map(0x42, 0x42).r(m_ay[1], FUNC(ay8910_device::data_r));
	map(0x80, 0x81).w(m_ay[0], FUNC(ay8910_device::address_data_w));
	map(0x82, 0x82).r(m_ay[0], FUNC(ay8910_device::data_r));
//  map(0xc0, 0xc0).nopw();
}


static INPUT_PORTS_START( akazukin )
	// up/down used on high score entry, useable with 2 way gate
	// 8 way causes locks in place during gameplay on diagonals, assume 4 way
	// TODO: is b2 really used?
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "255" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// Transitions from off -> on during gameplay will skip current played level.
	// This doesn't occur on attract mode.
	PORT_DIPNAME( 0x80, 0x80, "Skip current level (Cheat)" )   PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0xe0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW2:5,6,7,8")
	PORT_DIPSETTING(    0xb0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
//  PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
//  PORT_DIPSETTING(    0x70, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
//  PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
//  PORT_DIPSETTING(    0x60, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(akazukin_state::vblank_irq)
{
	if (m_nmi_mask)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INTERRUPT_GEN_MEMBER(akazukin_state::sub_irq)
{
	if (m_nmi_sub_mask)
		m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void akazukin_state::akazukin(machine_config &config)
{
	// m_maincpu NMI from vblank, IRQ0 from subcpu
	Z80(config, m_maincpu, XTAL(18'432'000) / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &akazukin_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &akazukin_state::main_io);
	m_maincpu->set_vblank_int("screen", FUNC(akazukin_state::vblank_irq));

	// TODO: same as sub.cpp
	Z80(config, m_subcpu, XTAL(18'432'000) / 6);
	m_subcpu->set_addrmap(AS_PROGRAM, &akazukin_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &akazukin_state::sub_io);
	m_subcpu->set_periodic_int(FUNC(akazukin_state::sub_irq), attotime::from_hz(60));

	config.set_maximum_quantum(attotime::from_hz(600));   // 10 CPU slices per frame - seems enough to ensure proper synchronization of the CPUs

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(akazukin_state::nmi_mask_w));
	m_mainlatch->q_out_cb<1>().set(FUNC(akazukin_state::flip_screen_set));
	m_mainlatch->q_out_cb<2>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert();

//  WATCHDOG_TIMER(config, "watchdog");

	GENERIC_LATCH_8(config, m_soundlatch[0]).data_pending_callback().set_inputline(m_subcpu, 0);
	GENERIC_LATCH_8(config, m_soundlatch[1]).data_pending_callback().set_inputline(m_maincpu, 0);

	// video hardware
	screen_device& screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(akazukin_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vastar);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay[0], XTAL(18'432'000) / 12);
	m_ay[0]->port_a_read_callback().set_ioport("DSW1");
	m_ay[0]->port_b_read_callback().set_constant(0xff);
	m_ay[0]->add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, m_ay[1], XTAL(18'432'000) / 12);
	m_ay[1]->port_a_read_callback().set_ioport("DSW2");
	m_ay[1]->port_b_read_callback().set_constant(0xff);
	m_ay[1]->add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( akazukin )
	ROM_REGION( 0x10000, "maincpu", 0 ) // on ORCA OVG-33c board
	ROM_LOAD( "1.j2",  0x0000, 0x2000, CRC(8987f1e0) SHA1(a7a705f83a8b53c3c7b74b695a086516e3316219) )
	ROM_LOAD( "2.l2",  0x2000, 0x2000, CRC(c8b040e4) SHA1(36b37f75080b682405ce1ad093538f6a1231fe72) )
	ROM_LOAD( "3.m2",  0x4000, 0x2000, CRC(262edb0d) SHA1(afb2994274ae3d189ca3ffad556727afb0a8b1a4) )
	ROM_LOAD( "4.n2",  0x6000, 0x2000, CRC(3569221f) SHA1(045cf99fe85b26503d368fecf1dad5f3785b5d79) )

	ROM_REGION( 0x10000, "sub", 0 ) // on ORCA OVG-33c board
	ROM_LOAD( "5.h7",  0x0000, 0x2000, CRC(c34486ae) SHA1(28b3db7e906202ee0d49907f66202544d352ff3f) )
	ROM_LOAD( "6.j7",  0x2000, 0x1000, CRC(b5e1d77f) SHA1(637f17d04976bc43801012280010df8500313331) )

	ROM_REGION( 0x1000, "fgtiles", 0 ) // on ORCA OVG-46C sub board
	ROM_LOAD( "10.b9",  0x0000, 0x1000, CRC(0145ded8) SHA1(515f3943579f2a8e17ca05959b9354981219d9d9) )

	ROM_REGION( 0x4000, "sprites", 0 ) // on ORCA OVG-46C sub board
	ROM_LOAD( "9.e9",   0x0000, 0x2000, CRC(5fecc3d7) SHA1(619584936382c38c391c654684b24f6d642dff03) )
	ROM_LOAD( "11.e7",  0x2000, 0x2000, CRC(448b28b8) SHA1(3a379c0a57ac89698b332d0af20ca8a3b0476d79) )

	ROM_REGION( 0x1000, "bgtiles0", 0 ) // on ORCA OVG-46C sub board
	ROM_LOAD( "7.t4",  0x0000, 0x1000, CRC(7b4124da) SHA1(9bd713ac660920f6c5b407e80df9438df575de88) )

	ROM_REGION( 0x2000, "bgtiles1", 0 ) // on ORCA OVG-46C sub board
	ROM_LOAD( "8.p4",  0x0000, 0x2000, CRC(193f6bcb) SHA1(0f4699052b2c66fabd293e7ef08fd25de7af42f3) )

	ROM_REGION( 0x0300, "proms", 0 ) // on ORCA OVG-46C sub board
	ROM_LOAD( "r.r6",  0x0000, 0x0100, CRC(77ccc932) SHA1(5bd23ca5ab80ac9c19e85ea79ba3d276c4be59cb) ) // red component
	ROM_LOAD( "g.m6",  0x0100, 0x0100, CRC(eddc3acf) SHA1(0fca5d36ccbd5191ce8a59d070112ae4a3297b0b) ) // green component
	ROM_LOAD( "b.l6",  0x0200, 0x0100, CRC(059dae45) SHA1(26c9b975804fc206e80ee21361e489824f39e0c0) ) // blue component

	ROM_REGION( 0x0100, "unkprom", 0 ) // on ORCA OVG-46C sub board
	ROM_LOAD( "8n",  0x0000, 0x0100, CRC(e1b815ca) SHA1(df2b99259bf1023d37aa3b54247721d657d7e9c9) ) // ????
ROM_END

} // anonymous namespace


// written as "Akazukin" on title screen
GAME( 1983, akazukin, 0, akazukin, akazukin, akazukin_state, empty_init, ROT0, "Sigma", "Aka Zukin (Japan)", MACHINE_SUPPORTS_SAVE )

