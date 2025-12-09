// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    The FairyLand Story

    added Victorious Nine by BUT

    TODO:
    - TA7630 emulation needs filter support (bass sounds from MSM5232 should
      be about 2 times louder)

***************************************************************************/

#include "emu.h"

#include "taito68705.h"

#include "cpu/m6805/m6805.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"
#include "sound/ta7630.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class flstory_state : public driver_device
{
public:
	flstory_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scrlram(*this, "scrlram"),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_ay(*this, "aysnd"),
		m_ta7630(*this, "ta7630"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundnmi(*this, "soundnmi"),
		m_extraio1(*this, "EXTRA_P1")
	{ }

	void onna34ro(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrlram;
	optional_shared_ptr<uint8_t> m_workram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5232_device> m_msm;
	required_device<ay8910_device> m_ay;
	required_device<ta7630_device> m_ta7630;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<input_merger_device> m_soundnmi;

	optional_ioport m_extraio1;

	// video-related
	tilemap_t  *m_bg_tilemap = nullptr;
	std::unique_ptr<uint8_t []> m_paletteram;
	std::unique_ptr<uint8_t []> m_paletteram_ext;
	uint8_t    m_gfxctrl = 0;
	uint8_t    m_char_bank = 0;
	uint8_t    m_palette_bank = 0;

	// sound-related
	uint8_t    m_snd_ctrl0 = 0;
	uint8_t    m_snd_ctrl1 = 0;
	uint8_t    m_snd_ctrl2 = 0;
	uint8_t    m_snd_ctrl3 = 0;

	uint8_t snd_flag_r();
	void snd_reset_w(uint8_t data);
	void flstory_videoram_w(offs_t offset, uint8_t data);
	void flstory_palette_w(offs_t offset, uint8_t data);
	uint8_t flstory_palette_r(offs_t offset);
	void flstory_gfxctrl_w(uint8_t data);
	void flstory_scrlram_w(offs_t offset, uint8_t data);
	void sound_control_0_w(uint8_t data);
	void sound_control_1_w(uint8_t data);
	void sound_control_2_w(uint8_t data);
	void sound_control_3_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_VIDEO_START(flstory);
	uint32_t screen_update_flstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void flstory_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void common(machine_config &config) ATTR_COLD;

	void base_map(address_map &map) ATTR_COLD;
	void onna34ro_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


class flstory_mcu_state : public flstory_state
{
public:
	flstory_mcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		flstory_state(mconfig, type, tag),
		m_bmcu(*this, "bmcu")
	{ }

	void flstory(machine_config &config) ATTR_COLD;
	void rumba(machine_config &config) ATTR_COLD;
	void victnine(machine_config &config) ATTR_COLD;
	void onna34ro_mcu(machine_config &config) ATTR_COLD;

private:
	// devices
	required_device<taito68705_mcu_device> m_bmcu;

	uint8_t flstory_mcu_status_r();
	uint8_t victnine_mcu_status_r();
	uint8_t victnine_gfxctrl_r();
	void victnine_gfxctrl_w(uint8_t data);
	TILE_GET_INFO_MEMBER(victnine_get_tile_info);
	TILE_GET_INFO_MEMBER(get_rumba_tile_info);
	DECLARE_VIDEO_START(victnine);
	DECLARE_VIDEO_START(rumba);
	uint32_t screen_update_victnine(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rumba(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void victnine_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void flstory_map(address_map &map) ATTR_COLD;
	void onna34ro_mcu_map(address_map &map) ATTR_COLD;
	void rumba_map(address_map &map) ATTR_COLD;
	void victnine_map(address_map &map) ATTR_COLD;
};


/***************************************************************************
  Functions to emulate the video hardware of the machine.
***************************************************************************/

TILE_GET_INFO_MEMBER(flstory_state::get_tile_info)
{
	uint8_t const code = m_videoram[tile_index * 2];
	uint8_t const attr = m_videoram[tile_index * 2 + 1];
	uint32_t const tile_number = code + ((attr & 0xc0) << 2) + 0x400 + 0x800 * m_char_bank;
	uint8_t const flags = TILE_FLIPYX((attr & 0x18) >> 3);

	tileinfo.category = BIT(attr, 5);
	tileinfo.group = BIT(attr, 5);
	tileinfo.set(0, tile_number, attr & 0x0f, flags);
}

TILE_GET_INFO_MEMBER(flstory_mcu_state::victnine_get_tile_info)
{
	uint8_t const code = m_videoram[tile_index * 2];
	uint8_t const attr = m_videoram[tile_index * 2 + 1];
	uint32_t const tile_number = ((attr & 0x38) << 5) + code;
	uint8_t const flags = (BIT(attr, 6) ? TILE_FLIPX : 0) | (BIT(attr, 7) ? TILE_FLIPY : 0);

	tileinfo.set(0, tile_number, attr & 0x07, flags);
}

TILE_GET_INFO_MEMBER(flstory_mcu_state::get_rumba_tile_info)
{
	uint8_t const code = m_videoram[tile_index * 2];
	uint8_t const attr = m_videoram[tile_index * 2 + 1];
	uint32_t const tile_number = code + ((attr & 0xc0) << 2) + 0x400 + 0x800 * m_char_bank;
	uint32_t const col = (attr & 0x0f);

	tileinfo.category = BIT(attr, 5);
	tileinfo.group = BIT(attr, 5);
	tileinfo.set(0, tile_number, col, 0);
}

VIDEO_START_MEMBER(flstory_state,flstory)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(flstory_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
//  m_bg_tilemap->set_transparent_pen(15);
	m_bg_tilemap->set_transmask(0, 0x3fff, 0xc000); /* split type 0 has pens 0-13 transparent in front half */
	m_bg_tilemap->set_transmask(1, 0x8000, 0x7fff); /* split type 1 has pen 15 transparent in front half */
	m_bg_tilemap->set_scroll_cols(32);

	m_paletteram = make_unique_clear<uint8_t []>(m_palette->entries());
	m_paletteram_ext = make_unique_clear<uint8_t []>(m_palette->entries());
	m_palette->basemem().set(m_paletteram.get(), m_palette->entries(), 8, ENDIANNESS_LITTLE, 1);
	m_palette->extmem().set(m_paletteram_ext.get(), m_palette->entries(), 8, ENDIANNESS_LITTLE, 1);

	save_pointer(NAME(m_paletteram), m_palette->entries());
	save_pointer(NAME(m_paletteram_ext), m_palette->entries());
}

VIDEO_START_MEMBER(flstory_mcu_state,rumba)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(flstory_mcu_state::get_rumba_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
//  m_bg_tilemap->set_transparent_pen(15);
	m_bg_tilemap->set_transmask(0, 0x3fff, 0xc000); /* split type 0 has pens 0-13 transparent in front half */
	m_bg_tilemap->set_transmask(1, 0x8000, 0x7fff); /* split type 1 has pen 15 transparent in front half */
	m_bg_tilemap->set_scroll_cols(32);

	m_paletteram = make_unique_clear<uint8_t []>(m_palette->entries());
	m_paletteram_ext = make_unique_clear<uint8_t []>(m_palette->entries());
	m_palette->basemem().set(m_paletteram.get(), m_palette->entries(), 8, ENDIANNESS_LITTLE, 1);
	m_palette->extmem().set(m_paletteram_ext.get(), m_palette->entries(), 8, ENDIANNESS_LITTLE, 1);

	save_pointer(NAME(m_paletteram), m_palette->entries());
	save_pointer(NAME(m_paletteram_ext), m_palette->entries());
}

VIDEO_START_MEMBER(flstory_mcu_state,victnine)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(flstory_mcu_state::victnine_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_cols(32);

	m_paletteram = make_unique_clear<uint8_t []>(m_palette->entries());
	m_paletteram_ext = make_unique_clear<uint8_t []>(m_palette->entries());
	m_palette->basemem().set(m_paletteram.get(), m_palette->entries(), 8, ENDIANNESS_LITTLE, 1);
	m_palette->extmem().set(m_paletteram_ext.get(), m_palette->entries(), 8, ENDIANNESS_LITTLE, 1);

	save_pointer(NAME(m_paletteram), m_palette->entries());
	save_pointer(NAME(m_paletteram_ext), m_palette->entries());
}

void flstory_state::flstory_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void flstory_state::flstory_palette_w(offs_t offset, uint8_t data)
{
	if (offset & 0x100)
		m_palette->write8_ext((offset & 0xff) + (m_palette_bank << 8), data);
	else
		m_palette->write8((offset & 0xff) + (m_palette_bank << 8), data);
}

uint8_t flstory_state::flstory_palette_r(offs_t offset)
{
	if (offset & 0x100)
		return m_paletteram_ext[(offset & 0xff) + (m_palette_bank << 8)];
	else
		return m_paletteram[(offset & 0xff) + (m_palette_bank << 8)];
}

void flstory_state::flstory_gfxctrl_w(uint8_t data)
{
	m_gfxctrl = data;

	flip_screen_set(BIT(~data, 0));
	if (m_char_bank != ((data & 0x10) >> 4))
	{
		m_char_bank = (data & 0x10) >> 4;
		m_bg_tilemap->mark_all_dirty();
	}
	m_palette_bank = (data & 0x20) >> 5;
}

uint8_t flstory_mcu_state::victnine_gfxctrl_r()
{
	return m_gfxctrl;
}

void flstory_mcu_state::victnine_gfxctrl_w(uint8_t data)
{
	m_gfxctrl = data;

	m_palette_bank = (data & 0x20) >> 5;

	if (BIT(data, 2))
		flip_screen_set(BIT(data, 0));
}

void flstory_state::flstory_scrlram_w(offs_t offset, uint8_t data)
{
	m_scrlram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}


void flstory_state::flstory_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool const flip = flip_screen();

	for (int i = 0x1f; i >= 0; i--)
	{
		uint8_t const pr = m_spriteram[m_spriteram.bytes() - 1 - i];
		uint32_t const offs = (pr & 0x1f) * 4;

		uint32_t const pri_mask = BIT(pr, 7) ? GFX_PMASK_8 : (GFX_PMASK_8 | GFX_PMASK_4);

		uint32_t const code = m_spriteram[offs + 2] + ((m_spriteram[offs + 1] & 0x30) << 4);
		int sx = m_spriteram[offs + 3];
		int sy = m_spriteram[offs + 0];

		bool flipx = BIT(m_spriteram[offs + 1], 6);
		bool flipy = BIT(m_spriteram[offs + 1], 7);

		if (flip)
		{
			sx = (240 - sx) & 0xff;
			sy = sy - 1;
			flipx = !flipx;
			flipy = !flipy;
		}
		else
			sy = 240 - sy - 1;

		m_gfxdecode->gfx(1)->prio_transpen(bitmap, cliprect,
				code,
				m_spriteram[offs + 1] & 0x0f,
				flipx, flipy,
				sx, sy,
				screen.priority(), pri_mask, 15);
		/* wrap around */
		if (sx > 240)
			m_gfxdecode->gfx(1)->prio_transpen(bitmap, cliprect,
					code,
					m_spriteram[offs + 1] & 0x0f,
					flipx, flipy,
					sx-256, sy,
					screen.priority(), pri_mask, 15);
	}
}

uint32_t flstory_state::screen_update_flstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER1, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER1, 2);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER0, 4);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER0, 8);
	flstory_draw_sprites(screen, bitmap, cliprect);

	return 0;
}

void flstory_mcu_state::victnine_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool const flip = flip_screen();

	for (int i = 0; i < 0x20; i++)
	{
		uint8_t const pr = m_spriteram[m_spriteram.bytes() - 1 - i];
		uint32_t const offs = (pr & 0x1f) * 4;

		//if ((pr & 0x80) == pri)
		{
			uint32_t const code = m_spriteram[offs + 2] + ((m_spriteram[offs + 1] & 0x20) << 3);
			int sx = m_spriteram[offs + 3];
			int sy = m_spriteram[offs + 0];

			bool flipx = BIT(m_spriteram[offs + 1], 6);
			bool flipy = BIT(m_spriteram[offs + 1], 7);

			if (flip)
			{
				sx = (240 - sx + 1) & 0xff;
				sy = sy + 1;
				flipx = !flipx;
				flipy = !flipy;
			}
			else
				sy = 240 - sy + 1;

			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					code,
					m_spriteram[offs + 1] & 0x0f,
					flipx, flipy,
					sx, sy, 15);
			/* wrap around */
			if (sx > 240)
				m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
						code,
						m_spriteram[offs + 1] & 0x0f,
						flipx, flipy,
						sx - 256, sy, 15);
		}
	}
}

uint32_t flstory_mcu_state::screen_update_victnine(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	victnine_draw_sprites(bitmap, cliprect);
	return 0;
}

uint32_t flstory_mcu_state::screen_update_rumba(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER1, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER1, 0);
	victnine_draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER0, 0);
	victnine_draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER0, 0);
	return 0;
}


uint8_t flstory_state::snd_flag_r()
{
	return (m_soundlatch->pending_r() ? 0 : 1) | (m_soundlatch2->pending_r() ? 2 : 0);
}

void flstory_state::snd_reset_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 1 ) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t flstory_mcu_state::flstory_mcu_status_r()
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}


uint8_t flstory_mcu_state::victnine_mcu_status_r()
{
	uint8_t ret = flstory_mcu_status_r() & 0x03;
	ret |= m_extraio1->read() & 0xfc;
	return ret;
}

void flstory_state::base_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();

	// rumba lumber reads area 0xc800-0xcfff
	// onna34ro checks the whole range during POST but having a mirror or not doesn't make any difference for the check to pass
	map(0xc000, 0xc7ff).mirror(0x800).ram().w(FUNC(flstory_state::flstory_videoram_w)).share(m_videoram);

	map(0xd001, 0xd001).nopw();    // watchdog
	map(0xd002, 0xd002).noprw();   // unknown read & coin lock out?

	map(0xd400, 0xd400).r(m_soundlatch2, FUNC(generic_latch_8_device::read));
	map(0xd400, 0xd400).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xd403, 0xd403).nopr().w(FUNC(flstory_state::snd_reset_w)); // unknown read (set/clr side effect?)

	map(0xd401, 0xd401).r(FUNC(flstory_state::snd_flag_r));
	map(0xd800, 0xd800).portr("SWA");
	map(0xd801, 0xd801).portr("SWB");
	map(0xd802, 0xd802).portr("SWC");
	map(0xd803, 0xd803).portr("SYSTEM");
	map(0xd804, 0xd804).portr("P1");
	map(0xd806, 0xd806).portr("P2");

	map(0xdc00, 0xdc9f).ram().share(m_spriteram);
	map(0xdca0, 0xdcbf).ram().w(FUNC(flstory_state::flstory_scrlram_w)).share(m_scrlram);

	map(0xdd00, 0xdeff).rw(FUNC(flstory_state::flstory_palette_r), FUNC(flstory_state::flstory_palette_w));

	// victorious nine read 0xf80a during attract, unknown purpose
	map(0xe000, 0xe7ff).mirror(0x1800).ram().share(m_workram);
}

void flstory_mcu_state::flstory_map(address_map &map)
{
	base_map(map);
	map(0xd000, 0xd000).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));

	map(0xd805, 0xd805).r(FUNC(flstory_mcu_state::flstory_mcu_status_r));
//  map(0xda00, 0xda00).writeonly();
	map(0xdcc0, 0xdcff).ram(); // unknown
	map(0xdf03, 0xdf03).w(FUNC(flstory_mcu_state::flstory_gfxctrl_w));
}

void flstory_state::onna34ro_map(address_map &map)
{
	base_map(map);
//  map(0xd000, 0xd000).rw("bmcu", FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
//  map(0xd805, 0xd805).r(FUNC(flstory_state::flstory_mcu_status_r));
//  map(0xda00, 0xda00).writeonly();
	map(0xdcc0, 0xdcff).ram(); // unknown
	map(0xdf03, 0xdf03).w(FUNC(flstory_state::flstory_gfxctrl_w));
}

void flstory_mcu_state::onna34ro_mcu_map(address_map &map)
{
	onna34ro_map(map);
	map(0xd000, 0xd000).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0xd805, 0xd805).r(FUNC(flstory_mcu_state::flstory_mcu_status_r));
}

void flstory_mcu_state::victnine_map(address_map &map)
{
	base_map(map);
	map(0xd000, 0xd000).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));

	map(0xd805, 0xd805).r(FUNC(flstory_mcu_state::victnine_mcu_status_r));
	map(0xd807, 0xd807).portr("EXTRA_P2");
//  map(0xda00, 0xda00).writeonly();
	map(0xdce0, 0xdce0).rw(FUNC(flstory_mcu_state::victnine_gfxctrl_r), FUNC(flstory_mcu_state::victnine_gfxctrl_w));
	map(0xdce1, 0xdce1).nopw();    // unknown
}

void flstory_mcu_state::rumba_map(address_map &map)
{
	base_map(map);
	map(0xd000, 0xd000).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));

	map(0xd805, 0xd805).r(FUNC(flstory_mcu_state::flstory_mcu_status_r));
	map(0xd807, 0xd807).portr("EXTRA_P2");
//  map(0xda00, 0xda00).writeonly();
	map(0xdce0, 0xdce0).rw(FUNC(flstory_mcu_state::victnine_gfxctrl_r), FUNC(flstory_mcu_state::victnine_gfxctrl_w));
//  map(0xdce1, 0xdce1).nopw();    // unknown
}



void flstory_state::sound_control_0_w(uint8_t data)
{
	m_snd_ctrl0 = data & 0xff;
	//popmessage("SND0 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);

	// this definitely controls main melody voice on 2'-1 and 4'-1 outputs
	for(int i=0;i<4;i++)
		m_ta7630->set_channel_volume(m_msm,i,m_snd_ctrl0 >> 4);
	//m_msm->set_output_gain(0, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */
	//m_msm->set_output_gain(1, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */
	//m_msm->set_output_gain(2, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */
	//m_msm->set_output_gain(3, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */

}
void flstory_state::sound_control_1_w(uint8_t data)
{
	m_snd_ctrl1 = data & 0xff;
	//popmessage("SND1 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);
	for(int i=0;i<4;i++)
		m_ta7630->set_channel_volume(m_msm,i+4,m_snd_ctrl1 >> 4);

	//m_msm->set_output_gain(4, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
	//m_msm->set_output_gain(5, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
	//m_msm->set_output_gain(6, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
	//m_msm->set_output_gain(7, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
}

void flstory_state::sound_control_2_w(uint8_t data)
{
	m_snd_ctrl2 = data & 0xff;
	//  popmessage("SND2 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);

	m_ta7630->set_device_volume(m_ay,m_snd_ctrl2 >> 4);
}

// ta7630 bass / treble for AY?
void flstory_state::sound_control_3_w(uint8_t data)
{
	m_snd_ctrl3 = data & 0xff;
	//  popmessage("SND3 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);
}


void flstory_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc801).w(m_ay, FUNC(ym2149_device::address_data_w));
	map(0xca00, 0xca0d).w(m_msm, FUNC(msm5232_device::write));
	map(0xcc00, 0xcc00).w(FUNC(flstory_state::sound_control_0_w));
	map(0xce00, 0xce00).w(FUNC(flstory_state::sound_control_1_w));
	map(0xd800, 0xd800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xd800, 0xd800).w(m_soundlatch2, FUNC(generic_latch_8_device::write));
	map(0xda00, 0xda00).r(FUNC(flstory_state::snd_flag_r)).w(m_soundnmi, FUNC(input_merger_device::in_set<1>));
	map(0xdc00, 0xdc00).w(m_soundnmi, FUNC(input_merger_device::in_clear<1>));
	map(0xde00, 0xde00).nopr().w("dac", FUNC(dac_byte_interface::data_w)); // signed 8-bit DAC &  unknown read
	map(0xe000, 0xefff).rom();                                         // space for diagnostics ROM
}


/* When "Debug Mode" DIP Switch is ON, keep IPT_SERVICE1 ('9') pressed to freeze the game.
   Once the game is frozen, you can press IPT_START1 ('5') to advance 1 frame, or IPT_START2
   ('6') to advance 6 frames.

   When "Continue" Dip Switch is ON, you can only continue in a 1 player game AND when level
   (0xe781) is between 8 and 98 (included).
*/

static INPUT_PORTS_START( flstory )
	PORT_START("SWA")      /*D800*/
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x00, "30000 100000" )
	PORT_DIPSETTING(    0x01, "30000 150000" )
	PORT_DIPSETTING(    0x02, "50000 150000" )
	PORT_DIPSETTING(    0x03, "70000 150000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x20, 0x20, "Debug Mode" )            PORT_DIPLOCATION("SWA:6")  // Check code at 0x0679
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("SWB")      /*D801*/
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWB:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWB:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("SWC")      /* D802 */
	PORT_DIPUNUSED_DIPLOC( 0x07, IP_ACTIVE_HIGH, "SWC:1,2,3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWC:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Attract Animation" )         PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Leave Off")                  PORT_DIPLOCATION("SWC:6")   // Check code at 0x7859
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )                                          // must be OFF or the game will
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )                                           // hang after the game is over !
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)" )   PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )                PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("SYSTEM")      /* D803 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // "BAD IO" if low
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // "BAD IO" if low

	PORT_START("P1")      /* D804 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")      /* D806 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( onna34ro )
	PORT_START("SWA")      /* D800*/
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Bonus_Life ) )    PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(   0x00, "200000 200000" )
	PORT_DIPSETTING(   0x01, "200000 300000" )
	PORT_DIPSETTING(   0x02, "100000 200000" )
	PORT_DIPSETTING(   0x03, "200000 100000" )
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Free_Play ) )     PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x04, DEF_STR( On ) )
	PORT_DIPNAME(0x18, 0x00, DEF_STR( Lives ) )         PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(   0x10, "1" )
	PORT_DIPSETTING(   0x08, "2" )
	PORT_DIPSETTING(   0x00, "3" )
	PORT_DIPSETTING(   0x18, "Endless (Cheat)")
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(   0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x40, DEF_STR( Flip_Screen ) )   PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(   0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Cabinet ) )       PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(   0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ) )

	PORT_START("SWB")      /* D801 */
	PORT_DIPNAME(0x0f, 0x00, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("SWB:1,2,3,4")
	PORT_DIPSETTING(   0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(   0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(   0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(   0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(   0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(   0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(   0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(   0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(   0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(   0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(   0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME(0xf0, 0x00, DEF_STR( Coin_B ) )        PORT_DIPLOCATION("SWB:5,6,7,8")
	PORT_DIPSETTING(   0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(   0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(   0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(   0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(   0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(   0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(   0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(   0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(   0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(   0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(   0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(   0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(   0x70, DEF_STR( 1C_8C ) )

	PORT_START("SWC")      /* D802 */
	PORT_DIPNAME(0x01, 0x00, "Invulnerability (Cheat)") PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x01, DEF_STR( On ) )
	PORT_DIPNAME(0x02, 0x00, "Rack Test" )              PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x02, DEF_STR( On ) )
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SWC:3")
	PORT_DIPSETTING(   0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x08, 0x00, "Freeze" )                 PORT_DIPLOCATION("SWC:4")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x08, DEF_STR( On ) )
	PORT_DIPNAME(0x10, 0x00, "Coinage Display" )        PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(   0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x60, 0x00, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("SWC:6,7")
	PORT_DIPSETTING(   0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(   0x60, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Coinage ) )       PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(   0x80, "A and B" )
	PORT_DIPSETTING(   0x00, "A only" )

	PORT_START("SYSTEM")      /* D803 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* "BAD IO" if low */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* "BAD IO" if low */

	PORT_START("P1")      /* D804 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")      /* D806 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( victnine )
	PORT_START("SWA")      /* D800 */
	PORT_DIPUNUSED_DIPLOC( 0x03, IP_ACTIVE_LOW, "SWA:1,2" )  // manual says 'No Use Set OFF'
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x18, IP_ACTIVE_LOW, "SWA:4,5" )  // manual says 'No Use Set OFF'
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xa0, 0x20, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:6,8")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x00, "MA / MB" )  // This is a small single player sit-down cab called 'Taito MA' or 'Taito MB', with only 1 joystick and 3 buttons.
										   // Only Player 1 can play and only buttons A, C and c are active.

	PORT_START("SWB")      /* D801 */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWB:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWB:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("SWC")      /* D802 */
	PORT_DIPUNUSED_DIPLOC( 0x0f, IP_ACTIVE_LOW, "SWC:1,2,3,4" )  // manual says 'No Use Set OFF'
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )       PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Show Year" )             PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "No Hit" )                PORT_DIPLOCATION("SWC:7")    // Allows playing the game regardless of the score until the 9th innings.
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x80, "2-Way" )
	PORT_DIPSETTING(    0x00, "1-Way" )

	PORT_START("SYSTEM")      /* D803 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")      /* D804 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )    // button A = substitute player
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )    // button C = select new player when subbing or swing bat when batting. When fielding, this picks up the ball.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EXTRA_P1")      /* D805 */
	/* bits 0,1 are MCU related:
	    - bit 0: MCU is ready to receive data from main CPU
	    - bit 1: MCU has sent data to the main CPU       */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)  // button a = 1st base
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)  // button b = 2nd base  |  Used to throw the ball back to a specific base when fielding
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )                                               // button c = 3rd base  |  On MA/MB cab, only button 'c' is active and throws only to 3rd base
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)  // button d = home base
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")      /* D806 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00) // C
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EXTRA_P2")      /* D807 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL PORT_CONDITION("SWA", 0xa0, NOTEQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( rumba )
	PORT_START("SWA")      /* D800 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x00, "20000 50000" )
	PORT_DIPSETTING(    0x01, "10000 60000" )
	PORT_DIPSETTING(    0x02, "10000 40000" )
	PORT_DIPSETTING(    0x03, "10000 20000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("SWB")      /* D801 */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWB:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWB:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("SWC")      /* D802 */
	PORT_DIPNAME( 0x01, 0x01, "Training Stage" )                 PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )               PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Language ) )              PORT_DIPLOCATION("SWC:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract Sound on Title Screen" )  PORT_DIPLOCATION("SWC:4")   /* At title sequence only - NOT Demo Sounds */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coinage Display" )                PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Copyright String" )               PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x20, "Taito Corp. MCMLXXXIV" )
	PORT_DIPSETTING(    0x00, "Taito Corporation" )
	PORT_DIPNAME( 0x40, 0x40, "Infinite Lives" )                 PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )               PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")      /* D803 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")      /* D804 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )    // A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )    // C
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")      /* D806 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL  // A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL  // C
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EXTRA_P2")      /* D807 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16*8+3, 16*8+2, 16*8+1, 16*8+0, 16*8+8+3, 16*8+8+2, 16*8+8+1, 16*8+8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8
};

static GFXDECODE_START( gfx_flstory )
	GFXDECODE_ENTRY( "tiles", 0, charlayout,     0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, spritelayout, 256, 16 )
GFXDECODE_END


void flstory_state::machine_start()
{
	// video
	save_item(NAME(m_gfxctrl));
	save_item(NAME(m_char_bank));
	save_item(NAME(m_palette_bank));

	// sound
	save_item(NAME(m_snd_ctrl0));
	save_item(NAME(m_snd_ctrl1));
	save_item(NAME(m_snd_ctrl2));
	save_item(NAME(m_snd_ctrl3));
}



void flstory_state::machine_reset()
{
	// video
	m_gfxctrl = 0;
	// onna34ro doesn't set this up when checking RAM/VRAM (available by keeping pressed service button at startup)
	// so we invert the logic here
	m_char_bank = 1;
	m_palette_bank = 0;

	// sound
	m_snd_ctrl0 = 0;
	m_snd_ctrl1 = 0;
	m_snd_ctrl2 = 0;
	m_snd_ctrl3 = 0;
}

void flstory_state::common(machine_config &config)
{
	Z80(config, m_maincpu, 10.733_MHz_XTAL / 2); // verified on PCB
	m_maincpu->set_vblank_int("screen", FUNC(flstory_state::irq0_line_hold));

	Z80(config, m_audiocpu, 8_MHz_XTAL / 2); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &flstory_state::sound_map);

	// IRQ generated with discrete-logic counter
	const attotime audio_irq_period = attotime::from_ticks(0x10000, 8_MHz_XTAL); // ~122Hz
	m_audiocpu->set_periodic_int(FUNC(flstory_state::irq0_line_hold), audio_irq_period);

	// 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs
	config.set_maximum_quantum(attotime::from_hz(6000));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(8_MHz_XTAL, 510, 0, 256, 262, 2*8, 30*8); // derived from ladyfrog.cpp, guess
	screen.set_screen_update(FUNC(flstory_state::screen_update_flstory));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_flstory);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 512);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(m_soundnmi, FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, m_soundnmi).output_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, m_soundlatch2);
	TA7630(config, m_ta7630);

	YM2149(config, m_ay, 8_MHz_XTAL / 4); // verified on PCB
	m_ay->port_a_write_callback().set(FUNC(flstory_state::sound_control_2_w));
	m_ay->port_b_write_callback().set(FUNC(flstory_state::sound_control_3_w));
	m_ay->add_route(ALL_OUTPUTS, "speaker", 0.1);

	MSM5232(config, m_msm, 8_MHz_XTAL / 4); // verified on PCB
	m_msm->set_capacitors(1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6); // 1.0 uF capacitors (verified on real PCB)
	m_msm->add_route(0, "speaker", 1.0); // pin 28  2'-1
	m_msm->add_route(1, "speaker", 1.0); // pin 29  4'-1
	m_msm->add_route(2, "speaker", 1.0); // pin 30  8'-1
	m_msm->add_route(3, "speaker", 1.0); // pin 31 16'-1
	m_msm->add_route(4, "speaker", 1.0); // pin 36  2'-2
	m_msm->add_route(5, "speaker", 1.0); // pin 35  4'-2
	m_msm->add_route(6, "speaker", 1.0); // pin 34  8'-2
	m_msm->add_route(7, "speaker", 1.0); // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.1); // unknown DAC
}

void flstory_mcu_state::flstory(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &flstory_mcu_state::flstory_map);

	TAITO68705_MCU(config, m_bmcu, 18.432_MHz_XTAL / 6); // verified on PCB

	MCFG_VIDEO_START_OVERRIDE(flstory_mcu_state,flstory)
}

void flstory_state::onna34ro(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &flstory_state::onna34ro_map);

	MCFG_VIDEO_START_OVERRIDE(flstory_state,flstory)
}

void flstory_mcu_state::onna34ro_mcu(machine_config &config)
{
	onna34ro(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &flstory_mcu_state::onna34ro_mcu_map);

	TAITO68705_MCU(config, m_bmcu, 18.432_MHz_XTAL / 6); // ?
}

void flstory_mcu_state::victnine(machine_config &config)
{
	common(config);

	// basic machine hardware
	m_maincpu->set_clock(8_MHz_XTAL / 2); // 4 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &flstory_mcu_state::victnine_map);

	TAITO68705_MCU(config, m_bmcu, 18.432_MHz_XTAL / 6);

	// video hardware
	subdevice<screen_device>("screen")->set_screen_update(FUNC(flstory_mcu_state::screen_update_victnine));
	MCFG_VIDEO_START_OVERRIDE(flstory_mcu_state,victnine)

	// sound hardware
	m_ay->reset_routes();
	m_ay->add_route(ALL_OUTPUTS, "speaker", 0.5);
}

void flstory_mcu_state::rumba(machine_config &config)
{
	common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &flstory_mcu_state::rumba_map);
	m_maincpu->set_clock(8_MHz_XTAL / 2); // verified on PCB

	TAITO68705_MCU(config, m_bmcu, 18.432_MHz_XTAL / 6); // ?

	// video hardware
	subdevice<screen_device>("screen")->set_screen_update(FUNC(flstory_mcu_state::screen_update_rumba));
	MCFG_VIDEO_START_OVERRIDE(flstory_mcu_state,rumba)
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( flstory )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "cpu-a45.15",   0x0000, 0x4000, CRC(f03fc969) SHA1(c8dd25ca25fd413b1a29bd4e58ce5820e5f852b2) )
	ROM_LOAD( "cpu-a45.16",   0x4000, 0x4000, CRC(311aa82e) SHA1(c2dd806f70ea917818ec844a275fb2fecc2e6c19) )
	ROM_LOAD( "cpu-a45.17",   0x8000, 0x4000, CRC(a2b5d17d) SHA1(0198d048aedcbd2498d490a5c0c506f8fc66ed03) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "snd.22",       0x0000, 0x2000, CRC(d58b201d) SHA1(1c9c2936ec95a8fa920d58668bea420c5e15008f) )
	ROM_LOAD( "snd.23",       0x2000, 0x2000, CRC(25e7fd9d) SHA1(b9237459e3d8acf8502a693914e50714a37d515e) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a45-20.mcu",   0x0000, 0x0800, CRC(7d2cdd9b) SHA1(b9a7b4c7d9d58b4b7cab1304beaa9d17f9559419) )

	ROM_REGION( 0x20000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "vid-a45.18",   0x00000, 0x4000, CRC(6f08f69e) SHA1(8f1b7e63a38f855cf26d57aed678da7cf1378fdf) )
	ROM_LOAD( "vid-a45.06",   0x04000, 0x4000, CRC(dc856a75) SHA1(6eedbf6b027c884502b6e7329f13829787138165) )
	ROM_LOAD( "vid-a45.08",   0x08000, 0x4000, CRC(d0b028ca) SHA1(c8bd9136ad3180002961ecfe600fc91a3c891539) )
	ROM_LOAD( "vid-a45.20",   0x0c000, 0x4000, CRC(1b0edf34) SHA1(e749c78053ed09bdb42c03cf4589b0fe122d9095) )
	ROM_LOAD( "vid-a45.19",   0x10000, 0x4000, CRC(2b572dc9) SHA1(9e14428663819e18829c625b4ae91a8a5530eb33) )
	ROM_LOAD( "vid-a45.07",   0x14000, 0x4000, CRC(aa4b0762) SHA1(6d4246753e80fe3ca05d47bd279f7ccc603f4700) )
	ROM_LOAD( "vid-a45.09",   0x18000, 0x4000, CRC(8336be58) SHA1(b92d37856870c4128a860d8ae02fa647743b99e3) )
	ROM_LOAD( "vid-a45.21",   0x1c000, 0x4000, CRC(fc382bd1) SHA1(a773c87454a3d7b80374a6d38ecb8633af2cd990) )
ROM_END

ROM_START( flstoryo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "cpu-a45.15",   0x0000, 0x4000, CRC(f03fc969) SHA1(c8dd25ca25fd413b1a29bd4e58ce5820e5f852b2) )
	ROM_LOAD( "cpu-a45.16",   0x4000, 0x4000, CRC(311aa82e) SHA1(c2dd806f70ea917818ec844a275fb2fecc2e6c19) )
	ROM_LOAD( "cpu-a45.17",   0x8000, 0x4000, CRC(a2b5d17d) SHA1(0198d048aedcbd2498d490a5c0c506f8fc66ed03) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a45_12.8",     0x0000, 0x2000, CRC(d6f593fb) SHA1(8551ef22c2cdd9df8d7949a178883f56ea56a4a2) )
	ROM_LOAD( "a45_13.9",     0x2000, 0x2000, CRC(451f92f9) SHA1(f4196e6d3420983b74001303936d086a48b10827) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a45-20.mcu",   0x0000, 0x0800, CRC(7d2cdd9b) SHA1(b9a7b4c7d9d58b4b7cab1304beaa9d17f9559419) )

	ROM_REGION( 0x20000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "vid-a45.18",   0x00000, 0x4000, CRC(6f08f69e) SHA1(8f1b7e63a38f855cf26d57aed678da7cf1378fdf) )
	ROM_LOAD( "vid-a45.06",   0x04000, 0x4000, CRC(dc856a75) SHA1(6eedbf6b027c884502b6e7329f13829787138165) )
	ROM_LOAD( "vid-a45.08",   0x08000, 0x4000, CRC(d0b028ca) SHA1(c8bd9136ad3180002961ecfe600fc91a3c891539) )
	ROM_LOAD( "vid-a45.20",   0x0c000, 0x4000, CRC(1b0edf34) SHA1(e749c78053ed09bdb42c03cf4589b0fe122d9095) )
	ROM_LOAD( "vid-a45.19",   0x10000, 0x4000, CRC(2b572dc9) SHA1(9e14428663819e18829c625b4ae91a8a5530eb33) )
	ROM_LOAD( "vid-a45.07",   0x14000, 0x4000, CRC(aa4b0762) SHA1(6d4246753e80fe3ca05d47bd279f7ccc603f4700) )
	ROM_LOAD( "vid-a45.09",   0x18000, 0x4000, CRC(8336be58) SHA1(b92d37856870c4128a860d8ae02fa647743b99e3) )
	ROM_LOAD( "vid-a45.21",   0x1c000, 0x4000, CRC(fc382bd1) SHA1(a773c87454a3d7b80374a6d38ecb8633af2cd990) )
ROM_END

/*
女三四郎
(Onna Sanshirou)
Taito, 1985
*/
ROM_START( onna34ro )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "a52-01-1.40c", 0x0000, 0x4000, CRC(ffddcb02) SHA1(d7002e8a577a5f9c2f63ec8d93076cd720443e05) )
	ROM_LOAD( "a52-02-1.41c", 0x4000, 0x4000, CRC(da97150d) SHA1(9b18f4d0bff811e332f6d2e151c7583400d60f23) )
	ROM_LOAD( "a52-03-1.42c", 0x8000, 0x4000, CRC(b9749a53) SHA1(15fd9624a500512f7b2c6766ed96f3734f61f160) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a52-12.08s",   0x0000, 0x2000, CRC(28f48096) SHA1(20aa5041cd71003e0981c32e34005bcbad53f707) )
	ROM_LOAD( "a52-13.09s",   0x2000, 0x2000, CRC(4d3b16f3) SHA1(8687b76398da875f69e9565277f00478c2b82a99) )
	ROM_LOAD( "a52-14.10s",   0x4000, 0x2000, CRC(90a6f4e8) SHA1(101767a90e963f3031e0830fd25a537ca8296de9) )
	ROM_LOAD( "a52-15.37s",   0x6000, 0x2000, CRC(5afc21d0) SHA1(317d5fb3a48ce5e13e02c5c6431fa08ada115d27) )
	ROM_LOAD( "a52-16.38s",   0x8000, 0x2000, CRC(ccf42aee) SHA1(a6eb01c5384724999631b55700dade430b71ca95) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a52_17.54c",   0x0000, 0x0800, CRC(0ab2612e) SHA1(2bc74e9ef5b9dd51d733dc62902d92c269f7d6a7) )

	ROM_REGION( 0x20000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "a52-04.11v",   0x00000, 0x4000, CRC(5b126294) SHA1(fc31e062e665f7313f923e84d6497716f0658ac0) )
	ROM_LOAD( "a52-06.10v",   0x04000, 0x4000, CRC(78114721) SHA1(d0e52544e05ab4fd1b131ed49beb252048bcbe31) )
	ROM_LOAD( "a52-08.09v",   0x08000, 0x4000, CRC(4a293745) SHA1(a54c1cfced63306db0ba7ee635dce41134c91dc8) )
	ROM_LOAD( "a52-10.08v",   0x0c000, 0x4000, CRC(8be7b4db) SHA1(e7ab373942b8ce75b36d0c9f547902fe65a3964d) )
	ROM_LOAD( "a52-05.35v",   0x10000, 0x4000, CRC(a1a99588) SHA1(eae63ae89058da1a92065e1d352cf81a15b556bc) )
	ROM_LOAD( "a52-07.34v",   0x14000, 0x4000, CRC(0bf420f2) SHA1(367e76efbed772fc8a6d7ac854407b62f8897d78) )
	ROM_LOAD( "a52-09.33v",   0x18000, 0x4000, CRC(39c543b5) SHA1(978c42f5eb23c15a96dae3578e742ef41bac689b) )
	ROM_LOAD( "a52-11.32v",   0x1c000, 0x4000, CRC(d1dda6b3) SHA1(fadf1404e8a03ec7e3fafb6281d33bc73bb5c473) )
ROM_END

ROM_START( onna34roa )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "ry-08.rom",    0x0000, 0x4000, CRC(e4587b85) SHA1(2fc4439953dd086eac11ba6d7937d8075fc39639) )
	ROM_LOAD( "ry-07.rom",    0x4000, 0x4000, CRC(6ffda515) SHA1(429e7bb22c66eb3c6d31981c2021af61c44ed51b) )
	ROM_LOAD( "ry-06.rom",    0x8000, 0x4000, CRC(6fefcda8) SHA1(f532e254a8bd7372bd9f8f21c907e44e0f5f4f32) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a52-12.08s",   0x0000, 0x2000, CRC(28f48096) SHA1(20aa5041cd71003e0981c32e34005bcbad53f707) )
	ROM_LOAD( "a52-13.09s",   0x2000, 0x2000, CRC(4d3b16f3) SHA1(8687b76398da875f69e9565277f00478c2b82a99) )
	ROM_LOAD( "a52-14.10s",   0x4000, 0x2000, CRC(90a6f4e8) SHA1(101767a90e963f3031e0830fd25a537ca8296de9) )
	ROM_LOAD( "a52-15.37s",   0x6000, 0x2000, CRC(5afc21d0) SHA1(317d5fb3a48ce5e13e02c5c6431fa08ada115d27) )
	ROM_LOAD( "a52-16.38s",   0x8000, 0x2000, CRC(ccf42aee) SHA1(a6eb01c5384724999631b55700dade430b71ca95) )

	ROM_REGION( 0x20000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "a52-04.11v",   0x00000, 0x4000, CRC(5b126294) SHA1(fc31e062e665f7313f923e84d6497716f0658ac0) )
	ROM_LOAD( "a52-06.10v",   0x04000, 0x4000, CRC(78114721) SHA1(d0e52544e05ab4fd1b131ed49beb252048bcbe31) )
	ROM_LOAD( "a52-08.09v",   0x08000, 0x4000, CRC(4a293745) SHA1(a54c1cfced63306db0ba7ee635dce41134c91dc8) )
	ROM_LOAD( "a52-10.08v",   0x0c000, 0x4000, CRC(8be7b4db) SHA1(e7ab373942b8ce75b36d0c9f547902fe65a3964d) )
	ROM_LOAD( "a52-05.35v",   0x10000, 0x4000, CRC(a1a99588) SHA1(eae63ae89058da1a92065e1d352cf81a15b556bc) )
	ROM_LOAD( "a52-07.34v",   0x14000, 0x4000, CRC(0bf420f2) SHA1(367e76efbed772fc8a6d7ac854407b62f8897d78) )
	ROM_LOAD( "a52-09.33v",   0x18000, 0x4000, CRC(39c543b5) SHA1(978c42f5eb23c15a96dae3578e742ef41bac689b) )
	ROM_LOAD( "a52-11.32v",   0x1c000, 0x4000, CRC(d1dda6b3) SHA1(fadf1404e8a03ec7e3fafb6281d33bc73bb5c473) )
ROM_END


/*
Victorious Nine hardware info by Guru
Taito, 1984

Hardware is similar to Elevator Action (uses same pinouts for wiring harness also)

Top Board (Sound)
---------
PCB No: J1100005A K1100011A (plus a sticker... K1100014A)
CPU   : NEC D780 (Z80)
SOUND : OKI M5232 (x1), YM2149 (x1), LM3900 (x3), TA7630 (x1)
XTAL  : 8.000MHz
RAM   : M5M5517 (=6116, x1)
OTHER : Volume Pot (x1)
PALs  : None
PROMs : None
DIPSW : None

            Byte
ROMs : (All type 2764)  C'sum
------------------------------
A16_12.8        059Bh
A16_13.9        3F12h
A16_14.10       CC99h
A16_15.37       9D55h
A16_16.38       B04Dh
A16_17.39       90B1h

*************


2nd Board (Small PCB contains ROMs ONLY, plugs into three empty sockets on 3rd PCB)
---------
PCB No: J9100006A K9100009A

            Byte
ROMs : (All type 2764)  C'sum
------------------------------
A16_19.1        22E3h
A16_20.2        D3AEh
A16_21.3        DB99h
A16_22.4        B4CDh
A16_23.5        92C8h
A16_24.6        1641h

*************


3rd PCB (Main Board with connectors G and H)
-------
PCB No: J1100007A K1100013A (plus 2 stickers... K1100027A  M4300007B)
CPU   : NEC D780 (Z80, plus one unpopulated socket for another Z80 CPU)
XTAL  : 8.000MHz
RAM   : M5M5517 (=6116, x1)
OTHER : MC68705P5S (labelled "A16 18", read-protected and not dumped - many years later dumped via m68705 dumper)
DIPSW : 8 position (x3, see archive for DSW info)
PALs  : None
PROMs : None
ROMs  : None

*************


4th PCB (Video with connector T)
-------
PCB No: J1100006A K1100012A
XTAL  : 18.432MHz
RAM   : 2148 (x9), M5M5517 (x1)

            Byte
ROMs : (All type 2764)  C'sum
------------------------------
A16_04.5        64A6h
A16_05-1.6      5DB1h
A16_06-1.7      25E2h
A16_07-2.8      E61Eh
A16_08.88       2718h
A16_09-1.89     57AAh
A16_10.90       7A95h
A16_11-1.91     4DD6h

*/

ROM_START( victnine )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "a16-19.1",     0x0000, 0x2000, CRC(deb7c439) SHA1(e87c8f95bc31d8450a3deed7a14b5fe139778d47) )
	ROM_LOAD( "a16-20.2",     0x2000, 0x2000, CRC(60cdb6ae) SHA1(65f09ef624d758b138a87c4cc80bc3539cc89507) )
	ROM_LOAD( "a16-21.3",     0x4000, 0x2000, CRC(121bea03) SHA1(4925b56a3f5725f1e00bd6aa87949aca5caf476b) )
	ROM_LOAD( "a16-22.4",     0x6000, 0x2000, CRC(b20e3027) SHA1(fab83afd1010fe6cebbeee06099eb2be9b96ec8a) )
	ROM_LOAD( "a16-23.5",     0x8000, 0x2000, CRC(95fe9cb7) SHA1(cfd7c0123940f680365500a516c8435330ed5f60) )
	ROM_LOAD( "a16-24.6",     0xa000, 0x2000, CRC(32b5c155) SHA1(34d25f3d4fae580757b69431b8b58f6f86d2282e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a16-12.8",     0x0000, 0x2000, CRC(4b9bff43) SHA1(4bcd52d6d72213f8fa7b544dbdd344312a9e2115) )
	ROM_LOAD( "a16-13.9",     0x2000, 0x2000, CRC(355121b9) SHA1(69cbe31eed53456f49a81c37b6661f7ba4a72fa6) )
	ROM_LOAD( "a16-14.10",    0x4000, 0x2000, CRC(0f33ef4d) SHA1(6916016d7cf43870d2e19fc1e6f1b20e48e07d76) )
	ROM_LOAD( "a16-15.37",    0x6000, 0x2000, CRC(f91d63dc) SHA1(4585d0c7ed05249c17385f20b6557e2e4375a6bb) )
	ROM_LOAD( "a16-16.38",    0x8000, 0x2000, CRC(9395351b) SHA1(8f97bdf03dec47bcaaa62fb66c545566776116be) )
	ROM_LOAD( "a16-17.39",    0xa000, 0x2000, CRC(872270b3) SHA1(2298cb8ced6c3e9afb430faab1b38ba8f2fa93b5) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 ) /* 2k for the microcontroller */
	ROM_LOAD( "a16-18.54",   0x0000, 0x0800, BAD_DUMP CRC(5198ef59) SHA1(05bde731ff580984dcf5a66e8465377c6dc03ec0) ) // dumped via m68705 dumper and hand-verified. Might still be imperfect but confirmed working on real PCB.

	ROM_REGION( 0x10000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "a16-06-1.7",   0x00000, 0x2000, CRC(b708134d) SHA1(9732be463cfbbe81ea0ad06da5a48b660ca429d0) )
	ROM_LOAD( "a16-07-2.8",   0x02000, 0x2000, CRC(cdaf7f83) SHA1(cf83af1655cb3ffce26c1b015b1e2249f7b12e3f) )
	ROM_LOAD( "a16-10.90",    0x04000, 0x2000, CRC(e8e42454) SHA1(c4923d4adfc0a48cf5a7d0145de5c9389495cac2) )
	ROM_LOAD( "a16-11-1.91",  0x06000, 0x2000, CRC(1f766661) SHA1(dfeecb587af7706e0e14539efc3386558f5d6da4) )
	ROM_LOAD( "a16-04.5",     0x08000, 0x2000, CRC(b2fae99f) SHA1(c8e56815159cd43a94c7e31b764d5bb996551a49) )
	ROM_LOAD( "a16-05-1.6",   0x0a000, 0x2000, CRC(85dfbb6e) SHA1(3643aab950d54eadded8d952033672aabb1e87c4) )
	ROM_LOAD( "a16-08.88",    0x0c000, 0x2000, CRC(1ddb6466) SHA1(0ea75c2fb584215f3cd4a7b7dfb3345a303e7e66) )
	ROM_LOAD( "a16-09-1.89",  0x0e000, 0x2000, CRC(23d4c43c) SHA1(ed0e059d3f97705331fdcc423a7c37aac9f07bb0) )
ROM_END


/*

RUMBA LUMBER by TAITO (1984)

Hardware similar to Fairyland Story except for the video board.
Wiring is the classic Taito one.
All clocks has been verified using a frequency counter.
Hardware is capable of playing samples (TTL circuit)


SOUND BOARD J1100022A / K1100066A

Xtal: 8mhz
Z80 NEC D780C-1 running at 8/2 = 4mhz
YM2149 running at 8/4 = 2mhz  pin 26 high
OKI M5232 running at 8/4 = 2mhz
2764 EPROM A23-08-1
2764 EPROM A23-09
2764 EPROM A23-10

CPU BOARD J1100024A / K1100065A

Xtal 8mhz
Z80 NEC D780C-1 running at 8/2 = 4mhz
MCU A23-11 MC68705P5S running at 18.432/6 = 3.072mhz
27128 EPROM A23-01-1
27128 EPROM A23-02-1
27128 EPROM A23-03-1

VIDEO BOARD J1100023A / K1100064A

xtal: 18.432mhz
2764 EPROM A23-04
2764 EPROM A23-05
2764 EPROM A23-06 (I cannot get a constant read,a couple of bytes differ everytime)
2764 EPROM A23-07

VSYNC = 60.55hz

Dumped by Corrado Tomaselli on 9/12/2010

*/


ROM_START( rumba )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "a23_01-1.bin",  0x0000, 0x4000, CRC(4bea6e18) SHA1(b9a85e65105773b5f93dcc5fc1e7c588b2d25056) )
	ROM_LOAD( "a23_02-1.bin",  0x4000, 0x4000, CRC(08f98c6f) SHA1(f2a850b1138cfefab6ff1d1adcda9e084f52e9c2) )
	ROM_LOAD( "a23_03-1.bin",  0x8000, 0x4000, CRC(ab595427) SHA1(1ff51740e1c7915e1f79a55801d11c8fdce764c8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a23_08-1.bin",  0x0000, 0x2000, CRC(a18eae00) SHA1(6ac1ad07bb5a97c6edaaf0e1fb842e1741f4cf1e) )
	ROM_LOAD( "a23_09.bin",    0x2000, 0x2000, CRC(d0a101d3) SHA1(c92bb1ce67bec394fd8ce303d9e61eac12493b5d) )
	ROM_LOAD( "a23_10.bin",    0x4000, 0x2000, CRC(f9447bd4) SHA1(68c02249ca0e5b923cddb4bff8d090963b9c78e4) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a23_11.bin",    0x0000, 0x0800, CRC(fddc99ce) SHA1(a9c7f76752ce74a780ca74004106c969d78ba931) )

	ROM_REGION( 0x8000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "a23_07.bin",   0x02000, 0x2000, CRC(c98fbea6) SHA1(edd1e0b2551f726018ca6e0b2cf629046a482711) )
	ROM_LOAD( "a23_06.bin",   0x00000, 0x2000, CRC(bf1e3a7f) SHA1(1258be10739cee6e6a8b2ce4d39f89bff1ea7f16) ) // should be a good read
	ROM_LOAD( "a23_05.bin",   0x06000, 0x2000, CRC(b40db231) SHA1(85204efc05e95334576807e4dab866f4f40081e6) )
	ROM_LOAD( "a23_04.bin",   0x04000, 0x2000, CRC(1d4f001f) SHA1(c3245650e57138ed89e7de8289fe37c5d933ddca) )
ROM_END

} // anonymous namespace


GAME( 1985, flstory,   0,        flstory,      flstory,  flstory_mcu_state, empty_init, ROT180, "Taito",   "The FairyLand Story",                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, flstoryo,  flstory,  flstory,      flstory,  flstory_mcu_state, empty_init, ROT180, "Taito",   "The FairyLand Story (earlier)",          MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, onna34ro,  0,        onna34ro_mcu, onna34ro, flstory_mcu_state, empty_init, ROT0,   "Taito",   "Onna Sanshirou - Typhoon Gal (rev 1)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, onna34roa, onna34ro, onna34ro,     onna34ro, flstory_state,     empty_init, ROT0,   "bootleg", "Onna Sanshirou - Typhoon Gal (bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, victnine,  0,        victnine,     victnine, flstory_mcu_state, empty_init, ROT0,   "Taito",   "Victorious Nine",                        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, rumba,     0,        rumba,        rumba,    flstory_mcu_state, empty_init, ROT270, "Taito",   "Rumba Lumber (rev 1)",                   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
