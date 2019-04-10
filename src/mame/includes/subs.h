// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Subs hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SUBS_H
#define MAME_INCLUDES_SUBS_H

#pragma once

#include "sound/discrete.h"
#include "emupal.h"

/* Discrete Sound Input Nodes */
#define SUBS_SONAR1_EN          NODE_01
#define SUBS_SONAR2_EN          NODE_02
#define SUBS_LAUNCH_DATA        NODE_03
#define SUBS_CRASH_DATA         NODE_04
#define SUBS_CRASH_EN           NODE_05
#define SUBS_EXPLODE_EN         NODE_06
#define SUBS_NOISE_RESET        NODE_07


class subs_state : public driver_device
{
public:
	subs_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_discrete(*this, "discrete"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram")
	{ }

	void subs(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<discrete_device> m_discrete;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	int m_steering_buf1;
	int m_steering_buf2;
	int m_steering_val1;
	int m_steering_val2;
	int m_last_val_1;
	int m_last_val_2;

	DECLARE_WRITE8_MEMBER(steer_reset_w);
	DECLARE_READ8_MEMBER(control_r);
	DECLARE_READ8_MEMBER(coin_r);
	DECLARE_READ8_MEMBER(options_r);
	DECLARE_WRITE_LINE_MEMBER(invert1_w);
	DECLARE_WRITE_LINE_MEMBER(invert2_w);
	DECLARE_WRITE8_MEMBER(noise_reset_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void subs_palette(palette_device &palette) const;

	uint32_t screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);

	int steering_1();
	int steering_2();

	void main_map(address_map &map);
};

/*----------- defined in audio/subs.c -----------*/

DISCRETE_SOUND_EXTERN( subs_discrete );

#endif // MAME_INCLUDES_SUBS_H
