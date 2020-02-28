// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#ifndef MAME_INCLUDES_SAURO_H
#define MAME_INCLUDES_SAURO_H

#pragma once

#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/sp0256.h"
#include "emupal.h"
#include "tilemap.h"

class sauro_state : public driver_device
{
public:
	sauro_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sp0256(*this, "speech"),
		m_mainlatch(*this, "mainlatch"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2")
	{ }

	void trckydoc(machine_config &config);
	void tecfri(machine_config &config);
	void sauro(machine_config &config);
	void saurob(machine_config &config);

	void init_tecfri();

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<sp0256_device> m_sp0256;
	optional_device<ls259_device> m_mainlatch;
	optional_device<generic_latch_8_device> m_soundlatch; // sauro only

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_videoram2;
	optional_shared_ptr<uint8_t> m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_palette_bank;

	bool m_irq_enable;

	virtual void machine_start() override;

	// common
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(irq_reset_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(scroll_bg_w);

	// sauro specific
	DECLARE_WRITE8_MEMBER(sauro_sound_command_w);
	DECLARE_READ8_MEMBER(sauro_sound_command_r);
	DECLARE_WRITE_LINE_MEMBER(sauro_palette_bank0_w);
	DECLARE_WRITE_LINE_MEMBER(sauro_palette_bank1_w);
	DECLARE_WRITE8_MEMBER(sauro_scroll_fg_w);
	DECLARE_WRITE8_MEMBER(sauro_videoram2_w);
	DECLARE_WRITE8_MEMBER(sauro_colorram2_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);

	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_fg);

	DECLARE_VIDEO_START(trckydoc);
	DECLARE_VIDEO_START(sauro);

	uint32_t screen_update_trckydoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sauro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sauro_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void trckydoc_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sauro_io_map(address_map &map);
	void sauro_map(address_map &map);
	void sauro_sound_map(address_map &map);
	void saurob_sound_map(address_map &map);
	void trckydoc_map(address_map &map);
};

#endif // MAME_INCLUDES_SAURO_H
