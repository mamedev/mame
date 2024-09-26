// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Fast Freddie/Jump Coaster hardware
  driver by Zsolt Vasvari

***************************************************************************/
#ifndef MAME_GALAXIAN_FASTFRED_H
#define MAME_GALAXIAN_FASTFRED_H

#pragma once

#include "galaxold.h"

#include "machine/74259.h"

#include "tilemap.h"

class fastfred_state : public galaxold_state
{
public:
	fastfred_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxold_state(mconfig, type, tag)
		, m_outlatch(*this, "outlatch")
		, m_videoram(*this, "videoram")
		, m_spriteram(*this, "spriteram")
		, m_attributesram(*this, "attributesram")
		, m_background_color(*this, "bgcolor")
		, m_imago_fg_videoram(*this, "imago_fg_vram")
	{ }

	void jumpcoas(machine_config &config);
	void imago(machine_config &config);
	void fastfred(machine_config &config);

	void init_fastfred();
	void init_flyboy();
	void init_flyboyb();
	void init_imago();
	void init_boggy84();
	void init_jumpcoas();
	void init_boggy84b();

private:
	required_device<ls259_device> m_outlatch;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_attributesram;
	optional_shared_ptr<uint8_t> m_background_color;
	optional_shared_ptr<uint8_t> m_imago_fg_videoram;

	int m_hardware_type = 0;
	uint16_t m_charbank = 0U;
	uint8_t m_colorbank = 0U;
	uint8_t m_nmi_mask = 0U;
	uint8_t m_sound_nmi_mask = 0U;
	uint8_t m_imago_sprites[0x800*3]{};
	uint16_t m_imago_sprites_address = 0U;
	uint8_t m_imago_sprites_bank = 0U;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_web_tilemap = nullptr;

	uint8_t fastfred_custom_io_r(offs_t offset);
	uint8_t flyboy_custom1_io_r(offs_t offset);
	uint8_t flyboy_custom2_io_r(offs_t offset);
	uint8_t jumpcoas_custom_io_r(offs_t offset);
	uint8_t boggy84_custom_io_r(offs_t offset);
	void imago_dma_irq_w(int state);
	void imago_sprites_bank_w(uint8_t data);
	void imago_sprites_dma_w(offs_t offset, uint8_t data);
	uint8_t imago_sprites_offset_r(offs_t offset);
	void nmi_mask_w(int state);
	void sound_nmi_mask_w(uint8_t data);
	void fastfred_videoram_w(offs_t offset, uint8_t data);
	void fastfred_attributes_w(offs_t offset, uint8_t data);
	void charbank1_w(int state);
	void charbank2_w(int state);
	void colorbank1_w(int state);
	void colorbank2_w(int state);
	void flip_screen_x_w(int state);
	void flip_screen_y_w(int state);
	void imago_fg_videoram_w(offs_t offset, uint8_t data);
	void imago_charbank_w(int state);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(imago_get_tile_info_bg);
	TILE_GET_INFO_MEMBER(imago_get_tile_info_fg);
	TILE_GET_INFO_MEMBER(imago_get_tile_info_web);

	void vblank_irq(int state);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);

	virtual void machine_start() override ATTR_COLD;
	void fastfred_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(imago);
	DECLARE_VIDEO_START(fastfred);
	DECLARE_VIDEO_START(imago);

	uint32_t screen_update_fastfred(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_imago(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fastfred_map(address_map &map) ATTR_COLD;
	void imago_map(address_map &map) ATTR_COLD;
	void jumpcoas_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_GALAXIAN_FASTFRED_H
