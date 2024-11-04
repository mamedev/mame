// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Atari Centipede hardware

*************************************************************************/
#ifndef MAME_ATARI_CENTIPED_H
#define MAME_ATARI_CENTIPED_H

#pragma once

#include "machine/74259.h"
#include "machine/eepromser.h"
#include "machine/er2055.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class centiped_state : public driver_device
{
public:
	centiped_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rambase(*this, "rambase"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_bullsdrt_tiles_bankram(*this, "bullsdrt_bank"),
		m_outlatch(*this, "outlatch"),
		m_earom(*this, "earom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_aysnd(*this, "aysnd")
	{ }

	void centiped_base(machine_config &config);
	void milliped(machine_config &config);
	void bullsdrt(machine_config &config);
	void centipdb(machine_config &config);
	void magworm(machine_config &config);
	void caterplr(machine_config &config);
	void centiped(machine_config &config);
	void centipedj(machine_config &config);
	void mazeinv(machine_config &config);
	void warlords(machine_config &config);

	void init_bullsdrt();

protected:
	required_device<cpu_device> m_maincpu;

	uint8_t m_gfx_bank = 0U;
	tilemap_t *m_bg_tilemap = nullptr;

	// drivers/centiped.cpp
	virtual void machine_start() override ATTR_COLD;

	void irq_ack_w(uint8_t data);
	uint8_t centiped_IN0_r();
	uint8_t centiped_IN2_r();
	uint8_t milliped_IN1_r();
	uint8_t milliped_IN2_r();

	// video/centiped.cpp
	void centiped_videoram_w(offs_t offset, uint8_t data);
	void milliped_paletteram_w(offs_t offset, uint8_t data);

private:
	optional_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_paletteram;
	optional_shared_ptr<uint8_t> m_bullsdrt_tiles_bankram;

	required_device<ls259_device> m_outlatch;
	optional_device<er2055_device> m_earom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<ay8910_device> m_aysnd;

	uint8_t m_oldpos[4]{};
	uint8_t m_sign[4]{};
	uint8_t m_dsw_select = 0U;
	uint8_t m_control_select = 0U;
	uint8_t m_flipscreen = 0U;
	uint8_t m_bullsdrt_sprites_bank = 0U;
	uint8_t m_penmask[64]{};

	// drivers/centiped.cpp
	void input_select_w(int state);
	void control_select_w(int state);
	uint8_t mazeinv_input_r();
	void mazeinv_input_select_w(offs_t offset, uint8_t data);
	uint8_t bullsdrt_data_port_r();
	void coin_counter_left_w(int state);
	void coin_counter_center_w(int state);
	void coin_counter_right_w(int state);
	void bullsdrt_coin_count_w(int state);
	uint8_t earom_read();
	void earom_write(offs_t offset, uint8_t data);
	void earom_control_w(uint8_t data);
	uint8_t caterplr_unknown_r();
	void caterplr_AY8910_w(offs_t offset, uint8_t data);
	uint8_t caterplr_AY8910_r(offs_t offset);

	// video/centiped.cpp
	void flip_screen_w(int state);
	void bullsdrt_tilesbank_w(offs_t offset, uint8_t data);
	void bullsdrt_sprites_bank_w(uint8_t data);
	void centiped_paletteram_w(offs_t offset, uint8_t data);
	void mazeinv_paletteram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(centiped_get_tile_info);
	TILE_GET_INFO_MEMBER(warlords_get_tile_info);
	TILE_GET_INFO_MEMBER(milliped_get_tile_info);
	TILE_GET_INFO_MEMBER(bullsdrt_get_tile_info);
	DECLARE_MACHINE_RESET(centiped);
	DECLARE_VIDEO_START(centiped);
	DECLARE_VIDEO_START(bullsdrt);
	DECLARE_MACHINE_RESET(magworm);
	DECLARE_VIDEO_START(milliped);
	DECLARE_VIDEO_START(warlords);
	void warlords_palette(palette_device &palette) const;
	uint32_t screen_update_centiped(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bullsdrt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_milliped(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_warlords(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(generate_interrupt);
	void init_penmask();
	void init_common();
	void milliped_set_color(offs_t offset, uint8_t data);
	inline int read_trackball(int idx, int switch_port);
	void bullsdrt_data_map(address_map &map) ATTR_COLD;
	void bullsdrt_map(address_map &map) ATTR_COLD;
	void bullsdrt_port_map(address_map &map) ATTR_COLD;
	void caterplr_map(address_map &map) ATTR_COLD;
	void centipdb_map(address_map &map) ATTR_COLD;
	void centiped_base_map(address_map &map) ATTR_COLD;
	void centiped_map(address_map &map) ATTR_COLD;
	void centipedj_map(address_map &map) ATTR_COLD;
	void magworm_map(address_map &map) ATTR_COLD;
	void mazeinv_map(address_map &map) ATTR_COLD;
	void milliped_map(address_map &map) ATTR_COLD;
	void warlords_map(address_map &map) ATTR_COLD;
};


class multiped_state : public centiped_state
{
public:
	multiped_state(const machine_config &mconfig, device_type type, const char *tag) :
		centiped_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom"),
		m_rombank(*this, "rombank")
	{ }

	void multiped(machine_config &config);

	void init_multiped();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_memory_bank m_rombank;

	uint8_t m_prg_bank = 0U;
	uint16_t m_0xxx_base = 0U;

	uint8_t multiped_0xxx_r(offs_t offset);
	void multiped_0xxx_w(offs_t offset, uint8_t data);
	uint8_t multiped_2xxx_r(offs_t offset);
	uint8_t multiped_eeprom_r();
	void multiped_eeprom_w(offs_t offset, uint8_t data);
	void multiped_prgbank_w(uint8_t data);

	void multiped_gfxbank_w(uint8_t data);

	void multiped_map(address_map &map) ATTR_COLD;
};

#endif // MAME_ATARI_CENTIPED_H
