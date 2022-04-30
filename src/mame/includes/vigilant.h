// license:BSD-3-Clause
// copyright-holders:Mike Balfour
#ifndef MAME_INCLUDES_VIGILANT_H
#define MAME_INCLUDES_VIGILANT_H

#pragma once

#include "audio/m72.h"
#include "emupal.h"

class vigilant_state : public driver_device
{
public:
	vigilant_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audio(*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_8(*this, "paletteram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram")
	{ }

	void vigilant(machine_config &config);
	void kikcubic(machine_config &config);
	void buccanrs(machine_config &config);
	void bowmen(machine_config &config);
	void init_bowmen();

private:
	required_device<cpu_device> m_maincpu;
	required_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	int m_horiz_scroll_low = 0;
	int m_horiz_scroll_high = 0;
	int m_rear_horiz_scroll_low = 0;
	int m_rear_horiz_scroll_high = 0;
	int m_rear_color = 0;
	int m_rear_disable = 0;
	int m_rear_refresh = 0;
	int m_rear_pages = 4;
	std::unique_ptr<bitmap_ind16> m_bg_bitmap;

	// common
	void bank_select_w(uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);

	// vigilant and buccanrs
	void vigilant_out2_w(uint8_t data);
	void vigilant_horiz_scroll_w(offs_t offset, uint8_t data);
	void vigilant_rear_horiz_scroll_w(offs_t offset, uint8_t data);
	void vigilant_rear_color_w(uint8_t data);
	void bowmen_rear_horiz_scroll_w(offs_t offset, uint8_t data);
	void bowmen_rear_color_w(uint8_t data);

	// kikcubic
	void kikcubic_coin_w(uint8_t data);

	virtual void machine_start() override;
	virtual void video_start() override;
	virtual void video_reset() override;

	uint32_t screen_update_vigilant(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kikcubic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bowmen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_background();
	void update_background_bowmen();
	void draw_foreground(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int opaque );
	void draw_foreground_bowmen(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int opaque );
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background_bowmen(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void vigilant_postload();

	void buccanrs_sound_io_map(address_map &map);
	void kikcubic_io_map(address_map &map);
	void kikcubic_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
	void vigilant_io_map(address_map &map);
	void vigilant_map(address_map &map);
	void bowmen_io_map(address_map &map);
};

#endif // MAME_INCLUDES_VIGILANT_H
