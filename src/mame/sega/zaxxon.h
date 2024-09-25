// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Sega Zaxxon hardware

***************************************************************************/
#ifndef MAME_SEGA_ZAXXON_H
#define MAME_SEGA_ZAXXON_H

#pragma once

#include "machine/74259.h"
#include "machine/i8255.h"
#include "sound/samples.h"
#include "emupal.h"
#include "tilemap.h"

class zaxxon_state : public driver_device
{
public:
	zaxxon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch%u", 1),
		m_ppi(*this, "ppi8255"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_dials(*this, "DIAL.%u", 0),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram", 0x100, ENDIANNESS_LITTLE),
		m_colorram(*this, "colorram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void root(machine_config &config);
	void ixion(machine_config &config);
	void futspye(machine_config &config);
	void congo(machine_config &config);
	void szaxxon(machine_config &config);
	void razmataze(machine_config &config);
	void szaxxone(machine_config &config);
	void zaxxon(machine_config &config);
	void zaxxon_samples(machine_config &config);
	void congo_samples(machine_config &config);

	void init_razmataz();
	void init_zaxxonj();

	template <int Num> ioport_value razmataz_dial_r();
	template <int Num> int zaxxon_coin_r();
	DECLARE_INPUT_CHANGED_MEMBER(service_switch);
	DECLARE_INPUT_CHANGED_MEMBER(zaxxon_coin_inserted);

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<ls259_device, 2> m_mainlatch;
	optional_device<i8255_device> m_ppi;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_ioport_array<2> m_dials;

	required_shared_ptr<uint8_t> m_videoram;
	memory_share_creator<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	uint8_t m_int_enabled = 0;
	uint8_t m_coin_status[3]{};

	uint8_t m_razmataz_dial_pos[2]{};
	uint16_t m_razmataz_counter = 0;

	uint8_t m_sound_state[3]{};
	uint8_t m_bg_enable = 0;
	uint8_t m_bg_color = 0;
	uint16_t m_bg_position = 0;
	uint8_t m_fg_color = 0;
	bool m_flip_screen = false;

	uint8_t m_congo_fg_bank = 0;
	uint8_t m_congo_color_bank = 0;
	uint8_t m_congo_custom[4]{};

	const uint8_t *m_color_codes;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	void int_enable_w(int state);
	uint8_t razmataz_counter_r();
	void zaxxon_control_w(offs_t offset, uint8_t data);
	void coin_counter_a_w(int state);
	void coin_counter_b_w(int state);
	void coin_enable_w(int state);
	void flipscreen_w(int state);
	void fg_color_w(int state);
	void bg_position_w(offs_t offset, uint8_t data);
	void bg_color_w(int state);
	void bg_enable_w(int state);
	void congo_fg_bank_w(int state);
	void congo_color_bank_w(int state);
	void zaxxon_videoram_w(offs_t offset, uint8_t data);
	void congo_colorram_w(offs_t offset, uint8_t data);
	void congo_sprite_custom_w(address_space &space, offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(zaxxon_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(razmataz_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(congo_get_fg_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void zaxxon_palette(palette_device &palette);
	DECLARE_VIDEO_START(razmataz);
	DECLARE_VIDEO_START(congo);
	uint32_t screen_update_zaxxon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_futspy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_razmataz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ixion(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_congo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_int(int state);
	void zaxxon_sound_a_w(uint8_t data);
	void zaxxon_sound_b_w(uint8_t data);
	void zaxxon_sound_c_w(uint8_t data);
	void congo_sound_b_w(uint8_t data);
	void congo_sound_c_w(uint8_t data);
	void video_start_common(tilemap_get_info_delegate &&fg_tile_info);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int skew);
	inline int find_minimum_y(uint8_t value, int flip);
	inline int find_minimum_x(uint8_t value, int flip);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t flipxmask, uint16_t flipymask);

	void congo_map(address_map &map) ATTR_COLD;
	void congo_sound_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void ixion_map(address_map &map) ATTR_COLD;
	void zaxxon_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SEGA_ZAXXON_H
