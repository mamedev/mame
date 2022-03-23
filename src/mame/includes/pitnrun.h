// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Pierpaolo Prazzoli
#ifndef MAME_INCLUDES_PITNRUN_H
#define MAME_INCLUDES_PITNRUN_H

#pragma once

#include "cpu/m6805/m68705.h"
#include "emupal.h"
#include "tilemap.h"

class pitnrun_state : public driver_device
{
public:
	pitnrun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram")
	{ }

	void pitnrun_mcu(machine_config &config);
	void pitnrun(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<m68705p5_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_nmi = 0;
	uint8_t m_fromz80 = 0;
	uint8_t m_toz80 = 0;
	int m_zaccept = 0;
	int m_zready = 0;
	uint8_t m_porta_in = 0;
	uint8_t m_porta_out = 0;
	int m_address = 0;
	int m_h_heed = 0;
	int m_v_heed = 0;
	int m_ha = 0;
	int m_scroll = 0;
	int m_char_bank = 0;
	int m_color_select = 0;
	std::unique_ptr<bitmap_ind16> m_tmp_bitmap[4];
	tilemap_t *m_bg = nullptr;
	tilemap_t *m_fg = nullptr;

	DECLARE_WRITE_LINE_MEMBER(nmi_enable_w);
	DECLARE_WRITE_LINE_MEMBER(hflip_w);
	DECLARE_WRITE_LINE_MEMBER(vflip_w);
	uint8_t mcu_data_r();
	void mcu_data_w(uint8_t data);
	uint8_t mcu_status_r();
	uint8_t m68705_porta_r();
	void m68705_porta_w(uint8_t data);
	uint8_t m68705_portb_r();
	void m68705_portb_w(uint8_t data);
	uint8_t m68705_portc_r();
	void videoram_w(offs_t offset, uint8_t data);
	void videoram2_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(char_bank_select_w);
	void scroll_w(offs_t offset, uint8_t data);
	void scroll_y_w(uint8_t data);
	void ha_w(uint8_t data);
	void h_heed_w(uint8_t data);
	void v_heed_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(color_select_w);

	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	INTERRUPT_GEN_MEMBER(nmi_source);
	TIMER_CALLBACK_MEMBER(mcu_real_data_r);
	TIMER_CALLBACK_MEMBER(mcu_real_data_w);
	TIMER_CALLBACK_MEMBER(mcu_data_real_r);
	TIMER_CALLBACK_MEMBER(mcu_status_real_w);

	void pitnrun_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void spotlights();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void pitnrun_map(address_map &map);
	void pitnrun_map_mcu(address_map &map);
	void pitnrun_sound_io_map(address_map &map);
	void pitnrun_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_PITNRUN_H
