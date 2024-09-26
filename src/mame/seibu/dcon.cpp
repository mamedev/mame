// license:BSD-3-Clause
// copyright-holders: Bryan McPhail

/***************************************************************************

    D-Con                                   (c) 1992 Success
    SD Gundam Psycho Salamander no Kyoui    (c) 1991 Banpresto/Bandai

    These games run on Seibu hardware somewhat similar to Blood Bros.

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"

#include "sei021x_sei0220_spr.h"
#include "seibu_crtc.h"
#include "seibusound.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class dcon_state : public driver_device, public seibu_sound_common
{
public:
	dcon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data"),
		m_mid_data(*this, "mid_data"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram")
	{ }

	void dcon(machine_config &config);
	void sdgndmps(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<sei0211_device> m_spritegen;

	required_shared_ptr<uint16_t> m_back_data;
	required_shared_ptr<uint16_t> m_fore_data;
	required_shared_ptr<uint16_t> m_mid_data;
	required_shared_ptr<uint16_t> m_textram;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_background_layer = nullptr;
	tilemap_t *m_foreground_layer = nullptr;
	tilemap_t *m_midground_layer = nullptr;
	tilemap_t *m_text_layer = nullptr;

	uint16_t m_gfx_bank_select = 0U;
	uint16_t m_last_gfx_bank = 0U;
	uint16_t m_scroll_ram[6]{};
	uint16_t m_layer_en = 0U;

	uint8_t sdgndmps_sound_comms_r(offs_t offset);

	void layer_en_w(uint16_t data);
	void layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gfxbank_w(uint16_t data);
	void background_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void midground_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	uint32_t screen_update_dcon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sdgndmps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t pri_cb(uint8_t pri, uint8_t ext);
	void dcon_map(address_map &map) ATTR_COLD;
	void sdgndmps_map(address_map &map) ATTR_COLD;
};


/******************************************************************************/

void dcon_state::gfxbank_w(uint16_t data)
{
	if (data & 1)
		m_gfx_bank_select = 0x1000;
	else
		m_gfx_bank_select = 0;
}

void dcon_state::background_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_back_data[offset]);
	m_background_layer->mark_tile_dirty(offset);
}

void dcon_state::foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fore_data[offset]);
	m_foreground_layer->mark_tile_dirty(offset);
}

void dcon_state::midground_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mid_data[offset]);
	m_midground_layer->mark_tile_dirty(offset);
}

void dcon_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_text_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(dcon_state::get_back_tile_info)
{
	int tile = m_back_data[tile_index];
	int const color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(dcon_state::get_fore_tile_info)
{
	int tile = m_fore_data[tile_index];
	int const color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tileinfo.set(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER(dcon_state::get_mid_tile_info)
{
	int tile = m_mid_data[tile_index];
	int const color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tileinfo.set(3, tile | m_gfx_bank_select, color, 0);
}

TILE_GET_INFO_MEMBER(dcon_state::get_text_tile_info)
{
	int tile = m_textram[tile_index];
	int const color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tileinfo.set(0, tile, color, 0);
}

void dcon_state::video_start()
{
	m_background_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dcon_state::get_back_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_foreground_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dcon_state::get_fore_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_midground_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dcon_state::get_mid_tile_info)),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_text_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dcon_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_midground_layer->set_transparent_pen(15);
	m_foreground_layer->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);

	m_gfx_bank_select = 0;

	save_item(NAME(m_gfx_bank_select));
	save_item(NAME(m_last_gfx_bank));
	save_item(NAME(m_scroll_ram));
	save_item(NAME(m_layer_en));
}

uint32_t dcon_state::pri_cb(uint8_t pri, uint8_t ext)
{
	switch(pri)
	{
		case 0: return 0xf0; // above foreground layer
		case 1: return 0xfc; // above midground layer
		case 2: return 0xfe; // above background layer
		case 3:
		default: return 0; // above text layer
	}
}

uint32_t dcon_state::screen_update_dcon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	// Setup the tilemaps
	m_background_layer->set_scrollx(0, m_scroll_ram[0]);
	m_background_layer->set_scrolly(0, m_scroll_ram[1]);
	m_midground_layer->set_scrollx(0, m_scroll_ram[2]);
	m_midground_layer->set_scrolly(0, m_scroll_ram[3]);
	m_foreground_layer->set_scrollx(0, m_scroll_ram[4]);
	m_foreground_layer->set_scrolly(0, m_scroll_ram[5]);

	if (BIT(~m_layer_en, 0))
		m_background_layer->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(15, cliprect); // Should always be black, not pen 15

	if (BIT(~m_layer_en, 1))
		m_midground_layer->draw(screen, bitmap, cliprect, 0, 1);

	if (BIT(~m_layer_en, 2))
		m_foreground_layer->draw(screen, bitmap, cliprect, 0, 2);

	if (BIT(~m_layer_en, 3))
		m_text_layer->draw(screen, bitmap, cliprect, 0, 4);

	if (BIT(~m_layer_en, 4))
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());

	return 0;
}

uint32_t dcon_state::screen_update_sdgndmps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	// Gfx banking
	if (m_last_gfx_bank != m_gfx_bank_select)
	{
		m_midground_layer->mark_all_dirty();
		m_last_gfx_bank = m_gfx_bank_select;
	}

	// Setup the tilemaps
	m_background_layer->set_scrollx(0, m_scroll_ram[0] + 128);
	m_background_layer->set_scrolly(0, m_scroll_ram[1]);
	m_midground_layer->set_scrollx(0, m_scroll_ram[2] + 128);
	m_midground_layer->set_scrolly(0, m_scroll_ram[3]);
	m_foreground_layer->set_scrollx(0, m_scroll_ram[4] + 128);
	m_foreground_layer->set_scrolly(0, m_scroll_ram[5]);
	m_text_layer->set_scrollx(0, /*m_scroll_ram[6] + */ 128);
	m_text_layer->set_scrolly(0, /*m_scroll_ram[7] + */ 0);

	if (BIT(~m_layer_en, 0))
		m_background_layer->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(15, cliprect); // Should always be black, not pen 15

	if (BIT(~m_layer_en, 1))
		m_midground_layer->draw(screen, bitmap, cliprect, 0, 1);

	if (BIT(~m_layer_en, 2))
		m_foreground_layer->draw(screen, bitmap, cliprect, 0, 2);

	if (BIT(~m_layer_en, 3))
		m_text_layer->draw(screen, bitmap, cliprect, 0, 4);

	if (BIT(~m_layer_en, 4))
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());

	return 0;
}


/***************************************************************************/

u8 dcon_state::sdgndmps_sound_comms_r(offs_t offset)
{
	// Routine at 134C sends no sound commands if lowest bit is 0
	if (offset == 5) // ($a000a)
		return 1;

	return m_seibu_sound->main_r(offset);
}

void dcon_state::dcon_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom();
	map(0x80000, 0x8bfff).ram();

	map(0x8c000, 0x8c7ff).ram().w(FUNC(dcon_state::background_w)).share(m_back_data);
	map(0x8c800, 0x8cfff).ram().w(FUNC(dcon_state::foreground_w)).share(m_fore_data);
	map(0x8d000, 0x8d7ff).ram().w(FUNC(dcon_state::midground_w)).share(m_mid_data);
	map(0x8d800, 0x8e7ff).ram().w(FUNC(dcon_state::text_w)).share(m_textram);
	map(0x8e800, 0x8f7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x8f800, 0x8ffff).ram().share(m_spriteram);
	map(0x9d000, 0x9d7ff).w(FUNC(dcon_state::gfxbank_w));

	map(0xa0000, 0xa000d).rw(m_seibu_sound, FUNC(seibu_sound_device::main_r), FUNC(seibu_sound_device::main_w)).umask16(0x00ff);
	map(0xc0000, 0xc004f).rw("crtc", FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0xc0080, 0xc0081).nopw();
	map(0xc00c0, 0xc00c1).nopw();
	map(0xe0000, 0xe0001).portr("DSW");
	map(0xe0002, 0xe0003).portr("P1_P2");
	map(0xe0004, 0xe0005).portr("SYSTEM");
}

void dcon_state::sdgndmps_map(address_map &map)
{
	dcon_map(map);
	map(0xa0000, 0xa000d).r(FUNC(dcon_state::sdgndmps_sound_comms_r)).umask16(0x00ff);
}

/******************************************************************************/

static INPUT_PORTS_START( common )
	SEIBU_COIN_INPUTS   // coin inputs read through sound CPU

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dcon )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sdgndmps )
	PORT_INCLUDE( common )

	PORT_MODIFY("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x0100, IP_ACTIVE_LOW )

	PORT_START("DSW")
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:8" )
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout dcon_charlayout =
{
	8,8,        // 8*8 characters
	RGN_FRAC(1,2),
	4,          // 4 bits per pixel
	{ 0,4,(0x10000*8)+0,0x10000*8+4 },
	{ 3,2,1,0, 11,10,9,8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout dcon_tilelayout =
{
	16,16,  // 16*16 tiles
	RGN_FRAC(1,1),
	4,      // 4 bits per pixel
	{ 8, 12, 0,4 },
	{
		3,2,1,0,19,18,17,16,
		512+3,512+2,512+1,512+0,
		512+11+8,512+10+8,512+9+8,512+8+8,
	},
	{
		0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
	},
	1024
};

static GFXDECODE_START( gfx_dcon )
	GFXDECODE_ENTRY( "txtiles", 0, dcon_charlayout, 1024+768, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0, dcon_tilelayout, 1024+0,   16 )
	GFXDECODE_ENTRY( "fgtiles", 0, dcon_tilelayout, 1024+512, 16 )
	GFXDECODE_ENTRY( "mgtiles", 0, dcon_tilelayout, 1024+256, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_dcon_spr )
	GFXDECODE_ENTRY( "sprites", 0, dcon_tilelayout,        0, 64 )
GFXDECODE_END

void dcon_state::layer_en_w(uint16_t data)
{
	m_layer_en = data;
}

void dcon_state::layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll_ram[offset]);
}

/******************************************************************************/

void dcon_state::dcon(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &dcon_state::dcon_map);
	m_maincpu->set_vblank_int("screen", FUNC(dcon_state::irq4_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 4'000'000)); // Perhaps 14'318'180 / 4?
	audiocpu.set_addrmap(AS_PROGRAM, &dcon_state::seibu_sound_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(dcon_state::screen_update_dcon));
	screen.set_palette(m_palette);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(dcon_state::layer_en_w));
	crtc.layer_scroll_callback().set(FUNC(dcon_state::layer_scroll_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dcon);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	SEI0211(config, m_spritegen, XTAL(14'318'181), m_palette, gfx_dcon_spr);
	m_spritegen->set_pri_callback(FUNC(dcon_state::pri_cb));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 4'000'000));
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

void dcon_state::sdgndmps(machine_config &config) // PCB number is PB91008
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dcon_state::sdgndmps_map);
	m_maincpu->set_vblank_int("screen", FUNC(dcon_state::irq4_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(14'318'181) / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &dcon_state::seibu_sound_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(dcon_state::screen_update_sdgndmps));
	screen.set_palette(m_palette);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(dcon_state::layer_en_w));
	crtc.layer_scroll_callback().set(FUNC(dcon_state::layer_scroll_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dcon);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	SEI0211(config, m_spritegen, XTAL(14'318'181), m_palette, gfx_dcon_spr);
	m_spritegen->set_pri_callback(FUNC(dcon_state::pri_cb));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181) / 4));
	ymsnd.irq_handler().set(m_seibu_sound, FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(20'000'000) / 16, okim6295_device::PIN7_LOW)); // 1.25Mhz? unverified clock & divisor (was 1320000)
	oki.add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline("audiocpu", 0);
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank1");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));
}

/***************************************************************************/

ROM_START( dcon )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE("p0-0",   0x000000, 0x20000, CRC(a767ec15) SHA1(5ceeba97b58c4e24d8c0991303dd6f7a2dfeda48) )
	ROM_LOAD16_BYTE("p0-1",   0x000001, 0x20000, CRC(a7efa091) SHA1(aa0e97d20f3bdc1adc019fe62112a8417bb3ddf1) )
	ROM_LOAD16_BYTE("p1-0",   0x040000, 0x20000, CRC(3ec1ef7d) SHA1(6195f1402dba5b3d3913e97cd78ba1e8865f7692) )
	ROM_LOAD16_BYTE("p1-1",   0x040001, 0x20000, CRC(4b8de320) SHA1(14a3ab347fc468869355951294c3e3a8f9211b6a) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "fmsnd",           0x000000, 0x08000, CRC(50450faa) SHA1(d4add7d357951b51d53ed7f143ece7f3bde7f4cb) )
	ROM_CONTINUE(                0x010000, 0x08000 )
	ROM_COPY( "audiocpu",        0x000000, 0x18000, 0x08000 )

	ROM_REGION( 0x020000, "txtiles", 0 )
	ROM_LOAD( "fix0",  0x000000, 0x10000, CRC(ab30061f) SHA1(14dba37fef7bd13c827fd542b24cc593dcdc9f99) )
	ROM_LOAD( "fix1",  0x010000, 0x10000, CRC(a0582115) SHA1(498d6e4f631a5dfe54d5c2813c47d40c466b694d) )

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "bg1",   0x000000, 0x80000, CRC(eac43283) SHA1(f5d384c98751002416013a9a920e2ab2cea61cb1) )

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "bg3",   0x000000, 0x80000, CRC(1408a1e0) SHA1(d96fb8a60af02df313ffc9e0284611d7ca50540d) )

	ROM_REGION( 0x080000, "mgtiles", 0 )
	ROM_LOAD( "bg2",   0x000000, 0x80000, CRC(01864eb6) SHA1(78f755d7462a787bd1a378184e8fce8fa889f258) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "obj0",  0x000000, 0x80000, CRC(c3af37db) SHA1(7d6ee07b6302aaec8d792faf78a37898a2ac3c4e) )
	ROM_LOAD( "obj1",  0x080000, 0x80000, CRC(be1f53ba) SHA1(061b80487e6c4040618af6ed9c5315fba44f5d0c) )
	ROM_LOAD( "obj2",  0x100000, 0x80000, CRC(24e0b51c) SHA1(434b4d58f785eefb5380c08a0704c8dea6609268) )
	ROM_LOAD( "obj3",  0x180000, 0x80000, CRC(5274f02d) SHA1(69b94363624177c92e1b3413244ce649c2e5a696) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pcm", 0x000000, 0x20000, CRC(d2133b85) SHA1(a2e61c9893da8a95c35c0b47e2c43c315b654de8) )
ROM_END

ROM_START( sdgndmps )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "911-a01.25",   0x00000, 0x20000, CRC(3362915d) SHA1(d98e2d4de402ca549664e148c9a6fe94fccfd5e9) )
	ROM_LOAD16_BYTE( "911-a02.29",   0x00001, 0x20000, CRC(fbc78285) SHA1(85d40b0e7bb923a0daacbd78ce7d5bb9c80b9ffc) )
	ROM_LOAD16_BYTE( "911-a03.27",   0x40000, 0x20000, CRC(6c24b4f2) SHA1(e9fb82884f47694bebcad9254cb57a0b01dcd9c8) )
	ROM_LOAD16_BYTE( "911-a04.28",   0x40001, 0x20000, CRC(6ff9d716) SHA1(303faec19a84afd6cbcf3ca5d4877693c11d406e) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "911-a05.010",   0x00000, 0x08000, CRC(90455406) SHA1(dd2c5b96ac4b51251a3d34d97cc9af360afaa38c) )
	ROM_CONTINUE(              0x10000, 0x08000 )
	ROM_COPY( "audiocpu",      0x00000, 0x18000, 0x08000 )

	ROM_REGION( 0x020000, "txtiles", 0 )
	ROM_LOAD( "911-a08.66",   0x000000, 0x10000, CRC(e7e04823) SHA1(d9b1ace5cd8218d5a4767cf5adbc267dce7c0668) )
	ROM_LOAD( "911-a07.73",   0x010000, 0x10000, CRC(6f40d4a9) SHA1(8abadb2dc07ac22081b2970358e9f92b90b174b0) )

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "911-a12.63",   0x000000, 0x080000, CRC(8976bbb6) SHA1(6f510d6506e54ddec7119d85dcc0a169d4901983) )

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "911-a11.65",   0x000000, 0x080000, CRC(3f3b7810) SHA1(0761c5fb0802fdd2ee7523f1f4e5cfb2c7a6fce6) )

	ROM_REGION( 0x100000, "mgtiles", 0 )
	ROM_LOAD( "911-a13.64",   0x000000, 0x100000, CRC(f38a584a) SHA1(16dd8e7086949d14e9185c37313290024d6dafdc) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "911-a10.73",   0x000000, 0x100000, CRC(80e341fb) SHA1(619e71aefd0b13a01a6a2ed5d8613fe56242d209) )
	ROM_LOAD( "911-a09.74",   0x100000, 0x100000, CRC(98f34519) SHA1(20319d546df104485ee553ce0e58364f927d1135) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "911-a06.97",   0x00000, 0x40000, CRC(12c79440) SHA1(9e9987527f64dfd8a51a2ab49afc465e76c5e7ac) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "bnd-007.88",   0x000, 0x200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) // Priority
ROM_END

} // anonymous namespace


/***************************************************************************/

GAME( 1991, sdgndmps, 0, sdgndmps, sdgndmps, dcon_state, empty_init, ROT0, "Banpresto / Bandai", "SD Gundam Psycho Salamander no Kyoui", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, dcon,     0, dcon,     dcon,     dcon_state, empty_init, ROT0, "Success",            "D-Con",                                MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
