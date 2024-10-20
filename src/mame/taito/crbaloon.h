// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

Crazy Ballooon

*************************************************************************/
#ifndef MAME_TAITO_CRBALOON_H
#define MAME_TAITO_CRBALOON_H

#pragma once

#include "sound/discrete.h"
#include "sound/sn76477.h"
#include "emupal.h"
#include "tilemap.h"

constexpr XTAL CRBALOON_MASTER_XTAL = 9.987_MHz_XTAL;


class crbaloon_state : public driver_device
{
public:
	crbaloon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_pc3092_data(*this, "pc3092_data"),
		m_maincpu(*this, "maincpu"),
		m_sn(*this, "snsnd"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void crbaloon(machine_config &config);
	ioport_value pc3092_r();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_pc3092_data;
	required_device<cpu_device> m_maincpu;
	required_device<sn76477_device> m_sn;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;

	uint16_t m_collision_address = 0;
	uint8_t m_collision_address_clear = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_irq_mask = 0U;
	void pc3092_w(offs_t offset, uint8_t data);
	uint8_t pc3259_r(offs_t offset);
	void port_sound_w(uint8_t data);
	void crbaloon_videoram_w(offs_t offset, uint8_t data);
	void crbaloon_colorram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void crbaloon_palette(palette_device &palette) const;
	uint32_t screen_update_crbaloon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vbl_int_w(int state);
	void crbaloon_audio_set_music_freq(uint8_t data);
	void crbaloon_audio_set_music_enable(uint8_t data);
	void crbaloon_audio_set_laugh_enable(uint8_t data);
	uint16_t crbaloon_get_collision_address();
	void crbaloon_set_clear_collision_address(int _crbaloon_collision_address_clear);
	void draw_sprite_and_check_collision(bitmap_ind16 &bitmap);
	void pc3092_reset(void);
	void pc3092_update();
	void pc3259_update(void);
	void crbaloon_audio_set_explosion_enable(int enabled);
	void crbaloon_audio_set_breath_enable(int enabled);
	void crbaloon_audio_set_appear_enable(int enabled);
	void crbaloon_audio(machine_config &config);
	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_CRBALOON_H
