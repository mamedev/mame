// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Orbit hardware

*************************************************************************/
#ifndef MAME_INCLUDES_ORBIT_H
#define MAME_INCLUDES_ORBIT_H

#pragma once

#include "machine/74259.h"
#include "machine/timer.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

/* Discrete Sound Input Nodes */
#define ORBIT_NOTE_FREQ       NODE_01
#define ORBIT_ANOTE1_AMP      NODE_02
#define ORBIT_ANOTE2_AMP      NODE_03
#define ORBIT_NOISE1_AMP      NODE_04
#define ORBIT_NOISE2_AMP      NODE_05
#define ORBIT_WARNING_EN      NODE_06
#define ORBIT_NOISE_EN        NODE_07

class orbit_state : public driver_device
{
public:
	orbit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_discrete(*this, "discrete"),
		m_bg_tilemap(nullptr),
		m_flip_screen(0),
		m_latch(*this, "latch"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void orbit(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_w);
	void playfield_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_32v);
	void note_w(uint8_t data);
	void note_amp_w(uint8_t data);
	void noise_amp_w(uint8_t data);
	void noise_rst_w(uint8_t data);

	void main_map(address_map &map);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_misc_flags(address_space &space, uint8_t val);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;

	required_device<discrete_sound_device> m_discrete;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int        m_flip_screen;
	emu_timer *m_irq_off_timer = nullptr;

	/* devices */
	required_device<f9334_device> m_latch;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
/*----------- defined in audio/orbit.c -----------*/

DISCRETE_SOUND_EXTERN( orbit_discrete );

#endif // MAME_INCLUDES_ORBIT_H
