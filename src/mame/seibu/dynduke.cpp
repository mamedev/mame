// license:BSD-3-Clause
// copyright-holders: Bryan McPhail, David Haywood

/***************************************************************************

    Dynamite Duke                       (c) 1989 Seibu Kaihatsu/Fabtek
    The Double Dynamites                (c) 1989 Seibu Kaihatsu/Fabtek


    To access test mode, reset with both start buttons held.

    Coin inputs are handled by the sound CPU, so they don't work with sound
    disabled. Just put the game in Free Play mode.

    The background layer is 5bpp and I'm not 100% sure the colours are
    correct on it, although the layer is 5bpp the palette data is 4bpp.
    My current implementation looks pretty good though I've never seen
    the real game.

    There is a country code byte in the program to select between
    Seibu Kaihatsu/Fabtek/Taito licenses.

    The Double Dynamites is an updated co-op version sporting different
    enemy patterns and drops.

    Emulation by Bryan McPhail, mish@tendril.co.uk


        SW#1
        --------------------------------------------------------------------
        DESCRIPTION                        1   2   3   4   5   6   7   8
        --------------------------------------------------------------------
        COIN MODE       MODE 1            OFF
                        MODE 2            ON
        --------------------------------------------------------------------
        COIN/CREDIT*
        MODE #1         1C/1P                 OFF OFF OFF OFF
                        2C/1P                 ON  OFF OFF OFF
                        3C/1P                 OFF ON  OFF OFF
                        4C/1P                 ON  ON  OFF OFF
                        FREE PLAY             ON  ON  ON  ON
        MODE #2
        COIN A          1C/1P                 OFF OFF
                        2C/1P                 ON  OFF
                        3C/1P                 OFF ON
                        5C/1P                 ON  ON
        COIN B          1C/2P                         OFF OFF
                        1C/3P                         ON  OFF
                        1C/5P                         OFF ON
                        1C/6P                         ON  ON
        --------------------------------------------------------------------
        STARTING COIN   NORMAL                                OFF
                        X2                                    ON
        --------------------------------------------------------------------
        CABINET TYPE    TABLE                                     ON
                        UPRIGHT                                   OFF
        --------------------------------------------------------------------
        VIDEO SCREEN    NORMAL                                        OFF
                        FLIP                                          ON
        --------------------------------------------------------------------
        FACTORY SETTINGS                     OFF OFF OFF OFF OFF OFF OFF OFF
        --------------------------------------------------------------------


2008-07
Dip locations and factory settings verified with dip listing
Also, implemented conditional port for Coin Mode (SW1:1)

***************************************************************************/

#include "emu.h"

#include "seibusound.h"

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class dynduke_state : public driver_device
{
public:
	dynduke_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_seibu_sound(*this, "seibu_sound"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") ,
		m_scroll_ram(*this, "scroll_ram"),
		m_videoram(*this, "videoram"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data")
	{ }

	void dynduke(machine_config &config);
	void dbldyn(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<uint16_t> m_scroll_ram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_back_data;
	required_shared_ptr<uint16_t> m_fore_data;

	tilemap_t *m_bg_layer = nullptr;
	tilemap_t *m_fg_layer = nullptr;
	tilemap_t *m_tx_layer = nullptr;
	uint32_t m_back_bankbase = 0;
	uint32_t m_fore_bankbase = 0;
	bool m_back_enable = false;
	bool m_fore_enable = false;
	bool m_sprite_enable = false;
	bool m_txt_enable = false;
	uint32_t m_old_back = 0;
	uint32_t m_old_fore = 0;

	void background_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, uint32_t pri_mask);

	void vblank_irq(int state);
	void master_map(address_map &map) ATTR_COLD;
	void masterj_map(address_map &map) ATTR_COLD;
	void sei80bu_encrypted_full_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
	void sound_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/******************************************************************************/

void dynduke_state::background_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_back_data[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

void dynduke_state::foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fore_data[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

void dynduke_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(dynduke_state::get_bg_tile_info)
{
	uint32_t tile = m_back_data[tile_index];
	uint32_t const color = tile >> 12;

	tile = tile & 0xfff;

	tileinfo.set(1,
			tile + m_back_bankbase,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dynduke_state::get_fg_tile_info)
{
	uint32_t tile = m_fore_data[tile_index];
	uint32_t const color = tile >> 12;

	tile = tile & 0xfff;

	tileinfo.set(2,
			tile + m_fore_bankbase,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dynduke_state::get_tx_tile_info)
{
	uint32_t tile = m_videoram[tile_index];
	uint32_t const color = (tile >> 8) & 0x0f;

	tile = (tile & 0xff) | ((tile & 0xc000) >> 6);

	tileinfo.set(0,
			tile,
			color,
			0);
}

void dynduke_state::video_start()
{
	m_bg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dynduke_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_fg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dynduke_state::get_fg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dynduke_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_layer->set_transparent_pen(15);
	m_tx_layer->set_transparent_pen(15);

	save_item(NAME(m_back_bankbase));
	save_item(NAME(m_fore_bankbase));
	save_item(NAME(m_back_enable));
	save_item(NAME(m_fore_enable));
	save_item(NAME(m_sprite_enable));
	save_item(NAME(m_txt_enable));
	save_item(NAME(m_old_back));
	save_item(NAME(m_old_fore));
}

void dynduke_state::gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_back_bankbase = BIT(data, 0) ? 0x1000 : 0;
		m_fore_bankbase = BIT(data, 4) ? 0x1000 : 0;

		if (m_back_bankbase != m_old_back)
			m_bg_layer->mark_all_dirty();
		if (m_fore_bankbase != m_old_fore)
			m_fg_layer->mark_all_dirty();

		m_old_back = m_back_bankbase;
		m_old_fore = m_fore_bankbase;
	}
}


void dynduke_state::control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		// bit 0x80 toggles, maybe sprite buffering?
		// bit 0x40 is flipscreen
		// bit 0x20 not used?
		// bit 0x10 not used?
		// bit 0x08 is set on the title screen (sprite disable?)
		// bit 0x04 unused? txt disable?
		// bit 0x02 is used on the map screen (fore disable?)
		// bit 0x01 set when inserting coin.. bg disable?

		m_back_enable = BIT(~data, 0);
		m_fore_enable = BIT(~data, 1);
		m_txt_enable = BIT(~data, 2);
		m_sprite_enable = BIT(~data, 3);

		flip_screen_set(BIT(data, 6));
	}
}

void dynduke_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_sprite_enable)
		return;

	uint16_t const *const buffered_spriteram16 = m_spriteram->buffer();

	constexpr uint32_t pri_mask[4] = {
		GFX_PMASK_8 | GFX_PMASK_4 | GFX_PMASK_2, // Untested: does anything use it? Could be behind background
		GFX_PMASK_8 | GFX_PMASK_4 | GFX_PMASK_2,
		GFX_PMASK_8 | GFX_PMASK_4,
		GFX_PMASK_8,
	};

	for (int offs = 0; offs < 0x800; offs += 4)
	{
		// Don't draw empty sprite table entries
		if ((buffered_spriteram16[offs + 3] >> 8) != 0xf)
			continue;

		uint32_t const pri = pri_mask[(buffered_spriteram16[offs + 2] >> 13) & 3];

		bool fx = BIT(buffered_spriteram16[offs + 0], 13);
		bool fy = BIT(buffered_spriteram16[offs + 0], 14);
		int32_t y = buffered_spriteram16[offs + 0] & 0xff;
		int32_t x = util::sext(buffered_spriteram16[offs + 2], 9);

		uint32_t const color = (buffered_spriteram16[offs + 0] >> 8) & 0x1f;
		uint32_t const sprite = buffered_spriteram16[offs + 1] & 0x3fff;

		if (flip_screen())
		{
			x = 240 - x;
			y = 240 - y;
			fx = !fx;
			fy = !fy;
		}

		m_gfxdecode->gfx(3)->prio_transpen(bitmap, cliprect,
				sprite,
				color, fx, fy, x, y,
				screen.priority(), pri, 15);
	}
}

void dynduke_state::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, uint32_t pri_mask)
{
	// The transparency / palette handling on the background layer is very strange
	bitmap_ind16 &bm = m_bg_layer->pixmap();
	// if we're disabled, don't draw
	if (!m_back_enable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return;
	}

	int scrolly = ((m_scroll_ram[0x01] & 0x30) << 4) +((m_scroll_ram[0x02] & 0x7f) << 1) + ((m_scroll_ram[0x02] & 0x80) >> 7);
	int scrollx = ((m_scroll_ram[0x09] & 0x30) << 4) +((m_scroll_ram[0x0a] & 0x7f) << 1) + ((m_scroll_ram[0x0a] & 0x80) >> 7);

	if (flip_screen())
	{
		scrolly = 256 - scrolly;
		scrollx = 256 - scrollx;
	}

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		int const realy = (y + scrolly) & 0x1ff;
		uint16_t const *const src = &bm.pix(realy);
		uint16_t *const dst = &bitmap.pix(y);
		uint8_t *const pdst = &screen.priority().pix(y);


		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			int const realx = (x + scrollx) & 0x1ff;
			uint16_t srcdat = src[realx];

			/* 0x01 - data bits
			   0x02
			   0x04
			   0x08
			   0x10 - extra colour bit? (first boss)
			   0x20 - priority over sprites
			   the old driver also had 'bg_palbase' but I don't see what it's for?
			*/

			if ((srcdat & 0x20) == pri)
			{
				if (srcdat & 0x10) srcdat += 0x400;
				//if (srcdat & 0x10) srcdat += machine().rand() & 0x1f;

				srcdat = (srcdat & 0x000f) | ((srcdat & 0xffc0) >> 2);
				dst[x] = srcdat;
				pdst[x] |= pri_mask;
			}


		}
	}
}

uint32_t dynduke_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	// Setup the tilemaps
	m_fg_layer->set_scrolly(0, ((m_scroll_ram[0x11] & 0x30) << 4) + ((m_scroll_ram[0x12] & 0x7f) << 1) + ((m_scroll_ram[0x12] & 0x80) >> 7));
	m_fg_layer->set_scrollx(0, ((m_scroll_ram[0x19] & 0x30) << 4) + ((m_scroll_ram[0x1a] & 0x7f) << 1) + ((m_scroll_ram[0x1a] & 0x80) >> 7));
	m_fg_layer->enable(m_fore_enable);
	m_tx_layer->enable(m_txt_enable);

	draw_background(screen, bitmap, cliprect, 0x00, 1);
	draw_background(screen, bitmap, cliprect, 0x20, 2);
	m_fg_layer->draw(screen, bitmap, cliprect, 0,4);
	m_tx_layer->draw(screen, bitmap, cliprect, 0,8);
	draw_sprites(screen, bitmap, cliprect);

	return 0;
}


// Memory Maps

void dynduke_state::master_map(address_map &map)
{
	map(0x00000, 0x06fff).ram();
	map(0x07000, 0x07fff).ram().share("spriteram");
	map(0x08000, 0x080ff).ram().share(m_scroll_ram);
	map(0x0a000, 0x0afff).ram().share("share");
	map(0x0b000, 0x0b001).portr("P1_P2");
	map(0x0b002, 0x0b003).portr("DSW");
	map(0x0b004, 0x0b005).nopw();
	map(0x0b006, 0x0b007).w(FUNC(dynduke_state::control_w));
	map(0x0c000, 0x0c7ff).ram().w(FUNC(dynduke_state::text_w)).share(m_videoram);
	map(0x0d000, 0x0d00d).rw(m_seibu_sound, FUNC(seibu_sound_device::main_r), FUNC(seibu_sound_device::main_w)).umask16(0x00ff);
	map(0xa0000, 0xfffff).rom();
}

void dynduke_state::slave_map(address_map &map)
{
	map(0x00000, 0x05fff).ram();
	map(0x06000, 0x067ff).ram().w(FUNC(dynduke_state::background_w)).share(m_back_data);
	map(0x06800, 0x06fff).ram().w(FUNC(dynduke_state::foreground_w)).share(m_fore_data);
	map(0x07000, 0x07fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x08000, 0x08fff).ram().share("share");
	map(0x0a000, 0x0a001).w(FUNC(dynduke_state::gfxbank_w));
	map(0x0c000, 0x0c001).nopw();
	map(0xc0000, 0xfffff).rom();
}

// Memory map used by DlbDyn - probably an addressing PAL is different
void dynduke_state::masterj_map(address_map &map)
{
	map(0x00000, 0x06fff).ram();
	map(0x07000, 0x07fff).ram().share("spriteram");
	map(0x08000, 0x087ff).ram().w(FUNC(dynduke_state::text_w)).share(m_videoram);
	map(0x09000, 0x0900d).rw(m_seibu_sound, FUNC(seibu_sound_device::main_r), FUNC(seibu_sound_device::main_w)).umask16(0x00ff);
	map(0x0c000, 0x0c0ff).ram().share(m_scroll_ram);
	map(0x0e000, 0x0efff).ram().share("share");
	map(0x0f000, 0x0f001).portr("P1_P2");
	map(0x0f002, 0x0f003).portr("DSW");
	map(0x0f004, 0x0f005).nopw();
	map(0x0f006, 0x0f007).w(FUNC(dynduke_state::control_w));
	map(0xa0000, 0xfffff).rom();
}

void dynduke_state::sound_map(address_map &map)
{
	map(0x0000, 0xffff).r("sei80bu", FUNC(sei80bu_device::data_r));
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x4000).w(m_seibu_sound, FUNC(seibu_sound_device::pending_w));
	map(0x4001, 0x4001).w(m_seibu_sound, FUNC(seibu_sound_device::irq_clear_w));
	map(0x4002, 0x4002).w(m_seibu_sound, FUNC(seibu_sound_device::rst10_ack_w));
	map(0x4003, 0x4003).w(m_seibu_sound, FUNC(seibu_sound_device::rst18_ack_w));
	map(0x4007, 0x4007).w(m_seibu_sound, FUNC(seibu_sound_device::bank_w));
	map(0x4008, 0x4009).rw(m_seibu_sound, FUNC(seibu_sound_device::ym_r), FUNC(seibu_sound_device::ym_w));
	map(0x4010, 0x4011).r(m_seibu_sound, FUNC(seibu_sound_device::soundlatch_r));
	map(0x4012, 0x4012).r(m_seibu_sound, FUNC(seibu_sound_device::main_data_pending_r));
	map(0x4013, 0x4013).portr("COIN");
	map(0x4018, 0x4019).w(m_seibu_sound, FUNC(seibu_sound_device::main_data_w));
	map(0x401b, 0x401b).w(m_seibu_sound, FUNC(seibu_sound_device::coin_w));
	map(0x6000, 0x6000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void dynduke_state::sound_decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).r("sei80bu", FUNC(sei80bu_device::opcode_r));
}

void dynduke_state::sei80bu_encrypted_full_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
	map(0x8000, 0xffff).bankr("seibu_bank1");
}

// Input Ports

static INPUT_PORTS_START( dynduke )
	SEIBU_COIN_INPUTS   // coin inputs read through sound CPU

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:2,3") PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5") PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:2,3,4,5") PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0001)
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Starting Coin" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0300, 0x0300, IPT_UNUSED )  // "SW2:1,2" - Always OFF according to the manual
	PORT_DIPNAME( 0x0c00, 0x0400, "Bonus D.Punch" ) PORT_DIPLOCATION("SW2:3,4") // smart bomb extends
	PORT_DIPSETTING(      0x0c00, "80K 100K+" )
	PORT_DIPSETTING(      0x0800, "100K 100K+" )
	PORT_DIPSETTING(      0x0400, "120K 100K+" )
	PORT_DIPSETTING(      0x0000, "120K 120K+" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END

// Graphics Layouts

static const gfx_layout charlayout =
{
	8,8,        // 8*8 characters
	1024,
	4,          // 4 bits per pixel
	{ 4,0,(0x10000*8)+4,0x10000*8 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 tiles
	0x4000,
	4,      // 4 bits per pixel
	{ 12, 8, 4, 0 },
	{
	0,1,2,3, 16,17,18,19,
	512+0,512+1,512+2,512+3,
	512+8+8,512+9+8,512+10+8,512+11+8,
	},
	{
	0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
	},
	1024
};

static const gfx_layout bg_layout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+4, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+4, RGN_FRAC(1,3)+0,
					4,               0 },
	{
		0,1,2,3,8,9,10,11,
		256+0,256+1,256+2,256+3,256+8,256+9,256+10,256+11
	},
	{
		0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16
	},
	512
};


static const gfx_layout fg_layout =
{
	16,16,
	0x2000,
	4,
	{ 0x80000*8+4, 0x80000*8, 4, 0 },
	{
		0,1,2,3,8,9,10,11,
		256+0,256+1,256+2,256+3,256+8,256+9,256+10,256+11
	},
	{
		0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16
	},
	512
};

// Graphics Decode Information

static GFXDECODE_START( gfx_dynduke )
	GFXDECODE_ENTRY( "chars",   0, charlayout,    0x500, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0, bg_layout,     0x000, 128 )
	GFXDECODE_ENTRY( "fgtiles", 0, fg_layout,     0x200, 16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,  0x300, 32 )
GFXDECODE_END

// Interrupt Generator

void dynduke_state::vblank_irq(int state)
{
	if (state)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xc8 / 4); // V30
		m_slave->set_input_line_and_vector(0, HOLD_LINE, 0xc8 / 4); // V30
	}
}

// Machine Driver

void dynduke_state::dynduke(machine_config &config)
{
	// basic machine hardware
	V30(config, m_maincpu, 16'000'000 / 2); // NEC V30-8 CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &dynduke_state::master_map);

	V30(config, m_slave, 16'000'000 / 2); // NEC V30-8 CPU
	m_slave->set_addrmap(AS_PROGRAM, &dynduke_state::slave_map);

	z80_device &audiocpu(Z80(config, "audiocpu", 14'318'180 / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &dynduke_state::sound_map);
	audiocpu.set_addrmap(AS_OPCODES, &dynduke_state::sound_decrypted_opcodes_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	sei80bu_device &sei80bu(SEI80BU(config, "sei80bu", 0));
	sei80bu.set_addrmap(AS_PROGRAM, &dynduke_state::sei80bu_encrypted_full_map);

	config.set_maximum_quantum(attotime::from_hz(3600));

	// video hardware
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(dynduke_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.screen_vblank().append(FUNC(dynduke_state::vblank_irq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dynduke);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 2048);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 14'318'180 / 4));
	ymsnd.irq_handler().set("seibu_sound", FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", 1'320'000, okim6295_device::PIN7_LOW));
	oki.add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline("audiocpu", 0);
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank1");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym3812_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym3812_device::write));
}

void dynduke_state::dbldyn(machine_config &config)
{
	dynduke(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &dynduke_state::masterj_map);
}

// ROMs

ROM_START( dynduke )
	ROM_REGION( 0x100000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.cd8",   0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7",   0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "dde3.e8", 0x0c0000, 0x20000, CRC(95336279) SHA1(0218640e57d0a6df03ce51f2afad9862d4b13a50) ) // Euro 03SEP89
	ROM_LOAD16_BYTE( "dde4.e7", 0x0c0001, 0x20000, CRC(eb2d8fea) SHA1(d6bb718ece9011f7e24ca1c2f70a513e1c13a7a8) ) // Euro 03SEP89

	ROM_REGION( 0x100000, "slave", 0 ) // v30
	ROM_LOAD16_BYTE( "5.p8", 0x0e0000, 0x10000, CRC(883d319c) SHA1(b0df05bfe342a5289a6368be26317fa879975463) )
	ROM_LOAD16_BYTE( "6.p7", 0x0e0001, 0x10000, CRC(d94cb4ff) SHA1(653247c420a2af037106470556e6801b29bc58e8) )

	ROM_REGION( 0x20000*2, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.w8",     0x000000, 0x08000, CRC(3c29480b) SHA1(031a0b808df32b5ae4f722c9e9f69554d30505c1) )
	ROM_CONTINUE(         0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "9.5k",   0x000000, 0x04000, CRC(f2bc9af4) SHA1(9092ebe9dced243c3a7f63198a1180143dd42cea) )
	ROM_LOAD( "10.34k", 0x010000, 0x04000, CRC(c2a9f19b) SHA1(0bcea042386109c277c6c5dbe52a020f9ea5972b) )

	ROM_REGION( 0x180000, "bgtiles", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) )
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "fgtiles", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) )
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) )
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "26.n2",   0x0000, 0x0100, CRC(ea6312c6) SHA1(44e2ae948cb79884a3acd8d7d3ff1c9e31562e3e) ) // N82S135N
	ROM_LOAD( "61-d.u3", 0x0100, 0x0200, CRC(4c6527d8) SHA1(d775a0c79adbf381b56977daa702d2de2736d862) ) // N82S147AN
ROM_END

ROM_START( dyndukea )
	ROM_REGION( 0x100000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.cd8", 0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7", 0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "3.e8",  0x0c0000, 0x20000, CRC(a56f8692) SHA1(00d86c660efae30c008f8220fdfd397b7d69b2cd) ) // Euro 25JUL89
	ROM_LOAD16_BYTE( "4e.e7", 0x0c0001, 0x20000, CRC(384c0635) SHA1(4b9332d8b91426c17a2b2a58633dc6dde526284d) ) // Euro 25JUL89

	ROM_REGION( 0x100000, "slave", 0 ) // v30
	ROM_LOAD16_BYTE( "5.p8", 0x0e0000, 0x10000, CRC(883d319c) SHA1(b0df05bfe342a5289a6368be26317fa879975463) )
	ROM_LOAD16_BYTE( "6.p7", 0x0e0001, 0x10000, CRC(d94cb4ff) SHA1(653247c420a2af037106470556e6801b29bc58e8) )

	ROM_REGION( 0x20000*2, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.w8",     0x000000, 0x08000, CRC(3c29480b) SHA1(031a0b808df32b5ae4f722c9e9f69554d30505c1) )
	ROM_CONTINUE(         0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "9.5k",   0x000000, 0x04000, CRC(f2bc9af4) SHA1(9092ebe9dced243c3a7f63198a1180143dd42cea) )
	ROM_LOAD( "10.34k", 0x010000, 0x04000, CRC(c2a9f19b) SHA1(0bcea042386109c277c6c5dbe52a020f9ea5972b) )

	ROM_REGION( 0x180000, "bgtiles", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) )
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "fgtiles", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) )
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) )
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "26.n2",   0x0000, 0x0100, CRC(ea6312c6) SHA1(44e2ae948cb79884a3acd8d7d3ff1c9e31562e3e) ) // N82S135N
	ROM_LOAD( "61-d.u3", 0x0100, 0x0200, CRC(4c6527d8) SHA1(d775a0c79adbf381b56977daa702d2de2736d862) ) // N82S147AN
ROM_END

ROM_START( dyndukej )
	ROM_REGION( 0x100000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.cd8", 0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7", 0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "3.e8",  0x0c0000, 0x20000, CRC(98b9d243) SHA1(db00ffafa1353425adb79f5bf6a0cf9223a0d031) ) // Japan 03SEP89
	ROM_LOAD16_BYTE( "4.e7",  0x0c0001, 0x20000, CRC(4f575177) SHA1(837e6bab531f16efb0d21ab5b88c529ee16b40d0) ) // Japan 03SEP89

	ROM_REGION( 0x100000, "slave", 0 ) // v30
	ROM_LOAD16_BYTE( "5.p8", 0x0e0000, 0x10000, CRC(883d319c) SHA1(b0df05bfe342a5289a6368be26317fa879975463) )
	ROM_LOAD16_BYTE( "6.p7", 0x0e0001, 0x10000, CRC(d94cb4ff) SHA1(653247c420a2af037106470556e6801b29bc58e8) )

	ROM_REGION( 0x20000*2, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.w8",     0x000000, 0x08000, CRC(3c29480b) SHA1(031a0b808df32b5ae4f722c9e9f69554d30505c1) )
	ROM_CONTINUE(         0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "9.5k",   0x000000, 0x04000, CRC(f2bc9af4) SHA1(9092ebe9dced243c3a7f63198a1180143dd42cea) )
	ROM_LOAD( "10.34k", 0x010000, 0x04000, CRC(c2a9f19b) SHA1(0bcea042386109c277c6c5dbe52a020f9ea5972b) )

	ROM_REGION( 0x180000, "bgtiles", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) )
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "fgtiles", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) )
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) )
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "26.n2",   0x0000, 0x0100, CRC(ea6312c6) SHA1(44e2ae948cb79884a3acd8d7d3ff1c9e31562e3e) ) // N82S135N
	ROM_LOAD( "61-d.u3", 0x0100, 0x0200, CRC(4c6527d8) SHA1(d775a0c79adbf381b56977daa702d2de2736d862) ) // N82S147AN
ROM_END

ROM_START( dyndukeja )
	ROM_REGION( 0x100000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.cd8", 0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7", 0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "3.e8",  0x0c0000, 0x20000, CRC(2f06ddce) SHA1(b1f8dfb0af6749cdcbca450463c8e70f14278691) ) // Japan 25JUL89
	ROM_LOAD16_BYTE( "4j.e7", 0x0c0001, 0x20000, CRC(63092078) SHA1(2810d41868340ca0f10ac64499545652841a7c1e) ) // Japan 25JUL89

	ROM_REGION( 0x100000, "slave", 0 ) // v30
	ROM_LOAD16_BYTE( "5.p8", 0x0e0000, 0x10000, CRC(883d319c) SHA1(b0df05bfe342a5289a6368be26317fa879975463) )
	ROM_LOAD16_BYTE( "6.p7", 0x0e0001, 0x10000, CRC(d94cb4ff) SHA1(653247c420a2af037106470556e6801b29bc58e8) )

	ROM_REGION( 0x20000*2, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.w8",     0x000000, 0x08000, CRC(3c29480b) SHA1(031a0b808df32b5ae4f722c9e9f69554d30505c1) )
	ROM_CONTINUE(         0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "9.5k",   0x000000, 0x04000, CRC(f2bc9af4) SHA1(9092ebe9dced243c3a7f63198a1180143dd42cea) )
	ROM_LOAD( "10.34k", 0x010000, 0x04000, CRC(c2a9f19b) SHA1(0bcea042386109c277c6c5dbe52a020f9ea5972b) )

	ROM_REGION( 0x180000, "bgtiles", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) )
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "fgtiles", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) )
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) )
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "26.n2",   0x0000, 0x0100, CRC(ea6312c6) SHA1(44e2ae948cb79884a3acd8d7d3ff1c9e31562e3e) ) // N82S135N
	ROM_LOAD( "61-d.u3", 0x0100, 0x0200, CRC(4c6527d8) SHA1(d775a0c79adbf381b56977daa702d2de2736d862) ) // N82S147AN
ROM_END

ROM_START( dyndukeu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.cd8",   0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7",   0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "dd3.ef8", 0x0c0000, 0x20000, CRC(a56f8692) SHA1(00d86c660efae30c008f8220fdfd397b7d69b2cd) ) // US 25JUL89
	ROM_LOAD16_BYTE( "dd4.ef7", 0x0c0001, 0x20000, CRC(ee4b87b3) SHA1(8e470543bce07cd8682f3745e15c4f1141d9549b) ) // US 25JUL89

	ROM_REGION( 0x100000, "slave", 0 ) // v30
	ROM_LOAD16_BYTE( "5.p8", 0x0e0000, 0x10000, CRC(883d319c) SHA1(b0df05bfe342a5289a6368be26317fa879975463) )
	ROM_LOAD16_BYTE( "6.p7", 0x0e0001, 0x10000, CRC(d94cb4ff) SHA1(653247c420a2af037106470556e6801b29bc58e8) )

	ROM_REGION( 0x20000*2, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.w8",     0x000000, 0x08000, CRC(3c29480b) SHA1(031a0b808df32b5ae4f722c9e9f69554d30505c1) )
	ROM_CONTINUE(         0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "9.5k",   0x000000, 0x04000, CRC(f2bc9af4) SHA1(9092ebe9dced243c3a7f63198a1180143dd42cea) )
	ROM_LOAD( "10.34k", 0x010000, 0x04000, CRC(c2a9f19b) SHA1(0bcea042386109c277c6c5dbe52a020f9ea5972b) )

	ROM_REGION( 0x180000, "bgtiles", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) )
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "fgtiles", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) )
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) )
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "26.n2",   0x0000, 0x0100, CRC(ea6312c6) SHA1(44e2ae948cb79884a3acd8d7d3ff1c9e31562e3e) ) // N82S135N
	ROM_LOAD( "61-d.u3", 0x0100, 0x0200, CRC(4c6527d8) SHA1(d775a0c79adbf381b56977daa702d2de2736d862) ) // N82S147AN
ROM_END

ROM_START( dbldynj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.cd8", 0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7", 0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "3x.e8", 0x0c0000, 0x20000, CRC(633db1fe) SHA1(b8d67c3eedaf72a0d85eff878595af212f1246eb) ) // Japan 13NOV89
	ROM_LOAD16_BYTE( "4x.e7", 0x0c0001, 0x20000, CRC(dc9ee263) SHA1(786bf36e21d9328662916181ec4b13cce8e14f24) ) // Japan 13NOV89

	ROM_REGION( 0x100000, "slave", 0 ) // v30
	ROM_LOAD16_BYTE( "5x.p8", 0x0e0000, 0x10000, CRC(ea56d719) SHA1(6cade731316c280ef4e809aa700fdbaaabff41d0) )
	ROM_LOAD16_BYTE( "6x.p7", 0x0e0001, 0x10000, CRC(9ffa0ecd) SHA1(a22c46312ab247cd824dadf840cf1f2b0305bb29) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8x.w8",    0x000000, 0x08000, CRC(f4066081) SHA1(0e5246f4f5513be11e6ed3ea26aada7e0a17a448) )
	ROM_CONTINUE(         0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "9x.5k",   0x000000, 0x04000, CRC(913709e3) SHA1(a469043a09718409f7af835f7c045baedad92061) )
	ROM_LOAD( "10x.34k", 0x010000, 0x04000, CRC(405daacb) SHA1(2b99af73baceb44d7f78aa4a436f6a45538e0876) )

	ROM_REGION( 0x180000, "bgtiles", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) )
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "fgtiles", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) )
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) )
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "26.n2",   0x0000, 0x0100, CRC(ea6312c6) SHA1(44e2ae948cb79884a3acd8d7d3ff1c9e31562e3e) ) // N82S135N
	ROM_LOAD( "61-d.u3", 0x0100, 0x0200, CRC(4c6527d8) SHA1(d775a0c79adbf381b56977daa702d2de2736d862) ) // N82S147AN
ROM_END

ROM_START( dbldynu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.cd8",   0x0a0000, 0x10000, CRC(a5e2a95a) SHA1(135d57073d826b9cf46fb43dc49439e1400fb021) )
	ROM_LOAD16_BYTE( "2.cd7",   0x0a0001, 0x10000, CRC(7e51af22) SHA1(b26103c0d41c469d1e2d1e4e89f591c0d9cdb67c) )
	ROM_LOAD16_BYTE( "dd3x.8e", 0x0c0000, 0x20000, CRC(9b785028) SHA1(d94c41f9f8969c0effc05d5d6c44474a396a8177) ) // US 13NOV89
	ROM_LOAD16_BYTE( "dd4x.7e", 0x0c0001, 0x20000, CRC(0d0f6350) SHA1(d289bd9ac308ba1079d5b8931cc913fd326129d3) ) // US 13NOV89

	ROM_REGION( 0x100000, "slave", 0 ) // v30
	ROM_LOAD16_BYTE( "5x.p8", 0x0e0000, 0x10000, CRC(ea56d719) SHA1(6cade731316c280ef4e809aa700fdbaaabff41d0) )
	ROM_LOAD16_BYTE( "6x.p7", 0x0e0001, 0x10000, CRC(9ffa0ecd) SHA1(a22c46312ab247cd824dadf840cf1f2b0305bb29) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8x.w8",    0x000000, 0x08000, CRC(f4066081) SHA1(0e5246f4f5513be11e6ed3ea26aada7e0a17a448) )
	ROM_CONTINUE(         0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "9x.5k",   0x000000, 0x04000, CRC(913709e3) SHA1(a469043a09718409f7af835f7c045baedad92061) )
	ROM_LOAD( "10x.34k", 0x010000, 0x04000, CRC(405daacb) SHA1(2b99af73baceb44d7f78aa4a436f6a45538e0876) )

	ROM_REGION( 0x180000, "bgtiles", 0 )
	ROM_LOAD( "dd.a2",  0x000000, 0x40000, CRC(598f343f) SHA1(eee794d9d0a92e066f00818bfb63e8ca46bda764) )
	ROM_LOAD( "dd.b2",  0x040000, 0x40000, CRC(41a9088d) SHA1(eb0b7370dc773cb6f5066b044934ffb42bb06587) )
	ROM_LOAD( "dd.c2",  0x080000, 0x40000, CRC(cc341b42) SHA1(8c3cf09a3a0080a1cd7c1049cb8d11f03de50919) )
	ROM_LOAD( "dd.d2",  0x0c0000, 0x40000, CRC(4752b4d7) SHA1(4625b7885ff9d302e78d7324b3592ac5a3cead86) )
	ROM_LOAD( "dd.de3", 0x100000, 0x40000, CRC(44a4cb62) SHA1(70b2043d0428c90ee22ccd479d9710af24d359f6) )
	ROM_LOAD( "dd.ef3", 0x140000, 0x40000, CRC(aa8aee1a) SHA1(8b2b8dcb2287318e314b256f84c23424cfe29462) )

	ROM_REGION( 0x100000, "fgtiles", 0 )
	ROM_LOAD( "dd.mn3", 0x000000, 0x40000, CRC(2ee0ca98) SHA1(2ef2c4fd337e0ee4685e4863909985ee0a4c4b91) )
	ROM_LOAD( "dd.mn4", 0x040000, 0x40000, CRC(6c71e2df) SHA1(fe87277a625010c214e05b43572fadb493b3d05d) )
	ROM_LOAD( "dd.n45", 0x080000, 0x40000, CRC(85d918e1) SHA1(882cdf633288c95f2349d7c86799875b707ca347) )
	ROM_LOAD( "dd.mn5", 0x0c0000, 0x40000, CRC(e71e34df) SHA1(dce8e3de61f3869da57d476bf861856154365058) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "dd.n1", 0x000000, 0x40000, CRC(cf1db927) SHA1(3fde2ca7e7e302773ae01ed89edf0bcf69fc7aa1) )
	ROM_LOAD16_BYTE( "dd.n2", 0x000001, 0x40000, CRC(5328150f) SHA1(bb847a2ff7e5ac668e974d2853519d86feb81e03) )
	ROM_LOAD16_BYTE( "dd.m1", 0x080000, 0x40000, CRC(80776452) SHA1(319bfc90ccf04b9e5aaac5701767d3f7bbb71626) )
	ROM_LOAD16_BYTE( "dd.m2", 0x080001, 0x40000, CRC(ff61a573) SHA1(cfbe6c017c276d2fc1f083013b5df3686381753b) )
	ROM_LOAD16_BYTE( "dd.e1", 0x100000, 0x40000, CRC(84a0b87c) SHA1(62075128093f21ee6ea09cc2d4bc8e630b275fce) )
	ROM_LOAD16_BYTE( "dd.e2", 0x100001, 0x40000, CRC(a9585df2) SHA1(2eeac27dd018dd334447d539fdae2989c731e764) )
	ROM_LOAD16_BYTE( "dd.f1", 0x180000, 0x40000, CRC(9aed24ba) SHA1(0068b5bc0d7c817eee3bfbf7de6d19652ba78d41) )
	ROM_LOAD16_BYTE( "dd.f2", 0x180001, 0x40000, CRC(3eb5783f) SHA1(5487ceb4f3241241af1a81b1bb686bd3af10b0d1) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "7.x10", 0x000000, 0x10000, CRC(9cbc7b41) SHA1(107c19d3d71ee6af63d03f7278310c5e3786f91d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "26.n2",   0x0000, 0x0100, CRC(ea6312c6) SHA1(44e2ae948cb79884a3acd8d7d3ff1c9e31562e3e) ) // N82S135N
	ROM_LOAD( "61-d.u3", 0x0100, 0x0200, CRC(4c6527d8) SHA1(d775a0c79adbf381b56977daa702d2de2736d862) ) // N82S147AN
ROM_END

} // anonymous namespace


// Game Drivers

GAME( 1989, dynduke,   0,       dynduke, dynduke, dynduke_state, empty_init, ROT0, "Seibu Kaihatsu",                  "Dynamite Duke (Europe, 03SEP89)",       MACHINE_SUPPORTS_SAVE )
GAME( 1989, dyndukea,  dynduke, dynduke, dynduke, dynduke_state, empty_init, ROT0, "Seibu Kaihatsu",                  "Dynamite Duke (Europe, 25JUL89)",       MACHINE_SUPPORTS_SAVE )
GAME( 1989, dyndukej,  dynduke, dynduke, dynduke, dynduke_state, empty_init, ROT0, "Seibu Kaihatsu",                  "Dynamite Duke (Japan, 03SEP89)",        MACHINE_SUPPORTS_SAVE )
GAME( 1989, dyndukeja, dynduke, dynduke, dynduke, dynduke_state, empty_init, ROT0, "Seibu Kaihatsu",                  "Dynamite Duke (Japan, 25JUL89)",        MACHINE_SUPPORTS_SAVE )
GAME( 1989, dyndukeu,  dynduke, dynduke, dynduke, dynduke_state, empty_init, ROT0, "Seibu Kaihatsu (Fabtek license)", "Dynamite Duke (US, 25JUL89)",           MACHINE_SUPPORTS_SAVE )
GAME( 1989, dbldynj,   0,       dbldyn,  dynduke, dynduke_state, empty_init, ROT0, "Seibu Kaihatsu",                  "The Double Dynamites (Japan, 13NOV89)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, dbldynu,   dbldynj, dynduke, dynduke, dynduke_state, empty_init, ROT0, "Seibu Kaihatsu (Fabtek license)", "The Double Dynamites (US, 13NOV89)",    MACHINE_SUPPORTS_SAVE )
