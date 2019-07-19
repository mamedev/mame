// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#ifndef MAME_INCLUDES_THEPIT_H
#define MAME_INCLUDES_THEPIT_H

#pragma once

#include "machine/74259.h"
#include "emupal.h"

class thepit_state : public driver_device
{
public:
	thepit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_attributesram(*this, "attributesram"),
		m_spriteram(*this, "spriteram")
	{ }

	void suprmous(machine_config &config);
	void desertdn(machine_config &config);
	void dockmanb(machine_config &config);
	void intrepid(machine_config &config);
	void thepit(machine_config &config);
	void fitter(machine_config &config);

	void init_rtriv();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_mainlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_attributesram;
	required_shared_ptr<uint8_t> m_spriteram;

	uint8_t m_graphics_bank;
	uint8_t m_flip_x;
	uint8_t m_flip_y;
	tilemap_t *m_solid_tilemap;
	tilemap_t *m_tilemap;
	std::unique_ptr<uint8_t[]> m_dummy_tile;
	uint8_t m_nmi_mask;

	int m_question_address;
	int m_question_rom;
	int m_remap_address[16];

	DECLARE_WRITE_LINE_MEMBER(coin_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(sound_enable_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_y_w);
	DECLARE_READ8_MEMBER(input_port_0_r);

	DECLARE_READ8_MEMBER(intrepid_colorram_mirror_r);
	DECLARE_WRITE_LINE_MEMBER(intrepid_graphics_bank_w);

	DECLARE_READ8_MEMBER(rtriv_question_r);

	TILE_GET_INFO_MEMBER(solid_get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info);

	void thepit_palette(palette_device &palette) const;
	void suprmous_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_desertdan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority_to_draw);

	INTERRUPT_GEN_MEMBER(vblank_irq);

	void audio_io_map(address_map &map);
	void audio_map(address_map &map);
	void desertdan_main_map(address_map &map);
	void dockmanb_main_map(address_map &map);
	void intrepid_main_map(address_map &map);
	void thepit_main_map(address_map &map);
};

#endif // MAME_INCLUDES_THEPIT_H
