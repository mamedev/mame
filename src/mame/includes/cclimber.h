// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_CCLIMBER_H
#define MAME_INCLUDES_CCLIMBER_H

#pragma once

#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/segacrpt_device.h"
#include "emupal.h"

class cclimber_state : public driver_device
{
public:
	cclimber_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainlatch(*this, "mainlatch"),
		m_soundlatch(*this, "soundlatch"),
		m_bigsprite_videoram(*this, "bigspriteram"),
		m_videoram(*this, "videoram"),
		m_column_scroll(*this, "column_scroll"),
		m_spriteram(*this, "spriteram"),
		m_bigsprite_control(*this, "bigspritectrl"),
		m_colorram(*this, "colorram"),
		m_swimmer_background_color(*this, "bgcolor"),
		m_toprollr_bg_videoram(*this, "bg_videoram"),
		m_toprollr_bg_coloram(*this, "bg_coloram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_mainlatch;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_bigsprite_videoram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_column_scroll;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bigsprite_control;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_swimmer_background_color;
	optional_shared_ptr<uint8_t> m_toprollr_bg_videoram;
	optional_shared_ptr<uint8_t> m_toprollr_bg_coloram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	bool m_flip_x;
	bool m_flip_y;
	bool m_swimmer_side_background_enabled;
	bool m_swimmer_palettebank;

	uint8_t m_yamato_p0;
	uint8_t m_yamato_p1;
	uint8_t m_toprollr_rombank;
	bool m_nmi_mask;
	tilemap_t *m_pf_tilemap;
	tilemap_t *m_bs_tilemap;
	tilemap_t *m_toproller_bg_tilemap;
	std::unique_ptr<uint8_t[]> m_opcodes;

	DECLARE_WRITE8_MEMBER(swimmer_sh_soundlatch_w);
	DECLARE_WRITE8_MEMBER(yamato_p0_w);
	DECLARE_WRITE8_MEMBER(yamato_p1_w);
	DECLARE_READ8_MEMBER(yamato_p0_r);
	DECLARE_READ8_MEMBER(yamato_p1_r);
	DECLARE_WRITE_LINE_MEMBER(toprollr_rombank_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	DECLARE_READ8_MEMBER(bagmanf_a000_r);
	DECLARE_WRITE8_MEMBER(cclimber_colorram_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_y_w);
	DECLARE_WRITE_LINE_MEMBER(sidebg_enable_w);
	DECLARE_WRITE_LINE_MEMBER(palette_bank_w);

	virtual void machine_start() override;
	void init_cclimber();
	void init_yamato();
	void init_ckongb();
	void init_toprollr();
	void init_cclimberj();
	void init_cannonb2();
	void init_cannonb();
	void init_dking();
	void init_rpatrol();
	DECLARE_VIDEO_START(cclimber);
	void cclimber_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(swimmer);
	void swimmer_palette(palette_device &palette) const;
	void yamato_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(toprollr);
	void toprollr_palette(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(cclimber_get_pf_tile_info);
	TILE_GET_INFO_MEMBER(swimmer_get_pf_tile_info);
	TILE_GET_INFO_MEMBER(toprollr_get_pf_tile_info);
	TILE_GET_INFO_MEMBER(cclimber_get_bs_tile_info);
	TILE_GET_INFO_MEMBER(toprollr_get_bs_tile_info);
	TILE_GET_INFO_MEMBER(toproller_get_bg_tile_info);

	uint32_t screen_update_cclimber(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_swimmer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_yamato(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_toprollr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void swimmer_set_background_pen();
	void draw_playfield(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cclimber_draw_bigsprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void toprollr_draw_bigsprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cclimber_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);
	void toprollr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);
	void swimmer_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);
	void cclimber_decode(const uint8_t convtable[8][16]);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(bagmanf_vblank_irq);
	void root(machine_config &config);
	void swimmer(machine_config &config);
	void guzzler(machine_config &config);
	void toprollr(machine_config &config);
	void cannonb(machine_config &config);
	void yamato(machine_config &config);
	void bagmanf(machine_config &config);
	void cclimber(machine_config &config);
	void cclimberx(machine_config &config);
	void ckongb(machine_config &config);
	void bagmanf_map(address_map &map);
	void cannonb_map(address_map &map);
	void cclimber_map(address_map &map);
	void cclimber_portmap(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void guzzler_map(address_map &map);
	void swimmer_audio_map(address_map &map);
	void swimmer_audio_portmap(address_map &map);
	void swimmer_map(address_map &map);
	void toprollr_decrypted_opcodes_map(address_map &map);
	void toprollr_map(address_map &map);
	void yamato_audio_map(address_map &map);
	void yamato_audio_portmap(address_map &map);
	void yamato_decrypted_opcodes_map(address_map &map);
	void yamato_map(address_map &map);
	void yamato_portmap(address_map &map);
};

#endif // MAME_INCLUDES_CCLIMBER_H
