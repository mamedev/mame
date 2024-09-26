// license:BSD-3-Clause
// copyright-holders: Bryan McPhail, Acho A. Tang, Nicola Salmoria

/***************************************************************************

    POW - Prisoners Of War (US version 1)    A7008   SNK 1988
    POW - Prisoners Of War (Japan)           A7008   SNK 1988
    SAR - Search And Rescue (World)          A8007   SNK 1989
    SAR - Search And Rescue (US)             A8007   SNK 1989
    SAR - Search And Rescue (Japan)          A8007   SNK 1989
    Street Smart (US version 1)              A8007   SNK 1989
    Street Smart (US version 2)              A7008   SNK 1989
    Street Smart (World version 1)           A8007   SNK 1989
    Street Smart (Japan version 1)           A8007   SNK 1989
    Ikari III - The Rescue (US)              A7007   SNK 1989

    For some strange reason version 2 of Street Smart runs on POW hardware!

    Driver by Bryan McPhail, Acho A. Tang, Nicola Salmoria


Notes:
------
- All evidence suggests that the sprite hardware doesn't have a frame buffer
  but just a raster line buffer, like NeoGeo (unsurprisingly). The maths
  confirm this:
  384 pixels per raster line at 4 clocks per pixel = 1536 clocks per line
  96 sprites * 16 pixels per sprite  = 1536 clocks to draw the sprites

  While this board doesn't have a raster interrupt capability, the way how
  sprites are drawn needs to be kept in consideration because at least in one
  case the program modifies the sprite list in the middle of the frame:
  bug 00871: pow: At 3/4 of the 1st level, there is a large pillar, which pops up too late.
  The problem in this case is that the sprite list is built by the IRQ handler,
  however there is code in the main loop that clears some portions of sprite
  RAM under certain conditions. Usually, this isn't a problem, but in that
  specific point the sprites added by the IRQ handler are erased during the
  frame.
  To avoid glitches in that case, we force a partial screen update every time
  sprite RAM changes. It's possible that there are other unknown small glitches
  fixed by this (earlier notes in this driver talked about "sprite flickerings
  and pop-ups" but I don't know where they happened).

***************************************************************************/

#include "emu.h"

#include "alpha68k_palette.h"
#include "snk68_spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/upd7759.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class snk68_state : public driver_device
{
public:
	snk68_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_sprites(*this, "sprites"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_p1_io(*this, "P1"),
		m_p2_io(*this, "P2"),
		m_system_io(*this, "SYSTEM")
	{
	}

	void streetsm(machine_config &config);
	void pow(machine_config &config);
	void powbl(machine_config &config);

	void init_powbl();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<snk68_spr_device> m_sprites;
	required_device<alpha68k_palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	required_ioport m_p1_io;
	required_ioport m_p2_io;
	required_ioport m_system_io;

	bool m_sprite_flip_axis;
	tilemap_t *m_fg_tilemap;

	// common
	void sound_w(uint8_t data);
	void d7759_write_port_0_w(uint8_t data);

	virtual void video_start() override ATTR_COLD;
	void common_video_start();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tile_callback_notpow(int &tile, int &fx, int &fy, int &region);

	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

private:
	uint32_t m_fg_tile_offset;

	// pow and streetsm
	uint16_t fg_videoram_r(offs_t offset);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void flipscreen_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void tile_callback_pow(int &tile, int &fx, int &fy, int &region);

	void program_map(address_map &map) ATTR_COLD;
	void powbl_sound_io_map(address_map &map) ATTR_COLD;
};

class searchar_state : public snk68_state
{
public:
	searchar_state(const machine_config &mconfig, device_type type, const char *tag) :
		snk68_state(mconfig, type, tag),
		m_rotary_io(*this, "ROT%u", 1U)
	{
	}

	void searchar(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	optional_ioport_array<2> m_rotary_io;

	uint8_t m_invert_controls = 0;

	// searchar and ikari3
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void flipscreen_w(uint8_t data);
	uint16_t rotary_1_r();
	uint16_t rotary_2_r();
	uint16_t rotary_lsb_r();

	TILE_GET_INFO_MEMBER(get_tile_info);

	void program_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

    SNK 68000 video routines

Notes:
    Search & Rescue uses Y flip on sprites only.
    Street Smart uses X flip on sprites only.

    Seems to be controlled in same byte as flipscreen.

***************************************************************************/

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(snk68_state::get_tile_info)
{
	int const tile = m_fg_tile_offset + (m_fg_videoram[2 * tile_index] & 0xff);
	int const color = m_fg_videoram[2 * tile_index + 1] & 0x07;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(searchar_state::get_tile_info)
{
	int const data = m_fg_videoram[2 * tile_index];
	int const tile = data & 0x7ff;
	int const color = (data & 0x7000) >> 12;

	// used in the ikari3 intro
	int const flags = (data & 0x8000) ? TILE_FORCE_LAYER0 : 0;

	tileinfo.set(0, tile, color, flags);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void snk68_state::common_video_start()
{
	m_fg_tilemap->set_transparent_pen(0);
	save_item(NAME(m_sprite_flip_axis));
}

void snk68_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(snk68_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fg_tile_offset = 0;

	common_video_start();

	save_item(NAME(m_fg_tile_offset));
}

void searchar_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(searchar_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	common_video_start();
}

/***************************************************************************

  Memory handlers

***************************************************************************/

uint16_t snk68_state::fg_videoram_r(offs_t offset)
{
	// RAM is only 8-bit
	return m_fg_videoram[offset] | 0xff00;
}

void snk68_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data |= 0xff00;
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

void searchar_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// RAM is full 16-bit, though only half of it is used by the hardware
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

void snk68_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(BIT(data, 3));
	m_sprites->set_flip(BIT(data, 3));
	m_sprite_flip_axis = BIT(data, 2);   // for streetsm? though might not be present on this board

	if (m_fg_tile_offset != ((data & 0x70) << 4))
	{
		m_fg_tile_offset = (data & 0x70) << 4;
		m_fg_tilemap->mark_all_dirty();
	}
}

void searchar_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(BIT(data, 3));
	m_sprites->set_flip(BIT(data, 3));
	m_sprite_flip_axis = BIT(data, 2);
}


/***************************************************************************

  Display refresh

***************************************************************************/


uint32_t snk68_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->get_backdrop_pen(), cliprect);

	m_sprites->draw_sprites_all(bitmap, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/******************************************************************************/

void snk68_state::sound_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_soundcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero); // caused by 74123
}

/*******************************************************************************/

void snk68_state::program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x043fff).ram();
	map(0x080000, 0x080000).lr8(NAME([this] () -> u8 { return m_p2_io->read(); }));
	map(0x080001, 0x080001).lr8(NAME([this] () -> u8 { return m_p1_io->read(); }));
	map(0x080000, 0x080000).w(FUNC(snk68_state::sound_w));
	map(0x0c0000, 0x0c0001).portr("SYSTEM");
	map(0x0c0001, 0x0c0001).w(FUNC(snk68_state::flipscreen_w));   // + char bank
	map(0x0e0000, 0x0e0001).nopr(); // Watchdog or IRQ ack
	map(0x0e8000, 0x0e8001).nopr(); // Watchdog or IRQ ack
	map(0x0f0000, 0x0f0001).portr("DSW1");
	map(0x0f0008, 0x0f0009).portr("DSW2");
//  map(0x0f0008, 0x0f0009).nopw();    // ??
	map(0x100000, 0x100fff).rw(FUNC(snk68_state::fg_videoram_r), FUNC(snk68_state::fg_videoram_w)).mirror(0x1000).share("fg_videoram");   // 8-bit
	map(0x200000, 0x207fff).rw(m_sprites, FUNC(snk68_spr_device::spriteram_r), FUNC(snk68_spr_device::spriteram_w)).share("spriteram");   // only partially populated
	map(0x400000, 0x400fff).rw(m_palette, FUNC(alpha68k_palette_device::read), FUNC(alpha68k_palette_device::write));
}

/*******************************************************************************/

void searchar_state::machine_start()
{
	save_item(NAME(m_invert_controls));
}

uint16_t searchar_state::rotary_1_r()
{
	return ((~(1 << m_rotary_io[0]->read())) << 8) & 0xff00;
}

uint16_t searchar_state::rotary_2_r()
{
	return ((~(1 << m_rotary_io[1]->read())) << 8) & 0xff00;
}

uint16_t searchar_state::rotary_lsb_r()
{
	return (((~(1 << m_rotary_io[1]->read())) << 4) & 0xf000)
			+ (((~(1 << m_rotary_io[0]->read()))) & 0x0f00);
}

/*******************************************************************************/

void searchar_state::program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x043fff).ram();
	map(0x080001, 0x080001).lr8(NAME([this] () -> u8 { return m_p1_io->read() ^ m_invert_controls; }));
	map(0x080003, 0x080003).lr8(NAME([this] () -> u8 { return m_p2_io->read() ^ m_invert_controls; }));
	map(0x080005, 0x080005).lr8(NAME([this] () -> u8 { return m_system_io->read() ^ m_invert_controls; }));
	map(0x080000, 0x080000).w(FUNC(searchar_state::sound_w));
	// top byte unknown, bottom is protection in ikari3 and streetsm
	map(0x080007, 0x080007).lw8(NAME([this] (u8 data){ m_invert_controls = ((data & 0xff) == 0x07) ? 0xff : 0x00; } ));
	map(0x0c0001, 0x0c0001).w(FUNC(searchar_state::flipscreen_w));
	map(0x0c0000, 0x0c0001).r(FUNC(searchar_state::rotary_1_r));
	map(0x0c8000, 0x0c8001).r(FUNC(searchar_state::rotary_2_r));
	map(0x0d0000, 0x0d0001).r(FUNC(searchar_state::rotary_lsb_r));
	map(0x0e0000, 0x0e0001).nopr(); // Watchdog or IRQ ack
	map(0x0e8000, 0x0e8001).nopr(); // Watchdog or IRQ ack
//  map(0x0f0000, 0x0f0001).nopw();    // ??
	map(0x0f0000, 0x0f0001).portr("DSW1");
	map(0x0f0008, 0x0f0009).portr("DSW2");
	map(0x0f8000, 0x0f8000).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0x100000, 0x107fff).rw(m_sprites, FUNC(snk68_spr_device::spriteram_r), FUNC(snk68_spr_device::spriteram_w)).share("spriteram");   // only partially populated
	map(0x200000, 0x200fff).ram().w(FUNC(searchar_state::fg_videoram_w)).mirror(0x1000).share("fg_videoram"); // Mirror is used by Ikari 3
	map(0x300000, 0x33ffff).rom().region("maincpu", 0x40000); // Extra code bank
	map(0x400000, 0x400fff).rw(m_palette, FUNC(alpha68k_palette_device::read), FUNC(alpha68k_palette_device::write));
}

/******************************************************************************/

void snk68_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w("soundlatch2", FUNC(generic_latch_8_device::write));
}

void snk68_state::d7759_write_port_0_w(uint8_t data)
{
	m_upd7759->port_w(data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}

void snk68_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x20, 0x20).w("ymsnd", FUNC(ym3812_device::data_w));
	map(0x40, 0x40).w(FUNC(snk68_state::d7759_write_port_0_w));
	map(0x80, 0x80).lw8(NAME([this] (u8 data) { m_upd7759->reset_w(BIT(data, 7)); } ));
}

void snk68_state::powbl_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x20, 0x20).w("ymsnd", FUNC(ym3812_device::data_w));
}

/******************************************************************************/

static INPUT_PORTS_START( pow )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )    // same as the service mode dsw
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!8,!7")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!6,!5")
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPNAME( 0x2000, 0x0000, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(      0x0000, "1st & 2nd only" )
	PORT_DIPSETTING(      0x2000, "1st & every 2nd" )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_HIGH, "SW2:!8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:!6,!5")
	PORT_DIPSETTING(      0x0000, "20k 50k" )
	PORT_DIPSETTING(      0x0800, "40k 100k" )
	PORT_DIPSETTING(      0x0400, "60k 150k" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x0000, "Game Mode" )         PORT_DIPLOCATION("SW2:!4,!3")
	PORT_DIPSETTING(      0x2000, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0000, "Demo Sounds On" )
	PORT_DIPSETTING(      0x3000, "Freeze" )
	PORT_DIPSETTING(      0x1000, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:!2,!1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Hardest ) )
INPUT_PORTS_END

// Identical to pow, but the Language dip switch has no effect
static INPUT_PORTS_START( powj )
	PORT_INCLUDE( pow )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_HIGH, "SW1:!2" )
INPUT_PORTS_END


static INPUT_PORTS_START( searchar )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )    // same as the service mode dsw
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Joystick ) )     PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(      0x0000, "Rotary Joystick" )
	PORT_DIPSETTING(      0x0100, "Standard Joystick" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!6,!5")
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0c00, "5" )
	PORT_DIPNAME( 0x3000, 0x0000, "Coin A & B" )            PORT_DIPLOCATION("SW1:!4,!3")
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x0000, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(      0x0000, "1st & 2nd only" )
	PORT_DIPSETTING(      0x4000, "1st & every 2nd" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_HIGH, "SW2:!8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:!6,!5")
	PORT_DIPSETTING(      0x0000, "50k 200k" )
	PORT_DIPSETTING(      0x0800, "70k 270k" )
	PORT_DIPSETTING(      0x0400, "90k 350k" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x0000, "Game Mode" )         PORT_DIPLOCATION("SW2:!4,!3")
	PORT_DIPSETTING(      0x2000, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0000, "Demo Sounds On" )
	PORT_DIPSETTING(      0x3000, "Freeze" )
	PORT_DIPSETTING(      0x1000, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:!2,!1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Hardest ) )

	PORT_START("ROT1")  // player 1 12-way rotary control
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL )  PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("ROT2")  // player 2 12-way rotary control
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL )  PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)
INPUT_PORTS_END


static INPUT_PORTS_START( streetsm )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )    // same as the service mode dsw
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  // Active high
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!8,7")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0300, "4" )
	PORT_DIPNAME( 0x0c00, 0x0000, "Coin A & B" )            PORT_DIPLOCATION("SW1:!6,5")
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(      0x0000, "1st & 2nd only" )
	PORT_DIPSETTING(      0x2000, "1st & every 2nd" )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2") // Active high
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_HIGH, "SW2:!8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:!6,!5")
	PORT_DIPSETTING(      0x0000, "200k 400k" )
	PORT_DIPSETTING(      0x0800, "400k 600k" )
	PORT_DIPSETTING(      0x0400, "600k 800k" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x0000, "Game Mode" )         PORT_DIPLOCATION("SW2:!4,!3")
	PORT_DIPSETTING(      0x2000, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0000, "Demo Sounds On" )
	PORT_DIPSETTING(      0x3000, "Freeze" )
	PORT_DIPSETTING(      0x1000, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:!2,!1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Hardest ) )

	PORT_START("ROT1")  // player 1 12-way rotary control - not used in this game
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROT2")  // player 2 12-way rotary control - not used in this game
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// Same as streetsm, but Coinage is different
static INPUT_PORTS_START( streetsj )
	PORT_INCLUDE( streetsm )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!6,5")
	PORT_DIPSETTING(      0x0c00, "A 4/1 B 1/4" )
	PORT_DIPSETTING(      0x0400, "A 3/1 B 1/3" )
	PORT_DIPSETTING(      0x0800, "A 2/1 B 1/2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ikari3 )
	PORT_START("P1")    // maybe all are active_high?
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )    // same as the service mode dsw
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  // Active high
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!8,!7")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0300, "5" )
	PORT_DIPNAME( 0x0c00, 0x0000, "Coin A & B" )            PORT_DIPLOCATION("SW1:!6,!5")
	PORT_DIPSETTING(      0x0800, "First 2 Coins/1 Credit then 1/1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, "First 1 Coin/2 Credits then 1/1" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(      0x0000, "1st & 2nd only" )
	PORT_DIPSETTING(      0x2000, "1st & every 2nd" )
	PORT_DIPNAME( 0x4000, 0x0000, "Blood" )             PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2") // Active high
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_HIGH, "SW2:!8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:!6,!5")
	PORT_DIPSETTING(      0x0000, "20k 50k" )
	PORT_DIPSETTING(      0x0800, "40k 100k" )
	PORT_DIPSETTING(      0x0400, "60k 150k" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x0000, "Game Mode" )         PORT_DIPLOCATION("SW2:!4,!3")
	PORT_DIPSETTING(      0x2000, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0000, "Demo Sounds On" )
	PORT_DIPSETTING(      0x3000, "Freeze" )
	PORT_DIPSETTING(      0x1000, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0xc000, 0x8000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:!2,!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Hardest ) )

	PORT_START("ROT1")  // player 1 12-way rotary control
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL )  PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("ROT2")  // player 2 12-way rotary control
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL )  PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(8*8+3,-1), STEP4(3,-1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 8, RGN_FRAC(1,2), RGN_FRAC(1,2)+8 },
	{ STEP8(32*8+7,-1), STEP8(7,-1) },
	{ STEP16(0,16) },
	64*8
};

static GFXDECODE_START( gfx_pow )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,   0,  0x80 >> 4 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0, 0x800 >> 4 )
GFXDECODE_END

/******************************************************************************/

// pow has 0x4000 tiles and independent x/y flipping
// the other games have > 0x4000 tiles and flipping in only one direction
// (globally selected)
void snk68_state::tile_callback_pow(int &tile, int &fx, int &fy, int &region)
{
	fx = tile & 0x4000;
	fy = tile & 0x8000;
	tile &= 0x3fff;
	region = 1;
}

void snk68_state::tile_callback_notpow(int &tile, int &fx, int &fy, int &region)
{
	if (m_sprite_flip_axis)
	{
		fx = 0;
		fy = tile & 0x8000;
	}
	else
	{
		fx = tile & 0x8000;
		fy = 0;
	}
	tile &= 0x7fff;
	region = 1;
}

void snk68_state::pow(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(18'000'000) / 2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &snk68_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(snk68_state::irq1_line_hold));

	Z80(config, m_soundcpu, XTAL(8'000'000) / 2); // verified on PCB
	m_soundcpu->set_addrmap(AS_PROGRAM, &snk68_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &snk68_state::sound_io_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// the screen parameters are guessed but should be accurate. They
	// give a theoretical refresh rate of 59.1856Hz while the measured
	// rate on a SAR board is 59.16Hz.
	m_screen->set_raw(XTAL(24'000'000) / 4, 384, 0, 256, 264, 16, 240);
	m_screen->set_screen_update(FUNC(snk68_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pow);

	ALPHA68K_PALETTE(config, m_palette, 0);
	m_palette->set_entries(2048);

	SNK68_SPR(config, m_sprites, 0);
	m_sprites->set_gfxdecode_tag(m_gfxdecode);
	m_sprites->set_tile_indirect_cb(FUNC(snk68_state::tile_callback_pow));
	m_sprites->set_xpos_shift(12);
	m_sprites->set_color_entry_mask(0x7f);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, "soundlatch2");

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(8'000'000) / 2)); // verified on PCB
	ymsnd.irq_handler().set_inputline(m_soundcpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void snk68_state::powbl(machine_config &config)
{
	pow(config);

	m_soundcpu->set_addrmap(AS_IO, &snk68_state::powbl_sound_io_map);

	config.device_remove("upd");

	MSM5205(config, "msm", 0).add_route(ALL_OUTPUTS, "mono", 0.50); // TODO: hook this up
}

void snk68_state::streetsm(machine_config &config)
{
	pow(config);
	m_sprites->set_tile_indirect_cb(FUNC(snk68_state::tile_callback_notpow));
}

void searchar_state::searchar(machine_config &config)
{
	streetsm(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &searchar_state::program_map);
}


/******************************************************************************/

ROM_START( pow )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dg1ver1.j14",   0x000000, 0x20000, CRC(8e71a8af) SHA1(72c2eb2316c2684491331e8adabcb2be084aa6a2) )
	ROM_LOAD16_BYTE( "dg2ver1.l14",   0x000001, 0x20000, CRC(4287affc) SHA1(59dfb37296edd3b42231319a9f4df819d384db38) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "dg8.e25",   0x000000, 0x10000, CRC(d1d61da3) SHA1(4e78643f8a7d44db3ff091acb0a5da1cc836e3cb) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "dg9.l25",   0x000000, 0x08000, CRC(df864a08) SHA1(dd996070077efbbf9d784299b6563cab258e4a8e) )
	ROM_LOAD( "dg10.m25",  0x008000, 0x08000, CRC(9e470d53) SHA1(f7dc6ac39ade573480e87170a2781f0f72930580) )

	ROM_REGION( 0x200000, "sprites", 0 )   // including backgrounds (231000 28-pin mask ROMs or 27C301 OTP EPROMs)
	ROM_LOAD16_BYTE( "snk88011a.1a", 0x000000, 0x20000, CRC(e70fd906) SHA1(b9e734c074ee1c8ae73e6041d739ab30d2df7d62) )
	ROM_LOAD16_BYTE( "snk88015a.2a", 0x000001, 0x20000, CRC(7a90e957) SHA1(9650d7cdbcbbbcdd7f75a1c3c08a195aa456e169) )
	ROM_LOAD16_BYTE( "snk88012a.1b", 0x040000, 0x20000, CRC(628b1aed) SHA1(1065ef835da03f7d9776e81c225c3ecdd2affae2) )
	ROM_LOAD16_BYTE( "snk88016a.2b", 0x040001, 0x20000, CRC(e40a6c13) SHA1(7ad9dfc65f8c8b316933f0fdd3bc7a243d6eff65) )
	ROM_LOAD16_BYTE( "snk88013a.1c", 0x080000, 0x20000, CRC(19dc8868) SHA1(82aaf93fc8f4b3bf947d373d0f41cc0044207c34) ) // Also can be labeled as DG13 using a 27C301 OTP EPROM
	ROM_LOAD16_BYTE( "snk88017a.2c", 0x080001, 0x20000, CRC(c7931cc2) SHA1(908313d9b7fa4395b5fb79925d068e4f5d354b21) )
	ROM_LOAD16_BYTE( "snk88014a.1d", 0x0c0000, 0x20000, CRC(47cd498b) SHA1(7fbc007f2d817c26af02fef233f5e0681a17052a) )
	ROM_LOAD16_BYTE( "snk88018a.2d", 0x0c0001, 0x20000, CRC(eed72232) SHA1(ad614e752cf1d3eac9a04cbc90435f988e90ace7) )
	ROM_LOAD16_BYTE( "snk88019a.3a", 0x100000, 0x20000, CRC(1775b8dd) SHA1(c01154749379be6e18baa99f4d94d97942f3dd85) )
	ROM_LOAD16_BYTE( "snk88023a.4a", 0x100001, 0x20000, CRC(adb6ad68) SHA1(ed4323d2dfa3efaa496b17f4719f9566d56725e5) )
	ROM_LOAD16_BYTE( "snk88020a.3b", 0x140000, 0x20000, CRC(f8e752ec) SHA1(1e1e178303f9af84cbb15249c49a870193ef805f) )
	ROM_LOAD16_BYTE( "snk88024a.4b", 0x140001, 0x20000, CRC(dd41865a) SHA1(c86f14342beca896784b88920d9e0879af4179ab) )
	ROM_LOAD16_BYTE( "snk88021a.3c", 0x180000, 0x20000, CRC(27e9fffe) SHA1(e8058db40832b986c5addd22dd69b0308d10ec71) )
	ROM_LOAD16_BYTE( "snk88025a.4c", 0x180001, 0x20000, CRC(055759ad) SHA1(f9b12320f142075d49d447fb107af99272567d58) )
	ROM_LOAD16_BYTE( "snk88022a.3d", 0x1c0000, 0x20000, CRC(aa9c00d8) SHA1(1017ed1cc036c6084b71204a998fd05557a6e59f) )
	ROM_LOAD16_BYTE( "snk88026a.4d", 0x1c0001, 0x20000, CRC(9bc261c5) SHA1(f07fef465191d48ccc149d1a62e6382d3fc0ef9f) )

	ROM_REGION( 0x10000, "upd", 0 )
	ROM_LOAD( "dg7.d20",  0x000000, 0x10000, CRC(aba9a9d3) SHA1(5098cd3a064b8ede24797de8879a277d79e79d75) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal20l10.a6", 0x0000, 0x00cc, CRC(c3d9e729) SHA1(f05f03eecf12b4d0793124ecd3195307be04046b) )
ROM_END

ROM_START( powa )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dg1ver1.j14",   0x000000, 0x20000, CRC(8e71a8af) SHA1(72c2eb2316c2684491331e8adabcb2be084aa6a2) )
	ROM_LOAD16_BYTE( "dg2ver1.l14",   0x000001, 0x20000, CRC(4287affc) SHA1(59dfb37296edd3b42231319a9f4df819d384db38) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "dg8.e25",   0x000000, 0x10000, CRC(d1d61da3) SHA1(4e78643f8a7d44db3ff091acb0a5da1cc836e3cb) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "dg9.l25",   0x000000, 0x08000, CRC(df864a08) SHA1(dd996070077efbbf9d784299b6563cab258e4a8e) )
	ROM_LOAD( "dg10.m25",  0x008000, 0x08000, CRC(9e470d53) SHA1(f7dc6ac39ade573480e87170a2781f0f72930580) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "pow8804_w50.4", 0x000000, 0x80000, CRC(18fd04a3) SHA1(c52847fd30d63bb5cb9469060e991f7c658466d7) )
	ROM_LOAD16_BYTE( "pow8806_w52.6", 0x000001, 0x80000, CRC(09b654e9) SHA1(2e68df3c8409ba8191ebd9ef3f9b91a07fccb8a3) )
	ROM_LOAD16_BYTE( "pow8803_w49.3", 0x100000, 0x80000, CRC(f68712a3) SHA1(98c77ed331bfb16685e0b469766c87fe71d3cd31) )
	ROM_LOAD16_BYTE( "pow8805_w51.5", 0x100001, 0x80000, CRC(8595cf76) SHA1(cc8298dc526cde195bfb9246f88c299e98a38a20) )

	ROM_REGION( 0x10000, "upd", 0 )
	ROM_LOAD( "dg7.d20",  0x000000, 0x10000, CRC(aba9a9d3) SHA1(5098cd3a064b8ede24797de8879a277d79e79d75) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal20l10.a6", 0x0000, 0x00cc, CRC(c3d9e729) SHA1(f05f03eecf12b4d0793124ecd3195307be04046b) )
ROM_END

ROM_START( powj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1-2",   0x000000, 0x20000, CRC(2f17bfb0) SHA1(8be18990829eb2586c00b9e8b35e8779bc48296a) )
	ROM_LOAD16_BYTE( "2-2",   0x000001, 0x20000, CRC(baa32354) SHA1(a235b82527dc025e699ba2e8e9797dac15ea9440) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "dg8.e25",   0x000000, 0x10000, CRC(d1d61da3) SHA1(4e78643f8a7d44db3ff091acb0a5da1cc836e3cb) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "dg9.l25",   0x000000, 0x08000, CRC(df864a08) SHA1(dd996070077efbbf9d784299b6563cab258e4a8e) )
	ROM_LOAD( "dg10.m25",  0x008000, 0x08000, CRC(9e470d53) SHA1(f7dc6ac39ade573480e87170a2781f0f72930580) )

	ROM_REGION( 0x200000, "sprites", 0 )   // including backgrounds (231000 28-pin mask ROMs or 27C301 OTP EPROMs)
	ROM_LOAD16_BYTE( "snk88011a.1a", 0x000000, 0x20000, CRC(e70fd906) SHA1(b9e734c074ee1c8ae73e6041d739ab30d2df7d62) )
	ROM_LOAD16_BYTE( "snk88015a.2a", 0x000001, 0x20000, CRC(7a90e957) SHA1(9650d7cdbcbbbcdd7f75a1c3c08a195aa456e169) )
	ROM_LOAD16_BYTE( "snk88012a.1b", 0x040000, 0x20000, CRC(628b1aed) SHA1(1065ef835da03f7d9776e81c225c3ecdd2affae2) )
	ROM_LOAD16_BYTE( "snk88016a.2b", 0x040001, 0x20000, CRC(e40a6c13) SHA1(7ad9dfc65f8c8b316933f0fdd3bc7a243d6eff65) )
	ROM_LOAD16_BYTE( "snk88013a.1c", 0x080000, 0x20000, CRC(19dc8868) SHA1(82aaf93fc8f4b3bf947d373d0f41cc0044207c34) ) // Also can be labeled as DG13 using a 27C301 OTP EPROM
	ROM_LOAD16_BYTE( "snk88017a.2c", 0x080001, 0x20000, CRC(c7931cc2) SHA1(908313d9b7fa4395b5fb79925d068e4f5d354b21) )
	ROM_LOAD16_BYTE( "snk88014a.1d", 0x0c0000, 0x20000, CRC(47cd498b) SHA1(7fbc007f2d817c26af02fef233f5e0681a17052a) )
	ROM_LOAD16_BYTE( "snk88018a.2d", 0x0c0001, 0x20000, CRC(eed72232) SHA1(ad614e752cf1d3eac9a04cbc90435f988e90ace7) )
	ROM_LOAD16_BYTE( "snk88019a.3a", 0x100000, 0x20000, CRC(1775b8dd) SHA1(c01154749379be6e18baa99f4d94d97942f3dd85) )
	ROM_LOAD16_BYTE( "snk88023a.4a", 0x100001, 0x20000, CRC(adb6ad68) SHA1(ed4323d2dfa3efaa496b17f4719f9566d56725e5) )
	ROM_LOAD16_BYTE( "snk88020a.3b", 0x140000, 0x20000, CRC(f8e752ec) SHA1(1e1e178303f9af84cbb15249c49a870193ef805f) )
	ROM_LOAD16_BYTE( "snk88024a.4b", 0x140001, 0x20000, CRC(dd41865a) SHA1(c86f14342beca896784b88920d9e0879af4179ab) )
	ROM_LOAD16_BYTE( "snk88021a.3c", 0x180000, 0x20000, CRC(27e9fffe) SHA1(e8058db40832b986c5addd22dd69b0308d10ec71) )
	ROM_LOAD16_BYTE( "snk88025a.4c", 0x180001, 0x20000, CRC(055759ad) SHA1(f9b12320f142075d49d447fb107af99272567d58) )
	ROM_LOAD16_BYTE( "snk88022a.3d", 0x1c0000, 0x20000, CRC(aa9c00d8) SHA1(1017ed1cc036c6084b71204a998fd05557a6e59f) )
	ROM_LOAD16_BYTE( "snk88026a.4d", 0x1c0001, 0x20000, CRC(9bc261c5) SHA1(f07fef465191d48ccc149d1a62e6382d3fc0ef9f) )

	ROM_REGION( 0x10000, "upd", 0 )
	ROM_LOAD( "dg7.d20",  0x000000, 0x10000, CRC(aba9a9d3) SHA1(5098cd3a064b8ede24797de8879a277d79e79d75) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal20l10.a6", 0x0000, 0x00cc, CRC(c3d9e729) SHA1(f05f03eecf12b4d0793124ecd3195307be04046b) )
ROM_END

ROM_START( powbl ) // main PCB + sprite ROM board
	ROM_REGION( 0x40000, "maincpu", 0 ) // identical to pow, but smaller ROMs
	ROM_LOAD16_BYTE( "pow36b.bin",   0x000000, 0x10000, CRC(a4de338d) SHA1(18ac22e5e99018cc794350faed4b75006737d2bc) )
	ROM_LOAD16_BYTE( "pow35b.bin",   0x000001, 0x10000, CRC(ba405691) SHA1(a21eab60efbe8c56524518c389ab7d545c41af55) )
	ROM_LOAD16_BYTE( "pow36a.bin",   0x020000, 0x10000, CRC(fa53460c) SHA1(f4a31e27c45ac2727cbf3e855ccc787392e17866) )
	ROM_LOAD16_BYTE( "pow35a.bin",   0x020001, 0x10000, CRC(a67c6495) SHA1(14066d314bbe42ab12d7287724e4071869f61157) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // very similar to pow, updated to support MSM5205 instead of UPD7759
	ROM_LOAD( "pow37.bin",   0x000000, 0x10000, CRC(0d22d25f) SHA1(0dca3e1bebe91da84b8537c0ff184241797ac8da) )

	ROM_REGION( 0x010000, "tiles", 0 )   // characters, identical to pow
	ROM_LOAD( "pow34.bin",  0x000000, 0x08000, CRC(df864a08) SHA1(dd996070077efbbf9d784299b6563cab258e4a8e) )
	ROM_LOAD( "pow33.bin",  0x008000, 0x08000, CRC(9e470d53) SHA1(f7dc6ac39ade573480e87170a2781f0f72930580) )

	ROM_REGION( 0x200000, "sprites", 0 )   // sprites, different format
	ROM_LOAD16_BYTE( "pow30.bin", 0x000000, 0x10000, CRC(40b43c09) SHA1(8b12b02284032e01ff8b9410948e1da8f88f124b) )
	ROM_LOAD16_BYTE( "pow24.bin", 0x000001, 0x10000, CRC(efbfdf59) SHA1(c02d18fb582bd4bdbf2b3ea6f2bd925e7d70a3af) )
	ROM_LOAD16_BYTE( "pow29.bin", 0x020000, 0x10000, CRC(61909aa4) SHA1(6981bf33f7261a0d5dc412e4f4bb72b523f4af37) )
	ROM_LOAD16_BYTE( "pow23.bin", 0x020001, 0x10000, CRC(ea9aca79) SHA1(693916685b23b10b966e6fb9e0f2846994b672c9) )
	ROM_LOAD16_BYTE( "pow28.bin", 0x040000, 0x10000, CRC(81da0f09) SHA1(0c788abce58b581790051741aafee72ddcdabd02) )
	ROM_LOAD16_BYTE( "pow22.bin", 0x040001, 0x10000, CRC(361f178b) SHA1(6a680e2b37e1ae825d548e90720999bd0188b151) )
	ROM_LOAD16_BYTE( "pow27.bin", 0x060000, 0x10000, CRC(f7db186c) SHA1(cbd17ee46563dcb7ad1e2f8b2c7481102ecb91f7) )
	ROM_LOAD16_BYTE( "pow21.bin", 0x060001, 0x10000, CRC(606fab0a) SHA1(7b60df4ee8096da6ad14d0aa197b20b7293f04a9) )
	ROM_LOAD16_BYTE( "pow26.bin", 0x080000, 0x10000, CRC(a028dfda) SHA1(7a84a44fc062aa946f1fddbefbde0dae0d0212a3) )
	ROM_LOAD16_BYTE( "pow20.bin", 0x080001, 0x10000, CRC(06be682c) SHA1(24abf7f3b525124a2b38aae6e1c313d1dc0ad1ff) )
	ROM_LOAD16_BYTE( "pow31.bin", 0x0a0000, 0x10000, CRC(f25ea217) SHA1(bde91824539ea037acdd3ccdbb3387f0364888ec) )
	ROM_LOAD16_BYTE( "pow19.bin", 0x0a0001, 0x10000, CRC(4e7b2b47) SHA1(344af976c4c82495a820437cbbdacb1027fa14c0) )
	ROM_LOAD16_BYTE( "pow25.bin", 0x0c0000, 0x10000, CRC(a4cf97d5) SHA1(8ff0a7fe61f3ee3167601e68a54960b3e712c4b5) )
	ROM_LOAD16_BYTE( "pow18.bin", 0x0c0001, 0x10000, CRC(b70c603d) SHA1(6434eeddf7009887889914edc63a348053a05bc0) )
	ROM_LOAD16_BYTE( "pow32.bin", 0x0e0000, 0x10000, CRC(9ffd27ea) SHA1(d693baa289811c8b612fb45b1186a361d73d223b) )
	ROM_LOAD16_BYTE( "pow17.bin", 0x0e0001, 0x10000, CRC(c91291ce) SHA1(1276caa972ba625f3aacbd11e5b37d66406fbe97) )
	ROM_LOAD16_BYTE( "pow15.bin", 0x100000, 0x10000, CRC(ffe660b2) SHA1(06b93e0f0b7dd83046c428459a47239454e9d7f2) )
	ROM_LOAD16_BYTE( "pow1.bin",  0x100001, 0x10000, CRC(4fc31abe) SHA1(2bdc3de7301ab512dbf3c3a9e4e249d258ea287f) )
	ROM_LOAD16_BYTE( "pow14.bin", 0x120000, 0x10000, CRC(07a08711) SHA1(d43707a6e8eb6925e1f46a495590c31a1683d073) )
	ROM_LOAD16_BYTE( "pow2.bin",  0x120001, 0x10000, CRC(0be02c0c) SHA1(7dcbc8c47982d78c887de4e8112cb17678f07213) )
	ROM_LOAD16_BYTE( "pow13.bin", 0x140000, 0x10000, CRC(37147ef2) SHA1(2970bfbdac856a02316ebef98c4da8fe451efc7b) )
	ROM_LOAD16_BYTE( "pow3.bin",  0x140001, 0x10000, CRC(fe4e3f95) SHA1(efe2665405dbd866de96a09bced43ff945d09ede) )
	ROM_LOAD16_BYTE( "pow12.bin", 0x160000, 0x10000, CRC(57085ebc) SHA1(ee1c4140bd24aa8721fcdba5545001ffbfb5df44) )
	ROM_LOAD16_BYTE( "pow4.bin",  0x160001, 0x10000, CRC(325ba653) SHA1(55e6c9d8bd61a02ef8334e9637e4be02e22750bd) )
	ROM_LOAD16_BYTE( "pow11.bin", 0x180000, 0x10000, CRC(2c90e2c2) SHA1(cc1d555076e780cacfeb8aa0e784da08cce7e23a) )
	ROM_LOAD16_BYTE( "pow5.bin",  0x180001, 0x10000, CRC(36d691e2) SHA1(b4c7fd340649380bb9736e325c31775eb515a642) )
	ROM_LOAD16_BYTE( "pow10.bin", 0x1a0000, 0x10000, CRC(6ac5e036) SHA1(05ec7e82080b8ea4426b18ccf1e79fc2f2f81e8b) )
	ROM_LOAD16_BYTE( "pow6.bin",  0x1a0001, 0x10000, CRC(e6d8123a) SHA1(23f21d0b857de6c099de5c4d139bc32475cb8b88) )
	ROM_LOAD16_BYTE( "pow9.bin",  0x1c0000, 0x10000, CRC(5b1a1c99) SHA1(d4748cc02021bd189e635a8578ce28201000ee4f) )
	ROM_LOAD16_BYTE( "pow7.bin",  0x1c0001, 0x10000, CRC(093fe9c6) SHA1(36090da050f66fea1d9edaf5673a156a3f43de77) )
	ROM_LOAD16_BYTE( "pow16.bin", 0x1e0000, 0x10000, CRC(bbc4d174) SHA1(73bbee1ae76a76a5c057f8ade05399c411640aaf) )
	ROM_LOAD16_BYTE( "pow8.bin",  0x1e0001, 0x10000, CRC(7f7c703e) SHA1(86d98d2028fef28ef629a417a1e06b7353766545) )

	ROM_REGION( 0x10000, "msm", 0 ) // unique
	ROM_LOAD( "pow38.bin",  0x000000, 0x10000, CRC(72f35d38) SHA1(072d2af2a3ffa4b46be471659fc4d9bf6e02b683) )
ROM_END

ROM_START( streetsm )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s2-1ver2.14h", 0x00000, 0x20000, CRC(655f4773) SHA1(5374a6cf0b895c5ff839b0f52402df4cc53241cf) )
	ROM_LOAD16_BYTE( "s2-2ver2.14k", 0x00001, 0x20000, CRC(efae4823) SHA1(f3be25b76cf13feeaaaf0e9640c30a6a7371f108) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "s2-5.16c",    0x000000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "s2-9.25l",    0x000000, 0x08000, CRC(09b6ac67) SHA1(0b1ef51d9cd755eacc25b33360811cc86c32c0b7) )
	ROM_LOAD( "s2-10.25m",   0x008000, 0x08000, CRC(89e4ee6f) SHA1(21797286836ad71d2497e3e6d4df1fbe545562ab) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "stsmart.900", 0x000000, 0x80000, CRC(a8279a7e) SHA1(244bdacb29b00f71da93ed8ddddbcffcce110be8) )
	ROM_LOAD( "stsmart.902", 0x080000, 0x80000, CRC(2f021aa1) SHA1(699d0b5ac79e34e4fc4cef70eb448f21f1c3e9e2) )
	ROM_LOAD( "stsmart.904", 0x100000, 0x80000, CRC(167346f7) SHA1(fb4ea412622245db49ec15449ee4fa0d90922f06) )
	// 180000-1fffff empty
	ROM_LOAD( "stsmart.901", 0x200000, 0x80000, CRC(c305af12) SHA1(18b5d448fe9608efcd2e5bb8faa24808d1489ec8) )
	ROM_LOAD( "stsmart.903", 0x280000, 0x80000, CRC(73c16d35) SHA1(40cf7a58926c649f89b08917afb35b08918d1a0f) )
	ROM_LOAD( "stsmart.905", 0x300000, 0x80000, CRC(a5beb4e2) SHA1(c26b7eee2ca32bd73fb7a09c6ef52c2ae1c7fc1c) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "s2-6.18d",    0x000000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pl20l10a.1n", 0x0000, 0x00cc, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( streetsm1 )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "s2-1ver1.9c",  0x00000, 0x20000, CRC(b59354c5) SHA1(086c87541d422f90bdaad8d63b14d0d520c12564) )
	ROM_LOAD16_BYTE( "s2-2ver1.10c", 0x00001, 0x20000, CRC(e448b68b) SHA1(08d674ab3d9bd3d3b1d50967a56fa6a002ce0b8d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "s2-5.16c",    0x000000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "s2-7.15l",    0x000000, 0x08000, CRC(22bedfe5) SHA1(64efb2281c32afe5a06f35cce939e6a53226c6ed) )
	ROM_LOAD( "s2-8.15m",    0x008000, 0x08000, CRC(6a1c70ab) SHA1(019538ddcb713d0810b26b6aa65f6e4596931621) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "stsmart.900", 0x000000, 0x80000, CRC(a8279a7e) SHA1(244bdacb29b00f71da93ed8ddddbcffcce110be8) )
	ROM_LOAD( "stsmart.902", 0x080000, 0x80000, CRC(2f021aa1) SHA1(699d0b5ac79e34e4fc4cef70eb448f21f1c3e9e2) )
	ROM_LOAD( "stsmart.904", 0x100000, 0x80000, CRC(167346f7) SHA1(fb4ea412622245db49ec15449ee4fa0d90922f06) )
	// 180000-1fffff empty
	ROM_LOAD( "stsmart.901", 0x200000, 0x80000, CRC(c305af12) SHA1(18b5d448fe9608efcd2e5bb8faa24808d1489ec8) )
	ROM_LOAD( "stsmart.903", 0x280000, 0x80000, CRC(73c16d35) SHA1(40cf7a58926c649f89b08917afb35b08918d1a0f) )
	ROM_LOAD( "stsmart.905", 0x300000, 0x80000, CRC(a5beb4e2) SHA1(c26b7eee2ca32bd73fb7a09c6ef52c2ae1c7fc1c) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "s2-6.18d",    0x000000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION( 0x0800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "a6.1n",    0x000, 0x2dd, CRC(5869332c) SHA1(b330243441b172235a2b0c79e8f396fef3012785) ) // PAL20L10
	ROM_LOAD( "p-a7.11d", 0x300, 0x2dd, CRC(e46f6fcd) SHA1(82ed194e9680ac6fd9d2ec83ffcc3e3fdc60e900) ) // PAL22L10
	ROM_LOAD( "p-a8.20m", 0x600, 0x117, CRC(6cd7beac) SHA1(ba1ddc3d39e18df9be7001ddf55a4aeb806eccec) ) // PAL16L8
ROM_END

ROM_START( streetsmw )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "s-smart1.bin", 0x00000, 0x20000, CRC(a1f5ceab) SHA1(74f5a4288618fbce6ed3dc75b6ccfa695396193c) )
	ROM_LOAD16_BYTE( "s-smart2.bin", 0x00001, 0x20000, CRC(263f615d) SHA1(4576f9d2abb31ecf747a5075716579e75613d57c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "s2-5.16c",    0x000000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "s2-7.15l",    0x000000, 0x08000, CRC(22bedfe5) SHA1(64efb2281c32afe5a06f35cce939e6a53226c6ed) )
	ROM_LOAD( "s2-8.15m",    0x008000, 0x08000, CRC(6a1c70ab) SHA1(019538ddcb713d0810b26b6aa65f6e4596931621) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "stsmart.900", 0x000000, 0x80000, CRC(a8279a7e) SHA1(244bdacb29b00f71da93ed8ddddbcffcce110be8) )
	ROM_LOAD( "stsmart.902", 0x080000, 0x80000, CRC(2f021aa1) SHA1(699d0b5ac79e34e4fc4cef70eb448f21f1c3e9e2) )
	ROM_LOAD( "stsmart.904", 0x100000, 0x80000, CRC(167346f7) SHA1(fb4ea412622245db49ec15449ee4fa0d90922f06) )
	// 180000-1fffff empty
	ROM_LOAD( "stsmart.901", 0x200000, 0x80000, CRC(c305af12) SHA1(18b5d448fe9608efcd2e5bb8faa24808d1489ec8) )
	ROM_LOAD( "stsmart.903", 0x280000, 0x80000, CRC(73c16d35) SHA1(40cf7a58926c649f89b08917afb35b08918d1a0f) )
	ROM_LOAD( "stsmart.905", 0x300000, 0x80000, CRC(a5beb4e2) SHA1(c26b7eee2ca32bd73fb7a09c6ef52c2ae1c7fc1c) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "s2-6.18d",    0x000000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION( 0x0800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "a6.1n",    0x000, 0x2dd, CRC(5869332c) SHA1(b330243441b172235a2b0c79e8f396fef3012785) ) // PAL20L10
	ROM_LOAD( "p-a7.11d", 0x300, 0x2dd, CRC(e46f6fcd) SHA1(82ed194e9680ac6fd9d2ec83ffcc3e3fdc60e900) ) // PAL22L10
	ROM_LOAD( "p-a8.20m", 0x600, 0x117, CRC(6cd7beac) SHA1(ba1ddc3d39e18df9be7001ddf55a4aeb806eccec) ) // PAL16L8
ROM_END

ROM_START( streetsmwbl )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 ) // same as the original
	ROM_LOAD16_BYTE( "1p.1", 0x00000, 0x20000, CRC(a1f5ceab) SHA1(74f5a4288618fbce6ed3dc75b6ccfa695396193c) )
	ROM_LOAD16_BYTE( "2p.2", 0x00001, 0x20000, CRC(263f615d) SHA1(4576f9d2abb31ecf747a5075716579e75613d57c) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // same as the original
	ROM_LOAD( "3s.3", 0x000000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x010000, "tiles", 0 ) // same as the original
	ROM_LOAD( "5c.5", 0x000000, 0x08000, CRC(22bedfe5) SHA1(64efb2281c32afe5a06f35cce939e6a53226c6ed) )
	ROM_LOAD( "6c.6", 0x008000, 0x08000, CRC(6a1c70ab) SHA1(019538ddcb713d0810b26b6aa65f6e4596931621) )

	ROM_REGION( 0x400000, "sprites", 0 )   // different format
	ROM_LOAD16_BYTE( "24l.24", 0x000000, 0x10000, CRC(01cd7ae0) SHA1(ca2e97716f79dd650fe4de77c3313236aad06015) )
	ROM_LOAD16_BYTE( "15l.15", 0x000001, 0x10000, CRC(9c81fcbe) SHA1(5a30064c55eb1f5ac5968924ce924a6be7a0b94c) )
	ROM_LOAD16_BYTE( "23l.23", 0x020000, 0x10000, CRC(441cd7a2) SHA1(72680a4d141d3306a8973c1dc79b63a66ecda92f) )
	ROM_LOAD16_BYTE( "14l.14", 0x020001, 0x10000, CRC(4741c5ad) SHA1(e9610f8183da25e1a96f0b7928252daa8d739e9e) )
	ROM_LOAD16_BYTE( "22l.22", 0x040000, 0x10000, CRC(e1da555c) SHA1(77d804f34facb600ff670cc1e75a30dfcb8a5b87) )
	ROM_LOAD16_BYTE( "13l.13", 0x040001, 0x10000, CRC(00861423) SHA1(ccee8a06295e9bb2866ac62336371cf14d3629af) )
	ROM_LOAD16_BYTE( "21l.21", 0x060000, 0x10000, CRC(f36b4997) SHA1(14bedeb26f0b57f1170936e1691586b06ebe67ac) )
	ROM_LOAD16_BYTE( "12l.12", 0x060001, 0x10000, CRC(ccaed9aa) SHA1(31917c666fc79e4a67e59105f3e1119a4f3754e4) )
	ROM_LOAD16_BYTE( "20l.20", 0x080000, 0x10000, CRC(12bc66ee) SHA1(589cd88fa48e2c41ebbadcfcc3b205ea006f7055) )
	ROM_LOAD16_BYTE( "11l.11", 0x080001, 0x10000, CRC(49e545a2) SHA1(19e3ed42ecac88649b1205bf85feee9a44e3f205) )
	ROM_LOAD16_BYTE( "19l.19", 0x0a0000, 0x10000, CRC(8f2f601d) SHA1(9b5a9290e9524956f83644cce9d2daa1a05a1d32) )
	ROM_LOAD16_BYTE( "10l.10", 0x0a0001, 0x10000, CRC(fe486cbf) SHA1(343cf99bb90039d0e047a66d526e1a351d7a27e5) )
	ROM_LOAD16_BYTE( "18l.18", 0x0c0000, 0x10000, CRC(6ac354cb) SHA1(fd5aa54aa81f648671779d2bb3393cf31c541e57) )
	ROM_LOAD16_BYTE( "9l.9",   0x0c0001, 0x10000, CRC(cf6f140d) SHA1(8bc80bb7382f6f0cba302cf7637911d24e48a09f) )
	ROM_LOAD16_BYTE( "17l.17", 0x0e0000, 0x10000, CRC(34aa51b1) SHA1(371a600136920b76dfd11ce798c8511fa9ae3411) )
	ROM_LOAD16_BYTE( "8l.8",   0x0e0001, 0x10000, CRC(28f1bdc4) SHA1(a2587c8a08eae4e41795713dfded7d007f3c01cc) )
	ROM_LOAD16_BYTE( "6l.6",   0x100000, 0x10000, CRC(56ebc520) SHA1(5689499947cc8db78ffca63081be8653c055cf00) )
	ROM_LOAD16_BYTE( "4l.4",   0x100001, 0x10000, CRC(54dc8dae) SHA1(aea0a516a27064488ec49ec040eb45c3fa264de6) )
	ROM_LOAD16_BYTE( "5l.5",   0x120000, 0x10000, CRC(9943ce65) SHA1(b13b5e9b332f400268f8d3d1e9e3c2a9a13b0cce) )
	ROM_LOAD16_BYTE( "3l.3",   0x120001, 0x10000, CRC(596e388e) SHA1(e3821466b9f2cdfc665b81945c1641c36afde1f2) )
	ROM_LOAD16_BYTE( "16l.16", 0x140000, 0x10000, CRC(7599dbb5) SHA1(d7982d8c7232defc67a47ddc9a6cfac4af6020ac) )
	ROM_LOAD16_BYTE( "24r.24", 0x140001, 0x10000, CRC(58099ef5) SHA1(dc1c6684836ff65623527b36bd3dee2d1843646b) )
	ROM_LOAD16_BYTE( "7l.7",   0x160000, 0x10000, CRC(b36db591) SHA1(ddf4595f276abd9456d3c72d0925121adea2ebc6) )
	ROM_LOAD16_BYTE( "15r.15", 0x160001, 0x10000, CRC(2e9b47c5) SHA1(0e5a3bc58fe2be9f1bf38cbbe524350e7a4a2cdd) )
	// 180000-1fffff empty
	ROM_LOAD16_BYTE( "32r.32", 0x200000, 0x10000, CRC(376f0976) SHA1(9dc3868848eb4bb25c9e81c6d710de2f7aa60e82) )
	ROM_LOAD16_BYTE( "23r.23", 0x200001, 0x10000, CRC(f5edc139) SHA1(93f6e9746ca07ef0dd16f58c37ec1789af5289be) )
	ROM_LOAD16_BYTE( "31r.31", 0x220000, 0x10000, CRC(72901ea3) SHA1(50aa4ef1c7c77c0dd8083b57fde98c17e2dbe9b4) )
	ROM_LOAD16_BYTE( "22r.22", 0x220001, 0x10000, CRC(f1cda176) SHA1(071d67c06dd219467a1031639e73593fb8d1e9ca) )
	ROM_LOAD16_BYTE( "30r.30", 0x240000, 0x10000, CRC(7084e2e2) SHA1(e5c79157402cd1b20ab902c0c79a8c8bfb51928d) )
	ROM_LOAD16_BYTE( "21r.21", 0x240001, 0x10000, CRC(19a6ad20) SHA1(002c485c9794fb809ac717cfbf08e850d59a719c) )
	ROM_LOAD16_BYTE( "29r.29", 0x260000, 0x10000, CRC(9a498dab) SHA1(a12142c154ed8e316aa1667253f58a0085b3b8e0) )
	ROM_LOAD16_BYTE( "20r.20", 0x260001, 0x10000, CRC(ea28d8e5) SHA1(2c04d8dab06f5fcddf6460863163dac453db8882) )
	ROM_LOAD16_BYTE( "28r.28", 0x280000, 0x10000, CRC(3ca47063) SHA1(2a0dc7b40e753a7a26e8c75451908d8c3fb2492b) )
	ROM_LOAD16_BYTE( "19r.19", 0x280001, 0x10000, CRC(c108820f) SHA1(b82267962dd97efdd5381cce458d2cc773f4df93) )
	ROM_LOAD16_BYTE( "27r.27", 0x2a0000, 0x10000, CRC(6986db59) SHA1(c941aa18bb2912a0e77e5065e70214d04bfff05f) )
	ROM_LOAD16_BYTE( "18r.18", 0x2a0001, 0x10000, CRC(a078895c) SHA1(eafcbc12b78c30a89213524d1074ea05be5f7beb) )
	ROM_LOAD16_BYTE( "26r.26", 0x2c0000, 0x10000, CRC(886a57ee) SHA1(732b131ad83158a53ad9fc522ce64460cc9f62df) )
	ROM_LOAD16_BYTE( "17r.17", 0x2c0001, 0x10000, CRC(af1e58e4) SHA1(ba36504ab728712a93af0ee86e8478b9cdaa413a) )
	ROM_LOAD16_BYTE( "25r.25", 0x2e0000, 0x10000, CRC(9fe6e970) SHA1(22306d58ffc1448c72715fa19cc4055c7b144f05) )
	ROM_LOAD16_BYTE( "16r.16", 0x2e0001, 0x10000, CRC(466b39b7) SHA1(e64f33cbcefac70f79aca76c85fd0fd2b20cabb1) )
	ROM_LOAD16_BYTE( "14r.14", 0x300000, 0x10000, CRC(e9697bd1) SHA1(ba0e98ea368540d1ccb736d48a112646de32fb3e) )
	ROM_LOAD16_BYTE( "12r.12", 0x300001, 0x10000, CRC(8307f243) SHA1(1631031986b6e198ed3f08a4d335c04af28d1b3f) )
	ROM_LOAD16_BYTE( "13r.13", 0x320000, 0x10000, CRC(3255af03) SHA1(78d121fe2e59824c7cd0941d14515f5da13a5d9d) )
	ROM_LOAD16_BYTE( "11r.11", 0x320001, 0x10000, CRC(5bad15c1) SHA1(58815fb6760ca177aa47d2ee2e58c5ec3d334c39) )
	ROM_LOAD16_BYTE( "8r.8",   0x340000, 0x10000, CRC(9c52f1ea) SHA1(1193cc473ced14f885cbb9aa818f461a9a3101c0) )
	ROM_LOAD16_BYTE( "9r.9",   0x340001, 0x10000, CRC(380dcee5) SHA1(d9f9c41826cc2bf49db0b9a06f71bdcca3292448) )
	ROM_LOAD16_BYTE( "7r.7",   0x360000, 0x10000, CRC(0c474722) SHA1(a52356bcbd86ccf49df1404404ea3f4ccee17da0) )
	ROM_LOAD16_BYTE( "10r.10", 0x360001, 0x10000, CRC(9d617689) SHA1(226cc4dc142d1a7ef1a937cbf8776de1ca4aab45) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) // same as the original
	ROM_LOAD( "4s.4", 0x000000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "pal20l8acns.pal1",  0x000, 0x144, CRC(f31415c9) SHA1(2e38993858d724089cf3237de2ab98adab6db547) )
	ROM_LOAD( "pal20l10acns.pal2", 0x200, 0x0cc, CRC(1cadf26d) SHA1(348a9e4727df0a15247c7b9c5cd5ee935edd9752) )
	ROM_LOAD( "pal20l10acns.pal3", 0x400, 0x0cc, CRC(c3d9e729) SHA1(f05f03eecf12b4d0793124ecd3195307be04046b) )
	ROM_LOAD( "pal16p8acn.pal4",   0x600, 0x104, CRC(e258b8d6) SHA1(9d000aa9a09b402208a5c2d98789cc62e23a2eb2) )
ROM_END

ROM_START( streetsmj )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "s2v1j_01.bin", 0x00000, 0x20000, CRC(f031413c) SHA1(5d7dfeac03f786736914f047c28a7a0488175176) )
	ROM_LOAD16_BYTE( "s2v1j_02.bin", 0x00001, 0x20000, CRC(e403a40b) SHA1(e740848d716586737eff6e3c201fb3e3da048a09) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "s2-5.16c",    0x000000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "s2-7.15l",    0x000000, 0x08000, CRC(22bedfe5) SHA1(64efb2281c32afe5a06f35cce939e6a53226c6ed) )
	ROM_LOAD( "s2-8.15m",    0x008000, 0x08000, CRC(6a1c70ab) SHA1(019538ddcb713d0810b26b6aa65f6e4596931621) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "stsmart.900", 0x000000, 0x80000, CRC(a8279a7e) SHA1(244bdacb29b00f71da93ed8ddddbcffcce110be8) )
	ROM_LOAD( "stsmart.902", 0x080000, 0x80000, CRC(2f021aa1) SHA1(699d0b5ac79e34e4fc4cef70eb448f21f1c3e9e2) )
	ROM_LOAD( "stsmart.904", 0x100000, 0x80000, CRC(167346f7) SHA1(fb4ea412622245db49ec15449ee4fa0d90922f06) )
	// 180000-1fffff empty
	ROM_LOAD( "stsmart.901", 0x200000, 0x80000, CRC(c305af12) SHA1(18b5d448fe9608efcd2e5bb8faa24808d1489ec8) )
	ROM_LOAD( "stsmart.903", 0x280000, 0x80000, CRC(73c16d35) SHA1(40cf7a58926c649f89b08917afb35b08918d1a0f) )
	ROM_LOAD( "stsmart.905", 0x300000, 0x80000, CRC(a5beb4e2) SHA1(c26b7eee2ca32bd73fb7a09c6ef52c2ae1c7fc1c) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "s2-6.18d",    0x000000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION( 0x0800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "a6.1n",    0x000, 0x2dd, CRC(5869332c) SHA1(b330243441b172235a2b0c79e8f396fef3012785) ) // PAL20L10
	ROM_LOAD( "p-a7.11d", 0x300, 0x2dd, CRC(e46f6fcd) SHA1(82ed194e9680ac6fd9d2ec83ffcc3e3fdc60e900) ) // PAL22L10
	ROM_LOAD( "p-a8.20m", 0x600, 0x117, CRC(6cd7beac) SHA1(ba1ddc3d39e18df9be7001ddf55a4aeb806eccec) ) // PAL16L8
ROM_END

ROM_START( ikari3 )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ik3-2-ver1.c10", 0x000000, 0x20000, CRC(1bae8023) SHA1(42d590a545cbabc596f2e0d9a3d56b1bc270ec9a) ) // 8-way joystick
	ROM_LOAD16_BYTE( "ik3-3-ver1.c9",  0x000001, 0x20000, CRC(10e38b66) SHA1(28cc82d868f59cd6dde1c4e4c890627012e5e978) ) // 8-way joystick
	ROM_LOAD16_BYTE( "ik3-1.c8",       0x040000, 0x10000, CRC(47e4d256) SHA1(7c6921cf2f1b8c3dae867eb1fc14e3da218cc1e0) )
	ROM_LOAD16_BYTE( "ik3-4.c12",      0x040001, 0x10000, CRC(a43af6b5) SHA1(1ad3acadbadd21642932028ecd7c282f7fd02856) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ik3-5.16d",  0x000000, 0x10000, CRC(ce6706fc) SHA1(95505b90a9524abf0c8c1ec6b2c40d8f25cb1d92) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "ik3-7.16l",  0x000000, 0x08000, CRC(0b4804df) SHA1(66d16d245bfc404366164823faaea0bfec83e487) )
	ROM_LOAD( "ik3-8.16m",  0x008000, 0x08000, CRC(10ab4e50) SHA1(dee8416eb720848cf6471e568dadc1cfc6c2e67f) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ik3-23.bin", 0x000000, 0x20000, CRC(d0fd5c77) SHA1(c171c64ad252f0ba5b0bbdf37808102fca37b488) )
	ROM_LOAD16_BYTE( "ik3-13.bin", 0x000001, 0x20000, CRC(9a56bd32) SHA1(9301b48f970b71a909fb44514b2e93c3f1516b38) )
	ROM_LOAD16_BYTE( "ik3-22.bin", 0x040000, 0x20000, CRC(4878d883) SHA1(8cdb541bad00e707fb65399d637b7cc9288ada77) )
	ROM_LOAD16_BYTE( "ik3-12.bin", 0x040001, 0x20000, CRC(0ce6a10a) SHA1(13a231aa0002b2c5a0d9404ba05a879e212d638e) )
	ROM_LOAD16_BYTE( "ik3-21.bin", 0x080000, 0x20000, CRC(50d0fbf0) SHA1(9ff5fbea8d35d0f9a38ddd7eb093edcd91d9f874) )
	ROM_LOAD16_BYTE( "ik3-11.bin", 0x080001, 0x20000, CRC(e4e2be43) SHA1(959d2799708ddae909b017c0696694c46a52697e) )
	ROM_LOAD16_BYTE( "ik3-20.bin", 0x0c0000, 0x20000, CRC(9a851efc) SHA1(bc7be338ee4da7fbfe6fe44a9c7889817416bc44) )
	ROM_LOAD16_BYTE( "ik3-10.bin", 0x0c0001, 0x20000, CRC(ac222372) SHA1(8a17e37699d691b962a6d0256a18550cc73ddfef) )
	ROM_LOAD16_BYTE( "ik3-19.bin", 0x100000, 0x20000, CRC(4ebdba89) SHA1(f3ecfef4c9d2aba58dc3e6aa3cf5813d68686909) )
	ROM_LOAD16_BYTE( "ik3-9.bin",  0x100001, 0x20000, CRC(c33971c2) SHA1(91f3eb301803f5a7027da1ff7dd2a28bc97e5125) )
	// 140000-1fffff empty
	ROM_LOAD16_BYTE( "ik3-14.bin", 0x200000, 0x20000, CRC(453bea77) SHA1(f8f8d0c048fcf32ad99e1de622d9ab635bb86eae) )
	ROM_LOAD16_BYTE( "ik3-24.bin", 0x200001, 0x20000, CRC(e9b26d68) SHA1(067d582d33157ed4b7980bd87f2f260ab74c347b) )
	ROM_LOAD16_BYTE( "ik3-15.bin", 0x240000, 0x20000, CRC(781a81fc) SHA1(e08a6cf9c632d1002176afe618605bc06168e8aa) )
	ROM_LOAD16_BYTE( "ik3-25.bin", 0x240001, 0x20000, CRC(073b03f1) SHA1(b8053139799fa06c7324cee928154c89d4425ab1) )
	ROM_LOAD16_BYTE( "ik3-16.bin", 0x280000, 0x20000, CRC(80ba400b) SHA1(2cc3e53c45f239516a60c461ad9cfa5955164262) )
	ROM_LOAD16_BYTE( "ik3-26.bin", 0x280001, 0x20000, CRC(9c613561) SHA1(fc7c9a642b18faa94e6a2ba53f35a4d756a25da3) )
	ROM_LOAD16_BYTE( "ik3-17.bin", 0x2c0000, 0x20000, CRC(0cc3ce4a) SHA1(7b34435d0bbb089055a183b821ab255170db6bec) )
	ROM_LOAD16_BYTE( "ik3-27.bin", 0x2c0001, 0x20000, CRC(16dd227e) SHA1(db3b1718dea65bc9a1a736aa62aa2be389313baf) )
	ROM_LOAD16_BYTE( "ik3-18.bin", 0x300000, 0x20000, CRC(ba106245) SHA1(ac609ec3046c21fe6058f91dd4528c5c6448dc15) )
	ROM_LOAD16_BYTE( "ik3-28.bin", 0x300001, 0x20000, CRC(711715ae) SHA1(90978c86884ca3d23c138d95b654e2fb3afc6f9a) )
	// 340000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "ik3-6.18e",  0x000000, 0x20000, CRC(59d256a4) SHA1(1e7b33329f761c695bc9a817bbc0c5e13386d073) )
ROM_END

ROM_START( ikari3w ) // Initial boot shows Ikari III The Rescue, then the title changes to the Japanese title - No demo play - proto or test set??
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ik_2.c10",  0x000000, 0x20000, CRC(d0b690d3) SHA1(6c31b27e6b9f1438e8ddbefe41fa8ded22cdb51c) ) // Rotary joystick - hand written label
	ROM_LOAD16_BYTE( "ik_3.c9",   0x000001, 0x20000, CRC(11a9e664) SHA1(bf2d8a5f3f2aeff99a45d26279c88ebf04b7f79b) ) // Rotary joystick - hand written label
	ROM_LOAD16_BYTE( "ik3-1.c8",  0x040000, 0x10000, CRC(47e4d256) SHA1(7c6921cf2f1b8c3dae867eb1fc14e3da218cc1e0) )
	ROM_LOAD16_BYTE( "ik3-4.c12", 0x040001, 0x10000, CRC(a43af6b5) SHA1(1ad3acadbadd21642932028ecd7c282f7fd02856) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ik3-5.16d",  0x000000, 0x10000, CRC(ce6706fc) SHA1(95505b90a9524abf0c8c1ec6b2c40d8f25cb1d92) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "ik3-7.16l",  0x000000, 0x08000, CRC(0b4804df) SHA1(66d16d245bfc404366164823faaea0bfec83e487) )
	ROM_LOAD( "ik3-8.16m",  0x008000, 0x08000, CRC(10ab4e50) SHA1(dee8416eb720848cf6471e568dadc1cfc6c2e67f) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ikari-880d_t53.d2", 0x000000, 0x80000, CRC(5855d95e) SHA1(808ed58fb927fb631ff54e8ae7a634d748739ffc) ) // 4M mask ROM located on the A7007-SUB4M daughter board
	ROM_LOAD16_BYTE( "ikari-880c_t54.c2", 0x000001, 0x80000, CRC(6d728362) SHA1(e94bf03bd5a21c8826930acb419d44c2076908cc) ) // 4M mask ROM located on the A7007-SUB4M daughter board
	ROM_LOAD16_BYTE( "ik_12.d1",          0x100000, 0x20000, CRC(4ebdba89) SHA1(f3ecfef4c9d2aba58dc3e6aa3cf5813d68686909) ) // located on the A7007-SUB4M daughter board - hand written label
	ROM_LOAD16_BYTE( "ik_11.c1",          0x100001, 0x20000, CRC(c33971c2) SHA1(91f3eb301803f5a7027da1ff7dd2a28bc97e5125) ) // located on the A7007-SUB4M daughter board - hand written label
	// 140000-1fffff empty
	ROM_LOAD16_BYTE( "ikari-880b_t51.b2", 0x200000, 0x80000, CRC(e25380e6) SHA1(10a0c7891ce64a538a92fe6bd40cb955305c090e) ) // 4M mask ROM located on the A7007-SUB4M daughter board
	ROM_LOAD16_BYTE( "ikari-880a_t52.a2", 0x200001, 0x80000, CRC(87607772) SHA1(cda4ab485fb9c930b564f98e8a776da111c66fe4) ) // 4M mask ROM located on the A7007-SUB4M daughter board
	ROM_LOAD16_BYTE( "ik_10.b1",          0x300000, 0x20000, CRC(ba106245) SHA1(ac609ec3046c21fe6058f91dd4528c5c6448dc15) ) // located on the A7007-SUB4M daughter board - hand written label
	ROM_LOAD16_BYTE( "ik_9.a1",           0x300001, 0x20000, CRC(711715ae) SHA1(90978c86884ca3d23c138d95b654e2fb3afc6f9a) ) // located on the A7007-SUB4M daughter board - hand written label
	// 340000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "ik3-6.18e",  0x000000, 0x20000, CRC(59d256a4) SHA1(1e7b33329f761c695bc9a817bbc0c5e13386d073) )

	// stuff below isn't used but loaded because it was on the board ..
	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "a_pal20l10a.ic1", 0x0000, 0x00cc, CRC(1cadf26d) SHA1(348a9e4727df0a15247c7b9c5cd5ee935edd9752) )
	ROM_LOAD( "b_pal20l10a.ic3", 0x0200, 0x00cc, CRC(c3d9e729) SHA1(f05f03eecf12b4d0793124ecd3195307be04046b) )
	ROM_LOAD( "c_pal16l8a.ic2",  0x0400, 0x0104, CRC(e258b8d6) SHA1(9d000aa9a09b402208a5c2d98789cc62e23a2eb2) )
ROM_END

ROM_START( ikari3u )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ik3-2.c10", 0x000000, 0x20000, CRC(a7b34dcd) SHA1(7c2f20ae4f7dbebd3dfa3ec5408ed714e6535b6a) ) // Rotary joystick
	ROM_LOAD16_BYTE( "ik3-3.c9",  0x000001, 0x20000, CRC(50f2b83d) SHA1(b1f0c554b262614dd2cff7a3857cb974d361937f) ) // Rotary joystick
	ROM_LOAD16_BYTE( "ik3-1.c8",  0x040000, 0x10000, CRC(47e4d256) SHA1(7c6921cf2f1b8c3dae867eb1fc14e3da218cc1e0) )
	ROM_LOAD16_BYTE( "ik3-4.c12", 0x040001, 0x10000, CRC(a43af6b5) SHA1(1ad3acadbadd21642932028ecd7c282f7fd02856) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ik3-5.15d",  0x000000, 0x10000, CRC(ce6706fc) SHA1(95505b90a9524abf0c8c1ec6b2c40d8f25cb1d92) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "ik3-7.16l",  0x000000, 0x08000, CRC(0b4804df) SHA1(66d16d245bfc404366164823faaea0bfec83e487) )
	ROM_LOAD( "ik3-8.16m",  0x008000, 0x08000, CRC(10ab4e50) SHA1(dee8416eb720848cf6471e568dadc1cfc6c2e67f) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ik3-23.bin", 0x000000, 0x20000, CRC(d0fd5c77) SHA1(c171c64ad252f0ba5b0bbdf37808102fca37b488) )
	ROM_LOAD16_BYTE( "ik3-13.bin", 0x000001, 0x20000, CRC(9a56bd32) SHA1(9301b48f970b71a909fb44514b2e93c3f1516b38) )
	ROM_LOAD16_BYTE( "ik3-22.bin", 0x040000, 0x20000, CRC(4878d883) SHA1(8cdb541bad00e707fb65399d637b7cc9288ada77) )
	ROM_LOAD16_BYTE( "ik3-12.bin", 0x040001, 0x20000, CRC(0ce6a10a) SHA1(13a231aa0002b2c5a0d9404ba05a879e212d638e) )
	ROM_LOAD16_BYTE( "ik3-21.bin", 0x080000, 0x20000, CRC(50d0fbf0) SHA1(9ff5fbea8d35d0f9a38ddd7eb093edcd91d9f874) )
	ROM_LOAD16_BYTE( "ik3-11.bin", 0x080001, 0x20000, CRC(e4e2be43) SHA1(959d2799708ddae909b017c0696694c46a52697e) )
	ROM_LOAD16_BYTE( "ik3-20.bin", 0x0c0000, 0x20000, CRC(9a851efc) SHA1(bc7be338ee4da7fbfe6fe44a9c7889817416bc44) )
	ROM_LOAD16_BYTE( "ik3-10.bin", 0x0c0001, 0x20000, CRC(ac222372) SHA1(8a17e37699d691b962a6d0256a18550cc73ddfef) )
	ROM_LOAD16_BYTE( "ik3-19.bin", 0x100000, 0x20000, CRC(4ebdba89) SHA1(f3ecfef4c9d2aba58dc3e6aa3cf5813d68686909) )
	ROM_LOAD16_BYTE( "ik3-9.bin",  0x100001, 0x20000, CRC(c33971c2) SHA1(91f3eb301803f5a7027da1ff7dd2a28bc97e5125) )
	// 140000-1fffff empty
	ROM_LOAD16_BYTE( "ik3-14.bin", 0x200000, 0x20000, CRC(453bea77) SHA1(f8f8d0c048fcf32ad99e1de622d9ab635bb86eae) )
	ROM_LOAD16_BYTE( "ik3-24.bin", 0x200001, 0x20000, CRC(e9b26d68) SHA1(067d582d33157ed4b7980bd87f2f260ab74c347b) )
	ROM_LOAD16_BYTE( "ik3-15.bin", 0x240000, 0x20000, CRC(781a81fc) SHA1(e08a6cf9c632d1002176afe618605bc06168e8aa) )
	ROM_LOAD16_BYTE( "ik3-25.bin", 0x240001, 0x20000, CRC(073b03f1) SHA1(b8053139799fa06c7324cee928154c89d4425ab1) )
	ROM_LOAD16_BYTE( "ik3-16.bin", 0x280000, 0x20000, CRC(80ba400b) SHA1(2cc3e53c45f239516a60c461ad9cfa5955164262) )
	ROM_LOAD16_BYTE( "ik3-26.bin", 0x280001, 0x20000, CRC(9c613561) SHA1(fc7c9a642b18faa94e6a2ba53f35a4d756a25da3) )
	ROM_LOAD16_BYTE( "ik3-17.bin", 0x2c0000, 0x20000, CRC(0cc3ce4a) SHA1(7b34435d0bbb089055a183b821ab255170db6bec) )
	ROM_LOAD16_BYTE( "ik3-27.bin", 0x2c0001, 0x20000, CRC(16dd227e) SHA1(db3b1718dea65bc9a1a736aa62aa2be389313baf) )
	ROM_LOAD16_BYTE( "ik3-18.bin", 0x300000, 0x20000, CRC(ba106245) SHA1(ac609ec3046c21fe6058f91dd4528c5c6448dc15) )
	ROM_LOAD16_BYTE( "ik3-28.bin", 0x300001, 0x20000, CRC(711715ae) SHA1(90978c86884ca3d23c138d95b654e2fb3afc6f9a) )
	// 340000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "ik3-6.18e",  0x000000, 0x20000, CRC(59d256a4) SHA1(1e7b33329f761c695bc9a817bbc0c5e13386d073) )
ROM_END

ROM_START( ikari3j )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ik3-2-j.c10", 0x000000, 0x20000, CRC(7b1b4be4) SHA1(22b7504040da7364b811c07014a776fa5d1d5d9f) ) // Rotary joystick
	ROM_LOAD16_BYTE( "ik3-3-j.c9",  0x000001, 0x20000, CRC(8e6e2aa9) SHA1(e624809c42a79510b34d99d9ca152a38c7051e87) ) // Rotary joystick
	ROM_LOAD16_BYTE( "ik3-1.c8",    0x040000, 0x10000, CRC(47e4d256) SHA1(7c6921cf2f1b8c3dae867eb1fc14e3da218cc1e0) )
	ROM_LOAD16_BYTE( "ik3-4.c12",   0x040001, 0x10000, CRC(a43af6b5) SHA1(1ad3acadbadd21642932028ecd7c282f7fd02856) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ik3-5.16d",  0x000000, 0x10000, CRC(ce6706fc) SHA1(95505b90a9524abf0c8c1ec6b2c40d8f25cb1d92) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "ik3-7.16l",  0x000000, 0x08000, CRC(0b4804df) SHA1(66d16d245bfc404366164823faaea0bfec83e487) )
	ROM_LOAD( "ik3-8.16m",  0x008000, 0x08000, CRC(10ab4e50) SHA1(dee8416eb720848cf6471e568dadc1cfc6c2e67f) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ik3-23.bin", 0x000000, 0x20000, CRC(d0fd5c77) SHA1(c171c64ad252f0ba5b0bbdf37808102fca37b488) )
	ROM_LOAD16_BYTE( "ik3-13.bin", 0x000001, 0x20000, CRC(9a56bd32) SHA1(9301b48f970b71a909fb44514b2e93c3f1516b38) )
	ROM_LOAD16_BYTE( "ik3-22.bin", 0x040000, 0x20000, CRC(4878d883) SHA1(8cdb541bad00e707fb65399d637b7cc9288ada77) )
	ROM_LOAD16_BYTE( "ik3-12.bin", 0x040001, 0x20000, CRC(0ce6a10a) SHA1(13a231aa0002b2c5a0d9404ba05a879e212d638e) )
	ROM_LOAD16_BYTE( "ik3-21.bin", 0x080000, 0x20000, CRC(50d0fbf0) SHA1(9ff5fbea8d35d0f9a38ddd7eb093edcd91d9f874) )
	ROM_LOAD16_BYTE( "ik3-11.bin", 0x080001, 0x20000, CRC(e4e2be43) SHA1(959d2799708ddae909b017c0696694c46a52697e) )
	ROM_LOAD16_BYTE( "ik3-20.bin", 0x0c0000, 0x20000, CRC(9a851efc) SHA1(bc7be338ee4da7fbfe6fe44a9c7889817416bc44) )
	ROM_LOAD16_BYTE( "ik3-10.bin", 0x0c0001, 0x20000, CRC(ac222372) SHA1(8a17e37699d691b962a6d0256a18550cc73ddfef) )
	ROM_LOAD16_BYTE( "ik3-19.bin", 0x100000, 0x20000, CRC(4ebdba89) SHA1(f3ecfef4c9d2aba58dc3e6aa3cf5813d68686909) )
	ROM_LOAD16_BYTE( "ik3-9.bin",  0x100001, 0x20000, CRC(c33971c2) SHA1(91f3eb301803f5a7027da1ff7dd2a28bc97e5125) )
	// 140000-1fffff empty
	ROM_LOAD16_BYTE( "ik3-14.bin", 0x200000, 0x20000, CRC(453bea77) SHA1(f8f8d0c048fcf32ad99e1de622d9ab635bb86eae) )
	ROM_LOAD16_BYTE( "ik3-24.bin", 0x200001, 0x20000, CRC(e9b26d68) SHA1(067d582d33157ed4b7980bd87f2f260ab74c347b) )
	ROM_LOAD16_BYTE( "ik3-15.bin", 0x240000, 0x20000, CRC(781a81fc) SHA1(e08a6cf9c632d1002176afe618605bc06168e8aa) )
	ROM_LOAD16_BYTE( "ik3-25.bin", 0x240001, 0x20000, CRC(073b03f1) SHA1(b8053139799fa06c7324cee928154c89d4425ab1) )
	ROM_LOAD16_BYTE( "ik3-16.bin", 0x280000, 0x20000, CRC(80ba400b) SHA1(2cc3e53c45f239516a60c461ad9cfa5955164262) )
	ROM_LOAD16_BYTE( "ik3-26.bin", 0x280001, 0x20000, CRC(9c613561) SHA1(fc7c9a642b18faa94e6a2ba53f35a4d756a25da3) )
	ROM_LOAD16_BYTE( "ik3-17.bin", 0x2c0000, 0x20000, CRC(0cc3ce4a) SHA1(7b34435d0bbb089055a183b821ab255170db6bec) )
	ROM_LOAD16_BYTE( "ik3-27.bin", 0x2c0001, 0x20000, CRC(16dd227e) SHA1(db3b1718dea65bc9a1a736aa62aa2be389313baf) )
	ROM_LOAD16_BYTE( "ik3-18.bin", 0x300000, 0x20000, CRC(ba106245) SHA1(ac609ec3046c21fe6058f91dd4528c5c6448dc15) )
	ROM_LOAD16_BYTE( "ik3-28.bin", 0x300001, 0x20000, CRC(711715ae) SHA1(90978c86884ca3d23c138d95b654e2fb3afc6f9a) )
	// 340000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "ik3-6.18e",  0x000000, 0x20000, CRC(59d256a4) SHA1(1e7b33329f761c695bc9a817bbc0c5e13386d073) )
ROM_END

ROM_START( ikari3k )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ik3-2k.c10", 0x000000, 0x20000, CRC(a15d2222) SHA1(7f9702516f9c74314b093435937dfecb69495b6c) ) // 8-way joystick
	ROM_LOAD16_BYTE( "ik3-3k.c9",  0x000001, 0x20000, CRC(e3fc006e) SHA1(14e8ba1064e9bd168a4f7e5b5cc2a4b1ddbc7e32) ) // 8-way joystick
	ROM_LOAD16_BYTE( "ik3-1.c8",   0x040000, 0x10000, CRC(47e4d256) SHA1(7c6921cf2f1b8c3dae867eb1fc14e3da218cc1e0) )
	ROM_LOAD16_BYTE( "ik3-4.c12",  0x040001, 0x10000, CRC(a43af6b5) SHA1(1ad3acadbadd21642932028ecd7c282f7fd02856) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ik3-5.16d",  0x000000, 0x10000, CRC(ce6706fc) SHA1(95505b90a9524abf0c8c1ec6b2c40d8f25cb1d92) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "ik3-7k.16l",  0x000000, 0x08000, CRC(8bfb399b) SHA1(f9f9d947a7739d13269e2ab84ab25a7e403aed34) )
	ROM_LOAD( "ik3-8k.16m",  0x008000, 0x08000, CRC(3f0fe576) SHA1(70f4438e31b06ee0dc4660c04f512ccf3b7fa55f) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ikari-880d_t53.d2", 0x000000, 0x80000, CRC(5855d95e) SHA1(808ed58fb927fb631ff54e8ae7a634d748739ffc) ) // 4M mask ROM located on the A7007-SUB4M daughter board
	ROM_LOAD16_BYTE( "ikari-880c_t54.c2", 0x000001, 0x80000, CRC(6d728362) SHA1(e94bf03bd5a21c8826930acb419d44c2076908cc) ) // 4M mask ROM located on the A7007-SUB4M daughter board
	ROM_LOAD16_BYTE( "ik12.d1",           0x100000, 0x20000, CRC(4ebdba89) SHA1(f3ecfef4c9d2aba58dc3e6aa3cf5813d68686909) ) // located on the A7007-SUB4M daughter board - hand written label
	ROM_LOAD16_BYTE( "ik11.c1",           0x100001, 0x20000, CRC(c33971c2) SHA1(91f3eb301803f5a7027da1ff7dd2a28bc97e5125) ) // located on the A7007-SUB4M daughter board - hand written label
	// 140000-1fffff empty
	ROM_LOAD16_BYTE( "ikari-880d_t52.b2", 0x200000, 0x80000, CRC(e25380e6) SHA1(10a0c7891ce64a538a92fe6bd40cb955305c090e) ) // 4M mask ROM located on the A7007-SUB4M daughter board
	ROM_LOAD16_BYTE( "ikari-880c_t51.a2", 0x200001, 0x80000, CRC(87607772) SHA1(cda4ab485fb9c930b564f98e8a776da111c66fe4) ) // 4M mask ROM located on the A7007-SUB4M daughter board
	ROM_LOAD16_BYTE( "ik10.b1",           0x300000, 0x20000, CRC(ba106245) SHA1(ac609ec3046c21fe6058f91dd4528c5c6448dc15) ) // located on the A7007-SUB4M daughter board - hand written label
	ROM_LOAD16_BYTE( "ik9.a1",            0x300001, 0x20000, CRC(711715ae) SHA1(90978c86884ca3d23c138d95b654e2fb3afc6f9a) ) // located on the A7007-SUB4M daughter board - hand written label
	// 340000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "ik3-6.18e",  0x000000, 0x20000, CRC(59d256a4) SHA1(1e7b33329f761c695bc9a817bbc0c5e13386d073) )
ROM_END

ROM_START( searchar )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "bhw.2", 0x000000, 0x20000, CRC(e1430138) SHA1(eddc192524a13b2c09bd2bddcd5f8e8b771ceb21) )
	ROM_LOAD16_BYTE( "bhw.3", 0x000001, 0x20000, CRC(ee1f9374) SHA1(fd41c74fd69d65713d8e1a9b8078328381119379) )
	ROM_LOAD16_BYTE( "bhw.1", 0x040000, 0x20000, CRC(62b60066) SHA1(f7e7985c8f5f8191c580e777e1b7ed29d944d23f) )
	ROM_LOAD16_BYTE( "bhw.4", 0x040001, 0x20000, CRC(16d8525c) SHA1(0098b0a7fcb23de2661bbec9a05254aa46579bb2) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bh.5",       0x000000, 0x10000, CRC(53e2fa76) SHA1(cf25b1def82545a1fd013822ab3cf02483074623) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "bh.7",       0x000000, 0x08000, CRC(b0f1b049) SHA1(ec276984d91b5759a5e2b6815d1db2abc37b99f8) )
	ROM_LOAD( "bh.8",       0x008000, 0x08000, CRC(174ddba7) SHA1(7b19087cd2ccc409878aefe7fa08bb2e9953d352) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "bh.c1",      0x000000, 0x80000, CRC(1fb8f0ae) SHA1(d63c7376aa5f01bc009176b23324e720bada4286) )
	ROM_LOAD( "bh.c3",      0x080000, 0x80000, CRC(fd8bc407) SHA1(88d750293808bf6ea23864b22070314b14fbee3c) )
	ROM_LOAD( "bh.c5",      0x100000, 0x80000, CRC(1d30acc3) SHA1(e5ca39853779475b83fe37304e7bed2c293bd587) )
	// 180000-1fffff empty
	ROM_LOAD( "bh.c2",      0x200000, 0x80000, CRC(7c803767) SHA1(992516fbb28d00feabbed5769fa3a5748199a7d8) )
	ROM_LOAD( "bh.c4",      0x280000, 0x80000, CRC(eede7c43) SHA1(7645acf0beb4fff9ec92205dcf34124360cd52f6) )
	ROM_LOAD( "bh.c6",      0x300000, 0x80000, CRC(9f785cd9) SHA1(e5c7797ae7a3139e1814b068c5ecfe5c6bf30d0f) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "bh.v1",      0x000000, 0x20000, CRC(07a6114b) SHA1(224df4616b77a56f33974d3b1793473d48ad52ca) )
ROM_END

ROM_START( searcharu )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "bh.2",  0x000000, 0x20000, CRC(c852e2e2) SHA1(c4b1b366f452122549046a3dec9b6b375bc273af) )
	ROM_LOAD16_BYTE( "bh.3",  0x000001, 0x20000, CRC(bc04a4a1) SHA1(aa91583b987248a3e99813ab5e8ee03c02dac9b9) )
	ROM_LOAD16_BYTE( "bh.1",  0x040000, 0x20000, CRC(ba9ca70b) SHA1(c46727473673554cbe4bbbc0288d66357f99a80e) )
	ROM_LOAD16_BYTE( "bh.4",  0x040001, 0x20000, CRC(eabc5ddf) SHA1(08a2a8fcdf6a08a2694e00f4232a5bfbec98fd27) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bh.5",       0x000000, 0x10000, CRC(53e2fa76) SHA1(cf25b1def82545a1fd013822ab3cf02483074623) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "bh.7",       0x000000, 0x08000, CRC(b0f1b049) SHA1(ec276984d91b5759a5e2b6815d1db2abc37b99f8) )
	ROM_LOAD( "bh.8",       0x008000, 0x08000, CRC(174ddba7) SHA1(7b19087cd2ccc409878aefe7fa08bb2e9953d352) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "bh.c1",      0x000000, 0x80000, CRC(1fb8f0ae) SHA1(d63c7376aa5f01bc009176b23324e720bada4286) )
	ROM_LOAD( "bh.c3",      0x080000, 0x80000, CRC(fd8bc407) SHA1(88d750293808bf6ea23864b22070314b14fbee3c) )
	ROM_LOAD( "bh.c5",      0x100000, 0x80000, CRC(1d30acc3) SHA1(e5ca39853779475b83fe37304e7bed2c293bd587) )
	// 180000-1fffff empty
	ROM_LOAD( "bh.c2",      0x200000, 0x80000, CRC(7c803767) SHA1(992516fbb28d00feabbed5769fa3a5748199a7d8) )
	ROM_LOAD( "bh.c4",      0x280000, 0x80000, CRC(eede7c43) SHA1(7645acf0beb4fff9ec92205dcf34124360cd52f6) )
	ROM_LOAD( "bh.c6",      0x300000, 0x80000, CRC(9f785cd9) SHA1(e5c7797ae7a3139e1814b068c5ecfe5c6bf30d0f) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "bh.v1",      0x000000, 0x20000, CRC(07a6114b) SHA1(224df4616b77a56f33974d3b1793473d48ad52ca) )
ROM_END

ROM_START( searcharj )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "bh2ver3j.9c",  0x000000, 0x20000, CRC(7ef7b172) SHA1(85669ba72f59e4ff3a483bf611bf41c73f4e1930) )
	ROM_LOAD16_BYTE( "bh3ver3j.10c", 0x000001, 0x20000, CRC(3fdea793) SHA1(49bafb53466afb7e4486a4894e4fd6fa08ea2eb2) )
	ROM_LOAD16_BYTE( "bhw.1",        0x040000, 0x20000, CRC(62b60066) SHA1(f7e7985c8f5f8191c580e777e1b7ed29d944d23f) )
	ROM_LOAD16_BYTE( "bhw.4",        0x040001, 0x20000, CRC(16d8525c) SHA1(0098b0a7fcb23de2661bbec9a05254aa46579bb2) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bh.5",       0x000000, 0x10000, CRC(53e2fa76) SHA1(cf25b1def82545a1fd013822ab3cf02483074623) )

	ROM_REGION( 0x010000, "tiles", 0 )
	ROM_LOAD( "bh.7",       0x000000, 0x08000, CRC(b0f1b049) SHA1(ec276984d91b5759a5e2b6815d1db2abc37b99f8) )
	ROM_LOAD( "bh.8",       0x008000, 0x08000, CRC(174ddba7) SHA1(7b19087cd2ccc409878aefe7fa08bb2e9953d352) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "bh.c1",      0x000000, 0x80000, CRC(1fb8f0ae) SHA1(d63c7376aa5f01bc009176b23324e720bada4286) )
	ROM_LOAD( "bh.c3",      0x080000, 0x80000, CRC(fd8bc407) SHA1(88d750293808bf6ea23864b22070314b14fbee3c) )
	ROM_LOAD( "bh.c5",      0x100000, 0x80000, CRC(1d30acc3) SHA1(e5ca39853779475b83fe37304e7bed2c293bd587) )
	// 180000-1fffff empty
	ROM_LOAD( "bh.c2",      0x200000, 0x80000, CRC(7c803767) SHA1(992516fbb28d00feabbed5769fa3a5748199a7d8) )
	ROM_LOAD( "bh.c4",      0x280000, 0x80000, CRC(eede7c43) SHA1(7645acf0beb4fff9ec92205dcf34124360cd52f6) )
	ROM_LOAD( "bh.c6",      0x300000, 0x80000, CRC(9f785cd9) SHA1(e5c7797ae7a3139e1814b068c5ecfe5c6bf30d0f) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "bh.v1",      0x000000, 0x20000, CRC(07a6114b) SHA1(224df4616b77a56f33974d3b1793473d48ad52ca) )
ROM_END


void snk68_state::init_powbl()
{
	uint8_t *gfx2 = memregion("sprites")->base();

	// rearrange sprites to what the driver expects
	for (int i = 0; i < 0x200000; i++)
	{
		gfx2[i] = bitswap(gfx2[i], 7, 3, 6, 2, 5, 1, 4, 0);
	}
}

} // anonymous namespace


/******************************************************************************/

//    YEAR  NAME         PARENT    MACHINE   INPUT     CLASS           INIT        SCREEN COMPANY    FULLNAME                                                      FLAGS
GAME( 1988, pow,         0,        pow,      pow,      snk68_state,    empty_init, ROT0,  "SNK",     "P.O.W. - Prisoners of War (US version 1)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1988, powa,        pow,      pow,      pow,      snk68_state,    empty_init, ROT0,  "SNK",     "P.O.W. - Prisoners of War (US version 1, mask ROM sprites)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, powj,        pow,      pow,      powj,     snk68_state,    empty_init, ROT0,  "SNK",     "Datsugoku - Prisoners of War (Japan)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1988, powbl,       pow,      powbl,    pow,      snk68_state,    init_powbl, ROT0,  "bootleg", "P.O.W. - Prisoners of War (bootleg of US version 1)",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // MSM5205 not hooked up
GAME( 1989, streetsm,    0,        streetsm, streetsm, snk68_state,    empty_init, ROT0,  "SNK",     "Street Smart (US version 2)",                                MACHINE_SUPPORTS_SAVE )
GAME( 1989, streetsm1,   streetsm, searchar, streetsm, searchar_state, empty_init, ROT0,  "SNK",     "Street Smart (US version 1)",                                MACHINE_SUPPORTS_SAVE )
GAME( 1989, streetsmw,   streetsm, searchar, streetsj, searchar_state, empty_init, ROT0,  "SNK",     "Street Smart (World version 1)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1989, streetsmj,   streetsm, searchar, streetsj, searchar_state, empty_init, ROT0,  "SNK",     "Street Smart (Japan version 1)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1989, streetsmwbl, streetsm, searchar, streetsj, searchar_state, empty_init, ROT0,  "bootleg", "Street Smart (bootleg of World version 1)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1989, ikari3,      0,        searchar, ikari3,   searchar_state, empty_init, ROT0,  "SNK",     "Ikari III - The Rescue (World version 1, 8-Way Joystick)",   MACHINE_SUPPORTS_SAVE )
GAME( 1989, ikari3w,     ikari3,   searchar, ikari3,   searchar_state, empty_init, ROT0,  "SNK",     "Ikari III - The Rescue (World, Rotary Joystick)",            MACHINE_SUPPORTS_SAVE )
GAME( 1989, ikari3u,     ikari3,   searchar, ikari3,   searchar_state, empty_init, ROT0,  "SNK",     "Ikari III - The Rescue (US, Rotary Joystick)",               MACHINE_SUPPORTS_SAVE )
GAME( 1989, ikari3j,     ikari3,   searchar, ikari3,   searchar_state, empty_init, ROT0,  "SNK",     "Ikari Three (Japan, Rotary Joystick)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1989, ikari3k,     ikari3,   searchar, ikari3,   searchar_state, empty_init, ROT0,  "SNK",     "Ikari Three (Korea, 8-Way Joystick)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1989, searchar,    0,        searchar, searchar, searchar_state, empty_init, ROT90, "SNK",     "SAR - Search And Rescue (World)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1989, searcharu,   searchar, searchar, searchar, searchar_state, empty_init, ROT90, "SNK",     "SAR - Search And Rescue (US)",                               MACHINE_SUPPORTS_SAVE )
GAME( 1989, searcharj,   searchar, searchar, searchar, searchar_state, empty_init, ROT90, "SNK",     "SAR - Search And Rescue (Japan version 3)",                  MACHINE_SUPPORTS_SAVE )
