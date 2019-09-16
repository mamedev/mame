// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
#ifndef MAME_INCLUDES_FGOAL_H
#define MAME_INCLUDES_FGOAL_H

#pragma once

#include "machine/mb14241.h"
#include "emupal.h"
#include "screen.h"

class fgoal_state : public driver_device
{
public:
	fgoal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb14241(*this, "mb14241"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram")
	{ }

	void fgoal(machine_config &config);

	DECLARE_READ_LINE_MEMBER(_80_r);

protected:
	enum
	{
		TIMER_INTERRUPT
	};

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<mb14241_device> m_mb14241;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_video_ram;

	/* video-related */
	bitmap_ind16   m_bgbitmap;
	bitmap_ind16   m_fgbitmap;
	uint8_t      m_xpos;
	uint8_t      m_ypos;
	int        m_current_color;

	/* misc */
	int        m_player;
	uint8_t      m_row;
	uint8_t      m_col;
	int        m_prev_coin;
	emu_timer  *m_interrupt_timer;

	DECLARE_READ8_MEMBER(analog_r);
	DECLARE_READ8_MEMBER(nmi_reset_r);
	DECLARE_READ8_MEMBER(irq_reset_r);
	DECLARE_READ8_MEMBER(row_r);
	DECLARE_WRITE8_MEMBER(row_w);
	DECLARE_WRITE8_MEMBER(col_w);
	DECLARE_READ8_MEMBER(address_hi_r);
	DECLARE_READ8_MEMBER(address_lo_r);
	DECLARE_READ8_MEMBER(shifter_r);
	DECLARE_READ8_MEMBER(shifter_reverse_r);
	DECLARE_WRITE8_MEMBER(sound1_w);
	DECLARE_WRITE8_MEMBER(sound2_w);
	DECLARE_WRITE8_MEMBER(color_w);
	DECLARE_WRITE8_MEMBER(ypos_w);
	DECLARE_WRITE8_MEMBER(xpos_w);

	TIMER_CALLBACK_MEMBER(interrupt_callback);

	void fgoal_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	static int intensity(int bits);
	unsigned video_ram_address();

	void cpu_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_FGOAL_H
