// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Atari Cops'n Robbers hardware

*************************************************************************/
#ifndef MAME_INCLUDES_COPSNROB_H
#define MAME_INCLUDES_COPSNROB_H

#pragma once

#include "machine/74259.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"


class copsnrob_state : public driver_device
{
public:
	copsnrob_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_trucky(*this, "trucky"),
		m_truckram(*this, "truckram"),
		m_bulletsram(*this, "bulletsram"),
		m_carimage(*this, "carimage"),
		m_cary(*this, "cary"),
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_leds(*this, "led%u", 0U)
	{ }

	void copsnrob(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void copsnrob_audio(machine_config &config);
	void main_map(address_map &map);

	uint8_t copsnrob_misc_r();
	void copsnrob_misc2_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(one_start_w);
	uint32_t screen_update_copsnrob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_trucky;
	required_shared_ptr<uint8_t> m_truckram;
	required_shared_ptr<uint8_t> m_bulletsram;
	required_shared_ptr<uint8_t> m_carimage;
	required_shared_ptr<uint8_t> m_cary;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<discrete_device> m_discrete;

	/* misc */
	uint8_t          m_misc = 0U;
	uint8_t          m_ic_h3_data = 0U;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	output_finder<2> m_leds;
};

#endif // MAME_INCLUDES_COPSNROB_H
