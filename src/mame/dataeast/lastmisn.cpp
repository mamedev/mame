// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Stephane Humbert
/***************************************************************************

Last Mission (rev 6)        (c) 1986 Data East USA (2*6809 + I8751)
Last Mission (rev 5)        (c) 1986 Data East USA (2*6809 + I8751)
Last Mission (Japan)        (c) 1986 Data East Corporation (2*6809 + I8751)
Shackled                    (c) 1986 Data East USA (2*6809 + I8751)
Breywood                    (c) 1986 Data East Corporation (2*6809 + I8751)
Captain Silver (World)      (c) 1987 Data East Corporation (2*6809 + I8751)
Captain Silver (Japan)      (c) 1987 Data East Corporation (2*6809 + I8751)

Emulation by Bryan McPhail, mish@tendril.co.uk

TODO:
- shackled continue after game over does not work, see MT0418. It's not that
  big of an issue user-wise, since credits add more health. For breywood, it
  appears to work OK after the 1st level.

***************************************************************************/

#include "emu.h"

#include "deckarn.h"
#include "decrmc3.h"

#include "cpu/m6502/r65c02.h"
#include "cpu/m6809/m6809.h"
#include "cpu/mcs51/i8051.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"

#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "multibyte.h"


namespace {

class lastmisn_state : public driver_device
{
public:
	lastmisn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_spritegen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundirq(*this, "soundirq"),
		m_soundlatch(*this, "soundlatch"),
		m_mainbank(*this, "mainbank"),
		m_videoram(*this, "videoram"),
		m_bg_ram(*this, "bg_ram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram16(*this, "spriteram16", 0x800, ENDIANNESS_BIG)
	{ }

	void lastmisn(machine_config &config) ATTR_COLD;
	void shackled(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void buffer_spriteram16_w(u8 data);
	void main_irq_on_w(u8 data);
	void main_irq_off_w(u8 data);
	void sub_irq_on_w(u8 data);
	void sub_irq_off_w(u8 data);
	void both_firq_off_w(u8 data);
	void flip_screen_w(u8 data);
	void bg_ram_w(offs_t offset, u8 data);
	u8 bg_ram_r(offs_t offset);
	void videoram_w(offs_t offset, u8 data);

	void lastmisn_control_w(u8 data);
	void shackled_control_w(u8 data);
	void lastmisn_scrollx_w(u8 data);
	void lastmisn_scrolly_w(u8 data);
	virtual void mcu_to_main_w(u8 data);

	u8 i8751_hi_r();
	u8 i8751_lo_r();
	void i8751_hi_w(u8 data);
	void i8751_lo_w(u8 data);

	u8 i8751_port0_r();
	void i8751_port0_w(u8 data);
	u8 i8751_port1_r();
	void i8751_port1_w(u8 data);

	TILEMAP_MAPPER_MEMBER(scan_rows);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);

	DECLARE_VIDEO_START(lastmisn);
	DECLARE_VIDEO_START(shackled);

	u32 screen_update_lastmisn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_shackled(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void shackled_coin_irq(int state);

	void set_screen_raw_params(machine_config &config) ATTR_COLD;

	void base_sound_map(address_map &map) ATTR_COLD;
	void lastmisn_main_map(address_map &map) ATTR_COLD;
	void lastmisn_sound_map(address_map &map) ATTR_COLD;
	void lastmisn_sub_map(address_map &map) ATTR_COLD;
	void shackled_main_map(address_map &map) ATTR_COLD;
	void shackled_sub_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8751_device> m_mcu;
	required_device<deco_karnovsprites_device> m_spritegen;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<deco_rmc3_device> m_palette;
	optional_device<input_merger_device> m_soundirq;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory regions
	required_memory_bank m_mainbank;

	// memory pointers
	required_shared_ptr<u8> m_videoram;
	optional_shared_ptr<u8> m_bg_ram;

	required_shared_ptr<u8> m_spriteram;
	memory_share_creator<u16> m_spriteram16;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fix_tilemap = nullptr;
	u8 m_scroll[4]{};
	u8 m_game_uses_priority = 0;

	// misc
	bool m_coin_state = false;
	u8 m_bank_mask = 0;

	// MCU communication
	u8 m_i8751_p2 = 0;
	u8 m_i8751_port0 = 0;
	u8 m_i8751_port1 = 0;
	u16 m_i8751_return = 0;
	u16 m_i8751_value = 0;
};

// with MSM5205 ADPCM
class csilver_state : public lastmisn_state
{
public:
	csilver_state(const machine_config &mconfig, device_type type, const char *tag) :
		lastmisn_state(mconfig, type, tag),
		m_msm(*this, "msm"),
		m_soundbank(*this, "soundbank")
	{ }

	void csilver(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void scroll_w(offs_t offset, u8 data);
	void control_w(u8 data);
	void adpcm_data_w(u8 data);
	void sound_bank_w(u8 data);
	virtual void mcu_to_main_w(u8 data) override;
	u8 adpcm_reset_r();
	void adpcm_int(int state);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	required_device<msm5205_device> m_msm;
	required_memory_bank m_soundbank;

	u8 m_toggle = 0;
	u8 m_msm5205next = 0;
};

/******************************************************************************/

void lastmisn_state::buffer_spriteram16_w(u8 data)
{
	// copy to a 16-bit region for the sprite chip
	for (int i = 0; i < 0x800/2 ; i++)
		m_spriteram16[i] = get_u16be(&m_spriteram[i * 2]);
}

void lastmisn_state::bg_ram_w(offs_t offset, u8 data)
{
	m_bg_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

u8 lastmisn_state::bg_ram_r(offs_t offset)
{
	return m_bg_ram[offset];
}


void lastmisn_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset / 2);
}

void csilver_state::scroll_w(offs_t offset, u8 data)
{
	m_scroll[offset] = data;
}

void lastmisn_state::lastmisn_control_w(u8 data)
{
	/*
	    Bit 0x0f - ROM bank switch.
	    Bit 0x10 - Unused
	    Bit 0x20 - X scroll MSB
	    Bit 0x40 - Y scroll MSB
	    Bit 0x80 - Hold subcpu reset line high if clear, else low
	*/
	m_mainbank->set_entry(data & m_bank_mask);

	m_scroll[0] = BIT(data, 5);
	m_scroll[2] = BIT(data, 6);

	if (data & 0x80)
		m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	else
		m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void lastmisn_state::shackled_control_w(u8 data)
{
	// Bottom 4 bits - bank switch, Bits 5 & 6 - Scroll MSBs
	m_mainbank->set_entry(data & m_bank_mask);

	m_scroll[0] = BIT(data, 5);
	m_scroll[2] = BIT(data, 6);
}

void lastmisn_state::lastmisn_scrollx_w(u8 data)
{
	m_scroll[1] = data;
}

void lastmisn_state::lastmisn_scrolly_w(u8 data)
{
	m_scroll[3] = data;
}

/******************************************************************************/

u32 lastmisn_state::screen_update_lastmisn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, get_u16be(&m_scroll[0]));
	m_bg_tilemap->set_scrolly(0, get_u16be(&m_scroll[2]));

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16.target(), 0x400);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

u32 lastmisn_state::screen_update_shackled(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, get_u16be(&m_scroll[0]));
	m_bg_tilemap->set_scrolly(0, get_u16be(&m_scroll[2]));

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16.target(), 0x400);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILEMAP_MAPPER_MEMBER(lastmisn_state::scan_rows)
{
	// logical (col,row) -> memory offset
	return ((col & 0x0f) + ((row & 0x0f) << 4)) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

TILE_GET_INFO_MEMBER(lastmisn_state::get_bg_tile_info)
{
	const u32 offs = tile_index * 2;
	const u16 tile = get_u16be(&m_bg_ram[offs]);
	const u8 color = tile >> 12;

	if (color & 8 && m_game_uses_priority)
		tileinfo.category = 1;
	else
		tileinfo.category = 0;

	tileinfo.set(1, tile & 0xfff, color, 0);
}

TILE_GET_INFO_MEMBER(lastmisn_state::get_fix_tile_info)
{
	const u32 offs = tile_index << 1;
	const u16 tile = get_u16be(&m_bg_ram[offs]);
	const u8 color = (tile & 0xc000) >> 14;

	tileinfo.set(0, tile & 0xfff, color, 0);
}

VIDEO_START_MEMBER(lastmisn_state,lastmisn)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastmisn_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(lastmisn_state::scan_rows)), 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastmisn_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_game_uses_priority = 0;
}

VIDEO_START_MEMBER(lastmisn_state,shackled)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastmisn_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(lastmisn_state::scan_rows)), 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lastmisn_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transmask(0, 0x000f, 0xfff0); // Bottom 12 pens
	m_game_uses_priority = 1;
}


/******************************************************************************/

u8 lastmisn_state::i8751_hi_r()
{
	return m_i8751_return >> 8; // MSB
}

u8 lastmisn_state::i8751_lo_r()
{
	return m_i8751_return & 0xff; // LSB
}


/***************************************************
*
* Hook-up for games that have a proper MCU dump.
*
***************************************************/

void lastmisn_state::i8751_lo_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff00) | data;
}

void lastmisn_state::i8751_hi_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff) | (u16(data) << 8);

	// SECIRQ is triggered on activating this latch
	if (m_i8751_p2 & 2)
		m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
}


/******************************************************************************/

void csilver_state::control_w(u8 data)
{
	/*
	    Bit 0x0f - ROM bank switch.
	    Bit 0x10 - Always set(?)
	    Bit 0x20 - Unused.
	    Bit 0x40 - Unused.
	    Bit 0x80 - Hold subcpu reset line high if clear, else low? (Not needed anyway)
	*/
	m_mainbank->set_entry(data & m_bank_mask);
}

void csilver_state::adpcm_int(int state)
{
	m_toggle ^= 1;
	if (m_toggle)
		m_audiocpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);

	m_msm->data_w(m_msm5205next >> 4);
	m_msm5205next <<= 4;
}

u8 csilver_state::adpcm_reset_r()
{
	if (!machine().side_effects_disabled())
		m_msm->reset_w(0);
	return 0;
}

void csilver_state::adpcm_data_w(u8 data)
{
	m_msm5205next = data;
	m_audiocpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
}

void csilver_state::sound_bank_w(u8 data)
{
	m_soundbank->set_entry((data & 0x08) >> 3);
}


/******************************************************************************/

void lastmisn_state::main_irq_on_w(u8 data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void lastmisn_state::main_irq_off_w(u8 data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void lastmisn_state::sub_irq_on_w(u8 data)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void lastmisn_state::sub_irq_off_w(u8 data)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void lastmisn_state::both_firq_off_w(u8 data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	m_subcpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}

void lastmisn_state::flip_screen_w(u8 data) { flip_screen_set(data); }


/******************************************************************************/

void lastmisn_state::lastmisn_main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN0").w(FUNC(lastmisn_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN1").w(FUNC(lastmisn_state::main_irq_off_w));
	map(0x1802, 0x1802).portr("IN2").w(FUNC(lastmisn_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("DSW0").w(FUNC(lastmisn_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(lastmisn_state::sub_irq_on_w));
	map(0x1805, 0x1805).w(FUNC(lastmisn_state::buffer_spriteram16_w));
	map(0x1806, 0x1806).r(FUNC(lastmisn_state::i8751_hi_r));
	map(0x1807, 0x1807).rw(FUNC(lastmisn_state::i8751_lo_r), FUNC(lastmisn_state::flip_screen_w));
	map(0x1809, 0x1809).w(FUNC(lastmisn_state::lastmisn_scrollx_w));
	map(0x180b, 0x180b).w(FUNC(lastmisn_state::lastmisn_scrolly_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180d, 0x180d).w(FUNC(lastmisn_state::lastmisn_control_w));
	map(0x180e, 0x180e).w(FUNC(lastmisn_state::i8751_hi_w));
	map(0x180f, 0x180f).w(FUNC(lastmisn_state::i8751_lo_w));
	map(0x2000, 0x27ff).ram().w(FUNC(lastmisn_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram().share(m_spriteram);
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(lastmisn_state::bg_ram_r), FUNC(lastmisn_state::bg_ram_w)).share(m_bg_ram);
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

void lastmisn_state::lastmisn_sub_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN0").w(FUNC(lastmisn_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN1").w(FUNC(lastmisn_state::main_irq_off_w));
	map(0x1802, 0x1802).portr("IN2").w(FUNC(lastmisn_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("DSW0").w(FUNC(lastmisn_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(lastmisn_state::sub_irq_on_w));
	map(0x1805, 0x1805).w(FUNC(lastmisn_state::buffer_spriteram16_w));
	map(0x1806, 0x1806).r(FUNC(lastmisn_state::i8751_hi_r));
	map(0x1807, 0x1807).rw(FUNC(lastmisn_state::i8751_lo_r), FUNC(lastmisn_state::flip_screen_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180e, 0x180e).w(FUNC(lastmisn_state::i8751_hi_w));
	map(0x180f, 0x180f).w(FUNC(lastmisn_state::i8751_lo_w));
	map(0x2000, 0x27ff).ram().w(FUNC(lastmisn_state::videoram_w));
	map(0x2800, 0x2fff).writeonly().share(m_spriteram);
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(lastmisn_state::bg_ram_r), FUNC(lastmisn_state::bg_ram_w));
	map(0x4000, 0xffff).rom().region("sub", 0x4000);
}

void lastmisn_state::shackled_main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN0").w(FUNC(lastmisn_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN1").w(FUNC(lastmisn_state::main_irq_off_w));
	map(0x1802, 0x1802).portr("IN2").w(FUNC(lastmisn_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("DSW0").w(FUNC(lastmisn_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(lastmisn_state::sub_irq_on_w));
	map(0x1805, 0x1805).w(FUNC(lastmisn_state::buffer_spriteram16_w));
	map(0x1807, 0x1807).w(FUNC(lastmisn_state::flip_screen_w));
	map(0x1809, 0x1809).w(FUNC(lastmisn_state::lastmisn_scrollx_w));
	map(0x180b, 0x180b).w(FUNC(lastmisn_state::lastmisn_scrolly_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180d, 0x180d).w(FUNC(lastmisn_state::shackled_control_w));
	map(0x2000, 0x27ff).ram().w(FUNC(lastmisn_state::videoram_w));
	map(0x2800, 0x2fff).ram().share(m_spriteram);
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(lastmisn_state::bg_ram_r), FUNC(lastmisn_state::bg_ram_w)).share(m_bg_ram);
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

void lastmisn_state::shackled_sub_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN0").w(FUNC(lastmisn_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN1").w(FUNC(lastmisn_state::main_irq_off_w));
	map(0x1802, 0x1802).portr("IN2").w(FUNC(lastmisn_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("DSW0").w(FUNC(lastmisn_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(lastmisn_state::sub_irq_on_w));
	map(0x1805, 0x1805).w(FUNC(lastmisn_state::buffer_spriteram16_w));
	map(0x1806, 0x1806).r(FUNC(lastmisn_state::i8751_hi_r));
	map(0x1807, 0x1807).rw(FUNC(lastmisn_state::i8751_lo_r), FUNC(lastmisn_state::flip_screen_w));
	map(0x1809, 0x1809).w(FUNC(lastmisn_state::lastmisn_scrollx_w));
	map(0x180b, 0x180b).w(FUNC(lastmisn_state::lastmisn_scrolly_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180d, 0x180d).w(FUNC(lastmisn_state::shackled_control_w));
	map(0x180e, 0x180e).w(FUNC(lastmisn_state::i8751_hi_w));
	map(0x180f, 0x180f).w(FUNC(lastmisn_state::i8751_lo_w));
	map(0x2000, 0x27ff).ram().w(FUNC(lastmisn_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram().share(m_spriteram);
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(lastmisn_state::bg_ram_r), FUNC(lastmisn_state::bg_ram_w));
	map(0x4000, 0xffff).rom().region("sub", 0x4000);
}

void csilver_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN1").w(FUNC(csilver_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN0").w(FUNC(csilver_state::main_irq_off_w));
	map(0x1802, 0x1802).w(FUNC(csilver_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("IN2").w(FUNC(csilver_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(csilver_state::sub_irq_on_w));
	map(0x1805, 0x1805).portr("DSW0").w(FUNC(csilver_state::buffer_spriteram16_w));
	map(0x1807, 0x1807).w(FUNC(csilver_state::flip_screen_w));
	map(0x1808, 0x180b).w(FUNC(csilver_state::scroll_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180d, 0x180d).w(FUNC(csilver_state::control_w));
	map(0x180e, 0x180e).w(FUNC(csilver_state::i8751_hi_w));
	map(0x180f, 0x180f).w(FUNC(csilver_state::i8751_lo_w));
	map(0x1c00, 0x1c00).r(FUNC(csilver_state::i8751_hi_r));
	map(0x1e00, 0x1e00).r(FUNC(csilver_state::i8751_lo_r));
	map(0x2000, 0x27ff).ram().w(FUNC(csilver_state::videoram_w));
	map(0x2800, 0x2fff).ram().share(m_spriteram);
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(csilver_state::bg_ram_r), FUNC(csilver_state::bg_ram_w)).share(m_bg_ram);
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

void csilver_state::sub_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).w(FUNC(csilver_state::sub_irq_off_w));
	map(0x1801, 0x1801).w(FUNC(csilver_state::main_irq_off_w));
	map(0x1802, 0x1802).w(FUNC(csilver_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("IN2").w(FUNC(csilver_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(csilver_state::sub_irq_on_w));
	map(0x1805, 0x1805).portr("DSW0").w(FUNC(csilver_state::buffer_spriteram16_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x2000, 0x27ff).ram().w(FUNC(csilver_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram().share(m_spriteram);
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(csilver_state::bg_ram_r), FUNC(csilver_state::bg_ram_w));
	map(0x4000, 0xffff).rom().region("sub", 0x4000);
}


/******************************************************************************/

void lastmisn_state::base_sound_map(address_map &map)
{
	map(0x0800, 0x0801).w("ym1", FUNC(ym2203_device::write));
	map(0x1000, 0x1001).w("ym2", FUNC(ym3526_device::write));
	map(0x3000, 0x3000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

// Used by Last Mission, Shackled & Breywood
void lastmisn_state::lastmisn_sound_map(address_map &map)
{
	base_sound_map(map);
	map(0x0000, 0x05ff).ram();
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}

// Captain Silver - same sound system as Pocket Gal
void csilver_state::sound_map(address_map &map)
{
	base_sound_map(map);
	map(0x0000, 0x07ff).ram();
	map(0x1800, 0x1800).w(FUNC(csilver_state::adpcm_data_w)); // ADPCM data for the MSM5205 chip
	map(0x2000, 0x2000).w(FUNC(csilver_state::sound_bank_w));
	map(0x3400, 0x3400).r(FUNC(csilver_state::adpcm_reset_r)); // ? not sure
	map(0x4000, 0x7fff).bankr(m_soundbank);
	map(0x8000, 0xffff).rom().region("audiocpu", 0x8000);
}


/******************************************************************************/

/*
    Gondomania schematics show the following:

    Port P0 - attached to 2 * LS374 at location 4C & 1C
    Port P1 - attached to 2 * LS374 at location 3C & 2C
    Port P2.2 -> SECIRQ (IRQ to main CPU)
    Port P2.3 -> 'COUNT' (Enable coin counter - also wired directly to coinage) [not emulated]
    Port P2.4-7 -> Enable latches 4C, 1C, 3C, 2C
    Port P3.4-7 -> Directly attached to coinage connector (3 coins & service)

*/

u8 lastmisn_state::i8751_port0_r()
{
	return m_i8751_port0;
}

void lastmisn_state::i8751_port0_w(u8 data)
{
	m_i8751_port0 = data;
}

u8 lastmisn_state::i8751_port1_r()
{
	return m_i8751_port1;
}

void lastmisn_state::i8751_port1_w(u8 data)
{
	m_i8751_port1 = data;
}

void lastmisn_state::mcu_to_main_w(u8 data)
{
	const u8 fall = ~data & m_i8751_p2;
	const u8 rise = data & ~m_i8751_p2;
	m_i8751_p2 = data;

	// P24-P27: controls latches for main CPU communication
	if (BIT(fall, 4))
	{
		m_i8751_port0 = m_i8751_value >> 8;

		// lastmisn mcu sets p0 to 0x00
		m_mcu->set_port_forced_input(0, m_i8751_port0);
	}
	if (BIT(fall, 5))
		m_i8751_port1 = m_i8751_value & 0xff;
	if (BIT(fall, 6))
		m_i8751_return = (m_i8751_return & 0xff) | (u16(m_i8751_port0) << 8);
	if (BIT(fall, 7))
		m_i8751_return = (m_i8751_return & 0xff00) | m_i8751_port1;

	// P22: FIRQ to both CPUs
	if (BIT(rise, 2))
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
		m_subcpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	}

	// P20,P21: clear MCU INT0/1
	if (BIT(~data, 0))
		m_mcu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
	if (BIT(~data, 1))
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}


void csilver_state::mcu_to_main_w(u8 data)
{
	const u8 fall = ~data & m_i8751_p2;
	const u8 rise = data & ~m_i8751_p2;
	m_i8751_p2 = data;

	// P24-P27: controls latches for main CPU communication
	if (BIT(fall, 4))
		m_i8751_port0 = m_i8751_value >> 8;
	if (BIT(fall, 5))
		m_i8751_port1 = m_i8751_value & 0xff;
	if (BIT(fall, 6))
		m_i8751_return = (m_i8751_return & 0xff) | (u16(m_i8751_port0) << 8);
	if (BIT(fall, 7))
		m_i8751_return = (m_i8751_return & 0xff00) | m_i8751_port1;

	// P22: FIRQ to both CPUs
	if (BIT(rise, 2))
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
		m_subcpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	}

	// P21: clear MCU INT1
	if (BIT(~data, 1))
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}


/******************************************************************************/

// verified from M6809 code
static INPUT_PORTS_START( lastmisn )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // bomb
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) // select
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // bomb
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL // select
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("COIN")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<1>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<0>))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<2>))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Infinite Lives (Cheat)")         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:2,3") // tables at 0x82c1 (4 words) and 0xde38 (3 words) in 'lastmisn', 0x82c1 and 0xde17 in 'lastmisno'
	PORT_DIPSETTING(    0x06, "30k 70k 70k+" )
	PORT_DIPSETTING(    0x04, "40k 90k 90k+" )
	PORT_DIPSETTING(    0x02, "40k and 80k" )
	PORT_DIPSETTING(    0x00, "50k only" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( lastmisnj )
	PORT_INCLUDE(lastmisn)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:2,3") // tables at 0x82b7 (4 words) and 0xdd29 (3 words)
	PORT_DIPSETTING(    0x06, "30k 50k 50k+" )
	PORT_DIPSETTING(    0x04, "30k 70k 70k+" )
	PORT_DIPSETTING(    0x02, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x00, "50k only" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )                 // "difficult"
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Hard ) )            // "very difficult"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )              // "top difficult"
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END


// verified from M6809 code
static INPUT_PORTS_START( shackled )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // tested and discarded by vestigial code at start
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("COIN")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<0>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<1>))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<2>))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:2")
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:3")
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:4")
	PORT_DIPNAME( 0x10, 0x10, "Leave Off" )                     PORT_DIPLOCATION("SW1:5") // game doesn't boot when this is On - code at 0x401a - related to MCU - "dias" in DIP Switches page
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:6")
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:7")
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	// tables in main CPU : 0x859b (Help), 0x85e9 (6-Help), 0x8fbe (Coin), 0x91b6 (Heart)
	PORT_DIPNAME( 0x07, 0x07, "Coin/Heart/Help/6-Help" )        PORT_DIPLOCATION("SW2:1,2,3") // name from DIP Switches page
	PORT_DIPSETTING( 0x00, "2/100/50/200" )
	PORT_DIPSETTING( 0x01, "4/100/60/300" )
	PORT_DIPSETTING( 0x02, "6/200/70/300" )
	PORT_DIPSETTING( 0x03, "8/200/80/400" )
	PORT_DIPSETTING( 0x07, "10/200/100/500" )
	PORT_DIPSETTING( 0x06, "12/300/100/600" )
	PORT_DIPSETTING( 0x05, "18/400/200/700" )
	PORT_DIPSETTING( 0x04, "20/500/200/800" )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:4")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( breywood )
	PORT_INCLUDE(shackled)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, "Power" )                         PORT_DIPLOCATION("SW2:1,2,3,4") // table at 0x41be in sub CPU
	PORT_DIPSETTING( 0x07, "200" )
	PORT_DIPSETTING( 0x0b, "300" )
	PORT_DIPSETTING( 0x03, "400" )
	PORT_DIPSETTING( 0x0d, "500" )
	PORT_DIPSETTING( 0x05, "600" )
	PORT_DIPSETTING( 0x09, "700" )
	PORT_DIPSETTING( 0x01, "800" )
	PORT_DIPSETTING( 0x0e, "900" )
	PORT_DIPSETTING( 0x0f, "1000" )
	PORT_DIPSETTING( 0x06, "2000" )
	PORT_DIPSETTING( 0x0a, "3000" )
	PORT_DIPSETTING( 0x02, "4000" )
	PORT_DIPSETTING( 0x0c, "5000" )
	PORT_DIPSETTING( 0x04, "6000" )
	PORT_DIPSETTING( 0x08, "7000" )
	PORT_DIPSETTING( 0x00, "8000" )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( csilver )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // sword
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // jump
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // sword
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // jump
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // ^
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("COIN") // hooked up on the i8751
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPNAME( 0x40, 0x40, "No Key for Door (Cheat)")        PORT_DIPLOCATION("SW2:7") // code at 0x9816 in sub CPU
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( csilverj )
	PORT_INCLUDE(csilver)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	3,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every sprite takes 8 consecutive bytes
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),0 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
		0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

static GFXDECODE_START( gfx_shackled )
	GFXDECODE_ENTRY( "char",    0, charlayout,   0,  4 )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout, 768, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_shackled_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 256, 16 )
GFXDECODE_END


/******************************************************************************/

void lastmisn_state::shackled_coin_irq(int state)
{
	if (state && !m_coin_state)
		m_mcu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
	m_coin_state = bool(state);
}


/******************************************************************************/

void lastmisn_state::machine_start()
{
	u8 *ROM = memregion("mainbank")->base();
	const u8 max_bank = memregion("mainbank")->bytes() / 0x4000;
	m_mainbank->configure_entries(0, max_bank, &ROM[0], 0x4000);
	m_bank_mask = (max_bank - 1) & 0xf;

	m_i8751_p2 = 0xff;

	save_item(NAME(m_coin_state));
	save_item(NAME(m_scroll));
	save_item(NAME(m_i8751_p2));
	save_item(NAME(m_i8751_port0));
	save_item(NAME(m_i8751_port1));
	save_item(NAME(m_i8751_return));
	save_item(NAME(m_i8751_value));
}

void lastmisn_state::machine_reset()
{
	m_scroll[0] = m_scroll[1] = m_scroll[2] = m_scroll[3] = 0;

	m_i8751_port0 = m_i8751_port1 = 0;
	m_i8751_return = m_i8751_value = 0;
}


void csilver_state::machine_start()
{
	lastmisn_state::machine_start();

	u8 *RAM = memregion("audiocpu")->base();
	m_soundbank->configure_entries(0, 2, &RAM[0], 0x4000);

	save_item(NAME(m_msm5205next));
	save_item(NAME(m_toggle));
}

void csilver_state::machine_reset()
{
	lastmisn_state::machine_reset();

	m_msm5205next = 0;
	m_toggle = 0;
}


void lastmisn_state::set_screen_raw_params(machine_config &config)
{
	// DECO video CRTC, matches PCB measurements
	m_screen->set_raw(12_MHz_XTAL / 2, 384, 0, 256, 272, 8, 248);
}

void lastmisn_state::lastmisn(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 12_MHz_XTAL / 8); // MC68B09EP in schematics
	m_maincpu->set_addrmap(AS_PROGRAM, &lastmisn_state::lastmisn_main_map);

	MC6809E(config, m_subcpu, 12_MHz_XTAL / 8); // MC68B09EP in schematics
	m_subcpu->set_addrmap(AS_PROGRAM, &lastmisn_state::lastmisn_sub_map);

	R65C02(config, m_audiocpu, 12_MHz_XTAL / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &lastmisn_state::lastmisn_sound_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(lastmisn_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(lastmisn_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(lastmisn_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(lastmisn_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(lastmisn_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	INPUT_MERGER_ANY_LOW(config, "coin").output_handler().set(FUNC(lastmisn_state::shackled_coin_irq));

	// video hardware
	DECO_KARNOVSPRITES(config, m_spritegen, m_palette, gfx_shackled_spr);

	SCREEN(config, m_screen);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(lastmisn_state::screen_update_lastmisn));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shackled);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(lastmisn_state,lastmisn)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL / 4));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void lastmisn_state::shackled(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 12_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &lastmisn_state::shackled_main_map);

	MC6809E(config, m_subcpu, 12_MHz_XTAL / 8);
	m_subcpu->set_addrmap(AS_PROGRAM, &lastmisn_state::shackled_sub_map);

	R65C02(config, m_audiocpu, 12_MHz_XTAL / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &lastmisn_state::lastmisn_sound_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(lastmisn_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(lastmisn_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(lastmisn_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(lastmisn_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(lastmisn_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_maincpu); // needs heavy sync, otherwise one of the two CPUs will miss an IRQ and cause the game to hang

	INPUT_MERGER_ANY_LOW(config, "coin").output_handler().set(FUNC(lastmisn_state::shackled_coin_irq));

	// video hardware
	DECO_KARNOVSPRITES(config, m_spritegen, m_palette, gfx_shackled_spr);

	SCREEN(config, m_screen);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(lastmisn_state::screen_update_shackled));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shackled);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(lastmisn_state,shackled)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL / 4));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void csilver_state::csilver(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 12_MHz_XTAL / 8); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &csilver_state::main_map);

	MC6809E(config, m_subcpu, 12_MHz_XTAL / 8); // verified on pcb
	m_subcpu->set_addrmap(AS_PROGRAM, &csilver_state::sub_map);

	R65C02(config, m_audiocpu, 12_MHz_XTAL / 8); // verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &csilver_state::sound_map); // NMIs are caused by the main CPU

	config.set_perfect_quantum(m_mcu);

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(csilver_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(csilver_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(csilver_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(csilver_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(csilver_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	// video hardware
	DECO_KARNOVSPRITES(config, m_spritegen, m_palette, gfx_shackled_spr);

	SCREEN(config, m_screen);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(csilver_state::screen_update_lastmisn));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_subcpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shackled);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(csilver_state,lastmisn)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8)); // verified on pcb
	ym1.add_route(0, "mono", 0.10);
	ym1.add_route(1, "mono", 0.10);
	ym1.add_route(2, "mono", 0.10);
	ym1.add_route(3, "mono", 0.20);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL / 4)); // verified on pcb
	ym2.add_route(ALL_OUTPUTS, "mono", 0.40);

	MSM5205(config, m_msm, 384_kHz_XTAL); // verified on pcb
	m_msm->vck_legacy_callback().set(FUNC(csilver_state::adpcm_int)); // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S48_4B); // 8KHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.80);
}


/******************************************************************************/

ROM_START( lastmisn )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "last_mission_dl03-8.13h", 0x00000, 0x08000, CRC(a4f8d54b) SHA1(4525826fa5d12c22e0f3bc1c3a9673b86a34aad1) ) // Rev 8 roms

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "last_mission_dl04-5.7h",  0x00000, 0x10000, CRC(7dea1552) SHA1(920684413e2ba4313111e79821c5714977b26b1a) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "last_mission_dl02-5.18h", 0x0000, 0x10000, CRC(ec9b5daf) SHA1(86d47bad123676abc82dd7c92943878c54c33075) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "last_mission_dl05-.5h",   0x0000, 0x8000, CRC(1a5df8c0) SHA1(83d36b1d5fb87f50c44f3110804d6bbdbbc0da99) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "last_mission_dl00-e.18a", 0x0000, 0x1000, CRC(e97481c6) SHA1(5c6b0e3585712c03b1b657c814c502c396ffa333) BAD_DUMP ) // not verified to be the same data as the "A" MCU dump

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "last_mission_dl01-.2a",   0x00000, 0x2000, CRC(f3787a5d) SHA1(3701df42cb2aca951963703e72c6c7b272eed82b) )
	ROM_CONTINUE(                        0x06000, 0x2000 )
	ROM_CONTINUE(                        0x04000, 0x2000 )
	ROM_CONTINUE(                        0x02000, 0x2000 )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "last_mission_dl11-.13f",  0x00000, 0x08000, CRC(36579d3b) SHA1(8edf952dafcd5bc66e08074687f0bec809fd4c2f) )
	ROM_LOAD( "last_mission_dl12-.9f",   0x08000, 0x08000, CRC(2ba6737e) SHA1(c5e4c27726bf14e9cd60d62e2f17ea5be8093c37) )
	ROM_LOAD( "last_mission_dl13-.8f",   0x10000, 0x08000, CRC(39a7dc93) SHA1(3b7968fd06ac0379525c1d3e73f8bbe18ea36439) )
	ROM_LOAD( "last_mission_dl10-.16f",  0x18000, 0x08000, CRC(fe275ea8) SHA1(2f089f96583235f1f5226ef2a64b430d84efbeee) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "last_mission_dl09-.12k",  0x00000, 0x10000, CRC(6a5a0c5d) SHA1(0106cf693c284be5faf96e56b651fab92a410915) )
	ROM_LOAD( "last_mission_dl08-.14k",  0x10000, 0x10000, CRC(3b38cfce) SHA1(d6829bed6916fb301c08031bd466ee4dcc05b275) )
	ROM_LOAD( "last_mission_dl07-.15k",  0x20000, 0x10000, CRC(1b60604d) SHA1(1ee15cfdac87f7eeb92050766293b894cfad1466) )
	ROM_LOAD( "last_mission_dl06-.17k",  0x30000, 0x10000, CRC(c43c26a7) SHA1(896e278935b100edc12cd970469f2e8293eb96cc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dl-14.9c",    0x0000, 0x0100, CRC(2e55aa12) SHA1(c0f2b9649467eb9d2c1e47589b5990f5c5e8cc93) ) // Priority (Not yet used)
ROM_END

ROM_START( lastmisnu6 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "last_mission_dl03-6.13h", 0x00000, 0x08000, CRC(47751a5e) SHA1(190970a6eb849781e8853f2bed7b34ac44e569ca) ) // Rev 6 roms

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "last_mission_dl04-5.7h",  0x00000, 0x10000, CRC(7dea1552) SHA1(920684413e2ba4313111e79821c5714977b26b1a) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "last_mission_dl02-5.18h", 0x0000, 0x10000, CRC(ec9b5daf) SHA1(86d47bad123676abc82dd7c92943878c54c33075) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "last_mission_dl05-.5h",   0x0000, 0x8000, CRC(1a5df8c0) SHA1(83d36b1d5fb87f50c44f3110804d6bbdbbc0da99) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "last_mission_dl00-a.18a", 0x0000, 0x1000, CRC(e97481c6) SHA1(5c6b0e3585712c03b1b657c814c502c396ffa333) ) // Hand written "A", some MCUs are known to be labeled DL00-7, it's not verified to be the same data

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "last_mission_dl01-.2a",   0x00000, 0x2000, CRC(f3787a5d) SHA1(3701df42cb2aca951963703e72c6c7b272eed82b) )
	ROM_CONTINUE(                        0x06000, 0x2000 )
	ROM_CONTINUE(                        0x04000, 0x2000 )
	ROM_CONTINUE(                        0x02000, 0x2000 )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "last_mission_dl11-.13f",  0x00000, 0x08000, CRC(36579d3b) SHA1(8edf952dafcd5bc66e08074687f0bec809fd4c2f) )
	ROM_LOAD( "last_mission_dl12-.9f",   0x08000, 0x08000, CRC(2ba6737e) SHA1(c5e4c27726bf14e9cd60d62e2f17ea5be8093c37) )
	ROM_LOAD( "last_mission_dl13-.8f",   0x10000, 0x08000, CRC(39a7dc93) SHA1(3b7968fd06ac0379525c1d3e73f8bbe18ea36439) )
	ROM_LOAD( "last_mission_dl10-.16f",  0x18000, 0x08000, CRC(fe275ea8) SHA1(2f089f96583235f1f5226ef2a64b430d84efbeee) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "last_mission_dl09-.12k",  0x00000, 0x10000, CRC(6a5a0c5d) SHA1(0106cf693c284be5faf96e56b651fab92a410915) )
	ROM_LOAD( "last_mission_dl08-.14k",  0x10000, 0x10000, CRC(3b38cfce) SHA1(d6829bed6916fb301c08031bd466ee4dcc05b275) )
	ROM_LOAD( "last_mission_dl07-.15k",  0x20000, 0x10000, CRC(1b60604d) SHA1(1ee15cfdac87f7eeb92050766293b894cfad1466) )
	ROM_LOAD( "last_mission_dl06-.17k",  0x30000, 0x10000, CRC(c43c26a7) SHA1(896e278935b100edc12cd970469f2e8293eb96cc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dl-14.9c",    0x0000, 0x0100, CRC(2e55aa12) SHA1(c0f2b9649467eb9d2c1e47589b5990f5c5e8cc93) ) // Priority (Not yet used)
ROM_END

ROM_START( lastmisnu5 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "last_mission_dl03-5.13h", 0x00000, 0x08000, CRC(357f5f6b) SHA1(a114aac50db62a6bcb943681e517ad7c88ec47f4) ) // Rev 5 roms

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "last_mission_dl04-5.7h",  0x00000, 0x10000, CRC(7dea1552) SHA1(920684413e2ba4313111e79821c5714977b26b1a) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "last_mission_dl02-5.18h", 0x0000, 0x10000, CRC(ec9b5daf) SHA1(86d47bad123676abc82dd7c92943878c54c33075) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "last_mission_dl05-.5h",   0x0000, 0x8000, CRC(1a5df8c0) SHA1(83d36b1d5fb87f50c44f3110804d6bbdbbc0da99) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "last_mission_dl00-a.18a", 0x0000, 0x1000, CRC(e97481c6) SHA1(5c6b0e3585712c03b1b657c814c502c396ffa333) ) // Hand written "A", some MCUs are known to be labeled DL00-7, it's not verified to be the same data

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "last_mission_dl01-.2a",   0x00000, 0x2000, CRC(f3787a5d) SHA1(3701df42cb2aca951963703e72c6c7b272eed82b) )
	ROM_CONTINUE(                        0x06000, 0x2000 )
	ROM_CONTINUE(                        0x04000, 0x2000 )
	ROM_CONTINUE(                        0x02000, 0x2000 )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "last_mission_dl11-.13f",  0x00000, 0x08000, CRC(36579d3b) SHA1(8edf952dafcd5bc66e08074687f0bec809fd4c2f) )
	ROM_LOAD( "last_mission_dl12-.9f",   0x08000, 0x08000, CRC(2ba6737e) SHA1(c5e4c27726bf14e9cd60d62e2f17ea5be8093c37) )
	ROM_LOAD( "last_mission_dl13-.8f",   0x10000, 0x08000, CRC(39a7dc93) SHA1(3b7968fd06ac0379525c1d3e73f8bbe18ea36439) )
	ROM_LOAD( "last_mission_dl10-.16f",  0x18000, 0x08000, CRC(fe275ea8) SHA1(2f089f96583235f1f5226ef2a64b430d84efbeee) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "last_mission_dl09-.12k",  0x00000, 0x10000, CRC(6a5a0c5d) SHA1(0106cf693c284be5faf96e56b651fab92a410915) )
	ROM_LOAD( "last_mission_dl08-.14k",  0x10000, 0x10000, CRC(3b38cfce) SHA1(d6829bed6916fb301c08031bd466ee4dcc05b275) )
	ROM_LOAD( "last_mission_dl07-.15k",  0x20000, 0x10000, CRC(1b60604d) SHA1(1ee15cfdac87f7eeb92050766293b894cfad1466) )
	ROM_LOAD( "last_mission_dl06-.17k",  0x30000, 0x10000, CRC(c43c26a7) SHA1(896e278935b100edc12cd970469f2e8293eb96cc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dl-14.9c",    0x00000, 0x0100, CRC(2e55aa12) SHA1(c0f2b9649467eb9d2c1e47589b5990f5c5e8cc93) ) // Priority (Not yet used)
ROM_END

ROM_START( lastmisnj )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dl03-.13h",   0x00000, 0x08000, CRC(4be5e7e1) SHA1(9f943658663da31947cebdcbcb5f4e2be0714c06) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "dl04-.7h",    0x00000, 0x10000, CRC(f026adf9) SHA1(4ccd0e714a6eb7cee388c93beee2d5510407c961) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dl02-.18h",   0x0000, 0x10000, CRC(d0de2b5d) SHA1(e0bb34c2a2ef6fc6f05ab9a98bd23a39004c0c05) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "last_mission_dl05-.5h",  0x0000, 0x8000, CRC(1a5df8c0) SHA1(83d36b1d5fb87f50c44f3110804d6bbdbbc0da99) )

	ROM_REGION( 0x1000, "mcu", 0 ) // created from dump of the US version
	ROM_LOAD( "last_mission_japan.18a", 0x0000, 0x1000, BAD_DUMP CRC(0d58c3a1) SHA1(184e75324b7ab2de8e6441f0c954046db80b2640) ) // correct ROM label when real MCU is dumped

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "last_mission_dl01-.2a",    0x00000, 0x2000, CRC(f3787a5d) SHA1(3701df42cb2aca951963703e72c6c7b272eed82b) )
	ROM_CONTINUE(                         0x06000, 0x2000 )
	ROM_CONTINUE(                         0x04000, 0x2000 )
	ROM_CONTINUE(                         0x02000, 0x2000 )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "last_mission_dl11-.13f",   0x00000, 0x08000, CRC(36579d3b) SHA1(8edf952dafcd5bc66e08074687f0bec809fd4c2f) )
	ROM_LOAD( "last_mission_dl12-.9f",    0x08000, 0x08000, CRC(2ba6737e) SHA1(c5e4c27726bf14e9cd60d62e2f17ea5be8093c37) )
	ROM_LOAD( "last_mission_dl13-.8f",    0x10000, 0x08000, CRC(39a7dc93) SHA1(3b7968fd06ac0379525c1d3e73f8bbe18ea36439) )
	ROM_LOAD( "last_mission_dl10-.16f",   0x18000, 0x08000, CRC(fe275ea8) SHA1(2f089f96583235f1f5226ef2a64b430d84efbeee) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "last_mission_dl09-.12k",   0x00000, 0x10000, CRC(6a5a0c5d) SHA1(0106cf693c284be5faf96e56b651fab92a410915) )
	ROM_LOAD( "last_mission_dl08-.14k",   0x10000, 0x10000, CRC(3b38cfce) SHA1(d6829bed6916fb301c08031bd466ee4dcc05b275) )
	ROM_LOAD( "last_mission_dl07-.15k",   0x20000, 0x10000, CRC(1b60604d) SHA1(1ee15cfdac87f7eeb92050766293b894cfad1466) )
	ROM_LOAD( "last_mission_dl06-.17k",   0x30000, 0x10000, CRC(c43c26a7) SHA1(896e278935b100edc12cd970469f2e8293eb96cc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dl-14.9c",    0x0000, 0x0100, CRC(2e55aa12) SHA1(c0f2b9649467eb9d2c1e47589b5990f5c5e8cc93) ) // Priority (Not yet used)
ROM_END

ROM_START( shackled )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dk-02.13h", 0x00000, 0x08000, CRC(87f8fa85) SHA1(1cb93a60eefdb453a3cc6ec9c5cc2e367fb8aeb0) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dk-06.7h",  0x00000, 0x10000, CRC(69ad62d1) SHA1(1aa23b12ab4f1908cddd25f091e1f9bd70a5113c) )
	ROM_LOAD( "dk-05.9h",  0x10000, 0x10000, CRC(598dd128) SHA1(10843c5352eef03c8675df6abaf23c9c9c795aa3) )
	ROM_LOAD( "dk-04.10h", 0x20000, 0x10000, CRC(36d305d4) SHA1(17586c316aff405cf20c1467d69c98fa2a3c2630) )
	ROM_LOAD( "dk-03.11h", 0x30000, 0x08000, CRC(6fd90fd1) SHA1(2f8db17e5545c82d243a7e23e7bda2c2a9101360) )
	ROM_RELOAD(            0x38000, 0x08000 )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dk-01.18h", 0x00000, 0x10000, CRC(71fe3bda) SHA1(959cce01362b2c670c2e15b03a78a1ff9cea4ee9) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dk-07.5h", 0x0000, 0x8000, CRC(887e4bcc) SHA1(6427396080e9cd8647adff47c8ed04593a14268c) )

	ROM_REGION( 0x1000, "mcu", 0 ) // ID8751H (fake) MCU (based on 'breywood' with ID byte changed from 00 to 01)
	ROM_LOAD( "dk-e.18a", 0x0000, 0x1000, CRC(1af06149) SHA1(b9cb2a4986dbcfc78b0cbea2c1e2bdac1db479cd) BAD_DUMP ) // Hand written "E"

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dk-00.2a", 0x00000, 0x08000, CRC(69b975aa) SHA1(38cb96768c79ff1aa1b4b190e08ec9155baf698a) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dk-12.15k", 0x00000, 0x10000, CRC(615c2371) SHA1(30b25dc27d34646d886a465c77622eaa894d83c3) )
	ROM_LOAD( "dk-13.14k", 0x10000, 0x10000, CRC(479aa503) SHA1(1167f0d15439c95a1094f81855203e863ce0488d) )
	ROM_LOAD( "dk-14.13k", 0x20000, 0x10000, CRC(cdc24246) SHA1(1a4189bc2b1fa99740dd7921608159936ba3bd07) )
	ROM_LOAD( "dk-15.11k", 0x30000, 0x10000, CRC(88db811b) SHA1(7d3c4a80925f323efb589798b4a341d1a2ca95f9) )
	ROM_LOAD( "dk-16.10k", 0x40000, 0x10000, CRC(061a76bd) SHA1(5bcb513e48bed9b7c4207d94531be691a85e295d) )
	ROM_LOAD( "dk-17.9k",  0x50000, 0x10000, CRC(a6c5d8af) SHA1(58f3fece9a5ef8b39090a2f39610381b8e7cdbf7) )
	ROM_LOAD( "dk-18.8k",  0x60000, 0x10000, CRC(4d466757) SHA1(701d79bebbba4f266e19080d16ff2f93ffa94287) )
	ROM_LOAD( "dk-19.6k",  0x70000, 0x10000, CRC(1911e83e) SHA1(174e9db3f2211ecbbb93c6bda8f6185dbfdbc818) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dk-11.12k", 0x00000, 0x10000, CRC(5cf5719f) SHA1(8c7582ac19010421ec748391a193aa18e51b981f) )
	ROM_LOAD( "dk-10.14k", 0x10000, 0x10000, CRC(408e6d08) SHA1(28cb76792e5f84bd101a91cb82597a5939804f84) )
	ROM_LOAD( "dk-09.15k", 0x20000, 0x10000, CRC(c1557fac) SHA1(7d39ec793113a48baf45c2ea07abb07e2e48985a) )
	ROM_LOAD( "dk-08.17k", 0x30000, 0x10000, CRC(5e54e9f5) SHA1(1ab41a3bde1f2c2be670e89cf402be28001c17d1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dk-20.9c", 0x0000, 0x0100, CRC(ff3cd588) SHA1(7360a9f046d517885d456d89026d047fb1fd8d5a) ) // Priority (Not yet used) BPROM type MB7052
ROM_END

ROM_START( breywood )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dj02-2.13h", 0x00000, 0x08000, CRC(c19856b9) SHA1(766994703bb59879c311675353d7231ad27c7c16) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dj06-2.7h",  0x00000, 0x10000, CRC(2860ea02) SHA1(7ac090c3ae9d71baa6227ec9555f1c9f2d25ea0d) )
	ROM_LOAD( "dj05-2.9h",  0x10000, 0x10000, CRC(0fdd915e) SHA1(262df956dfc727c710ade28af7f33fddaafd7ee2) )
	ROM_LOAD( "dj04-2.10h", 0x20000, 0x10000, CRC(71036579) SHA1(c58ff3222b5bcd75d58c5f282554e92103e80916) )
	ROM_LOAD( "dj03-2.11h", 0x30000, 0x08000, CRC(308f4893) SHA1(539c138ff01c5718cc8a982482b989468d532699) )
	ROM_RELOAD(             0x38000, 0x08000 )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dj1-2y.18h", 0x0000, 0x10000, CRC(3d9fb623) SHA1(6e5eaad9bb0a432e2da5da5b18a2ed36617bdde2) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dj07-1.5h", 0x0000, 0x8000, CRC(4a471c38) SHA1(963ed7b6afeefdfc2cf0d65b0998f973330e6495) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dj.18a", 0x0000, 0x1000, CRC(4cb20332) SHA1(e0bbba7be22e7bcff82fb0ae441410e559ec4566) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dj-00.2a",  0x00000, 0x08000, CRC(815a891a) SHA1(e557d6a35821a8589d9e3df0f42131b58b08c8ca) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dj12.15k", 0x00000, 0x10000, CRC(2b7634f2) SHA1(56d963d4960d9b3e888c8107340763e176adfa9b) )
	ROM_LOAD( "dj13.14k", 0x10000, 0x10000, CRC(4530a952) SHA1(99251a21347815cba465669e18df31262bcdaba1) )
	ROM_LOAD( "dj14.13k", 0x20000, 0x10000, CRC(87c28833) SHA1(3f1a294065326389d304e540bc880844c6c7cb06) )
	ROM_LOAD( "dj15.11k", 0x30000, 0x10000, CRC(bfb43a4d) SHA1(56092935147a3b643a9b39eb7cfc067a764644c5) )
	ROM_LOAD( "dj16.10k", 0x40000, 0x10000, CRC(f9848cc4) SHA1(6d8e77b67ce4d418defba6f6979632f31d2307c6) )
	ROM_LOAD( "dj17.9k",  0x50000, 0x10000, CRC(baa3d218) SHA1(3c31df23cc871cffd9a4dafae106e4a98f5af848) )
	ROM_LOAD( "dj18.8k",  0x60000, 0x10000, CRC(12afe533) SHA1(6df3471c16a714d118717da549a7523aa388ddd3) )
	ROM_LOAD( "dj19.6k",  0x70000, 0x10000, CRC(03373755) SHA1(d2541dd957803168f246d96b7cd74eae7fd43188) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dj11.12k", 0x00000, 0x10000, CRC(067e2a43) SHA1(f1da7455aab21f94ed25a93b0ebfde69baa475d1) )
	ROM_LOAD( "dj10.14k", 0x10000, 0x10000, CRC(c19733aa) SHA1(3dfcfd33c5c4f792bb941ac933301c03ddd72b03) )
	ROM_LOAD( "dj09.15k", 0x20000, 0x10000, CRC(e37d5dbe) SHA1(ff79b4f6d8b0a3061e78d15480df0155650f347f) )
	ROM_LOAD( "dj08.17k", 0x30000, 0x10000, CRC(beee880f) SHA1(9a818a75cbec425a13f629bda6d50aa341aa1896) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dk-20.9c", 0x0000, 0x0100, CRC(ff3cd588) SHA1(7360a9f046d517885d456d89026d047fb1fd8d5a) ) // Priority (Not yet used) BPROM type MB7052
ROM_END

/*

Captain Silver

Main Components
---------------

Top board (DATA EAST DE-0250-3):
2x MC68B09EP (18e,19e)(main)
1x RP65C02A (3f)(sound)
1x YM3812 (1e)(sound)
1x YM2203 (1f)(sound)
2x Y30148 (1j,2j)(sound)
1x OKI M5205 (3j)(sound)
1x NEC PC3403C (1j)(sound)
1x C4558C (2j)(sound)
1x oscillator 8.000 (x1)
1x ID8751H (read protected)

Lower board (DATA EAST DE-0251-2):
1x DECO TC15G032AY-0013-8644a-DSPC10 (square component, with 135 pass-through pins)(14h)
1x DECO VSC30-M60348-6102 (DIL40)(9a)
1x DECO HMC20-M60232-6902 (DIL28)(14a)
1x oscillator 12.000 (x1)

ROMs
----

Top board (DATA EAST DE-0250-3):
2x MBM27256 (00,03)
10x MBM27C512 (01,02,04,05,06,07,08,09,10,11)
1x MB7122 (DIL18) (15)

Lower board (DATA EAST DE-0251-2):
3x MBM27C512

Notes
-----

Top board (DATA EAST DE-0250-3):
1x JAMMA edge connector
2x 25x2 legs connectors to lower board (cn1,cn2)
1x trimmer (volume)
2x 8 switches DIP (7k,16k)

Lower board (DATA EAST DE-0251-2):
2x 25x2 legs connectors to top board (cn1,cn2)

*/

ROM_START( csilver )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dx03-12.18d", 0x00000, 0x08000, CRC(2d926e7c) SHA1(cf38e92904edb1870b0a4965f9049d67efe8cf6a) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "dx01.12d", 0x00000, 0x10000, CRC(570fb50c) SHA1(3002f53182834a060fc282be1bc5767906e19ba2) )
	ROM_LOAD( "dx02.13d", 0x10000, 0x10000, CRC(58625890) SHA1(503a969085f6dcb16687217c48136ea22d07c89f) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dx04-1.19d", 0x0000, 0x10000,  CRC(29432691) SHA1(a76ecd27d217c66a0e43f93e29efe83c657925c3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dx05.3f", 0x00000, 0x10000,  CRC(eb32cf25) SHA1(9390c88033259c65eb15320e31f5d696970987cc) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	// 017F: B4 4C 0D : cjne  a,#$4C,$018F (ID code 0x4c = World version)
	ROM_LOAD( "dx-8.19a", 0x0000, 0x1000, CRC(c0266263) SHA1(27ac6fa4af7195f04249c04dec168ab82158704e) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dx00.3d",  0x00000, 0x08000, CRC(f01ef985) SHA1(d5b823bd7c0efcf3137f8643c5d99a260bed5675) )

	ROM_REGION( 0x40000, "sprites", 0 ) // 3bpp
	ROM_LOAD( "dx14.15k",  0x00000, 0x10000, CRC(80f07915) SHA1(ea100f12ef3a68110af911fa9beeb73b388f069d) )
	ROM_LOAD( "dx13.13k",  0x10000, 0x10000, CRC(d32c02e7) SHA1(d0518ec31e9e3f7b4e76fba5d7c05c33c61a9c72) )
	ROM_LOAD( "dx12.10k",  0x20000, 0x10000, CRC(ac78b76b) SHA1(c2be347fd950894401123ada8b27bfcfce53e66b) )
	// 0x30000-0x3ffff empty (no 4th plane)

	ROM_REGION( 0x80000, "tiles", 0 ) // 3bpp
	ROM_LOAD( "dx06.5f",  0x00000, 0x10000, CRC(b6fb208c) SHA1(027d33f0b5feb6f0433134213cfcef96790eaace) )
	ROM_LOAD( "dx07.7f",  0x10000, 0x10000, CRC(ee3e1817) SHA1(013496976a9ffacf1587b3a6fc0f548becb1ab0e) )
	ROM_LOAD( "dx08.8f",  0x20000, 0x10000, CRC(705900fe) SHA1(53b9d09f9780a3bf3545bc27a2855ebee3884124) )
	ROM_LOAD( "dx09.10f", 0x30000, 0x10000, CRC(3192571d) SHA1(240c6c099f1e6edbf0be7d5a4ec396b056c9f70f) )
	ROM_LOAD( "dx10.12f", 0x40000, 0x10000, CRC(3ef77a32) SHA1(97b97c35a6ca994d2e7a6e7a63101eda9709bcb1) )
	ROM_LOAD( "dx11.13f", 0x50000, 0x10000, CRC(9cf3d5b8) SHA1(df4974f8412ab1cf65871b8e4e3dbee478bf4d21) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "dx-15.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for priority (Not yet used), location on alternate board unknown
ROM_END

// There is known to exist an identical ROM set with different PCB locations designated for an alternate ROM board (noted on the right of the ROM definition)
ROM_START( csilverj )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dx03-2.18d", 0x00000, 0x08000, CRC(02dd8cfc) SHA1(f29c0d9dd03e8c52672c0f3dbee44a93c5b4261d) ) // dx03-3.a4 (Different ROM label but identical to dx03-2.18d)

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "dx01.12d",   0x00000, 0x10000, CRC(570fb50c) SHA1(3002f53182834a060fc282be1bc5767906e19ba2) ) // dx01.a2
	ROM_LOAD( "dx02.13d",   0x10000, 0x10000, CRC(58625890) SHA1(503a969085f6dcb16687217c48136ea22d07c89f) ) // dx01.a3

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dx04-1.19d", 0x0000, 0x10000,  CRC(29432691) SHA1(a76ecd27d217c66a0e43f93e29efe83c657925c3) ) // dx04-1.a5

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dx05.3f", 0x00000, 0x10000,  CRC(eb32cf25) SHA1(9390c88033259c65eb15320e31f5d696970987cc) ) // dx05.a6

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	// hand modified version of csilver ROM
	ROM_LOAD( "id8751h_japan.mcu", 0x0000, 0x1000, BAD_DUMP CRC(6e801217) SHA1(2d8f7ae533dd8146acf8461d61ddd839544adf55) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dx00.3d",  0x00000, 0x08000, CRC(f01ef985) SHA1(d5b823bd7c0efcf3137f8643c5d99a260bed5675) ) // dx00.a1

	ROM_REGION( 0x40000, "sprites", 0 ) // 3bpp
	ROM_LOAD( "dx14.15k",  0x00000, 0x10000, CRC(80f07915) SHA1(ea100f12ef3a68110af911fa9beeb73b388f069d) ) // dx14.b5
	ROM_LOAD( "dx13.13k",  0x10000, 0x10000, CRC(d32c02e7) SHA1(d0518ec31e9e3f7b4e76fba5d7c05c33c61a9c72) ) // dx13.b4
	ROM_LOAD( "dx12.10k",  0x20000, 0x10000, CRC(ac78b76b) SHA1(c2be347fd950894401123ada8b27bfcfce53e66b) ) // dx12.b3
	// 0x30000-0x3ffff empty (no 4th plane)

	ROM_REGION( 0x80000, "tiles", 0 ) // 3bpp
	ROM_LOAD( "dx06.5f",  0x00000, 0x10000, CRC(b6fb208c) SHA1(027d33f0b5feb6f0433134213cfcef96790eaace) ) // dx06.a7
	ROM_LOAD( "dx07.7f",  0x10000, 0x10000, CRC(ee3e1817) SHA1(013496976a9ffacf1587b3a6fc0f548becb1ab0e) ) // dx07.a8
	ROM_LOAD( "dx08.8f",  0x20000, 0x10000, CRC(705900fe) SHA1(53b9d09f9780a3bf3545bc27a2855ebee3884124) ) // dx08.a9
	ROM_LOAD( "dx09.10f", 0x30000, 0x10000, CRC(3192571d) SHA1(240c6c099f1e6edbf0be7d5a4ec396b056c9f70f) ) // dx09.a10
	ROM_LOAD( "dx10.12f", 0x40000, 0x10000, CRC(3ef77a32) SHA1(97b97c35a6ca994d2e7a6e7a63101eda9709bcb1) ) // dx10.b1
	ROM_LOAD( "dx11.13f", 0x50000, 0x10000, CRC(9cf3d5b8) SHA1(df4974f8412ab1cf65871b8e4e3dbee478bf4d21) ) // dx11.b2

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "dx-15.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for priority (Not yet used), location on alternate board unknown
ROM_END

ROM_START( csilverja ) // DE-0250-3 + DE-0251-2
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dx03-1.18d", 0x00000, 0x08000, CRC(d42905be) SHA1(5a406466aa9bb2b2591d02fc87289cb93f7358c6) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "dx01.12d",   0x00000, 0x10000, CRC(570fb50c) SHA1(3002f53182834a060fc282be1bc5767906e19ba2) )
	ROM_LOAD( "dx02.13d",   0x10000, 0x10000, CRC(58625890) SHA1(503a969085f6dcb16687217c48136ea22d07c89f) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dx04-1.19d", 0x0000, 0x10000,  CRC(29432691) SHA1(a76ecd27d217c66a0e43f93e29efe83c657925c3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dx05.3f", 0x00000, 0x10000,  CRC(eb32cf25) SHA1(9390c88033259c65eb15320e31f5d696970987cc) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	// hand modified version of csilver ROM
	ROM_LOAD( "id8751h_japan.mcu", 0x0000, 0x1000, BAD_DUMP CRC(6e801217) SHA1(2d8f7ae533dd8146acf8461d61ddd839544adf55) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dx00.3d",  0x00000, 0x08000, CRC(f01ef985) SHA1(d5b823bd7c0efcf3137f8643c5d99a260bed5675) )

	ROM_REGION( 0x40000, "sprites", 0 ) // 3bpp
	ROM_LOAD( "dx14.15k",  0x00000, 0x10000, CRC(80f07915) SHA1(ea100f12ef3a68110af911fa9beeb73b388f069d) )
	ROM_LOAD( "dx13.13k",  0x10000, 0x10000, CRC(d32c02e7) SHA1(d0518ec31e9e3f7b4e76fba5d7c05c33c61a9c72) )
	ROM_LOAD( "dx12.10k",  0x20000, 0x10000, CRC(ac78b76b) SHA1(c2be347fd950894401123ada8b27bfcfce53e66b) )
	// 0x30000-0x3ffff empty (no 4th plane)

	ROM_REGION( 0x80000, "tiles", 0 ) // 3bpp
	ROM_LOAD( "dx06.5f",  0x00000, 0x10000, CRC(b6fb208c) SHA1(027d33f0b5feb6f0433134213cfcef96790eaace) )
	ROM_LOAD( "dx07.7f",  0x10000, 0x10000, CRC(ee3e1817) SHA1(013496976a9ffacf1587b3a6fc0f548becb1ab0e) )
	ROM_LOAD( "dx08.8f",  0x20000, 0x10000, CRC(705900fe) SHA1(53b9d09f9780a3bf3545bc27a2855ebee3884124) )
	ROM_LOAD( "dx09.10f", 0x30000, 0x10000, CRC(3192571d) SHA1(240c6c099f1e6edbf0be7d5a4ec396b056c9f70f) )
	ROM_LOAD( "dx10.12f", 0x40000, 0x10000, CRC(3ef77a32) SHA1(97b97c35a6ca994d2e7a6e7a63101eda9709bcb1) )
	ROM_LOAD( "dx11.13f", 0x50000, 0x10000, CRC(9cf3d5b8) SHA1(df4974f8412ab1cf65871b8e4e3dbee478bf4d21) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "dx-15.b10", 0x0000, 0x400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for priority (Not yet used), location on alternate board unknown
ROM_END


/******************************************************************************/

} // anonymous namespace

GAME( 1986, lastmisn,   0,        lastmisn,  lastmisn,  lastmisn_state, empty_init,     ROT270, "Data East Corporation", "Last Mission (World revision 8)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lastmisnu6, lastmisn, lastmisn,  lastmisn,  lastmisn_state, empty_init,     ROT270, "Data East USA",         "Last Mission (US revision 6)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lastmisnu5, lastmisn, lastmisn,  lastmisn,  lastmisn_state, empty_init,     ROT270, "Data East USA",         "Last Mission (US revision 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lastmisnj,  lastmisn, lastmisn,  lastmisnj, lastmisn_state, empty_init,     ROT270, "Data East Corporation", "Last Mission (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, shackled,   0,        shackled,  shackled,  lastmisn_state, empty_init,     ROT0,   "Data East USA",         "Shackled (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, breywood,   shackled, shackled,  breywood,  lastmisn_state, empty_init,     ROT0,   "Data East Corporation", "Breywood (Japan revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, csilver,    0,        csilver,   csilver,   csilver_state,  empty_init,     ROT0,   "Data East Corporation", "Captain Silver (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, csilverj,   csilver,  csilver,   csilverj,  csilver_state,  empty_init,     ROT0,   "Data East Corporation", "Captain Silver (Japan, revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, csilverja,  csilver,  csilver,   csilver,   csilver_state,  empty_init,     ROT0,   "Data East Corporation", "Captain Silver (Japan, revision 1)", MACHINE_SUPPORTS_SAVE )
