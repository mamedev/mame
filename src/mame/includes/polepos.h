// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria
/*************************************************************************

    Pole Position hardware

*************************************************************************/
#ifndef MAME_INCLUDES_POLEPOS_H
#define MAME_INCLUDES_POLEPOS_H

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

	DECLARE_READ_LINE_MEMBER(auto_start_r);

	void init_polepos2();

	void polepos2bi(machine_config &config);
	void topracern(machine_config &config);
	void polepos(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

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

	uint8_t m_steer_last;
	uint8_t m_steer_delta;
	int16_t m_steer_accum;
	int16_t m_last_result;
	int8_t m_last_signed;
	uint8_t m_last_unsigned;
	int m_adc_input;
	int m_auto_start_mask;

	uint16_t m_vertical_position_modifier[256];
	uint16_t m_road16_vscroll;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_chacl;
	uint16_t m_scroll;
	uint8_t m_sub_irq_mask;

	DECLARE_READ16_MEMBER(polepos2_ic25_r);
	uint8_t analog_r();
	DECLARE_READ8_MEMBER(ready_r);
	DECLARE_WRITE_LINE_MEMBER(iosel_w);
	DECLARE_WRITE_LINE_MEMBER(gasel_w);
	DECLARE_WRITE_LINE_MEMBER(sb0_w);
	DECLARE_WRITE_LINE_MEMBER(chacl_w);
	template<bool sub1> DECLARE_WRITE16_MEMBER(z8002_nvi_enable_w);
	DECLARE_READ8_MEMBER(sprite_r);
	DECLARE_WRITE8_MEMBER(sprite_w);
	DECLARE_READ8_MEMBER(road_r);
	DECLARE_WRITE8_MEMBER(road_w);
	DECLARE_WRITE16_MEMBER(road16_vscroll_w);
	DECLARE_WRITE16_MEMBER(view16_w);
	DECLARE_READ8_MEMBER(view_r);
	DECLARE_WRITE8_MEMBER(view_w);
	DECLARE_WRITE16_MEMBER(view16_hscroll_w);
	DECLARE_WRITE16_MEMBER(alpha16_w);
	DECLARE_READ8_MEMBER(alpha_r);
	DECLARE_WRITE8_MEMBER(alpha_w);
	DECLARE_WRITE8_MEMBER(out_0);
	DECLARE_WRITE8_MEMBER(out_1);
	DECLARE_READ8_MEMBER(namco_52xx_rom_r);
	DECLARE_READ8_MEMBER(namco_52xx_si_r);
	DECLARE_READ8_MEMBER(namco_53xx_k_r);
	DECLARE_READ8_MEMBER(steering_changed_r);
	DECLARE_READ8_MEMBER(steering_delta_r);
	DECLARE_WRITE8_MEMBER(bootleg_soundlatch_w);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	void polepos_palette(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void draw_road(bitmap_ind16 &bitmap);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void zoom_sprite(bitmap_ind16 &bitmap,int big,uint32_t code,uint32_t color,int flipx,int sx,int sy,int sizex,int sizey);
	void sound_z80_bootleg_iomap(address_map &map);
	void sound_z80_bootleg_map(address_map &map);
	void topracern_io(address_map &map);
	void z8002_map(address_map &map);
	void z8002_map_1(address_map &map);
	void z8002_map_2(address_map &map);
	void z80_io(address_map &map);
	void z80_map(address_map &map);
};

#endif // MAME_INCLUDES_POLEPOS_H
