// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#ifndef MAME_THEPIT_THEPIT_H
#define MAME_THEPIT_THEPIT_H

#pragma once

#include "machine/74157.h"
#include "machine/74259.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class thepit_state : public driver_device
{
public:
	thepit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mainlatch(*this, "mainlatch"),
		m_inputmux(*this, "inputmux"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
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
	void theportr(machine_config &config);

	void init_rtriv();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ls259_device> m_mainlatch;
	required_device<ls157_device> m_inputmux;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_attributesram;
	required_shared_ptr<uint8_t> m_spriteram;

	uint8_t m_graphics_bank = 0;
	uint8_t m_flip_x = 0;
	uint8_t m_flip_y = 0;
	tilemap_t *m_solid_tilemap = nullptr;
	tilemap_t *m_tilemap = nullptr;
	std::unique_ptr<uint8_t[]> m_dummy_tile;
	uint8_t m_nmi_mask = 0;
	emu_timer *m_vsync_timer = nullptr;

	int m_question_address = 0;
	int m_question_rom = 0;
	int m_remap_address[16]{};

	void coin_lockout_w(int state);
	void sound_enable_w(int state);
	void nmi_mask_w(int state);
	void vblank_w(int state);
	TIMER_CALLBACK_MEMBER(vsync_callback);
	IRQ_CALLBACK_MEMBER(vsync_int_ack);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void flip_screen_x_w(int state);
	void flip_screen_y_w(int state);
	uint8_t input_port_0_r();

	uint8_t intrepid_colorram_mirror_r(offs_t offset);
	void intrepid_graphics_bank_w(int state);

	uint8_t rtriv_question_r(offs_t offset);

	TILE_GET_INFO_MEMBER(solid_get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info);

	void thepit_palette(palette_device &palette) const;
	void suprmous_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_desertdan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority_to_draw);

	void audio_io_map(address_map &map) ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;
	void desertdan_main_map(address_map &map) ATTR_COLD;
	void dockmanb_main_map(address_map &map) ATTR_COLD;
	void intrepid_main_map(address_map &map) ATTR_COLD;
	void thepit_main_map(address_map &map) ATTR_COLD;
	void theportr_main_map(address_map &map) ATTR_COLD;
};

#endif // MAME_THEPIT_THEPIT_H
