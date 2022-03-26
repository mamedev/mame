// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Triple Hunt hardware

*************************************************************************/
#ifndef MAME_INCLUDES_TRIPLHNT_H
#define MAME_INCLUDES_TRIPLHNT_H

#pragma once

#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


/* Discrete Sound Input Nodes */
#define TRIPLHNT_BEAR_ROAR_DATA NODE_01
#define TRIPLHNT_BEAR_EN        NODE_02
#define TRIPLHNT_SHOT_DATA      NODE_03
#define TRIPLHNT_SCREECH_EN     NODE_04
#define TRIPLHNT_LAMP_EN        NODE_05


class triplhnt_state : public driver_device
{
public:
	triplhnt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_latch(*this, "latch"),
		m_watchdog(*this, "watchdog"),
		m_discrete(*this, "discrete"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_playfield_ram(*this, "playfield_ram"),
		m_vpos_ram(*this, "vpos_ram"),
		m_hpos_ram(*this, "hpos_ram"),
		m_orga_ram(*this, "orga_ram"),
		m_code_ram(*this, "code_ram"),
		m_lamp(*this, "lamp0")
	{ }

	void triplhnt(machine_config &config);

	void init_triplhnt();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	enum
	{
		TIMER_HIT
	};

	DECLARE_WRITE_LINE_MEMBER(coin_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(tape_control_w);

	uint8_t cmos_r(offs_t offset);
	uint8_t input_port_4_r();
	uint8_t misc_r(offs_t offset);
	uint8_t da_latch_r(offs_t offset);

	TILE_GET_INFO_MEMBER(get_tile_info);
	void triplhnt_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_collision(int code);

	void triplhnt_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<f9334_device> m_latch;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_sound_device> m_discrete;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_vpos_ram;
	required_shared_ptr<uint8_t> m_hpos_ram;
	required_shared_ptr<uint8_t> m_orga_ram;
	required_shared_ptr<uint8_t> m_code_ram;
	output_finder<> m_lamp;

	uint8_t m_cmos[16]{};
	uint8_t m_da_latch = 0;
	uint8_t m_cmos_latch = 0;
	uint8_t m_hit_code = 0;
	int m_sprite_zoom = 0;
	int m_sprite_bank = 0;
	bitmap_ind16 m_helper;
	emu_timer *m_hit_timer = nullptr;
	tilemap_t* m_bg_tilemap = nullptr;
};

/*----------- defined in audio/triplhnt.cpp -----------*/
DISCRETE_SOUND_EXTERN( triplhnt_discrete );
extern const char *const triplhnt_sample_names[];

#endif // MAME_INCLUDES_TRIPLHNT_H
