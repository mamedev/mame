// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria
/*************************************************************************

    Pole Position hardware

*************************************************************************/
#ifndef MAME_NAMCO_POLEPOS_H
#define MAME_NAMCO_POLEPOS_H

#pragma once

#include "machine/74259.h"
#include "machine/adc0804.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/namco.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class polepos_state : public driver_device
{
public:
	polepos_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_sound_z80(*this, "soundz80bl"),
		m_soundlatch(*this, "soundlatch"),
		m_namco_sound(*this, "namco"),
		m_latch(*this, "latch"),
		m_adc(*this, "adc"),
		m_sprite16_memory(*this, "sprite16_memory"),
		m_road16_memory(*this, "road16_memory"),
		m_alpha16_memory(*this, "alpha16_memory"),
		m_view16_memory(*this, "view16_memory"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	int auto_start_r();

	void init_polepos2();

	void polepos2bi(machine_config &config);
	void topracern(machine_config &config);
	void polepos(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	optional_device<cpu_device> m_sound_z80;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<namco_device> m_namco_sound;
	required_device<ls259_device> m_latch;
	required_device<adc0804_device> m_adc;
	required_shared_ptr<uint16_t> m_sprite16_memory;
	required_shared_ptr<uint16_t> m_road16_memory;
	required_shared_ptr<uint16_t> m_alpha16_memory;
	required_shared_ptr<uint16_t> m_view16_memory;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t m_steer_last = 0;
	uint8_t m_steer_delta = 0;
	int16_t m_steer_accum = 0;
	int16_t m_last_result = 0;
	int8_t m_last_signed = 0;
	uint8_t m_last_unsigned = 0;
	int m_adc_input = 0;
	int m_auto_start_mask = 0;

	uint16_t m_vertical_position_modifier[256];
	uint16_t m_road16_vscroll = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;
	int m_chacl = 0;
	uint16_t m_scroll = 0;
	uint8_t m_sub_irq_mask = 0;

	uint16_t polepos2_ic25_r(offs_t offset);
	uint8_t analog_r();
	uint8_t ready_r();
	void gasel_w(int state);
	void sb0_w(int state);
	void chacl_w(int state);
	template<bool sub1> void z8002_nvi_enable_w(uint16_t data);
	uint8_t sprite_r(offs_t offset);
	void sprite_w(offs_t offset, uint8_t data);
	uint8_t road_r(offs_t offset);
	void road_w(offs_t offset, uint8_t data);
	void road16_vscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0);
	void view16_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0);
	uint8_t view_r(offs_t offset);
	void view_w(offs_t offset, uint8_t data);
	void view16_hscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0);
	void alpha16_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0);
	uint8_t alpha_r(offs_t offset);
	void alpha_w(offs_t offset, uint8_t data);
	void out(uint8_t data);
	void lockout(int state);
	uint8_t namco_52xx_rom_r(offs_t offset);
	uint8_t namco_52xx_si_r();
	uint8_t namco_53xx_k_r();
	uint8_t steering_changed_r();
	uint8_t steering_delta_r();
	void bootleg_soundlatch_w(uint8_t data);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	void polepos_palette(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void draw_road(bitmap_ind16 &bitmap);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void zoom_sprite(bitmap_ind16 &bitmap,int big,uint32_t code,uint32_t color,int flipx,int sx,int sy,int sizex,int sizey);
	void sound_z80_bootleg_iomap(address_map &map) ATTR_COLD;
	void sound_z80_bootleg_map(address_map &map) ATTR_COLD;
	void topracern_io(address_map &map) ATTR_COLD;
	void z8002_map(address_map &map) ATTR_COLD;
	void z8002_map_1(address_map &map) ATTR_COLD;
	void z8002_map_2(address_map &map) ATTR_COLD;
	void z80_io(address_map &map) ATTR_COLD;
	void z80_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_POLEPOS_H
