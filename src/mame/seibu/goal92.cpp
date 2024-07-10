// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/***************************************************************************

    Goal! '92

    driver by Pierpaolo Prazzoli
    and some bits by David Haywood

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class goal92_state : public driver_device
{
public:
	goal92_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_data(*this, "bg_data"),
		m_fg_data(*this, "fg_data"),
		m_tx_data(*this, "tx_data"),
		m_scrollram(*this, "scrollram"),
		m_soundbank(*this, "soundbank"),
		m_in(*this, "IN%u", 1U),
		m_dsw(*this, "DSW%u", 1U),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_buffered_spriteram(*this, "buffered_spriteram")
	{ }

	void goal92(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// memory pointers
	required_shared_ptr<uint16_t> m_bg_data;
	required_shared_ptr<uint16_t> m_fg_data;
	required_shared_ptr<uint16_t> m_tx_data;
	required_shared_ptr<uint16_t> m_scrollram;
	required_memory_bank m_soundbank;

	// video-related
	tilemap_t *m_bg_layer = nullptr;
	tilemap_t *m_fg_layer = nullptr;
	tilemap_t *m_tx_layer = nullptr;
	uint16_t m_fg_bank = 0U;

	// misc
	uint8_t m_msm5205next = 0;
	uint8_t m_adpcm_toggle = 0;
	required_ioport_array<3> m_in;
	required_ioport_array<2> m_dsw;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_buffered_spriteram;

	uint16_t inputs_r(offs_t offset, uint16_t mem_mask = ~0);
	void adpcm_data_w(uint8_t data);
	uint16_t fg_bank_r();
	void fg_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void background_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void adpcm_control_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void irqhandler(int state);
	void adpcm_int(int state);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};


uint16_t goal92_state::fg_bank_r()
{
	return m_fg_bank;
}

void goal92_state::fg_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_bank);

	if (ACCESSING_BITS_0_7)
	{
		m_fg_layer->mark_all_dirty();
	}
}

void goal92_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tx_data[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}

void goal92_state::background_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_data[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

void goal92_state::foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_data[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(goal92_state::get_text_tile_info)
{
	int tile = m_tx_data[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tile |= 0xc000;

	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(goal92_state::get_back_tile_info)
{
	int tile = m_bg_data[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tileinfo.set(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER(goal92_state::get_fore_tile_info)
{
	int tile = m_fg_data[tile_index];
	int color = (tile >> 12) & 0xf;
	int region;

	tile &= 0xfff;

	if(m_fg_bank & 0xff)
	{
		region = 3;
		tile |= 0x1000;
	}
	else
	{
		region = 4;
		tile |= 0x2000;
	}

	tileinfo.set(region, tile, color, 0);
}

void goal92_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	uint16_t *buffered_spriteram = m_buffered_spriteram->buffer();

	for (int offs = 3; offs <= 0x400 - 5; offs += 4)
	{
		uint16_t data = buffered_spriteram[offs + 2];

		int y = buffered_spriteram[offs + 0];

		if (y & 0x8000)
			break;

		if (!(data & 0x8000))
			continue;

		int sprite = buffered_spriteram[offs + 1];

		if ((sprite >> 14) != pri)
			continue;

		int x = buffered_spriteram[offs + 3];

		sprite &= 0x1fff;

		x &= 0x1ff;
		y &= 0x1ff;

		int color = (data & 0x3f) + 0x40;
		int fx = (data & 0x4000) >> 14;
		int fy = 0;

		x -= 320 / 4 - 16 - 1;

		y = 256 - (y + 7);

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				sprite,
				color, fx, fy, x, y, 15);
	}
}


void goal92_state::video_start()
{
	m_bg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goal92_state::get_back_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goal92_state::get_fore_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goal92_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_layer->set_transparent_pen(15);
	m_fg_layer->set_transparent_pen(15);
	m_tx_layer->set_transparent_pen(15);
}

uint32_t goal92_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_layer->set_scrollx(0, m_scrollram[0] + 60);
	m_bg_layer->set_scrolly(0, m_scrollram[1] + 8);

	if (m_fg_bank & 0xff)
	{
		m_fg_layer->set_scrollx(0, m_scrollram[0] + 60);
		m_fg_layer->set_scrolly(0, m_scrollram[1] + 8);
	}
	else
	{
		m_fg_layer->set_scrollx(0, m_scrollram[2] + 60);
		m_fg_layer->set_scrolly(0, m_scrollram[3] + 8);
	}

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_bg_layer->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 2);

	if (!(m_fg_bank & 0xff))
		draw_sprites(bitmap, cliprect, 1);

	m_fg_layer->draw(screen, bitmap, cliprect, 0, 0);

	if(m_fg_bank & 0xff)
		draw_sprites(bitmap, cliprect, 1);

	draw_sprites(bitmap, cliprect, 0);
	draw_sprites(bitmap, cliprect, 3);
	m_tx_layer->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


uint16_t goal92_state::inputs_r(offs_t offset, uint16_t mem_mask)
{
	switch(offset)
	{
		case 0:
			return m_dsw[0]->read();
		case 1:
			return m_in[0]->read();
		case 2:
			return m_in[1]->read();
		case 3:
			return m_in[2]->read();
		case 7:
			return m_dsw[1]->read();

		default:
			logerror("reading unhandled goal92 inputs %04X %04X @ PC = %04X\n", offset, mem_mask, m_maincpu->pc());
	}

	return 0;
}

void goal92_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1007ff).ram();
	map(0x100800, 0x100fff).ram().w(FUNC(goal92_state::background_w)).share(m_bg_data);
	map(0x101000, 0x1017ff).ram().w(FUNC(goal92_state::foreground_w)).share(m_fg_data);
	map(0x101800, 0x101fff).ram(); // it has tiles for clouds, but they aren't used
	map(0x102000, 0x102fff).ram().w(FUNC(goal92_state::text_w)).share(m_tx_data);
	map(0x103000, 0x103fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x104000, 0x13ffff).ram();
	map(0x140000, 0x1407ff).ram().share("buffered_spriteram");
	map(0x140800, 0x140801).nopw();
	map(0x140802, 0x140803).nopw();
	map(0x180000, 0x18000f).r(FUNC(goal92_state::inputs_r));
	map(0x180008, 0x180008).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x18000a, 0x18000b).nopw();
	map(0x180010, 0x180017).writeonly().share(m_scrollram);
	map(0x18001c, 0x18001d).rw(FUNC(goal92_state::fg_bank_r), FUNC(goal92_state::fg_bank_w));
}

// Sound CPU

void goal92_state::adpcm_control_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x01);

	m_msm->reset_w(data & 0x08);
}

void goal92_state::adpcm_data_w(uint8_t data)
{
	m_msm5205next = data;
}

void goal92_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_soundbank);
	map(0xe000, 0xe000).w(FUNC(goal92_state::adpcm_control_w));
	map(0xe400, 0xe400).w(FUNC(goal92_state::adpcm_data_w));
	map(0xe800, 0xe801).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xec00, 0xec01).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r("soundlatch", FUNC(generic_latch_8_device::read));
}

static INPUT_PORTS_START( goal92 )
	
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin 1 (3)" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, "Coin 2 (4)" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" ) PORT_DIPLOCATION("SW1:7") // x2 means at least 2 players must start which
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )                         // is similar to Heated Barrel SW2: 3,4
	PORT_DIPSETTING(      0x0000, "x2" )

	// NOTE: It should be the screen flip DIP, but isn't working
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8") 
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	// Match actual playing time is correctly set with these DIP, but the on-screen counter start always from 90.
	// The countdown is slowed/speeded up according to the choosen DIP.
	PORT_DIPNAME( 0x0300, 0x0300, "Time vs Computer, 1 Player" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "1:30" )
	PORT_DIPSETTING(      0x0300, "2:00" )
	PORT_DIPSETTING(      0x0100, "2:30" )
	PORT_DIPSETTING(      0x0000, "3:00" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Time Player vs Player, 2 Players" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "2:00" )
	PORT_DIPSETTING(      0x0c00, "2:30" )
	PORT_DIPSETTING(      0x0400, "3:00" )
	PORT_DIPSETTING(      0x0000, "2:00 (duplicate)" )
	PORT_DIPNAME( 0x3000, 0x3000, "Time Player vs Player, 3 Players" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, "2:30" )
	PORT_DIPSETTING(      0x3000, "3:00" )
	PORT_DIPSETTING(      0x1000, "3:30" )
	PORT_DIPSETTING(      0x0000, "2:30 (duplicate)" )
	PORT_DIPNAME( 0xc000, 0xc000, "Time Player vs Player, 4 Players" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x8000, "3:00" )
	PORT_DIPSETTING(      0xc000, "3:30" )
	PORT_DIPSETTING(      0x4000, "4:00" )
	PORT_DIPSETTING(      0x0000, "3:00 (duplicate)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Cabinet Setting" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x000c, "2 Players" )
	PORT_DIPSETTING(      0x0008, "4 Players & 4 Coin Slots" )
	PORT_DIPSETTING(      0x0004, "4 Players (2x 2P Linked) & 1-4 Coin Slots" )
	PORT_DIPSETTING(      0x0000, "4 Players & 1 Coin Slot" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:7") // toggling this has no effect in test mode, always shown on.
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:8") // toggling this has no effect in test mode, always shown on.
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_COIN2 )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_COIN4 )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_START3 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_START4 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // unused?
INPUT_PORTS_END

// handler called by the 2203 emulator when the internal timers cause an IRQ
void goal92_state::irqhandler(int state)
{
	// NMI writes to MSM ports *only*! -AS
	//m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

void goal92_state::adpcm_int(int state)
{
	m_msm->data_w(m_msm5205next);
	m_msm5205next >>= 4;
	m_adpcm_toggle ^= 1;

	if (m_adpcm_toggle)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(8*8*2,1) },
	{ STEP8(0,8), STEP8(8*8*1,8) },
	16*16
};

static GFXDECODE_START( gfx_goal92 )
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4,    0*16, 8*16 )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x4_planar, 48*16,   16 ) // Text Layer
	GFXDECODE_ENTRY( "tiles",   0, layout_16x16x4,    0*16,   16 ) // BG Layer
	GFXDECODE_ENTRY( "tiles",   0, layout_16x16x4,   16*16,   16 ) // Mid Layer
	GFXDECODE_ENTRY( "tiles", 0  , layout_16x16x4,   32*16,   16 ) // FG Layer
GFXDECODE_END


void goal92_state::machine_start()
{
	uint8_t *rom = memregion("audiocpu")->base();

	m_soundbank->configure_entries(0, 2, &rom[0x8000], 0x4000);

	save_item(NAME(m_fg_bank));
	save_item(NAME(m_msm5205next));
	save_item(NAME(m_adpcm_toggle));
}

void goal92_state::machine_reset()
{
	m_fg_bank = 0;
	m_msm5205next = 0;
	m_adpcm_toggle = 0;
}

void goal92_state::goal92(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &goal92_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(goal92_state::irq6_line_hold)); // VBL

	Z80(config, m_audiocpu, 10_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &goal92_state::sound_map);  // IRQs are triggered by the main CPU

	// video hardware
	BUFFERED_SPRITERAM16(config, m_buffered_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1); // black border at bottom is a game bug...
	screen.set_screen_update(FUNC(goal92_state::screen_update));
	screen.screen_vblank().set(m_buffered_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_goal92);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 128*16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	ym2203_device &ym1(YM2203(config, "ym1", 10_MHz_XTAL / 8));
	ym1.irq_handler().set(FUNC(goal92_state::irqhandler));
	ym1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ym2203_device &ym2(YM2203(config, "ym2", 10_MHz_XTAL / 8));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.25);

	MSM5205(config, m_msm, 400_kHz_XTAL);
	m_msm->vck_legacy_callback().set(FUNC(goal92_state::adpcm_int));   // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);      // 4KHz 4-bit
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.60);
}

/*

Goal '92 (bootleg of Olympic Soccer '92 on non-Seibu board)


PCB Layout

----------------------------------------------------------
| 400KHz  Z80                 10MHz            68000     |
|   6116   1                                   2   3     |
|  YM2203           6116                    681000 681000|
| 5205              6116                                 |
|                   6116                                 |
| 10MHz             6116                       24.0MHz   |
|                         6264                           |
|                         6264        TPC1020            |
|                                                        |
|     6116                                               |
|                                                        |
|     6116                                               |
|                                                        |
| DSW1  DSW3                                             |
|                                                        |
| DSW2                                                   |
|                             4             8            |
|                   6116      5             9            |
|                             6             10           |
|                   6116      7             11           |
|                                                        |
----------------------------------------------------------

Notes:
Z80 clock: 2.51MHz
68k clock: 12.0MHz
    VSync: 60Hz
    HSync: 15.27kHz

*/

ROM_START( goal92 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code - first 0x20000 bytes are a heavily patched copy of olysoc92a
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x80000, CRC(db0a6c7c) SHA1(b609db7806b99bc921806d8b3e5e515b4651c375) )
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x80000, CRC(e4c45dee) SHA1(542749bd1ff51220a151fe66acdadac83df8f0ee) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // Z80 code
	ROM_LOAD( "1.bin",        0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )// second half banked at 8000-bfff

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "6.bin",        0x000000, 0x040000, CRC(83cadc8f) SHA1(1d3309750347c5d6d661f5cf452235e5a83a7483) )
	ROM_LOAD( "7.bin",        0x040000, 0x040000, CRC(067e10fc) SHA1(9831b8dc9b8efa6f7797b2946ee5be03fb36de7b) )
	ROM_LOAD( "5.bin",        0x080000, 0x040000, CRC(9a390af2) SHA1(8bc46f8cc7823b8caf381866bea016ebfad9d5d3) )
	ROM_LOAD( "4.bin",        0x0c0000, 0x040000, CRC(69b118d5) SHA1(80ab6f03e1254ba47c27299ce11559b244a024ad) )  // sldh

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD( "11.bin",       0x000000, 0x080000, CRC(5701e626) SHA1(e6915714e9ca90be8fa8ab1bf7fd1f23a83fb82c) )
	ROM_LOAD( "10.bin",       0x080000, 0x080000, CRC(ebb359cc) SHA1(b2f724ef7a91fca0ff0b7d7abe1c37816464b37d) )
	ROM_LOAD( "9.bin",        0x100000, 0x080000, CRC(c9f0dd07) SHA1(d70cdb24b7df521255b5841f01dd9e8344ab7bdb) )
	ROM_LOAD( "8.bin",        0x180000, 0x080000, CRC(aeab3534) SHA1(af91238f412bfcff3a52232278d81276584614a7) )
ROM_END

} // anonymous namespace


GAME( 1992, goal92, cupsoc, goal92, goal92, goal92_state, empty_init, ROT0, "bootleg", "Goal! '92", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
