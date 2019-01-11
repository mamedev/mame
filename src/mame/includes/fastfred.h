// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Fast Freddie/Jump Coaster hardware
  driver by Zsolt Vasvari

***************************************************************************/
#ifndef MAME_INCLUDES_FASTFRED_H
#define MAME_INCLUDES_FASTFRED_H

#pragma once

#include "machine/74259.h"
#include "includes/galaxold.h"

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

	int m_hardware_type;
	uint16_t m_charbank;
	uint8_t m_colorbank;
	uint8_t m_nmi_mask;
	uint8_t m_sound_nmi_mask;
	uint8_t m_imago_sprites[0x800*3];
	uint16_t m_imago_sprites_address;
	uint8_t m_imago_sprites_bank;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_web_tilemap;

	DECLARE_READ8_MEMBER(fastfred_custom_io_r);
	DECLARE_READ8_MEMBER(flyboy_custom1_io_r);
	DECLARE_READ8_MEMBER(flyboy_custom2_io_r);
	DECLARE_READ8_MEMBER(jumpcoas_custom_io_r);
	DECLARE_READ8_MEMBER(boggy84_custom_io_r);
	DECLARE_WRITE_LINE_MEMBER(imago_dma_irq_w);
	DECLARE_WRITE8_MEMBER(imago_sprites_bank_w);
	DECLARE_WRITE8_MEMBER(imago_sprites_dma_w);
	DECLARE_READ8_MEMBER(imago_sprites_offset_r);
	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(fastfred_videoram_w);
	DECLARE_WRITE8_MEMBER(fastfred_attributes_w);
	DECLARE_WRITE_LINE_MEMBER(charbank1_w);
	DECLARE_WRITE_LINE_MEMBER(charbank2_w);
	DECLARE_WRITE_LINE_MEMBER(colorbank1_w);
	DECLARE_WRITE_LINE_MEMBER(colorbank2_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_y_w);
	DECLARE_WRITE8_MEMBER(imago_fg_videoram_w);
	DECLARE_WRITE_LINE_MEMBER(imago_charbank_w);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(imago_get_tile_info_bg);
	TILE_GET_INFO_MEMBER(imago_get_tile_info_fg);
	TILE_GET_INFO_MEMBER(imago_get_tile_info_web);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);

	virtual void machine_start() override;
	void fastfred_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(imago);
	DECLARE_VIDEO_START(fastfred);
	DECLARE_VIDEO_START(imago);

	uint32_t screen_update_fastfred(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_imago(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fastfred_map(address_map &map);
	void imago_map(address_map &map);
	void jumpcoas_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_FASTFRED_H
