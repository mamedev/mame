// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Basketball hardware

*************************************************************************/
#ifndef MAME_ATARI_BSKTBALL_H
#define MAME_ATARI_BSKTBALL_H

#pragma once

#include "machine/timer.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

/* Discrete Sound Input Nodes */
#define BSKTBALL_NOTE_DATA      NODE_01
#define BSKTBALL_CROWD_DATA     NODE_02
#define BSKTBALL_NOISE_EN       NODE_03
#define BSKTBALL_BOUNCE_EN      NODE_04


class bsktball_state : public driver_device
{
public:
	bsktball_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_discrete(*this, "discrete"),
		m_videoram(*this, "videoram"),
		m_motion(*this, "motion")
	{ }

	void bsktball(machine_config &config);

protected:
	void nmion_w(int state);
	void ld1_w(int state);
	void ld2_w(int state);
	uint8_t bsktball_in0_r();
	void bsktball_videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void bsktball_palette(palette_device &palette) const;
	uint32_t screen_update_bsktball(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bsktball_scanline);
	void bsktball_bounce_w(uint8_t data);
	void bsktball_note_w(uint8_t data);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<discrete_device> m_discrete;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_motion;

	/* video-related */
	tilemap_t  *m_bg_tilemap = nullptr;

	/* misc */
	uint32_t   m_nmi_on = 0U;

	/* input-related */
	int m_ld1 = 0;
	int m_ld2 = 0;
	int m_dir0 = 0;
	int m_dir1 = 0;
	int m_dir2 = 0;
	int m_dir3 = 0;
	int m_last_p1_horiz = 0;
	int m_last_p1_vert = 0;
	int m_last_p2_horiz = 0;
	int m_last_p2_vert = 0;
};

/*----------- defined in audio/bsktball.cpp -----------*/

DISCRETE_SOUND_EXTERN( bsktball_discrete );

#endif // MAME_ATARI_BSKTBALL_H
